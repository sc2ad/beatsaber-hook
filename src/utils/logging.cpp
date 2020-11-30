// TODO: Move this to another place and make it #define agnostic
// Ideally, we would be able to call a one time setup function, perhaps with a ModInfo parameter
// This would either return or initialize a logger instance for us to use with future calls to "log"

#include "../../shared/utils/logging.hpp"
#include "modloader/shared/modloader.hpp"
#include <string_view>
#include <string>
#include <memory>
#include "../../shared/utils/utils-functions.h"
#include "../../shared/utils/utils.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifndef VERSION
#define VERSION "0.0.0"
#endif

std::list<LoggerBuffer*> Logger::buffers;
bool Logger::consumerStarted = false;
std::mutex Logger::bufferMutex;

const char* get_level(Logging::Level level) {
    switch (level)
    {
    case Logging::Level::CRITICAL:
        return "CRITICAL";
    case Logging::Level::ERROR:
        return "ERROR";
    case Logging::Level::WARNING:
        return "WARNING";
    case Logging::Level::INFO:
        return "INFO";
    case Logging::Level::DEBUG:
        return "DEBUG";
    default:
        return "UNKNOWN";
    }
}

bool createdGlobal = false;

LoggerBuffer& get_global() {
    static LoggerBuffer g(ModInfo{"GlobalLog", VERSION});
    if (!createdGlobal) {
        if (fileexists(g.get_path())) {
            deletefile(g.get_path());
        }
        __android_log_print(Logging::INFO, "QuestHook[Logging]", "Created get_global() log at path: %s", g.get_path().c_str());
        createdGlobal = true;
    }
    return g;
}

Logger& Logger::get() {
    // UtilsLogger will (by default) log to file.
    static Logger utilsLogger(ModInfo{"UtilsLogger", VERSION}, LoggerOptions(false, true));
    return utilsLogger;
}

void LoggerBuffer::flush() {
    if (length() == 0) {
        // If we have nothing to write, exit early.
        return;
    }
    // We can open the file without locking path, because it is only created on initialization.
    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        __android_log_print(Logging::CRITICAL, Logger::get().tag.c_str(), "Could not open file: %s when flushing buffer!", path.c_str());
    }
    // Then, iterate over all messages and write each of them to the file.
    // We already must hold the lock for this call.
    // Assuming we always append to the END of the list, we could theoretically get away without locking on this call (except for length 1)
    messageLock.lock();
    for (; !messages.empty(); messages.pop_front()) {
        file << messages.front().c_str() << '\n';
    }
    messageLock.unlock();
    file.close();
}

std::size_t LoggerBuffer::length() {
    if (closed) {
        // Ignore messages to write if we are closed.
        return 0;
    }
    return messages.size();
}

void LoggerBuffer::addMessage(std::string_view msg) {
    if (closed) {
        return;
    }
    messageLock.lock();
    messages.emplace_back(msg.data());
    messageLock.unlock();
}

// Now, we COULD be a lot more reasonable and spawn a thread for each buffer logger
// However, I think having one should be fine.
// Flushing while holding the bufferMutex means that new loggers take awhile to create (everything else must be flushed)
// Flushing itself is pretty quick, new messages aren't allowed to be added while the writing is happening (I'm not sure HOW quick it is)
class Consumer {
    public:
    void operator()() {
        // Goal here is that we want to iterate over all of the buffers
        // For each one, we flush our log to the file specified by the path of that buffer.
        while (true) {
            // Lock our bufferMutex
            Logger::bufferMutex.lock();
            for (auto* buffer : Logger::buffers) {
                // For each buffer, we want to flush all of the messages.
                // However, we want to do so in a fashion that isn't terribly unreasonable.
                buffer->flush();
                // Ideally, we thread_yield after each buffer flush (may not need to, though)
                // std::this_thread::yield();
            }
            // Also do the get_global() buffer
            get_global().flush();
            Logger::bufferMutex.unlock();
            // Sleep for a bit without the lock to allow other threads to create loggers and add them
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }
};

void Logger::flushAll() {
    __android_log_write(Logging::CRITICAL, Logger::get().tag.c_str(), "Flushing all buffers!");
    Logger::bufferMutex.lock();
    for (auto* buffer : Logger::buffers) {
        buffer->flush();
    }
    get_global().flush();
    Logger::bufferMutex.unlock();
    __android_log_write(Logging::CRITICAL, Logger::get().tag.c_str(), "All buffers flushed!");
}

void Logger::closeAll() {
    __android_log_write(Logging::CRITICAL, Logger::get().tag.c_str(), "Closing all buffers!");
    Logger::bufferMutex.lock();
    for (auto* buffer : Logger::buffers) {
        buffer->flush();
        buffer->closed = true;
    }
    get_global().flush();
    get_global().closed = true;
    Logger::bufferMutex.unlock();
    __android_log_write(Logging::CRITICAL, Logger::get().tag.c_str(), "All buffers closed!");
}

bool Logger::init() {
    // So, we want to take a look at our options.
    // If we have fileLog set to true, we want to clear the file pointed to by this log.
    // That means that we want to delete the existing file (because storing a bunch is pretty obnoxious)
    if (options.toFile) {
        if (fileexists(buffer.get_path())) {
            deletefile(buffer.get_path());
        }
        // Now, create the file and paths as necessary.
        if (!direxists(buffer.get_logDir())) {
            mkpath(buffer.get_logDir());
            __android_log_print(Logging::INFO, tag.c_str(), "Created logger buffer dir: %s", buffer.get_logDir().c_str());
        }
        std::ofstream str(buffer.get_path());
        if (!str.is_open()) {
            __android_log_print(Logging::CRITICAL, tag.c_str(), "Could not open logger buffer file: %s!", buffer.get_path().c_str());
            return false;
        } else {
            str.close();
        }
    }
    return true;
}

void Logger::flush() {
    // Flush our buffer.
    // We do this by locking it and reading all of its messages to completion.
    Logger::bufferMutex.lock();
    buffer.flush();
    get_global().flush();
    Logger::bufferMutex.unlock();
}

void Logger::close() {
    Logger::bufferMutex.lock();
    buffer.flush();
    get_global().flush();
    buffer.closed = true;
    Logger::bufferMutex.unlock();
}

void Logger::startConsumer() {
    if (!Logger::consumerStarted) {
        consumerStarted = true;
        __android_log_write(Logging::INFO, Logger::get().tag.c_str(), "Started consumer thread!");
        std::thread(Consumer()).detach();
    }
}

LoggerContextObject Logger::WithContext(std::string_view context) {
    contextMutex.lock();
    for (auto item : disabledContexts) {
        if (context.starts_with(item)) {
            // Disable context
            contextMutex.unlock();
            return LoggerContextObject(*this, context, false);
        }
    }
    // If I am silent, my context objects are disabled.
    contextMutex.unlock();
    return LoggerContextObject(*this, context, !options.silent);
}

LoggerContextObject Logger::WithContext(LoggerContextObject* parent, std::string_view context) {
    contextMutex.lock();
    for (auto item : disabledContexts) {
        if (context.starts_with(item)) {
            // Disable context
            if (parent) {
                contextMutex.unlock();
                return LoggerContextObject(parent, parent->context + options.contextSeparator + context.data(), false);
            } else {
                contextMutex.unlock();
                return LoggerContextObject(*this, context, false);
            }
        }
    }
    // If I am silent, my context objects are disabled.
    if (parent) {
        contextMutex.unlock();
        return LoggerContextObject(parent, parent->context + options.contextSeparator + context.data(), !options.silent);
    } else {
        contextMutex.unlock();
        return LoggerContextObject(*this, context, !options.silent);
    }
}

void Logger::RecurseChangeContext(LoggerContextObject* ctx, std::string_view context, bool enable) {
    if (!ctx) {
        // Failsafe
        return;
    }
    if (ctx->context.starts_with(context)) {
        ctx->enabled = enable;
        ctx->changeChildren(enable);
    } else {
        // For each child, if we didn't match, check to see if its context starts with the context to disable
        for (auto* child : ctx->childrenContexts) {
            RecurseChangeContext(child, context, enable);
        }
    }
}

void Logger::DisableContext(std::string_view context) {
    contextMutex.lock();
    disabledContexts.emplace(context.data());
    // We should also iterate over all disabledContexts and determine if any existing contexts need to be disabled.
    // Existing contexts could have parent contexts, though.
    for (auto* ctx : contexts) {
        // For each context, only check those without parents (top level, recurse down)
        if (ctx->parentContext == nullptr) {
            RecurseChangeContext(ctx, context, false);
        }
    }
    contextMutex.unlock();
}

void Logger::EnableContext(std::string_view context) {
    contextMutex.lock();
    auto itr = disabledContexts.find(std::string(context));
    if (itr != disabledContexts.end()) {
        disabledContexts.erase(itr);
    }
    for (auto* ctx : contexts) {
        // For each context, only check those without parents (top level, recurse down)
        if (ctx->parentContext == nullptr) {
            RecurseChangeContext(ctx, context, true);
        }
    }
    contextMutex.unlock();
}

const std::unordered_set<std::string> Logger::GetDisabledContexts() {
    return disabledContexts;
}

#define LOG_MAX_CHARS 1000
void Logger::log(Logging::Level lvl, std::string str) {
    if (options.silent) {
        return;
    }
    // Chunk string for logcat buffer
    if (str.length() > LOG_MAX_CHARS) {
        std::size_t i = 0;
        while (i < str.length()) {
            auto sub = str.substr(i, LOG_MAX_CHARS);
            auto newline = sub.find('\n');
            if (newline != std::string::npos) {
                sub = sub.substr(0, newline);
                i += newline + 1; // Skip actual newline character
            } else {
                i += LOG_MAX_CHARS;
            }
            __android_log_write(lvl, tag.c_str(), sub.c_str());
        }
    } else {
        __android_log_write(lvl, tag.c_str(), str.c_str());
    }
    if (options.toFile) {
        // If we want to log to file, we want to write to a shared buffer.
        // This buffer should be consumed by a separate thread (started if we haven't yet started any consumer)
        // The overhead of this thread should be pretty minimal, all things considered, even if it handles every Logger instance.
        // The first thing we need to do is create our buffer, and add it to our buffers (lock while doing so)
        // Then, we need to start our thread if we haven't started it already
        // Then, we need to add our data to the buffer (lock while doing so)
        // The thread needs to consume from these buffers (locks while doing so)
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto in_time = std::chrono::system_clock::to_time_t(now);
        std::tm bt = *std::localtime(&in_time);
        std::ostringstream oss;
        oss << std::put_time(&bt, "%m-%d %H:%M:%S.") << std::setfill('0') << std::setw(3) << ms.count();
        auto msg = oss.str() + " " + get_level(lvl) + " " + tag + ": " + str.c_str();
        // __android_log_print(Logging::DEBUG, tag.c_str(), "Logging message: %s to file!", msg.c_str());
        Logger::bufferMutex.lock();
        buffer.addMessage(msg);
        get_global().addMessage(msg);
        Logger::bufferMutex.unlock();
        startConsumer();
    }
}

void Logger::log(Logging::Level lvl, std::string_view fmt, ...) {
    if (options.silent) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    auto s = string_vformat(fmt, args);
    va_end(args);
    log(lvl, s);
}

void Logger::critical(std::string_view fmt, ...) {
    if (options.silent) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    auto s = string_vformat(fmt, args);
    va_end(args);
    log(Logging::CRITICAL, s);
}

void Logger::error(std::string_view fmt, ...) {
    if (options.silent) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    auto s = string_vformat(fmt, args);
    va_end(args);
    log(Logging::ERROR, s);
}

void Logger::warning(std::string_view fmt, ...) {
    if (options.silent) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    auto s = string_vformat(fmt, args);
    va_end(args);
    log(Logging::WARNING, s);
}

void Logger::info(std::string_view fmt, ...) {
    if (options.silent) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    auto s = string_vformat(fmt, args);
    va_end(args);
    log(Logging::INFO, s);
}

void Logger::debug(std::string_view fmt, ...) {
    if (options.silent) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    auto s = string_vformat(fmt, args);
    va_end(args);
    log(Logging::DEBUG, s);
}

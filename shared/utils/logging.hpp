#pragma once

#include <android/log.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>
#include <unistd.h>
#include <cmath> // Included to support cmath's definition of log
#include <string_view>
#include <algorithm>
#include <string>
#include <list>
#include <mutex>
#include "modloader/shared/modloader.hpp"
#include "utils-functions.h"
#include <thread>
#include <unordered_set>
#include <unordered_map>

namespace Logging {
    enum Level {
        CRITICAL = ANDROID_LOG_FATAL,
        ERROR = ANDROID_LOG_ERROR,
        WARNING = ANDROID_LOG_WARN,
        INFO = ANDROID_LOG_INFO,
        DEBUG = ANDROID_LOG_DEBUG
    };
}

#ifdef log
#undef log
#endif

#ifndef LOG_PATH
#define LOG_PATH "/sdcard/Android/data/%s/files/logs/"
#endif

class Logger;

/// @class Logger Buffer
/// @brief A buffer for logger data. Used for logging to file in a buffered fashion.
/// Each LoggerBuffer exists to wrap around a single logger instance.
/// Every time log is called on the instance, this buffer is updated (assuming options.toFile is true for the instance)
/// Each buffer contains multiple messages that need to be written out, stored as an std::list
class LoggerBuffer {
    friend Logger;
    private:
    std::string path;
    public:
    std::list<std::string> messages;
    const ModInfo modInfo;
    bool closed = false;
    static std::string get_logDir() {
        // Copy it
        static std::string d = string_format(LOG_PATH, Modloader::getApplicationId().c_str());
        return d;
    }
    std::string get_path() {
        std::string cpy = modInfo.version;
        std::replace(cpy.begin(), cpy.end(), '.', '_');
        auto val = get_logDir() + modInfo.id + "_" + cpy + ".log";
        return val;
    }
    std::size_t length();
    void addMessage(std::string_view msg);
    public:
    void flush();
    LoggerBuffer(const ModInfo info) : modInfo(info) {
        path = get_path();
    }
};

/// @struct Logger Options
/// @brief Provides various options for loggers, including silencing them and logging to file.
struct LoggerOptions {
    bool silent = false;
    bool toFile = false;
    std::string contextSeparator = "::";
    LoggerOptions(bool silent_ = false, bool toFile_ = false) : silent(silent_), toFile(toFile_) {}
    LoggerOptions(std::string_view contextSeparator_, bool silent_ = false, bool toFile_ = false) :
        silent(silent_), toFile(toFile_), contextSeparator(contextSeparator_) {}
};

class LoggerContextObject;
class Consumer;

class Logger {
    friend Consumer;
    friend LoggerBuffer;
    public:
        Logger(const ModInfo info, LoggerOptions options_) : options(options_), modInfo(info), buff(emplace_safe(modInfo)) {
            tag = "QuestHook[" + info.id + "|v" + info.version + "]";
            init();
        }
        Logger(const ModInfo info) : Logger(info, LoggerOptions{false, false}) {}
        void log(Logging::Level lvl, std::string str) const;
        void log(Logging::Level lvl, std::string_view fmt, ...) const;
        void critical(std::string_view fmt, ...) const;
        void error(std::string_view fmt, ...) const;
        void warning(std::string_view fmt, ...) const;
        void info(std::string_view fmt, ...) const;
        void debug(std::string_view fmt, ...) const;
        /// @brief Flushes the buffer for this logger instance.
        void flush() const;
        /// @brief Closes the buffer for this logger instance, flushing as necessary.
        /// After this call, this logger will no longer log to a buffer, nor will it log to a file.
        void close() const;
        /// @brief Returns the logger that is used within the utils library. This function should not be used outside of the main library
        static Logger& get();
        /// @brief Close all open LoggerBuffer objects. Should only be called on a crash or exit of the game.
        static void closeAll();
        /// @brief Flush all open LoggerBuffer objects.
        static void flushAll();
        /// @brief Initialize this logger. Deletes existing file logs.
        /// This happens on default when this instance is constructed.
        /// This should also be called anytime the options field is modified.
        void init() const;
        /// @brief Call this to silence logs from this logger. Should improve performance slightly.
        /// Note that this call causes ALL calls to this particular logger to be silent, including from other mods.
        /// Should only be used in particular cases.
        constexpr void disable() {
            options.silent = true;
        }
        /// @brief Call this to re-enable logs for this logger. Decreases performance slightly, but provides debug information.
        /// Note that this call causes ALL calls to this particular logger to be enabled again, including from other mods that want silence.
        /// Should only be used in particular cases.
        constexpr void enable() {
            options.silent = false;
        }
        /// @brief Returns the current options for this logger
        const LoggerOptions getOptions() const {
            return options;
        }
        /// @brief Enters a logging context. Should be used for more specific logging information.
        /// @param context The context name to enter
        /// @returns LoggerContextObject that is used within the context.
        LoggerContextObject WithContext(std::string_view context);
        /// @brief Disable logging for any contexts that start with the provided string.
        /// This is thread independent, and will silence across all threads
        void DisableContext(std::string_view context);
        /// @brief Enables logging for the context.
        /// This function does nothing if DisableContext was not called with an exactly matching string before this call.
        void EnableContext(std::string_view context);
    private:
        /// @brief The options associated with this logger
        LoggerOptions options;
        std::unordered_set<std::string> disabledContexts;
        /// @brief The mutex for the contexts maps/sets
        std::mutex contextMutex;
        std::string tag;
        const ModInfo modInfo;
        LoggerBuffer& buff;
        static bool consumerStarted;
        static std::vector<LoggerBuffer> buffers;
        static std::mutex bufferMutex;

        static LoggerBuffer& emplace_safe(const ModInfo& info) {
            // Obtain lock
            bufferMutex.lock();
            // Emplace, lock is released
            auto& itr = Logger::buffers.emplace_back(info);
            bufferMutex.unlock();
            return itr;
        }
        static void startConsumer();
};

class LoggerContextObject {
    std::string tag;
    public:
    /// @brief a const Logger reference. If you wish to modify the original logger, modify it before calling .WithContext
    const Logger& logger;
    /// @brief The context of this LoggerContextObject
    const std::string context;
    LoggerContextObject(const Logger& l, std::string_view context_) : logger(l), context(context_) {
        tag.append("(").append(context_).append(") ");
    }
    // Cannot copy a context object
    LoggerContextObject(const LoggerContextObject&) = delete;
    // Can move a context object
    LoggerContextObject(LoggerContextObject&& other) = default;
    // Can delete a context object
    ~LoggerContextObject() = default;

    void log(Logging::Level lvl, std::string str) const {
        logger.log(lvl, tag + str);
    }
    template<typename... TArgs>
    void log(Logging::Level lvl, std::string_view fmt, TArgs... args) const {
        logger.log(lvl, tag + fmt.data(), args...);
    }
    template<typename... TArgs>
    void critical(std::string_view fmt, TArgs... args) const {
        logger.critical(tag + fmt.data(), args...);
    }
    template<typename... TArgs>
    void error(std::string_view fmt, TArgs... args) const {
        logger.error(tag + fmt.data(), args...);
    }
    template<typename... TArgs>
    void warning(std::string_view fmt, TArgs... args) const {
        logger.warning(tag + fmt.data(), args...);
    }
    template<typename... TArgs>
    void info(std::string_view fmt, TArgs... args) const {
        logger.info(tag + fmt.data(), args...);
    }
    template<typename... TArgs>
    void debug(std::string_view fmt, TArgs... args) const {
        logger.debug(tag + fmt.data(), args...);
    }
    /// @brief Enter a new context. The context entered is separated by logger.options.contextSeparator
    /// @param ctx The context name to enter
    /// @returns The LoggerContextObject in the context
    LoggerContextObject WithContext(std::string_view ctx) {
        return LoggerContextObject(logger, context + logger.getOptions().contextSeparator + std::string(ctx));
    }
};
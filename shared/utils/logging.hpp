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
#include <list>
#include <mutex>
#include "modloader/shared/modloader.hpp"
#include "utils-functions.h"

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
    std::mutex mut;
    std::list<std::string> messages;
    const ModInfo& modInfo;
    const std::string path;
    bool closed;
    static inline std::string get_path(const ModInfo& info) noexcept {
        std::string cpy(info.version);
        std::replace(cpy.begin(), cpy.end(), '.', '_');
        return string_format(LOG_PATH, Modloader::getApplicationId().c_str()) + info.id + "_" + cpy + ".log";
    }
    template<typename T>
    static inline T&& safe_move(std::mutex& mtx, T& t) noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        return std::move(t);
    }
    std::size_t length() noexcept {
        std::unique_lock<std::mutex> lock(mut);
        if (closed) {
            // Ignore messages to write if we are closed.
            return 0;
        }
        return messages.size();
    }
    void addMessage(std::string msg) {
        std::unique_lock<std::mutex> lock(mut);
        if (closed) {
            return;
        }
        messages.push_back(msg);
    }
    public:
    void flush();
    LoggerBuffer(const ModInfo& info) : modInfo(info), path(get_path(modInfo)), closed(false) {}
    LoggerBuffer(LoggerBuffer&& other) : messages(safe_move(other.mut, other.messages)), modInfo(safe_move(other.mut, other.modInfo)), path(safe_move(other.mut, other.path)), closed(safe_move(other.mut, other.closed)) {}
};

/// @struct Logger Options
/// @brief Provides various options for loggers, including silencing them and logging to file.
struct LoggerOptions {
    bool silent = false;
    bool toFile = false;
    constexpr LoggerOptions(bool silent_ = false, bool toFile_ = false) : silent(silent_), toFile(toFile_) {}
};

class Consumer;

class Logger {
    friend Consumer;
    friend LoggerBuffer;
    public:
        Logger(const ModInfo info, LoggerOptions options_) : options(options_), modInfo(info), buff(emplace_safe(modInfo)) {
            tag = info.id + "v|" + info.version;
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
        static const Logger& get();
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
        LoggerOptions options;
    private:
        std::string tag;
        const ModInfo modInfo;
        LoggerBuffer& buff;
        static bool consumerStarted;
        static std::vector<LoggerBuffer> buffers;
        static std::mutex bufferMutex;
        static inline LoggerBuffer& emplace_safe(const ModInfo& info) {
            // Obtain lock
            std::unique_lock<std::mutex> lck(bufferMutex);
            // Emplace, lock is released
            return buffers.emplace_back(info);
        }
        static void startConsumer();
};

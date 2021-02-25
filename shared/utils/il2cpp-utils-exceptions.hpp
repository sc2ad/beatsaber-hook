#ifndef IL2CPP_UTILS_EXCEPTIONS
#define IL2CPP_UTILS_EXCEPTIONS

#pragma pack(push)

#include <exception>
#include <string_view>
#include <string>

struct Il2CppException;

namespace il2cpp_utils {
    // Returns a legible string from an Il2CppException*
    ::std::string ExceptionToString(Il2CppException* exp) noexcept;

    #if __has_feature(cxx_exceptions)
    struct Il2CppUtilsException : std::runtime_error {
        std::string context;
        std::string msg;
        std::string func = "unknown";
        std::string file = "unknown";
        int line = -1;
        Il2CppUtilsException(std::string_view msg_) : msg(msg_.data()), std::runtime_error(CreateMessage(msg_.data())) {}
        Il2CppUtilsException(std::string_view context_, std::string_view msg_) : context(context_.data()), msg(msg_.data()), std::runtime_error(CreateMessage(msg_.data(), context_.data())) {}
        Il2CppUtilsException(std::string_view msg_, std::string_view func_, std::string_view file_, int line_)
            : msg(msg_.data()), func(func_.data()), file(file_.data()), line(line_), std::runtime_error(CreateMessage(msg_.data(), "", func_.data(), file_.data(), line_)) {}
        Il2CppUtilsException(std::string_view context_, std::string_view msg_, std::string_view func_, std::string_view file_, int line_)
            : context(context_.data()), msg(msg_.data()), func(func_.data()), file(file_.data()), line(line_), std::runtime_error(CreateMessage(msg_.data(), context_.data(), func_.data(), file_.data(), line_)) {}

        std::string CreateMessage(std::string msg, std::string context = "", std::string func = "unknown", std::string file = "unknown", int line = -1) {
            return ((context.size() > 0 ? ("(" + context + ") ") : "") + msg + " in: " + func + " " + file + ":" + std::to_string(line));
        }
    };
    struct RunMethodException : std::runtime_error {
        const Il2CppException* ex;
        const MethodInfo* info;
        RunMethodException(std::string_view msg, const MethodInfo* inf) : std::runtime_error(msg.data()), ex(nullptr), info(inf) {}
        RunMethodException(Il2CppException* exp, const MethodInfo* inf) : std::runtime_error(ExceptionToString(exp).c_str()), ex(exp), info(inf) {}
    };
    #endif
}

#pragma pack(pop)

#endif
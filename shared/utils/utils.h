#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#ifndef __aarch64__
#define LIBIL2CPP_PATH "/data/data/com.beatgames.beatsaber-1/lib/armeabi-v7a/libil2cpp.so"
#endif

#include <cxxabi.h>
#if __has_include(<string_view>)
#include <string_view>
#elif __has_include(<experimental/string_view>)
#include <experimental/string_view>
namespace std {
    using experimental::string_view;
    using experimental::basic_string_view;
    using experimental::u16string_view;
}
#else
#error No string_view implementation available!
#endif
#include <thread>
#include <optional>

// For use in SAFE_ABORT/CRASH_UNLESS (& RET_UNLESS if possible)
#if __has_include(<source_location>)
#error please alert sc2ad/beatsaber-hook that "std::source_location is live" (sharing your Android NDK version) then comment this out!
#elif __has_include(<experimental/source_location>)
#error please alert sc2ad/beatsaber-hook that "std::experimental::source_location is live" (sharing your Android NDK version) then comment this out!
#endif

// For use in ClassOrInstance concept
#if __has_include(<concepts>)
#error please alert sc2ad/beatsaber-hook that "std::concepts are live" (sharing your Android NDK version) then comment this out!
#elif __has_include(<experimental/concepts>)
#error please alert sc2ad/beatsaber-hook that "std::experimental::concepts are live" (sharing your Android NDK version) then comment this out!
#endif

template <typename Container> struct is_vector : std::false_type { };
template <typename... Ts> struct is_vector<std::vector<Ts...> > : std::true_type { };
// TODO: figure out how to write an is_vector_v that compiles properly?

#define MACRO_WRAP(expr) do { \
    expr; \
} while(0)

template <class, template <class, class...> class>
struct is_instance : public std::false_type {};

template <class...Ts, template <class, class...> class U>
struct is_instance<U<Ts...>, U> : public std::true_type {};

template<class T>
auto&& unwrap_optionals(T&& arg) {
    if constexpr (is_instance<std::decay_t<T>, std::optional>::value) {
        return *arg;
    } else {
        return arg;
    }
}

// >>>>>>>>>>>>>>>>> DO NOT NEST X_UNLESS MACROS <<<<<<<<<<<<<<<<<
// Prefer RunMethod.value_or to a nested X_UNLESS(RunMethod)

// Logs error and RETURNS argument 1 IFF argument 2 boolean evaluates as false; else EVALUATES to argument 2
// thank god for this GCC ({}) extension which "evaluates to the last statement"
#ifndef SUPPRESS_MACRO_LOGS
#define RET_UNLESS(retval, expr) ({ \
    auto&& __temp__ = (expr); \
    if (!__temp__) { \
        Logger::get().error("%s (in %s at %s:%i) returned false!", #expr, __PRETTY_FUNCTION__, __FILE__, __LINE__); \
        return retval; \
    } \
    unwrap_optionals(__temp__); })
#else
#define RET_UNLESS(retval, expr) ({ \
    auto&& __temp__ = (expr); \
    if (!__temp__) { \
        return retval; \
    } \
    unwrap_optionals(__temp__); })
#endif

#define RET_V_UNLESS(expr) RET_UNLESS(, expr)
#define RET_DEFAULT_UNLESS(expr) RET_UNLESS({}, expr)
#define RET_0_UNLESS(expr) RET_DEFAULT_UNLESS(expr)
#define RET_NULLOPT_UNLESS(expr) RET_DEFAULT_UNLESS(expr)

// Produces a has_[member]<T, U> type trait whose ::value tells you whether T has a member named [member] with type U.
#define DEFINE_MEMBER_CHECKER(member) \
    template<typename T, typename U, typename Enable = void> \
    struct has_ ## member : std::false_type { }; \
    template<typename T, typename U> \
    struct has_ ## member<T, U, \
        typename std::enable_if_t< \
            std::is_same_v<decltype(T::member), U>> \
        > : std::true_type { };

#if __has_feature(cxx_rtti)
// from https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template#comment77016419_19123821 (https://ideone.com/sqFWir)
template<typename T>
std::string type_name() {
	std::string tname = typeid(T).name();
	#if defined(__clang__) || defined(__GNUG__)
	int status;
	char *demangled_name = abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
	if (status == 0) {
		tname = demangled_name;
		std::free(demangled_name);
	}
	#endif
	return tname;
}
#endif

// For use in fire-if-compiled asserts e.g. static_assert(false_t<T>, "message")
template <class...> constexpr std::false_type false_t{};

#include "utils-functions.h"
#include "../inline-hook/And64InlineHook.hpp"
#include "logging.hpp"

#ifdef __cplusplus

template <typename Function, typename... Args>
static void StartCoroutine(Function&& fun, Args&&... args) {
    auto t = new std::thread(std::forward<Function>(fun), std::forward<Args>(args)...);
    t->detach();
}

// logs the function, file and line, sleeps to allow logs to flush, then terminates program
void safeAbort(const char* func, const char* file, int line);
// sets "file" and "line" to the file and line you call this macro from
#ifndef SUPPRESS_MACRO_LOGS
#define SAFE_ABORT() safeAbort(__PRETTY_FUNCTION__, __FILE__, __LINE__)
#else
#define SAFE_ABORT() safeAbort("undefined_function", "undefined_file", -1)
#endif

template<class T>
auto crashUnless(T&& arg, const char* func, const char* file, int line) {
    if (!arg) safeAbort(func, file, line);
    return unwrap_optionals(arg);
}
#ifndef SUPPRESS_MACRO_LOGS
#define CRASH_UNLESS(expr) crashUnless(expr, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#else
#define CRASH_UNLESS(expr) crashUnless(expr, "undefined_function", "undefined_file", -1)
#endif

template<class T>
intptr_t getBase(T pc) {
    static_assert(sizeof(T) >= sizeof(void*));
    Dl_info info;
    RET_0_UNLESS(dladdr((void*)pc, &info));
    return (intptr_t)info.dli_fbase;
}

template<class T>
ptrdiff_t asOffset(T pc) {
    auto base = getBase(pc);
    return (ptrdiff_t)(((intptr_t)pc) - base);
}

// function_ptr_t courtesy of DaNike
template<typename TRet, typename ...TArgs>
// A generic function pointer, which can be called with and set to a `getRealOffset` call
using function_ptr_t = TRet(*)(TArgs...);

// Yoinked from: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
// TODO: This should be removed once std::format exists
template<typename... TArgs>
std::string string_format(const std::string_view format, TArgs ... args)
{
    size_t size = snprintf(nullptr, 0, format.data(), args ...) + 1; // Extra space for '\0'
    if (size <= 0)
        return "";
    std::unique_ptr<char[]> buf(new char[size]); 
    snprintf(buf.get(), size, format.data(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#if __has_feature(cxx_exceptions)
template<class T>
auto throwUnless(T&& arg, const char* func, const char* file, int line) {
    if (!arg) throw std::runtime_error(string_format("Throwing in %s at %s:%i", func, file, line));
    return unwrap_optionals(arg);
}
#ifndef SUPPRESS_MACRO_LOGS
#define THROW_UNLESS(expr) throwUnless(expr, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#else
#define THROW_UNLESS(expr) throwUnless(expr, "undefined_function", "undefined_file", -1)
#endif /* SUPPRESS_MACRO_LOGS */
#endif /* __has_feature(cxx_exceptions) */

extern "C" {
#endif /* __cplusplus */

// Creates all directories for a provided file_path
// Ex: /sdcard/Android/data/something/files/libs/
int mkpath(char* file_path, mode_t mode);

// Restores an existing stringstream to a newly created state.
void resetSS(std::stringstream& ss);
// Prints the given number of "tabs" as spaces to the given output stream.
void tabs(std::ostream& os, int tabs, int spacesPerTab = 2);
// Logs the given stringstream and clears it.
void print(std::stringstream& ss, Logging::Level lvl = Logging::INFO);

// Attempts to print what is stored at the given pointer.
// For a given pointer, it will scan 4 void*'s worth of bytes at the location pointed to.
// For each void* of bytes, it will print the raw bytes and interpretations of the bytes as ints and char*s.
// When the bytes look like a valid pointer, it will attempt to follow that pointer, increasing the indentation.
//   It will not follow pointers that it has already analyzed as a result of the current call.
void analyzeBytes(const void* ptr);

intptr_t getRealOffset(const void* offset);
intptr_t baseAddr(const char* soname);

// Only wildcard is ? and ?? - both are handled the same way. They will skip exactly 1 byte (2 hex digits)
intptr_t findPattern(intptr_t dwAddress, const char* pattern, intptr_t dwSearchRangeLen = 0x1000000);
// Same as findPattern but will continue scanning to make sure your pattern is sufficiently specific.
// Each candidate will be logged. label should describe what you're looking for, like "Class::Init".
// Sets "multiple" iff multiple matches are found, and outputs a log warning message.
// Returns the first match, if any.
intptr_t findUniquePattern(bool& multiple, intptr_t dwAddress, const char* pattern, const char* label = 0, intptr_t dwSearchRangeLen = 0x1000000);

#define MAKE_HOOK(name, addr, retval, ...) \
void* addr_ ## name = (void*) addr; \
retval (*name)(__VA_ARGS__) = NULL; \
retval hook_ ## name(__VA_ARGS__)

#define MAKE_HOOK_OFFSETLESS(name, retval, ...) \
retval (*name)(__VA_ARGS__) = NULL; \
retval hook_ ## name(__VA_ARGS__)

#define MAKE_HOOK_NAT(name, addr, retval, ...) \
void* addr_ ## name = (void*) addr; \
retval (*name)(__VA_ARGS__) = NULL; \
retval hook_ ## name(__VA_ARGS__)

#ifndef SUPPRESS_MACRO_LOGS

#ifdef __aarch64__

#define INSTALL_HOOK(name) MACRO_WRAP( \
Logger::get().info("Installing 64 bit hook: %s", #name); \
A64HookFunction((void*)getRealOffset(addr_ ## name),(void*) hook_ ## name, (void**)&name); \
)

#define INSTALL_HOOK_OFFSETLESS(name, methodInfo) MACRO_WRAP( \
Logger::get().info("Installing 64 bit offsetless hook: %s at %lX", #name, asOffset(methodInfo->methodPointer)); \
A64HookFunction((void*)methodInfo->methodPointer,(void*) hook_ ## name, (void**)&name); \
)

#define INSTALL_HOOK_NAT(name) MACRO_WRAP( \
Logger::get().info("Installing 64 bit native hook: %s", #name); \
A64HookFunction((void*)(addr_ ## name),(void*) hook_ ## name, (void**)&name); \
)

#define INSTALL_HOOK_DIRECT(name, addr) MACRO_WRAP( \
Logger::get().info("Installing 64 bit direct hook: %s", #name); \
A64HookFunction((void*)addr, (void*) hook_ ## name, (void**)&name); \
)

// Uninstalls currently just creates a hook at the hooked address
// and sets the hook to call the original function
// No original trampoline is created when uninstalling a hook, hence the nullptr

#define UNINSTALL_HOOK(name) MACRO_WRAP( \
Logger::get().info("Uninstalling 64 bit hook: %s", #name); \
A64HookFunction((void*)getRealOffset(addr_ ## name),(void*)name, (void**)nullptr); \
)

#define UNINSTALL_HOOK_OFFSETLESS(name, methodInfo) MACRO_WRAP( \
Logger::get().info("Uninstalling 64 bit offsetless hook: %s at %lX", #name, asOffset(methodInfo->methodPointer)); \
A64HookFunction((void*)methodInfo->methodPointer,(void*)name, (void**)nullptr); \
)

#define UNINSTALL_HOOK_NAT(name) MACRO_WRAP( \
Logger::get().info("Uninstalling 64 bit native hook: %s", #name); \
A64HookFunction((void*)(addr_ ## name),(void*)name, (void**)nullptr); \
)

#define UNINSTALL_HOOK_DIRECT(name, addr) MACRO_WRAP( \
Logger::get().info("Uninstalling 64 bit direct hook: %s", #name); \
A64HookFunction((void*)addr, (void*)name, (void**)nullptr); \
)

#else

#define INSTALL_HOOK(name) MACRO_WRAP( \
Logger::get().info("Installing 32 bit hook!"); \
registerInlineHook((uint32_t)getRealOffset(addr_ ## name), (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)getRealOffset(addr_ ## name)); \
)

#define INSTALL_HOOK_OFFSETLESS(name, methodInfo) MACRO_WRAP( \
Logger::get().info("Installing 32 bit offsetless hook: %s at %lX", #name, asOffset(methodInfo->methodPointer)); \
registerInlineHook((uint32_t)methodInfo->methodPointer, (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)methodInfo->methodPointer); \
)

#define INSTALL_HOOK_NAT(name) MACRO_WRAP( \
Logger::get().info("Installing 32 bit native hook!"); \
registerInlineHook((uint32_t)(addr_ ## name), (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)(addr_ ## name)); \
)

#define INSTALL_HOOK_DIRECT(name, addr) MACRO_WRAP( \
Logger::get().info("Installing 32 bit offsetless hook!"); \
registerInlineHook((uint32_t)addr, (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)addr); \
)

#endif /* __aarch64__ */

#else /* SUPPRESS_MACRO_LOGS */

#ifdef __aarch64__

#define INSTALL_HOOK(name) MACRO_WRAP( \
A64HookFunction((void*)getRealOffset(addr_ ## name),(void*) hook_ ## name, (void**)&name); \
)

#define INSTALL_HOOK_OFFSETLESS(name, methodInfo) MACRO_WRAP( \
A64HookFunction((void*)methodInfo->methodPointer,(void*) hook_ ## name, (void**)&name); \
)

#define INSTALL_HOOK_NAT(name) MACRO_WRAP( \
A64HookFunction((void*)(addr_ ## name),(void*) hook_ ## name, (void**)&name); \
)

#define INSTALL_HOOK_DIRECT(name, addr) MACRO_WRAP( \
A64HookFunction((void*)addr, (void*) hook_ ## name, (void**)&name); \
)

// Uninstalls currently just creates a hook at the hooked address
// and sets the hook to call the original function
// No original trampoline is created when uninstalling a hook, hence the nullptr

#define UNINSTALL_HOOK(name) MACRO_WRAP( \
A64HookFunction((void*)getRealOffset(addr_ ## name),(void*)name, (void**)nullptr); \
)

#define UNINSTALL_HOOK_OFFSETLESS(name, methodInfo) MACRO_WRAP( \
A64HookFunction((void*)methodInfo->methodPointer,(void*)name, (void**)nullptr); \
)

#define UNINSTALL_HOOK_NAT(name) MACRO_WRAP( \
A64HookFunction((void*)(addr_ ## name),(void*)name, (void**)nullptr); \
)

#define UNINSTALL_HOOK_DIRECT(name, addr) MACRO_WRAP( \
A64HookFunction((void*)addr, (void*)name, (void**)nullptr); \
)

#else __aarch64__

#define INSTALL_HOOK(name) MACRO_WRAP( \
registerInlineHook((uint32_t)getRealOffset(addr_ ## name), (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)getRealOffset(addr_ ## name)); \
)

#define INSTALL_HOOK_OFFSETLESS(name, methodInfo) MACRO_WRAP( \
registerInlineHook((uint32_t)methodInfo->methodPointer, (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)methodInfo->methodPointer); \
)

#define INSTALL_HOOK_NAT(name) MACRO_WRAP( \
registerInlineHook((uint32_t)(addr_ ## name), (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)(addr_ ## name)); \
)

#define INSTALL_HOOK_DIRECT(name, addr) MACRO_WRAP( \
registerInlineHook((uint32_t)addr, (uint32_t)hook_ ## name, (uint32_t **)&name); \
inlineHook((uint32_t)addr); \
)

#endif /* __aarch64__ */

#endif /* SUPPRESS_MACRO_LOGS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTILS_H_INCLUDED */

#pragma once

#include "../inline-hook/And64InlineHook.hpp"
#include "hook-tracker.hpp"
#include <type_traits>
#include <utility>
#include "utils.h"
#include "typedefs.h"
#include "logging.hpp"
#include "il2cpp-utils.hpp"

namespace Hooking {
// For use in MAKE_HOOK_AUTO bodies.
// Currently unused.
template<std::size_t N, typename... TArgs>
requires (sizeof...(TArgs) == 0 && N >= 0)
decltype(auto) param_get([[maybe_unused]] TArgs&&... as) noexcept {
    static_assert(N < 0, "Parameter index out of bounds!");
}

template<std::size_t N, typename Tfirst, typename... Args, std::enable_if_t<N == 0, int>...>
decltype(auto) param_get(Tfirst&& first, [[maybe_unused]] Args&&... as) noexcept {
    return std::forward<Tfirst>(first);
}

template<std::size_t N, typename Tfirst, typename... Args, std::enable_if_t<N != 0, int>...>
decltype(auto) param_get([[maybe_unused]] Tfirst&& first, Args&&... as) noexcept {
    return param_get<N - 1>(std::forward<Args>(as)...);
}

// Gets the parameter at the given index (starting at 0, not including self) to return in a MAKE_HOOK_AUTO body.
#define PARAM(i) \
::Hooking::param_get<i>(__args...)

template<typename L>
concept is_logger = requires (L& l) {
    l.info("");
    l.debug("");
    l.error("");
    l.warning("");
    l.critical("");
};

template<typename T>
concept is_hook = requires {
    // Must have a name
    {T::name()} -> std::same_as<const char*>;
    // Must have a trampoline that returns the funcType
    {T::trampoline()} -> std::same_as<typename T::funcType*>;
    // Must have a hook that returns the funcType
    {T::hook()} -> std::same_as<typename T::funcType>;
};

template<typename T>
concept has_addr = requires {
    {T::addr()} -> std::same_as<void*>;
};

template<typename T>
concept is_addr_hook = is_hook<T> && has_addr<T>;

template<typename T>
concept is_findCall_hook = is_hook<T> && requires {
    // Must have a getInfo() method
    {T::getInfo()} -> std::same_as<const MethodInfo*>;
};

/// @brief Represents a type that provides the funcType from a provided method pointer type.
/// This type is used in determining hook checks.
template<typename T>
struct InternalMethodCheck;

template<typename R, typename... TArgs>
struct InternalMethodCheck<R (*)(TArgs...)> {
    using funcType = R (*)(TArgs...);
};
template<typename R, typename T, typename... TArgs>
struct InternalMethodCheck<R (T::*)(TArgs...)> {
    using funcType = R (*)(T*, TArgs...);
};

/// @brief Exposes the instance type of the provided instance method.
template<typename T>
struct InternalClassGetter {
    using instanceType = void;
};

template<typename R, typename T, typename... TArgs>
struct InternalClassGetter<R (T::*)(TArgs...)> {
    using instanceType = T;
};

// Make an address-specified hook.
#define MAKE_HOOK(name_, addr_, retval, ...) \
struct Hook_##name_ { \
    constexpr static const char* name() { return #name_; } \
    constexpr static void* addr() { return (void*) addr_; } \
    using funcType = retval (*)(__VA_ARGS__); \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that resolves the 'infoGet' expression an installs the hook to that MethodInfo*.
#define MAKE_HOOK_FIND_VERBOSE(name_, infoGet, retval, ...) \
struct Hook_##name_ { \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return infoGet; } \
    using funcType = retval (*)(__VA_ARGS__); \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that finds a method that matches the signature provided and exists on the provided Il2CppClass* with the provided method name.
#define MAKE_HOOK_FIND(name_, klass, mName, retval, ...) \
struct Hook_##name_ { \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return ::il2cpp_utils::MethodTypeCheck<funcType>::find(klass, mName); } \
    using funcType = retval (*)(__VA_ARGS__); \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that finds a method that matches the signature provided and exists on the provided namespace and type name, with the provided method name.
#define MAKE_HOOK_FIND_CLASS(name_, namespaze, klassName, mName, retval, ...) \
struct Hook_##name_ { \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return ::il2cpp_utils::MethodTypeCheck<funcType>::find(namespaze, klassName, mName); } \
    using funcType = retval (*)(__VA_ARGS__); \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that would be installed in a particular address, but ensures the signature matches the provided method pointer.
#define MAKE_HOOK_CHECKED_ADDR(name_, mPtr, addr_, retval, ...) \
struct Hook_##name_ { \
    using funcType = retval (*)(__VA_ARGS__); \
    static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!"); \
    constexpr static const char* name() { return #name_; } \
    constexpr static void* addr() { return addr_; } \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that ensures the signature matches the provided method pointer and finds a matching method from a class and method name.
#define MAKE_HOOK_CHECKED_FIND(name_, mPtr, klass, mName, retval, ...) \
struct Hook_##name_ { \
    using funcType = retval (*)(__VA_ARGS__); \
    static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!"); \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return ::il2cpp_utils::MethodTypeCheck<funcType>::find(klass, mName); } \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that ensures the signature matches the provided method pointer and finds a matching method from namespace, name, and method name.
#define MAKE_HOOK_CHECKED_FIND_CLASS(name_, mPtr, namespaze, klassName, mName, retval, ...) \
struct Hook_##name_ { \
    using funcType = retval (*)(__VA_ARGS__); \
    static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!"); \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return ::il2cpp_utils::MethodTypeCheck<funcType>::find(namespaze, klassName, mName); } \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that ensures the signature matches the provided method pointer and finds a matching method from a class and method name.
#define MAKE_HOOK_INSTANCE_FIND(name_, mPtr, mName, retval, ...) \
struct Hook_##name_ { \
    using funcType = retval (*)(__VA_ARGS__); \
    using classType = ::Hooking::InternalClassGetter<decltype(mPtr)>::instanceType; \
    static_assert(!std::is_same_v<classType, void>, "MAKE_HOOK_INSTANCE_FIND was not provided an instance method pointer!"); \
    static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!"); \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return ::il2cpp_utils::MethodTypeCheck<funcType>::find(classof(classType), mName); } \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// Make a hook that uses the provided method pointer in a match an ensures the signature matches.
// This should be your go-to hook macro when hooking anything that has a codegen equivalent.
#define MAKE_HOOK_MATCH(name_, mPtr, retval, ...) \
struct Hook_##name_ { \
    using funcType = retval (*)(__VA_ARGS__); \
    static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!"); \
    constexpr static const char* name() { return #name_; } \
    static const MethodInfo* getInfo() { return ::il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get(); } \
    static funcType* trampoline() { return &name_; } \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; \
    static funcType hook() { return hook_##name_; } \
    static retval hook_##name_(__VA_ARGS__); \
}; \
retval Hook_##name_::hook_##name_(__VA_ARGS__)

// TODO: IMPLEMENT AUTO HOOKS!
#define MAKE_HOOK_AUTO(...) void

#define MAKE_HOOK_INSTANCE_AUTO(...) void

// Make a hook that writes the signature based off of the provided static method.
// To access parameters, see the PARAM(i) macro.
// #define MAKE_HOOK_STATIC_AUTO(name, ...) \
// template<class T> \
// struct __##name; \
// template<typename R, typename... TArgs> \
// struct __##name<R (*)(TArgs...)> { \
//     static inline R (*name)(TArgs... __args); \
//     static R hook_##name(TArgs... __args) { \
//         __VA_ARGS__ \
//     } \
// }; \
// template<typename R, typename T, typename... TArgs> \
// struct __##name<R (T::*)(TArgs...)> { \
//     static_assert(!std::is_same_v<R, R>, "Attempting to MAKE_HOOK_STATIC_AUTO with an instance method! See MAKE_HOOK_INSTANCE_AUTO instead!"); \
// };

// // Make a hook that writes the signature based off of the provided static method.
// // To access parameters, use `self` and see the PARAM(i) macro (where self is not included as a PARAM)
// // Should only be used with INSTALL_HOOK_AUTO
// #define MAKE_HOOK_INSTANCE_AUTO(name, ...) \
// template<class T> \
// struct __##name; \
// template<typename R, typename T, typename... TArgs> \
// struct __##name<R (T::*)(TArgs...)> { \
//     static inline R (*name)(T* self, TArgs... __args); \
//     static R hook_##name(T* self, TArgs... __args) { \
//         __VA_ARGS__ \
//     } \
// }; \
// template<typename R, typename... TArgs> \
// struct __##name<R (*)(TArgs...)> { \
//     static_assert(!std::is_same_v<R, R>, "Attempting to MAKE_HOOK_INSTANCE_AUTO with a static method! See MAKE_HOOK_STATIC_AUTO instead!"); \
// };

template<typename T, typename L, bool track = true>
inline void __InstallHook(L& logger, void* addr) {
    #ifndef SUPPRESS_MACRO_LOGS
    logger.info("Installing hook: %s to offset: %p", T::name(), addr);
    #endif
    #ifdef __aarch64__
    A64HookFunction(addr, (void*) T::hook(), (void**) T::trampoline());
    if constexpr (track) {
        HookTracker::AddHook(T::name(), addr, (void*) T::hook(), (void*) *T::trampoline());
    }
    #else
    registerInlineHook((uint32_t) addr, (uint32_t) T::hook(), (uint32_t **) T::trampoline());
    inlineHook((uint32_t) addr);
    #endif
}

template<typename T, typename L>
requires (is_addr_hook<T> && !is_findCall_hook<T> && is_logger<L>)
void InstallHook(L& logger) {
    // Install T assuming it is an address hook.
    auto addr = (void*) getRealOffset(T::addr());
    __InstallHook<T>(logger, addr);
}
template<typename T, typename L>
requires (is_findCall_hook<T> && !is_addr_hook<T> && is_logger<L>)
void InstallHook(L& logger) {
    // Install T assuming it is a hook that should call FindMethod.
    auto info = T::getInfo();
    if (!info) {
        #ifndef SUPPRESS_MACRO_LOGS
        logger.critical("Attempting to install hook: %s, but method could not be found!", T::name());
        #endif
        SAFE_ABORT();
    }
    auto addr = (void*) info->methodPointer;
    __InstallHook<T>(logger, addr);
}
template<typename T, typename L>
requires (is_findCall_hook<T> && !is_addr_hook<T> && is_logger<L>)
void InstallOrigHook(L& logger) {
    // Install T assuming it is a hook that should call FindMethod.
    auto info = T::getInfo();
    if (!info) {
        #ifndef SUPPRESS_MACRO_LOGS
        logger.critical("Attempting to install hook: %s, but method could not be found!", T::name());
        #endif
        SAFE_ABORT();
    }
    auto addr = (void*) info->methodPointer;
    auto* origAddr = const_cast<void*>(HookTracker::GetOrig(addr));
    if (origAddr != addr) { \
        auto* hooks = const_cast<std::unordered_map<const void*, std::list<HookInfo>>*>(HookTracker::GetHooks());
        auto itr = hooks->find(addr);
        if (itr != hooks->end() && itr->second.size() > 0) {
            itr->second.front().orig = (void*) *T::trampoline();
        }
    }
    __InstallHook<T, L, false>(logger, addr);
}
template<typename T, typename L>
requires (is_hook<T> && is_logger<L>)
void InstallHookDirect(L& logger, void* dst) {
    // Install T into the specified address. Null checks dst.
    if (!dst) {
        #ifndef SUPPRESS_MACRO_LOGS
        logger.critical("Attempting to install direct hook: %s, but was installing to an invalid destination!", T::name());
        #endif
        SAFE_ABORT();
    }
    __InstallHook<T>(logger, dst);
}

// Installs the provided hook using the logger provided.
// This properly specializes based off of whichever MAKE_HOOK macro you used, but is only valid if the name is from a MAKE_HOOK... macro.
#define INSTALL_HOOK(logger, name) ::Hooking::InstallHook<Hook_##name>(logger);

// Installs the provided hook using the logger provided to the address specified directly.
// This is only valid if the name is from a MAKE_HOOK... macro.
#define INSTALL_HOOK_DIRECT(logger, name, addr) ::Hooking::InstallHookDirect<Hook_##name>(logger, addr);

// Installs the provided hook using the logger provided to the 'orig' point of the hook.
// This is only valid if the name is from a MAKE_HOOK macro that does not use a fixed offset.
// This also ensures HookTracker validity after the hooking process.
#define INSTALL_HOOK_ORIG(logger, name) ::Hooking::InstallOrigHook<Hook_##name>(logger);

// TODO: Not yet implemented
#define UNINSTALL_HOOK(logger, name) void

// TODO: Not yet implemented
#define UNINSTALL_HOOK_DIRECT(logger, name, addr) void
}
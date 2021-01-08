#pragma once
#include <string_view>
#include <string>
#include <list>
#include <unordered_map>

/// @brief Stores information about an installed hook.
struct HookInfo {
    const std::string name;
    const void* const destination;
    const void* const trampoline;
    const void* const orig;
    HookInfo(std::string_view name_, void* dst, void* src, void* orig_)
        : name(name_.data()), destination(dst), trampoline(src), orig(orig_) {}
    bool operator==(const HookInfo& other) const {
        return name == other.name && destination == other.destination && trampoline == other.trampoline && orig == other.orig;
    }
};

struct HookTracker {
    /// @brief Adds a HookInfo to be tracked.
    /// @param info The HookInfo to track.
    static void AddHook(HookInfo info) noexcept;
    /// @brief Calls AddHook
    template<typename... TArgs>
    static void AddHook(TArgs&&... args) noexcept {
        AddHook(HookInfo(std::forward<TArgs>(args)...));
    }
    /// @brief Stops tracking the provided HookInfo.
    /// @param info The HookInfo to stop tracking.
    static void RemoveHook(HookInfo info) noexcept;
    /// @brief Calls RemoveHook
    template<typename... TArgs>
    static void RemoveHook(TArgs&&... args) noexcept {
        RemoveHook(HookInfo(std::forward<TArgs>(args)...));
    }
    /// @brief Stop tracking all hooks.
    static void RemoveHooks() noexcept;
    /// @brief Stop tracking all hooks at a certain offset.
    /// @param location The offset to check for any installed hooks.
    static void RemoveHooks(const void* const location) noexcept;
    /// @brief Combines all HookTrackers from all bs-hook libraries together.
    static void CombineHooks() noexcept;
    /// @brief Checks to see if there are any hooks installed at the offset provided.
    /// Returns true if at least one hook is installed, false otherwise.
    /// @param location The offset to check for.
    /// @returns Whether there exists at least one hook acting on this location.
    static bool IsHooked(const void* const location) noexcept;
    /// @brief Returns any hooks that access this location, or an empty list if there are none.
    /// @param location The offset to check for.
    /// @returns An std::list<HookInfo> of hooks.
    static const std::list<HookInfo> GetHooks(const void* const location) noexcept;
    /// @brief Returns all hooks.
    /// @returns The installed hooks.
    static const std::unordered_map<const void*, std::list<HookInfo>>* GetHooks() noexcept;
    /// @brief Returns the original location of a function that may or may not be hooked.
    /// If the function is not hooked, it returns the input.
    /// If the function is hooked, it returns the first installed hook's original location.
    /// @param location The offset to get the original function for.
    /// @returns The returned address.
    template<typename T>
    static const void* GetOrig(T location) noexcept {
        return GetOrigInternal(reinterpret_cast<void*>(location));
    }
    /// @brief Returns the original location of a function that may or may not be hooked, but confirms it with instruction parsing.
    /// If the function is not hooked, it returns the input.
    /// If the function is hooked (instruction check), it returns the first installed hooks original location.
    /// Note that this should only be called on functions that don't have matching instructions to hook installations, otherwise it may trace too far.
    /// @param location The offset to get the original function for.
    /// @returns The returned address.
    static const void* InstructionGetOrig(const void* const location) noexcept;
    /// @brief Checks to see if there is a hook installed (via instruction parsing) at the offset provided.
    /// Returns true if the first instructions at this location match the instructions created by a hook, implying a hook has been installed.
    /// Note that this should only be called on functions that don't have matching instructions to hook installations, otherwise it may false positive.
    /// @param location The offset to check for.
    /// @returns Whether there exists an instruction hook acting on this location.
    static bool InstructionIsHooked(const void* const location) noexcept;
    private:
    static std::unordered_map<const void*, std::list<HookInfo>> hooks;
    static const void* GetOrigInternal(const void* const) noexcept;
};
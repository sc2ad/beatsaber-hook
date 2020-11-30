#pragma once
#include <string_view>
#include <string>
#include <list>
#include <unordered_map>

/// @brief Stores information about an installed hook.
struct HookInfo {
    const std::string name;
    void* const destination;
    void* const trampoline;
    void* const orig;
    HookInfo(std::string_view name_, void* dst, void* src, void* orig_)
        : name(name_.data()), destination(dst), trampoline(src), orig(orig_) {}
    bool operator==(const HookInfo& other) const {
        return name == other.name && destination == other.destination && trampoline == other.trampoline && orig == other.orig;
    }
};

struct HookTracker {
    /// @brief Adds a HookInfo to be tracked.
    /// @param info The HookInfo to track.
    static void AddHook(HookInfo info);
    /// @brief Calls AddHook
    template<typename... TArgs>
    static void AddHook(TArgs&&... args) {
        AddHook(HookInfo(std::forward<TArgs>(args)...));
    }
    /// @brief Stops tracking the provided HookInfo.
    /// @param info The HookInfo to stop tracking.
    static void RemoveHook(HookInfo info);
    /// @brief Calls RemoveHook
    template<typename... TArgs>
    static void RemoveHook(TArgs&&... args) {
        RemoveHook(HookInfo(std::forward<TArgs>(args)...));
    }
    /// @brief Stop tracking all hooks.
    static void RemoveHooks();
    /// @brief Stop tracking all hooks at a certain offset.
    /// @param location The offset to check for any installed hooks.
    static void RemoveHooks(void* location);
    /// @brief Checks to see if there are any hooks installed at the offset provided.
    /// Returns true if at least one hook is installed, false otherwise.
    /// @param location The offset to check for.
    /// @returns Whether there exists at least one hook acting on this location.
    static bool IsHooked(void* location);
    /// @brief Returns any hooks that access this location, or an empty list if there are none.
    /// @param location The offset to check for.
    /// @returns An std::list<HookInfo> of hooks.
    static const std::list<HookInfo> GetHooks(void* location);
    private:
    static std::unordered_map<void*, std::list<HookInfo>> hooks;
};
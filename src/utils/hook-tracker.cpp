#include "../../shared/utils/hook-tracker.hpp"

std::unordered_map<void*, std::list<HookInfo>> HookTracker::hooks;

void HookTracker::AddHook(HookInfo info) {
    auto itr = hooks.find(info.destination);
    if (itr == hooks.end()) {
        hooks.emplace(info.destination, std::list<HookInfo>()).first->second.emplace_back(info);
    } else {
        itr->second.emplace_back(info);
    }
}

void HookTracker::RemoveHook(HookInfo info) {
    auto itr = hooks.find(info.destination);
    if (itr != hooks.end()) {
        for (auto& item : itr->second) {
            if (item == info) {
                itr->second.remove(item);
            }
        }
    }
}

void HookTracker::RemoveHooks() {
    hooks.clear();
}

void HookTracker::RemoveHooks(void* location) {
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        hooks.erase(itr);
    }
}

bool HookTracker::IsHooked(void* location) {
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        return itr->second.size() > 0;
    }
    return false;
}

const std::list<HookInfo> HookTracker::GetHooks(void* location) {
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        return itr->second;
    }
    return std::list<HookInfo>();
}
#include "../../shared/utils/hook-tracker.hpp"
#include "shared/utils/instruction-parsing.hpp"

std::unordered_map<const void*, std::list<HookInfo>> HookTracker::hooks;

void HookTracker::AddHook(HookInfo info) noexcept {
    auto itr = hooks.find(info.destination);
    if (itr == hooks.end()) {
        hooks.emplace(info.destination, std::list<HookInfo>()).first->second.emplace_back(info);
    } else {
        itr->second.emplace_back(info);
    }
}

void HookTracker::RemoveHook(HookInfo info) noexcept {
    auto itr = hooks.find(info.destination);
    if (itr != hooks.end()) {
        for (auto& item : itr->second) {
            if (item == info) {
                itr->second.remove(item);
            }
        }
    }
}

void HookTracker::RemoveHooks() noexcept {
    hooks.clear();
}

void HookTracker::RemoveHooks(const void* const location) noexcept {
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        hooks.erase(itr);
    }
}

bool HookTracker::IsHooked(const void* const location) noexcept {
    CombineHooks();
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        return itr->second.size() > 0;
    }
    return false;
}

const std::list<HookInfo> HookTracker::GetHooks(const void* const location) noexcept {
    CombineHooks();
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        return itr->second;
    }
    return std::list<HookInfo>();
}

const std::unordered_map<const void*, std::list<HookInfo>>* HookTracker::GetHooks() noexcept {
    return &hooks;
}

const void* HookTracker::GetOrigInternal(const void* const location) noexcept {
    CombineHooks();
    auto itr = hooks.find(location);
    if (itr != hooks.end() && itr->second.size() > 0) {
        return itr->second.front().orig;
    }
    return location;
}

#ifndef LIBS_FILE_PATH
#define LIBS_FILE_PATH "/sdcard/Android/data/%s/files/libs"
#endif

#define MOD_TEMP_PATH_FMT "/data/data/%s/"

#include <dirent.h>

void HookTracker::CombineHooks() noexcept {
    static auto logger = Logger::get().WithContext("HookTracker");
    auto libsFolder = string_format(LIBS_FILE_PATH, Modloader::getApplicationId().c_str());
    auto tmpPath = string_format(MOD_TEMP_PATH_FMT, Modloader::getApplicationId().c_str());
    DIR* dir = opendir(libsFolder.c_str());
    if (dir == nullptr) {
        logger.warning("Failed to open libs folder! At path: %s", libsFolder.c_str());
        return;
    }
    struct dirent* dp;
    while ((dp = readdir(dir)) != NULL) {
        if (std::string(dp->d_name).starts_with("libbeatsaber-hook")) {
            // Find and combine
            auto path = (tmpPath + dp->d_name);
            auto* image = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
            auto* err = dlerror();
            if (image == nullptr || err != nullptr) {
                logger.warning("Failed to dlopen: %s! %s", path.c_str(), err);
                continue;
            }
            // Open the library, look for a function called: __HOOKTRACKER_GET_HOOKS
            auto* getter = dlsym(image, "__HOOKTRACKER_GET_HOOKS");
            if (getter == nullptr) {
                logger.warning("Failed to find symbol: %s", "__HOOKTRACKER_GET_HOOKS");
                continue;
            }
            // Of course, if the function returns something that is of a different HookInfo type, for example, this may cause all sorts of pain.
            auto otherHooks = *reinterpret_cast<const std::unordered_map<const void*, std::list<HookInfo>>*(*)()>(getter)();
            for (auto itr : otherHooks) {
                // For each void*, find our match
                auto match = hooks.find(itr.first);
                if (match == hooks.end()) {
                    hooks.insert({itr.first, itr.second});
                } else {
                    // Add only unique items
                    for (auto item : itr.second) {
                        bool unique = true;
                        for (auto& existing : match->second) {
                            if (existing == item) {
                                unique = false;
                                break;
                            }
                        }
                        if (unique) {
                            match->second.push_back(item);
                        }
                    }
                }
            }
            dlclose(image);
        }
    }
}

extern "C" const void* __HOOKTRACKER_GET_HOOKS() {
    return reinterpret_cast<const void*>(HookTracker::GetHooks());
}

// Represents the number of instructions to search for the original (BLR)
#ifndef HOOKTRACKER_ORIG_SEARCH_COUNT
#define HOOKTRACKER_ORIG_SEARCH_COUNT 500
#endif

// TODO: Implement and use this function
const void* getOrigHelper(const void* const location, bool inTrampoline = false) noexcept {
    // If we are a hook, we can't find our orig here, we must travel to our hook implementation.
    Instruction inst(reinterpret_cast<const int32_t*>(location));
    if (inst.isNOP()) {
        inst = Instruction(reinterpret_cast<const int32_t*>(location) + 1);
    }
    if (inst.isLoad() && inst.size == 8 && inst.label == inst.addr + 2) {
        // For hooks, label will always be 8 past address of function.
        // This label points to the trampoline.
        // However, this isn't necessarily all we need to do, since we could have MULTIPLE functions hooking this.
        // So, we will need to recurse here as well.
        return getOrigHelper(reinterpret_cast<const void*>(*inst.label), true);
    }
    if (!inTrampoline) {
        // If we are not in a trampoline and we are not hooked, we simply exit.
        return location;
    }
    // Look for all BLs in this function, if we find any, trace their remaining asm to see if they match the remaining instructions from our location
    // We shall assume for the sake of simplicity that the original function is going to be called within 100 instructions
    for (int i = 0; i < HOOKTRACKER_ORIG_SEARCH_COUNT; i++) {
        if (inst.branchType == Instruction::BranchType::INDCALL) {
            // Source register needs to be emulated/traced
            // It should be from a LDR from a static field
            // inst.Rs[0]
        }
    }
    // We can't find the orig function, but we know we are in a trampoline. Chances are the orig is not called or this function is too big.

    return location;
}

const void* HookTracker::InstructionGetOrig(const void* const location) noexcept {
    Instruction inst(reinterpret_cast<const int32_t*>(location));
    if (inst.isNOP()) {
        inst = Instruction(reinterpret_cast<const int32_t*>(location) + 1);
    }
    if (inst.isLoad() && inst.size == 8 && inst.label == inst.addr + 2) {
        // For hooks, label will always be 8 past address of function.
        // This label points to the trampoline.
        // However, this isn't necessarily all we need to do, since we could have MULTIPLE functions hooking this.
        // So, we will need to recurse here as well.
        return HookTracker::InstructionGetOrig(*reinterpret_cast<const void* const*>(*inst.label));
    }
    // If we aren't any of these things, we exit out.
    return location;
}

bool HookTracker::InstructionIsHooked(const void* const location) noexcept {
    // First actual instruction should be an LDR (literal) PC + 0x8
    Instruction inst(reinterpret_cast<const int32_t*>(location));
    if (inst.isNOP()) {
        inst = Instruction(reinterpret_cast<const int32_t*>(location) + 1);
    }
    return inst.isLoad() && inst.size == 8 && inst.label == inst.addr + 2;
    // // The third instruction should be a BR x17
    // Instruction instr(reinterpret_cast<const int32_t *>(location) + 2);
    // return instr.isIndirectBranch() && instr.numSourceRegisters == 1 && instr.Rs[0] == 17;
}
#include "shared/utils/gc-alloc.hpp"
#include "shared/utils/il2cpp-functions.hpp"
#include "shared/utils/logging.hpp"
#include "shared/utils/utils.h"

[[nodiscard]] void* gc_alloc_specific(size_t sz) {
    // This function assumes il2cpp_functions will be called at a reasonable time, instead will warn you on allocating unsafe memory.
    if (il2cpp_functions::GarbageCollector_AllocateFixed && il2cpp_functions::GC_free) {
        // We should absolutely panic if we thought we had the allocation function, but it gave us null.
        auto* ptr = CRASH_UNLESS(il2cpp_functions::GarbageCollector_AllocateFixed(sz, nullptr));
        return ptr;
    } else {
        auto* ptr = calloc(1, sz);
        // We cannot use our logger because we allocate it using this function.
        __android_log_print(Logging::WARNING, "QuestHook[GC_Alloc]", "Allocation at: %p for size: %lu fallback to calloc!", ptr, sz);
        return ptr;
    }
}

[[nodiscard]] void* gc_realloc_specific(void* ptr,size_t new_size) {
    auto* nPtr = gc_alloc_specific(new_size);
    memcpy(nPtr, ptr, new_size);
    gc_free_specific(ptr);
    return nPtr;
}

void gc_free_specific(void* ptr) noexcept {
    if (il2cpp_functions::GarbageCollector_AllocateFixed && il2cpp_functions::GC_free) {
        il2cpp_functions::GC_free(ptr);
    }
    // TODO: Also check if a GC free would have been valid?
    else {
        free(ptr);
    }
}
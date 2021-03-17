#include "shared/utils/gc-alloc.hpp"
#include "shared/utils/il2cpp-functions.hpp"
#include "shared/utils/logging.hpp"

[[nodiscard]] void* gc_alloc_specific(std::size_t sz) {
    // This function assumes il2cpp_functions will be called at a reasonable time, instead will warn you on allocating unsafe memory.
    if (il2cpp_functions::GarbageCollector_AllocateFixed) {
        return il2cpp_functions::GarbageCollector_AllocateFixed(sz, nullptr);
    } else {
        auto* ptr = calloc(1, sz);
        Logger::get().warning("Allocation at: %p for size: %u MAY be overwritten by GC!", ptr, sz);
        return ptr;
    }
}

void gc_free_specific(void* ptr) {
    // Note that an instance allocated via GC_AllocateFixed should ALSO be freed via GC_free, if not, strange things may happen
    // (Including segfaults) Thus, note that the free call in the else block may throw, or may cause future throwing when GC cleans it up itself.
    if (il2cpp_functions::GC_free) {
        il2cpp_functions::GC_free(ptr);
    } else {
        free(ptr);
    }
}

[[nodiscard]] void* __attribute__((noinline)) operator new(std::size_t sz) {
    return gc_alloc_specific(sz);
}

void __attribute__((noinline)) operator delete(void* ptr) noexcept {
    gc_free_specific(ptr);
}
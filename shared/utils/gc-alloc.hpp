#pragma once
#include <stddef.h>

// Okay, so because we know that GC isn't overwriting heap (since it just calls the same shared calloc/malloc impls)
// we are confident that a new operator here isn't necessary or useful.
// We can safely say that the heap is shared properly, abeit without references.
// If references ARE desired, use one of the gc allocation functions in here expicitly.

/// @brief Returns an allocated instance of the provided size that will not be written over by future GC allocations and holds references.
/// You MUST use the gc_free_specific function defined here to destroy it.
/// This function fallsback to calloc if no GC_Alloc or GC_Free implementations are found via xref/sigscan.
/// @param sz The size to allocate an instance of.
/// @return The allocated instance.
[[nodiscard]] void* gc_alloc_specific(size_t sz);

/// @brief Deletes the provided allocated instance from the gc_alloc_specific function defined here.
/// Other pointers will cause undefined behavior.
/// This function will call GC_free if there is both a GC_Alloc and GC_Free implementation available, free otherwise.
/// @param sz The pointer to free explicitly.
/// @return The allocated instance.
void gc_free_specific(void* ptr) noexcept;

/// @brief Reallocation implementation is equivalent to: alloc + free
/// @param ptr The pointer to resize.
/// @param new_size The new size of the memory.
/// @return The resized instance.
[[nodiscard]] void* gc_realloc_specific(void* ptr, size_t new_size);

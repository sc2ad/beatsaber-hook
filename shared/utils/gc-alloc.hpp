#pragma once

#include <new>

/// @brief Identical to operator new, yet callable explicitly.
/// @param sz The size to allocate an instance of.
/// @return The allocated instance.
[[nodiscard]] void* gc_alloc_specific(std::size_t sz);

/// @brief Identical to operator delete yet callable explicitly.
/// @param sz The pointer to free explicitly.
/// @return The allocated instance.
void gc_free_specific(void* ptr) noexcept;

/// @brief Returns an allocated instance of the provided size that will not be written over by future GC allocations.
/// You MUST use the delete operator defined here to destroy it.
/// @param sz The size to allocate an instance of.
/// @return The allocated instance.
[[nodiscard]] void* __attribute__((noinline)) operator new(std::size_t sz);

/// @brief Deletes the provided allocated instance from the new operator defined here.
/// Other pointers will cause undefined behavior.
/// @param ptr The pointer to an instance to destroy.
void __attribute__((noinline)) operator delete(void* ptr) noexcept;


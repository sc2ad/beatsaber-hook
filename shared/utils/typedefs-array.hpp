#pragma once
#include <optional>
#include <vector>
#include <span>

template<class T, class U>
/// @brief If type T can be assigned to by type U&&
/// @tparam T The left hand side of the assignment
/// @tparam U The right hand side of the assignment
concept has_assignment = requires (T arg, U&& other) {
    arg = other;
};

template<class T>
requires (!std::is_reference_v<T>)
/// @brief A std::wrapper_reference implementation that perfect forwards to contained assignment operators.
struct WrapperRef {
    /// @brief Explicitly create a wrapper reference from a reference who MUST LIVE LONGER THAN THIS INSTANCE!
    explicit constexpr WrapperRef(T& instance) : ptr(std::addressof(instance)) {}
    // Assignment for any U type that is valid to be assigned.
    template<class U>
    requires (has_assignment<T, U>)
    WrapperRef& operator=(U&& other) {
        *ptr = other;
        return *this;
    }
    // Standard Assignment
    WrapperRef& operator=(const WrapperRef& x) noexcept = default;
    // Getter operation, implicit conversion
    constexpr operator T& () const noexcept {
        return *ptr;
    }
    // Getter operation, explicit conversion
    constexpr T& get() const noexcept {
        return *ptr;
    }
    // Invoke operation
    template<class... ArgTypes>
    constexpr std::invoke_result_t<T&, ArgTypes...> operator() (ArgTypes&&... args) const {
        return std::invoke(get(), std::forward<ArgTypes>(args)...);
    }
    private:
    T* ptr;
};

#pragma pack(push)

#ifdef HAS_CODEGEN
typedef int32_t il2cpp_array_lower_bound_t;
#define IL2CPP_ARRAY_MAX_INDEX ((int32_t) 0x7fffffff)
#define IL2CPP_ARRAY_MAX_SIZE  ((uint32_t) 0xffffffff)

typedef struct Il2CppArrayBounds
{
    il2cpp_array_size_t length;
    il2cpp_array_lower_bound_t lower_bound;
} Il2CppArrayBounds;

#if IL2CPP_COMPILER_MSVC
#pragma warning( push )
#pragma warning( disable : 4200 )
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

#include "System/Array.hpp"
struct Il2CppArray : public System::Array {
    /* bounds is NULL for szarrays */
    Il2CppArrayBounds *bounds;
    /* total number of elements of the array */
    il2cpp_array_size_t max_length;
};

struct Il2CppArraySize : public Il2CppArray {
    ALIGN_TYPE(8) void* vector[IL2CPP_ZERO_LEN_ARRAY];
};

static const size_t kIl2CppSizeOfArray = offsetof(Il2CppArraySize, vector);
static const size_t kIl2CppOffsetOfArrayBounds = offsetof(Il2CppArray, bounds);
static const size_t kIl2CppOffsetOfArrayLength = offsetof(Il2CppArray, max_length);
#endif

#include "utils.h"
#include "il2cpp-utils-methods.hpp"
#include <initializer_list>

#ifdef HAS_CODEGEN
#include "System/Collections/Generic/IReadOnlyList_1.hpp"
#include "System/Collections/Generic/IList_1.hpp"
template<class T>
struct Array : public Il2CppArray, public System::Collections::Generic::IReadOnlyList_1<T>,
  public System::Collections::Generic::IList_1<T>
#else
template<class T>
struct Array : public Il2CppArray
#endif
{
    static_assert(is_value_type_v<T>, "T must be a C# value type! (primitive, pointer or Struct)");
    ALIGN_TYPE(8) T values[IL2CPP_ZERO_LEN_ARRAY];

    inline il2cpp_array_size_t Length() const {
        if (bounds) {
            return bounds->length;
        }
        return max_length;
    }
    T& operator[](size_t i) {
        return values[i];
    }
    const T& operator[](size_t i) const {
        return values[i];
    }

    /// @brief Get a given index, performs bound checking and throws std::runtime_error on failure.
    /// @param i The index to get.
    /// @return The reference to the item.
    T& get(size_t i) {
        THROW_UNLESS(i < Length() && i >= 0);
        return values[i];
    }
    /// @brief Get a given index, performs bound checking and throws std::runtime_error on failure.
    /// @param i The index to get.
    /// @return The const reference to the item.
    const T& get(size_t i) const {
        THROW_UNLESS(i < Length() && i >= 0);
        return values[i];
    }
    /// @brief Tries to get a given index, performs bound checking and returns a std::nullopt on failure.
    /// @param i The index to get.
    /// @return The WrapperRef<T> to the item, mostly considered to be a T&.
    std::optional<WrapperRef<T>> try_get(size_t i) {
        if (i >= Length() || i < 0) {
            return std::nullopt;
        }
        return WrapperRef(values[i]);
    }
    /// @brief Tries to get a given index, performs bound checking and returns a std::nullopt on failure.
    /// @param i The index to get.
    /// @return The WrapperRef<const T> to the item, mostly considered to be a const T&.
    std::optional<WrapperRef<const T>> try_get(size_t i) const {
        if (i >= Length() || i < 0) {
            return std::nullopt;
        }
        return WrapperRef(values[i]);
    }

    static Array<T>* New(std::initializer_list<T> vals) {
        il2cpp_functions::Init();
        auto* arr = reinterpret_cast<Array<T>*>(il2cpp_functions::array_new(
            il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<T>::get(), vals.size()));
        memcpy(arr->values, vals.begin(), sizeof(T)*vals.size());
        return arr;
    }

    static Array<T>* NewLength(il2cpp_array_size_t size) {
        il2cpp_functions::Init();
        return reinterpret_cast<Array<T>*>(il2cpp_functions::array_new(
            il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<T>::get(), size));
    }

    template<typename... TArgs>
    static Array<T>* New(TArgs&&... args) {
        return New({args...});
    }

  #ifdef HAS_CODEGEN
    System::Collections::Generic::IEnumerator_1<T>* GetEnumerator() {
  #else
    Il2CppObject* GetEnumerator() {
  #endif
        static auto* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe(
            this, "System.Collections.Generic.IEnumerable`1.GetEnumerator", 0));
      #ifdef HAS_CODEGEN
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe<System::Collections::Generic::IEnumerator_1<T>*>(
      #else
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe(
      #endif
            this, method));
    }

    bool Contains(T item) {
        // TODO: find a better way around the existence of 2 methods with this name (the 2nd not being generic at all)
        static auto* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe(
            this, "System.Collections.Generic.ICollection`1.Contains", 1));
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe<bool>(this, method, item));
    }
    void CopyTo(::Array<T>* array, int arrayIndex) {
        static auto* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe(
            this, "System.Collections.Generic.ICollection`1.CopyTo", 2));
        CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe(this, method, array, arrayIndex));
    }
    int IndexOf(T item) {
        static auto* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe(this, "System.Collections.Generic.IList`1.IndexOf", 1));
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe<int>(this, method, item));
    }
    /// @brief Copies the array to the provided vector reference of same type.
    /// @param vec The vector to copy to.
    void copy_to(std::vector<T>& vec) const {
        vec.assign(values, values + Length());
    }
    /// @brief Provides a reference span of the held data within this array. The span should NOT outlive this instance.
    /// @return The created span.
    std::span<T> ref_to() {
        return std::span(values, Length());
    }
    /// @brief Provides a reference span of the held data within this array. The span should NOT outlive this instance.
    /// @return The created span.
    const std::span<T> ref_to() const {
        return std::span(values, Length());
    }
};

#pragma pack(pop)
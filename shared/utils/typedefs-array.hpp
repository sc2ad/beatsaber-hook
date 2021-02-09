#pragma once

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

    il2cpp_array_size_t Length() {
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
};

#pragma pack(pop)
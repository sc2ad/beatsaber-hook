#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
#include <initializer_list>

// from https://gcc.gnu.org/bugzilla//show_bug.cgi?id=71579#c4, leading underscores removed
namespace std {
    template <class _Tp>
    struct is_complete_impl
    {
        template <class _Up, size_t = sizeof(_Up)>
        static true_type _S_test(int);

        template <class _Up>
        static false_type _S_test(...);

        using type = decltype(_S_test<_Tp>(0));
    };

    template<typename _Tp>
    using is_complete = typename is_complete_impl<_Tp>::type;

    // my own (trivial) addition
    template<typename _Tp>
    constexpr bool is_complete_v = is_complete<_Tp>::value;
}

#include <cassert>
// For including il2cpp properly
#ifdef _MSC_VER
#undef _MSC_VER
#endif
#ifndef __GNUC__
#define __GNUC__
#endif

#define NET_4_0 true
#include "il2cpp-config.h"
#include "il2cpp-api-types.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-tabledefs.h"

#ifdef __cplusplus
template<class T, class Enable = void>
struct is_value_type : std::integral_constant< 
    bool,
    (std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pointer_v<T> || std::is_standard_layout_v<T>) && !std::is_base_of_v<Il2CppObject, T>
> {};
template<class _T> constexpr bool is_value_type_v = is_value_type<_T>::value;

template<class T>
struct Array;

#endif /* __cplusplus */

#include "il2cpp-functions.hpp"
#include "il2cpp-utils-methods.hpp"
#include "il2cpp-type-check.hpp"

#if __has_include("System/Array.hpp") && !defined(NO_CODEGEN_USE)
#define HAS_CODEGEN
#include <stdint.h>
#include <stddef.h>
#include "il2cpp-windowsruntime-types.h"
#else
// TODO: find a way to include this without putting the types in the global namespace?
#include "il2cpp-object-internals.h"
#endif

struct DelegateData;
// See il2cpp-object-internals.h/Il2CppDelegate
// System.Delegate
typedef struct Delegate : Il2CppObject {
    Il2CppMethodPointer method_ptr; // 0x8
    InvokerMethod invoke_impl; // 0xC
    Il2CppObject* m_target; // 0x10
    void* method; // 0x14
    void* delegate_trampoline; // 0x18
    intptr_t extra_arg; // 0x1C

    /*
    * If non-NULL, this points to a memory location which stores the address of
    * the compiled code of the method, or NULL if it is not yet compiled.
    */
    uint8_t** method_code; // 0x20
    Il2CppReflectionMethod* method_info; // 0x24
    Il2CppReflectionMethod* original_method_info; // 0x28
    DelegateData* data; // 0x2C
    bool method_is_virtual; // 0x30
} Delegate;

// System.MulticastDelegate
typedef struct MulticastDelegate : Delegate {
    ::Array<Delegate*>* delegates;
} MulticastDelegate;

namespace il2cpp_utils {
    namespace array_utils {
        static char* il2cpp_array_addr_with_size(Il2CppArray *array, int32_t size, uintptr_t idx)
        {
            return ((char*)array) + kIl2CppSizeOfArray + size * idx;
        }
        #define load_array_elema(arr, idx, size) ((((uint8_t*)(arr)) + kIl2CppSizeOfArray) + ((size) * (idx)))

        #define il2cpp_array_setwithsize(array, elementSize, index, value)  \
            do {    \
                void*__p = (void*) il2cpp_utils::array_utils::il2cpp_array_addr_with_size ((array), elementSize, (index)); \
                memcpy(__p, &(value), elementSize); \
            } while (0)
        #define il2cpp_array_setrefwithsize(array, elementSize, index, value)  \
            do {    \
                void*__p = (void*) il2cpp_utils::array_utils::il2cpp_array_addr_with_size ((array), elementSize, (index)); \
                memcpy(__p, value, elementSize); \
                } while (0)
        #define il2cpp_array_addr(array, type, index) ((type*)(void*) il2cpp_utils::array_utils::il2cpp_array_addr_with_size (array, sizeof (type), index))
        #define il2cpp_array_get(array, type, index) ( *(type*)il2cpp_array_addr ((array), type, (index)) )
        #define il2cpp_array_set(array, type, index, value)    \
            do {    \
                type *__p = (type *) il2cpp_array_addr ((array), type, (index));    \
                *__p = (value); \
            } while (0)
        #define il2cpp_array_setref(array, index, value)  \
            do {    \
                void* *__p = (void* *) il2cpp_array_addr ((array), void*, (index)); \
                /* il2cpp_gc_wbarrier_set_arrayref ((array), __p, (MonoObject*)(value));    */\
                *__p = (value);    \
            } while (0)
    }
}

// System.DelegateData
typedef struct DelegateData : Il2CppObject {
    Il2CppReflectionType* target_type;
    Il2CppString* method_name;
    bool curied_first_arg;
} DelegateData;

template<class T>
struct Array : public Il2CppArray
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

    template<typename Q = Il2CppObject>
    Q* GetEnumerator() {
        static auto* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe(
            this, "System.Collections.Generic.IEnumerable`1.GetEnumerator", 0));
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe<Q*>(
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
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe(this, method, array, arrayIndex));
    }
    int IndexOf(T item) {
        static auto* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe(this, "System.Collections.Generic.IList`1.IndexOf", 1));
        return CRASH_UNLESS(il2cpp_utils::RunMethodUnsafe<int>(this, method, item));
    }
};

#ifdef HAS_CODEGEN
// TODO: Fix
// #include "System/Collections/Generic/List_1.hpp"
// template<class T>
// using List = System::Collections::Generic::List_1<T>;
#else
// System.Collections.Generic.List
template<class T>
struct List : Il2CppObject
{
    Array<T>* items;
    int size;
    int version;
    Il2CppObject* syncRoot;
};
#endif

#ifdef HAS_CODEGEN
// TODO: QiCache and Il2CppComObject ("System.__Il2CppComObject (dummy type that replaces System.__ComObject)")

// #include "System/AppDomain.hpp"
// // self-typedef'd in il2cpp-class-internals.h
// struct Il2CppAppDomain : public System::AppDomain {};

// #include "System/AppDomainSetup.hpp"
// // self-typedef'd in il2cpp-class-internals.h
// struct Il2CppAppDomainSetup : public System::AppDomainSetup {};

// #include "System/ArgumentException.hpp"
// typedef System::ArgumentException Il2CppArgumentException;

// // TODO: Il2CppDecimal is System::Decimal?

// typedef enum Il2CppDecimalCompareResult
// {
//     IL2CPP_DECIMAL_CMP_LT = -1,
//     IL2CPP_DECIMAL_CMP_EQ,
//     IL2CPP_DECIMAL_CMP_GT
// } Il2CppDecimalCompareResult;

// // TODO: Il2CppDouble, Il2CppDouble_double are System::Double?

// #include "System/Exception.hpp"
// // self-typedef'd in il2cpp-api-types.h
// struct Il2CppException : public System::Exception {};

// // TODO: Il2CppExceptionWrapper?

// #include "System/IOAsyncResult.hpp"
// typedef System::IOAsyncResult Il2CppIOAsyncResult;

// #include "System/IOSelectorJob.hpp"
// typedef System::IOSelectorJob Il2CppIOSelectorJob;

// #include "System/MarshalByRefObject.hpp"
// typedef System::MarshalByRefObject Il2CppMarshalByRefObject;

// #include "System/MonoAsyncCall.hpp"
// typedef System::MonoAsyncCall Il2CppAsyncCall;

// #include "System/MonoType.hpp"
// struct Il2CppReflectionMonoType : public System::MonoType {
//     const Il2CppType* GetIl2CppType() const {
//         return reinterpret_cast<Il2CppType*>(impl.value.m_value);
//     }
// };

// #include "System/RuntimeType.hpp"
// struct Il2CppReflectionRuntimeType : public System::RuntimeType {};

// // TODO: Il2CppSingle, Il2CppSingle_float are System::Single?

// #include "System/SystemException.hpp"
// typedef System::SystemException Il2CppSystemException;

// #include "System/Type.hpp"
// // self-typedef'd in il2cpp-api-types.h
// struct Il2CppReflectionType : public System::Type {};

// #include "System/TypedReference.hpp"
// typedef System::TypedReference Il2CppTypedRef;

// #include "System/Diagnostics/StackFrame.hpp"
// typedef System::Diagnostics::StackFrame Il2CppStackFrame;

// // TODO: Il2CppCalendarData is System::Globalization::CalendarData minus 4 fields at the end?

// // TODO: Il2CppCultureData is System::Globalization::CultureData minus 13 fields at the end?

// #include "System/Globalization/CultureInfo.hpp"
// typedef System::Globalization::CultureInfo Il2CppCultureInfo;

// #include "System/Globalization/DateTimeFormatInfo.hpp"
// typedef System::Globalization::DateTimeFormatInfo Il2CppDateTimeFormatInfo;

// #include "System/Globalization/NumberFormatInfo.hpp"
// typedef System::Globalization::NumberFormatInfo Il2CppNumberFormatInfo;

// #include "System/Globalization/RegionInfo.hpp"
// typedef System::Globalization::RegionInfo Il2CppRegionInfo;

// #include "System/Globalization/SortKey.hpp"
// typedef System::Globalization::SortKey Il2CppSortKey;

// #include "System/Net/SocketAddress.hpp"
// typedef System::Net::SocketAddress Il2CppSocketAddress;

// // "Corresponds to Mono's internal System.Net.Sockets.Socket.SocketAsyncResult class. Has no relation to Il2CppAsyncResult."
// #include "System/Net/Sockets/SocketAsyncResult.hpp"
// typedef System::Net::Sockets::SocketAsyncResult Il2CppSocketAsyncResult;

// #include "System/Reflection/EventInfo.hpp"
// typedef System::Reflection::EventInfo Il2CppReflectionEvent;

// #include "System/Reflection/MonoEvent.hpp"
// typedef System::Reflection::MonoEvent Il2CppReflectionMonoEvent;

// #include "System/Reflection/MonoEventInfo.hpp"
// typedef System::Reflection::MonoEventInfo Il2CppReflectionMonoEventInfo;

// #include "System/Reflection/MonoField.hpp"
// typedef System::Reflection::MonoField Il2CppReflectionField;

// #include "System/Reflection/MonoProperty.hpp"
// typedef System::Reflection::MonoProperty Il2CppReflectionProperty;

// #include "System/Reflection/MonoMethod.hpp"
// // self-typedef'd in il2cpp-api-types.h
// struct Il2CppReflectionMethod : public System::Reflection::MonoMethod {};

// #if __has_include("System/Reflection/MonoGenericMethod.hpp")
// #include "System/Reflection/MonoGenericMethod.hpp"
// typedef System::Reflection::MonoGenericMethod Il2CppReflectionGenericMethod;
// #endif

// #include "System/Reflection/MonoMethodInfo.hpp"
// typedef System::Reflection::MonoMethodInfo Il2CppMethodInfo;

// #include "System/Reflection/MonoPropertyInfo.hpp"
// typedef System::Reflection::MonoPropertyInfo Il2CppPropertyInfo;

// #include "System/Reflection/ParameterInfo.hpp"
// typedef System::Reflection::ParameterInfo Il2CppReflectionParameter;

// #include "System/Reflection/Module.hpp"
// typedef System::Reflection::Module Il2CppReflectionModule;

// #include "System/Reflection/AssemblyName.hpp"
// typedef System::Reflection::AssemblyName Il2CppReflectionAssemblyName;

// #include "System/Reflection/Assembly.hpp"
// typedef System::Reflection::Assembly Il2CppReflectionAssembly;

// #include "System/Reflection/Emit/UnmanagedMarshal.hpp"
// typedef System::Reflection::Emit::UnmanagedMarshal Il2CppReflectionMarshal;

// #include "System/Reflection/ManifestResourceInfo.hpp"
// typedef System::Reflection::ManifestResourceInfo Il2CppManifestResourceInfo;

// #include "System/Reflection/Pointer.hpp"
// typedef System::Reflection::Pointer Il2CppReflectionPointer;

// // TODO: Il2CppResourceLocation seems to be the System.Reflection.ResourceLocation enum

// #include "System/Runtime/InteropServices/ErrorWrapper.hpp"
// typedef System::Runtime::InteropServices::ErrorWrapper Il2CppErrorWrapper;

// // "Inherited by Microsoft.Win32.SafeHandles.SafeWaitHandle"
// #include "System/Runtime/InteropServices/SafeHandle.hpp"
// typedef System::Runtime::InteropServices::SafeHandle Il2CppSafeHandle;

// #include "System/Runtime/Remoting/Contexts/Context.hpp"
// // self-typedef'd in il2cpp-class-internals.h
// struct Il2CppAppContext : public System::Runtime::Remoting::Contexts::Context {};

// #include "System/Runtime/Remoting/Messaging/AsyncResult.hpp"
// // self-typedef'd in il2cpp-api-types.h
// struct Il2CppAsyncResult : public System::Runtime::Remoting::Messaging::AsyncResult {};

// // TODO: Il2CppCallType which "is a copy of System.Runtime.Remoting.Messaging.CallType" enum

// // TODO: Il2CppMethodMessage is System::Runtime::Remoting::Messaging::MonoMethodMessage minus 4 fields at the end?

// #include "System/Text/StringBuilder.hpp"
// typedef System::Text::StringBuilder Il2CppStringBuilder;

// #include "System/Threading/InternalThread.hpp"
// typedef System::Threading::InternalThread Il2CppInternalThread;

// #include "System/Threading/Thread.hpp"
// // self-typedef'd in il2cpp-api-types.h
// struct Il2CppThread : public System::Threading::Thread {};

#endif
// From Runtime.cpp (some may need the * removed):
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppMulticastDelegate*, multicastdelegate);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppAsyncCall*, async_call);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppInternalThread*, internal_thread);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionEvent*, event_info);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppStringBuilder*, stringbuilder);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppStackFrame*, stack_frame);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionAssemblyName*, assembly_name);
// DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionAssembly*, assembly);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionAssembly*, mono_assembly);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionField*, mono_field);
// DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionParameter*, parameter_info);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionParameter*, mono_parameter_info);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionModule*, module);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionPointer*, pointer);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppSystemException*, system_exception);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppArgumentException*, argument_exception);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppMarshalByRefObject*, marshalbyrefobject);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppSafeHandle*, safe_handle);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppSortKey*, sort_key);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppErrorWrapper*, error_wrapper);
// TODO: attempt to move out of this conditional if codegen ever gets an Il2CppComObject?
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppComObject*, il2cpp_com_object);

DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppDelegate*, delegate);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionMonoType*, monotype);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppThread*, thread);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionRuntimeType*, runtimetype);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionMonoEventInfo*, mono_event_info);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppTypedRef*, typed_reference);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionMethod*, mono_method);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppMethodInfo*, mono_method_info);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppPropertyInfo*, mono_property_info);
DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppException*, exception);

#include "utils/Il2CppHashMap.h"
#include "utils/HashUtils.h"
#include "utils/StringUtils.h"

struct NamespaceAndNamePairHash
{
    size_t operator()(const std::pair<const char*, const char*>& pair) const
    {
        return il2cpp::utils::HashUtils::Combine(il2cpp::utils::StringUtils::Hash(pair.first), il2cpp::utils::StringUtils::Hash(pair.second));
    }
};

struct NamespaceAndNamePairEquals
{
    bool operator()(const std::pair<const char*, const char*>& p1, const std::pair<const char*, const char*>& p2) const
    {
        return !strcmp(p1.first, p2.first) && !strcmp(p1.second, p2.second);
    }
};

struct Il2CppNameToTypeDefinitionIndexHashTable : public Il2CppHashMap<std::pair<const char*, const char*>, TypeDefinitionIndex, NamespaceAndNamePairHash, NamespaceAndNamePairEquals>
{
    typedef Il2CppHashMap<std::pair<const char*, const char*>, TypeDefinitionIndex, NamespaceAndNamePairHash, NamespaceAndNamePairEquals> Base;
    Il2CppNameToTypeDefinitionIndexHashTable() : Base()
    {
    }
};

#endif /* TYPEDEFS_H */

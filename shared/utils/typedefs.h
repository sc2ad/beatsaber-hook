#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
#include <initializer_list>

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

template<class T>
struct Array;

extern "C" {
#endif

// UNITY SPECIFIC

// UnityEngine.Color
typedef struct Color {
    float r;
    float g;
    float b;
    float a;
} Color;

// UnityEngine.Vector2
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

// UnityEngine.Vector3
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

// UnityEngine.Vector4
typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

// UnityEngine.Quaternion
typedef struct Quaternion {
    float x;
    float y;
    float z;
    float w;
} Quaternion;

// UnityEngine.Rect
typedef struct Rect {
    float m_XMin;
    float m_YMin;
    float m_Width;
    float m_Height;
} Rect;

// UnityEngine.Scene
typedef struct Scene {
    int m_Handle;
} Scene;

// C# SPECIFIC

// System.IntPtr
typedef struct IntPtr {
    void* value;
} IntPtr;

#ifdef __cplusplus
}  /* extern "C" */
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

#include "typedefs-object.hpp"
#include "typedefs-delegate.hpp"
#include "typedefs-array.hpp"

#include <stdint.h>

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

#ifdef HAS_CODEGEN
#include "System/String.hpp"
struct Il2CppString : public System::String {};
#endif

#ifdef HAS_CODEGEN
#include "System/Collections/Generic/List_1.hpp"
template<class T>
using List = System::Collections::Generic::List_1<T>;
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

#include "System/AppDomain.hpp"
// self-typedef'd in il2cpp-class-internals.h
struct Il2CppAppDomain : public System::AppDomain {};

#include "System/AppDomainSetup.hpp"
// self-typedef'd in il2cpp-class-internals.h
struct Il2CppAppDomainSetup : public System::AppDomainSetup {};

#include "System/ArgumentException.hpp"
typedef System::ArgumentException Il2CppArgumentException;

// TODO: Il2CppDecimal is System::Decimal?

typedef enum Il2CppDecimalCompareResult
{
    IL2CPP_DECIMAL_CMP_LT = -1,
    IL2CPP_DECIMAL_CMP_EQ,
    IL2CPP_DECIMAL_CMP_GT
} Il2CppDecimalCompareResult;

// TODO: Il2CppDouble, Il2CppDouble_double are System::Double?

#include "System/Exception.hpp"
// self-typedef'd in il2cpp-api-types.h
struct Il2CppException : public System::Exception {};

// TODO: Il2CppExceptionWrapper?

#include "System/IOAsyncResult.hpp"
typedef System::IOAsyncResult Il2CppIOAsyncResult;

#include "System/IOSelectorJob.hpp"
typedef System::IOSelectorJob Il2CppIOSelectorJob;

#include "System/MarshalByRefObject.hpp"
typedef System::MarshalByRefObject Il2CppMarshalByRefObject;

#include "System/MonoAsyncCall.hpp"
typedef System::MonoAsyncCall Il2CppAsyncCall;

#include "System/MonoType.hpp"
struct Il2CppReflectionMonoType : public System::MonoType {
    const Il2CppType* GetIl2CppType() const {
        return reinterpret_cast<Il2CppType*>(impl.value.m_value);
    }
};

#include "System/RuntimeType.hpp"
struct Il2CppReflectionRuntimeType : public System::RuntimeType {};

// TODO: Il2CppSingle, Il2CppSingle_float are System::Single?

#include "System/SystemException.hpp"
typedef System::SystemException Il2CppSystemException;

#include "System/Type.hpp"
// self-typedef'd in il2cpp-api-types.h
struct Il2CppReflectionType : public System::Type {};

#include "System/TypedReference.hpp"
typedef System::TypedReference Il2CppTypedRef;

#include "System/Diagnostics/StackFrame.hpp"
typedef System::Diagnostics::StackFrame Il2CppStackFrame;

// TODO: Il2CppCalendarData is System::Globalization::CalendarData minus 4 fields at the end?

// TODO: Il2CppCultureData is System::Globalization::CultureData minus 13 fields at the end?

#include "System/Globalization/CultureInfo.hpp"
typedef System::Globalization::CultureInfo Il2CppCultureInfo;

#include "System/Globalization/DateTimeFormatInfo.hpp"
typedef System::Globalization::DateTimeFormatInfo Il2CppDateTimeFormatInfo;

#include "System/Globalization/NumberFormatInfo.hpp"
typedef System::Globalization::NumberFormatInfo Il2CppNumberFormatInfo;

#include "System/Globalization/RegionInfo.hpp"
typedef System::Globalization::RegionInfo Il2CppRegionInfo;

#include "System/Globalization/SortKey.hpp"
typedef System::Globalization::SortKey Il2CppSortKey;

#include "System/Net/SocketAddress.hpp"
typedef System::Net::SocketAddress Il2CppSocketAddress;

// "Corresponds to Mono's internal System.Net.Sockets.Socket.SocketAsyncResult class. Has no relation to Il2CppAsyncResult."
#include "System/Net/Sockets/SocketAsyncResult.hpp"
typedef System::Net::Sockets::SocketAsyncResult Il2CppSocketAsyncResult;

#include "System/Reflection/EventInfo.hpp"
typedef System::Reflection::EventInfo Il2CppReflectionEvent;

#include "System/Reflection/MonoEvent.hpp"
typedef System::Reflection::MonoEvent Il2CppReflectionMonoEvent;

#include "System/Reflection/MonoEventInfo.hpp"
typedef System::Reflection::MonoEventInfo Il2CppReflectionMonoEventInfo;

#include "System/Reflection/MonoField.hpp"
typedef System::Reflection::MonoField Il2CppReflectionField;

#include "System/Reflection/MonoProperty.hpp"
typedef System::Reflection::MonoProperty Il2CppReflectionProperty;

#include "System/Reflection/MonoMethod.hpp"
// self-typedef'd in il2cpp-api-types.h
struct Il2CppReflectionMethod : public System::Reflection::MonoMethod {};

#if __has_include("System/Reflection/MonoGenericMethod.hpp")
#include "System/Reflection/MonoGenericMethod.hpp"
typedef System::Reflection::MonoGenericMethod Il2CppReflectionGenericMethod;
#endif

#include "System/Reflection/MonoMethodInfo.hpp"
typedef System::Reflection::MonoMethodInfo Il2CppMethodInfo;

#include "System/Reflection/MonoPropertyInfo.hpp"
typedef System::Reflection::MonoPropertyInfo Il2CppPropertyInfo;

#include "System/Reflection/ParameterInfo.hpp"
typedef System::Reflection::ParameterInfo Il2CppReflectionParameter;

#include "System/Reflection/Module.hpp"
typedef System::Reflection::Module Il2CppReflectionModule;

#include "System/Reflection/AssemblyName.hpp"
typedef System::Reflection::AssemblyName Il2CppReflectionAssemblyName;

#include "System/Reflection/Assembly.hpp"
typedef System::Reflection::Assembly Il2CppReflectionAssembly;

#include "System/Reflection/Emit/UnmanagedMarshal.hpp"
typedef System::Reflection::Emit::UnmanagedMarshal Il2CppReflectionMarshal;

#include "System/Reflection/ManifestResourceInfo.hpp"
typedef System::Reflection::ManifestResourceInfo Il2CppManifestResourceInfo;

#include "System/Reflection/Pointer.hpp"
typedef System::Reflection::Pointer Il2CppReflectionPointer;

// TODO: Il2CppResourceLocation seems to be the System.Reflection.ResourceLocation enum

#include "System/Runtime/InteropServices/ErrorWrapper.hpp"
typedef System::Runtime::InteropServices::ErrorWrapper Il2CppErrorWrapper;

// "Inherited by Microsoft.Win32.SafeHandles.SafeWaitHandle"
#include "System/Runtime/InteropServices/SafeHandle.hpp"
typedef System::Runtime::InteropServices::SafeHandle Il2CppSafeHandle;

#include "System/Runtime/Remoting/Contexts/Context.hpp"
// self-typedef'd in il2cpp-class-internals.h
struct Il2CppAppContext : public System::Runtime::Remoting::Contexts::Context {};

#include "System/Runtime/Remoting/Messaging/AsyncResult.hpp"
// self-typedef'd in il2cpp-api-types.h
struct Il2CppAsyncResult : public System::Runtime::Remoting::Messaging::AsyncResult {};

// TODO: Il2CppCallType which "is a copy of System.Runtime.Remoting.Messaging.CallType" enum

// TODO: Il2CppMethodMessage is System::Runtime::Remoting::Messaging::MonoMethodMessage minus 4 fields at the end?

#include "System/Text/StringBuilder.hpp"
typedef System::Text::StringBuilder Il2CppStringBuilder;

#include "System/Threading/InternalThread.hpp"
typedef System::Threading::InternalThread Il2CppInternalThread;

#include "System/Threading/Thread.hpp"
// self-typedef'd in il2cpp-api-types.h
struct Il2CppThread : public System::Threading::Thread {};

#else
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
#endif
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

DEFINE_IL2CPP_ARG_TYPE(long double, "System", "Decimal");
DEFINE_IL2CPP_ARG_TYPE(Color, "UnityEngine", "Color");
DEFINE_IL2CPP_ARG_TYPE(Vector2, "UnityEngine", "Vector2");
DEFINE_IL2CPP_ARG_TYPE(Vector3, "UnityEngine", "Vector3");
DEFINE_IL2CPP_ARG_TYPE(Vector4, "UnityEngine", "Vector4");
DEFINE_IL2CPP_ARG_TYPE(Quaternion, "UnityEngine", "Quaternion");
DEFINE_IL2CPP_ARG_TYPE(Rect, "UnityEngine", "Rect");
DEFINE_IL2CPP_ARG_TYPE(Scene, "UnityEngine.SceneManagement", "Scene");

template<typename TArg>
struct ::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<Array<TArg>*> {
    static inline Il2CppClass* get() {
        il2cpp_functions::Init();
        if constexpr (std::is_same_v<std::decay_t<TArg>, Il2CppObject*>) {
            il2cpp_functions::CheckS_GlobalMetadata();
            return il2cpp_functions::array_class_get(il2cpp_functions::defaults->object_class, 1);
        } else {
            static auto& logger = getLogger();
            Il2CppClass* eClass = RET_0_UNLESS(logger, il2cpp_no_arg_class<TArg>::get());
            return il2cpp_functions::array_class_get(eClass, 1);
        }
    }
};

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

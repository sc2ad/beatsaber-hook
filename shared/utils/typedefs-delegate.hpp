#pragma once

#include "typedefs-object.hpp"

#ifdef HAS_CODEGEN
#include "System/Delegate.hpp"
// self-typedef'd in il2cpp-class-internals.h
struct Il2CppDelegate : public System::Delegate {};
typedef System::Delegate Delegate;
#else
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
#endif

#ifdef HAS_CODEGEN
#include "System/MulticastDelegate.hpp"
typedef System::MulticastDelegate Il2CppMulticastDelegate;
typedef System::MulticastDelegate MulticastDelegate;
#else
// System.MulticastDelegate
typedef struct MulticastDelegate : Delegate {
    ::Array<Delegate*>* delegates;
} MulticastDelegate;
#endif

#ifndef HAS_CODEGEN
// System.DelegateData
typedef struct DelegateData : Il2CppObject {
    Il2CppReflectionType* target_type;
    Il2CppString* method_name;
    bool curied_first_arg;
} DelegateData;
#endif
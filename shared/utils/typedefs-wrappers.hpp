#pragma once
#include "il2cpp-functions.hpp"
#include "il2cpp-type-check.hpp"

#if __has_feature(cxx_exceptions)
struct CreatedTooEarlyException : std::runtime_error {
    CreatedTooEarlyException() : std::runtime_error("A SafePtr<T> instance was created too early or a necessary GC function was not found!") {}
};
#else
#include "utils.h"
#endif

/// @brief Represents a C++ type that wraps a C# pointer that will be valid for the entire lifetime of this instance.
/// This instance must be created at a time such that il2cpp_functions::Init is valid, or else it will throw a CreatedTooEarlyException
/// @tparam T The type of the instance to wrap.
template<class T>
struct SafePtr {
    SafePtr(T* wrappableInstance) : internalHandle(SafePointerWrapper::New(wrappableInstance)) {}
    SafePtr(T& wrappableInstance) : internalHandle(SafePointerWrapper::New(std::addressof(wrappableInstance))) {}
    ~SafePtr() {
        il2cpp_functions::Init();
        if (!il2cpp_functions::GC_free) {
            #if __has_feature(cxx_exceptions)
            throw CreatedTooEarlyException();
            #else
            CRASH_UNLESS(false);
            #endif
        }
        il2cpp_functions::GC_free(internalHandle);
    }

    T& operator *() {
        return *internalHandle->instancePointer;
    }

    operator T&() {
        return *internalHandle->instancePointer;
    }

    operator T*() {
        return internalHandle->instancePointer;
    }

    T* operator ->() {
        return internalHandle->instancePointer;
    }

    T& operator *() const {
        return *internalHandle->instancePointer;
    }

    operator const T&() const {
        return *internalHandle->instancePointer;
    }

    operator const T*() const {
        return internalHandle->instancePointer;
    }

    const T* operator ->() const {
        return internalHandle->instancePointer;
    }

    private:
    struct SafePointerWrapper {
        static SafePointerWrapper* New(T* instance) {
            il2cpp_functions::Init();
            if (!il2cpp_functions::GarbageCollector_AllocateFixed) {
                #if __has_feature(cxx_exceptions)
                throw CreatedTooEarlyException();
                #else
                CRASH_UNLESS(false);
                #endif
            }
            auto* wrapper = reinterpret_cast<SafePointerWrapper*>(il2cpp_functions::GarbageCollector_AllocateFixed(sizeof(SafePointerWrapper), nullptr));
            wrapper->instancePointer = instance;
            return wrapper;
        }
        // Must be explicitly GC freed and allocated
        SafePointerWrapper() = delete;
        ~SafePointerWrapper() = delete;
        T* instancePointer;
    };
    SafePointerWrapper* internalHandle;
};

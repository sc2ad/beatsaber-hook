#pragma once
#include "il2cpp-functions.hpp"
#include "il2cpp-type-check.hpp"

#if __has_feature(cxx_exceptions)
struct CreatedTooEarlyException : std::runtime_error {
    CreatedTooEarlyException() : std::runtime_error("A SafePtr<T> instance was created too early or a necessary GC function was not found!") {}
};
struct NullHandleException : std::runtime_error {
    NullHandleException() : std::runtime_error("A SafePtr<T> instance is holding a null handle!") {}
};
#define __SAFE_PTR_NULL_HANDLE_CHECK(handle, ...) \
if (handle) \
return __VA_ARGS__; \
throw NullHandleException()

#else
#include "utils.h"
#define __SAFE_PTR_NULL_HANDLE_CHECK(handle, ...) \
if (handle) \
return __VA_ARGS__; \
CRASH_UNLESS(false)
#endif


// TODO: Test to see if gc alloc works
// TODO: Make an overall Ptr interface type, virtual destructor and *, -> operators
// TODO: Remove all conversion operators? (Basically force people to guarantee lifetime of held instance?)

/// @brief Represents a C++ type that wraps a C# pointer that will be valid for the entire lifetime of this instance.
/// This instance must be created at a time such that il2cpp_functions::Init is valid, or else it will throw a CreatedTooEarlyException
/// @tparam T The type of the instance to wrap.
template<class T>
struct SafePtr {
    /// @brief Default constructor
    SafePtr() : internalHandle(nullptr) {}
    /// @brief Construct a SafePtr<T> with the provided instance pointer (which may be nullptr).
    /// If you wish to wrap a non-existent pointer (ex, use as a default constructor) see the 0 arg constructor instead.
    SafePtr(T* wrappableInstance) : internalHandle(SafePointerWrapper::New(wrappableInstance)) {}
    /// @brief Construct a SafePtr<T> with the provided reference
    SafePtr(T& wrappableInstance) : internalHandle(SafePointerWrapper::New(std::addressof(wrappableInstance))) {}
    /// Explicitly delete copy constructor
    SafePtr(const SafePtr& other) = delete;
    /// @brief Move constructor is default
    SafePtr(SafePtr&& other) = default;
    /// @brief Destructor. Destroys the internal wrapper type, if necessary.
    /// Throws a CreateTooEarlyException/CRASH_UNLESS if a wrapper type exists and must be freed, yet GC_free does not exist.
    ~SafePtr() {
        if (!internalHandle) {
            // Destructor without an internal handle is trivial
            return;
        }
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

    /// @brief Emplace a new value into this SafePtr, freeing an existing one, if it exists.
    /// @param other The instance to emplace.
    void emplace(T& other) {
        this->~SafePtr();
        internalHandle = SafePointerWrapper::New(std::addressof(other));
    }

    /// @brief Emplace a new value into this SafePtr, freeing an existing one, if it exists.
    /// @param other The instance to emplace.
    void emplace(T* other) {
        this->~SafePtr();
        internalHandle = SafePointerWrapper::New(other);
    }

    inline SafePtr<T>& operator=(T* other) noexcept {
        emplace(other);
    }

    inline SafePtr<T>& operator=(T& other) noexcept {
        emplace(other);
    }

    /// @brief Returns true if this instance's internal handle holds a pointer of ANY value (including nullptr)
    /// false otherwise.
    operator bool() const {
        return internalHandle != nullptr;
    }

    /// @brief Dereferences the instance pointer to a reference type of the held instance.
    /// Throws a NullHandleException if there is no internal handle.
    T& operator *() {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, *internalHandle->instancePointer);
    }

    operator T&() {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, *internalHandle->instancePointer);
    }

    operator T*() {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, internalHandle->instancePointer);
    }

    T* operator ->() {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, internalHandle->instancePointer);
    }

    const T& operator *() const {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, *internalHandle->instancePointer);
    }

    operator const T&() const {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, *internalHandle->instancePointer);
    }

    operator const T*() const {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, internalHandle->instancePointer);
    }

    const T* operator ->() const {
        __SAFE_PTR_NULL_HANDLE_CHECK(internalHandle, internalHandle->instancePointer);
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
            // It should be safe to assume that GC_AllocateFixed returns a non-null pointer. If it does return null, we have a pretty big issue.
            auto* wrapper = reinterpret_cast<SafePointerWrapper*>(il2cpp_functions::GarbageCollector_AllocateFixed(sizeof(SafePointerWrapper), nullptr));
            CRASH_UNLESS(wrapper);
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


/// @brief Represents a pointer that may be GC'd, but will notify you when it has.
/// Currently unimplemented, requires a hook into all GC frees/collections
template<class T>
struct WeakPtr {

};
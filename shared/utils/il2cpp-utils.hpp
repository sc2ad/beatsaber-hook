#ifndef IL2CPP_UTILS_H
#define IL2CPP_UTILS_H

#pragma pack(push)

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <optional>
#include <vector>

#include "gc-alloc.hpp"

#include "il2cpp-functions.hpp"
#include "logging.hpp"
#include "utils.h"
#include "il2cpp-type-check.hpp"
#include "il2cpp-utils-methods.hpp"
#include "il2cpp-utils-classes.hpp"
#include "il2cpp-utils-exceptions.hpp"
#include "il2cpp-utils-properties.hpp"
#include "il2cpp-utils-fields.hpp"
#include <string>
#include <string_view>
#include <optional>
#include "typedefs.h"
#include <functional>

namespace il2cpp_utils {
    // Seriously, don't un-const the returned Type
    const Il2CppType* MakeRef(const Il2CppType* type);

    // Generally, it's better to just use class_from_type!
    const Il2CppType* UnRef(const Il2CppType* type);

    ::std::vector<const Il2CppType*> ClassVecToTypes(::std::vector<const Il2CppClass*> seq);

    bool IsInterface(const Il2CppClass* klass);

    ::std::vector<Il2CppClass*> ClassesFrom(::std::vector<Il2CppClass*> classes);
    ::std::vector<Il2CppClass*> ClassesFrom(::std::vector<::std::string_view> strings);

    ::std::vector<const Il2CppType*> TypesFrom(::std::vector<const Il2CppType*> types);
    ::std::vector<const Il2CppType*> TypesFrom(::std::vector<const Il2CppClass*> classes);
    ::std::vector<const Il2CppType*> TypesFrom(::std::vector<::std::string_view> strings);

    Il2CppClass* GetParamClass(const MethodInfo* method, int paramIdx);

    /// @brief Clears all allocated delegates.
    /// THIS SHOULD NOT BE CALLED UNLESS YOU ARE CERTAIN ALL ALLOCATED DELEGATES NO LONGER EXIST IN IL2CPP!
    void ClearDelegates();

    /// @brief Clears the specified delegate.
    /// @param delegate The Delegate* to clear the allocated MethodInfo* from, if it exists
    void ClearDelegate(void* delegate);

    /// @brief Adds the allocated Delegate* to the set of mapped delegates.
    /// @param delegate The Delegate* to add
    /// @remarks See ClearDelegates() and ClearDelegate(Delegate* delegate)
    void AddAllocatedDelegate(void* delegate);

    /// @brief Creates a delegate of return type T, with target TObj, using the provided Il2CppClass*
    /// @tparam T The type to return
    /// @tparam TObj The type of the target object
    /// @param delegateClass The delegate Il2CppClass* to use
    /// @param obj The target instance
    /// @param callback The callback function_ptr_t
    /// @returns The created delegate
    template<typename T = MulticastDelegate*, typename TObj = Il2CppObject*, typename R, typename... TArgs>
    T MakeDelegate(const Il2CppClass* delegateClass, TObj obj, function_ptr_t<R, TArgs...> callback) {
        static_assert(std::is_pointer_v<TObj>, "TObj must be a pointer!");
        static_assert(std::is_pointer_v<T>, "T must be a pointer!");
        static auto& logger = getLogger();
        /*
        * TODO: call PlatformInvoke::MarshalFunctionPointerToDelegate directly instead of copying code from it,
        * or at least use a cache like utils::NativeDelegateMethodCache::GetNativeDelegate(nativeFunctionPointer);
        */
        // Lets cache this method. Well formed delegates have only one Invoke method, so ignore param count.
        auto* invoke = il2cpp_utils::FindMethodUnsafe(delegateClass, "Invoke", -1);
        auto* method = reinterpret_cast<MethodInfo*>(gc_alloc_specific(sizeof(MethodInfo)));
        // Add the allocated delegate so we can free it later.
        method->methodPointer = (Il2CppMethodPointer)callback;
        method->invoker_method = NULL;
        method->parameters_count = invoke->parameters_count;
        method->slot = kInvalidIl2CppMethodSlot;
        method->is_marshaled_from_native = true;  // "a fake MethodInfo wrapping a native function pointer"
        // In the event that a function is static, this will behave as normal
        if (obj == nullptr) method->flags |= METHOD_ATTRIBUTE_STATIC;

        // TODO: figure out why passing method directly doesn't work
        // TODO: auto-free MethodInfo* after Delegate instance is GC'd
        auto* delegate = RET_DEFAULT_UNLESS(logger, il2cpp_utils::NewUnsafe<T>(delegateClass, obj, &method));
        auto* asDelegate = reinterpret_cast<Delegate*>(delegate);
        AddAllocatedDelegate(asDelegate);
        if ((void*)asDelegate->method_ptr != (void*)callback) {
            logger.error("Created Delegate's method_ptr (%p) is incorrect (should be %p)!", asDelegate->method_ptr, callback);
            return nullptr;
        }
        return delegate;
    }

    /// @brief Creates a delegate of return type T, with target TObj, using the provided Il2CppClass*.
    /// Assumes the callback has no parameters and a void return.
    /// @tparam T The type to return
    /// @tparam TObj The type of the target object
    /// @param delegateClass The delegate Il2CppClass* to use
    /// @param obj The target instance
    /// @param callback The callback function_ptr_t
    /// @returns The created delegate
    template<typename T = MulticastDelegate*, typename TObj = Il2CppObject*>
    T MakeDelegate(const Il2CppClass* delegateClass, TObj obj, void* callback) {
        auto tmp = reinterpret_cast<function_ptr_t<void>>(callback);
        return MakeDelegate(delegateClass, obj, tmp);
    }

    /// @brief Creates a delegate of return type T, with target TObj, using the provided Il2CppType*
    /// PLEASE!!! use the FieldInfo*, MethodInfo*, or Il2CppClass* versions instead if you can.
    /// @tparam T The type to return
    /// @tparam TObj The type of the target object
    /// @param delegateType The delegate Il2CppType* to use
    /// @param obj The target instance
    /// @param callback The callback function_ptr_t
    /// @returns The created delegate
    template<typename T = MulticastDelegate*, typename TObj = Il2CppObject*, typename R, typename... TArgs>
    T MakeDelegate(const Il2CppType* actionType, TObj obj, function_ptr_t<R, TArgs...> callback) {
        il2cpp_functions::Init();
        Il2CppClass* delegateClass = il2cpp_functions::class_from_il2cpp_type(actionType);
        return MakeDelegate(delegateClass, obj, callback);
    }

    /// @brief Creates a delegate of return type T, with target TObj, using the provided Il2CppType* and void* callback.
    /// Assumes the callback has no parameters and a void return.
    /// @tparam T The type to return
    /// @tparam TObj The type of the target object
    /// @param delegateType The delegate Il2CppType* to use
    /// @param obj The target instance
    /// @param callback The callback function
    /// @returns The created delegate
    template<typename T = MulticastDelegate*, typename TObj = Il2CppObject*>
    T MakeDelegate(const Il2CppType* delegateType, TObj obj, void* callback) {
        auto tmp = reinterpret_cast<function_ptr_t<void>>(callback);
        return MakeDelegate(delegateType, obj, tmp);
    }

    /// @brief Creates a delegate fit to be passed in the given parameter position to the given method.
    /// @tparam T The type to return
    /// @tparam T1 The type to forward to another call of MakeDelegate
    /// @tparam T2 The type to forward to another call of MakeDelegate
    /// @param method The MethodInfo* to grab the parameter from
    /// @param paramIdx The parameter to grab the type from
    /// @param arg1 Forwarded to another MakeDelegate
    /// @param arg2 Forwarded to another MakeDelegate
    /// @returns The created delegate
    template<typename T = MulticastDelegate*, typename T1, typename T2>
    T MakeDelegate(const MethodInfo* method, int paramIdx, T1&& arg1, T2&& arg2) {
        il2cpp_functions::Init();
        static auto& logger = getLogger();
        auto* delegateType = RET_0_UNLESS(logger, il2cpp_functions::method_get_param(method, paramIdx));
        return MakeDelegate<T>(delegateType, arg1, arg2);
    }

    /// @brief Creates a delegate fit to be assigned to the given field.
    /// @tparam T The type to return
    /// @tparam T1 The type to forward to another call of MakeDelegate
    /// @tparam T2 The type to forward to another call of MakeDelegate
    /// @param field The FieldInfo* to grab the parameter from
    /// @param arg1 Forwarded to another MakeDelegate
    /// @param arg2 Forwarded to another MakeDelegate
    /// @returns The created delegate
    template<typename T = MulticastDelegate*, typename T1, typename T2>
    T MakeDelegate(FieldInfo* field, T1&& arg1, T2&& arg2) {
        il2cpp_functions::Init();
        static auto& logger = getLogger();
        auto* delegateType = RET_0_UNLESS(logger, il2cpp_functions::field_get_type(field));
        return MakeDelegate<T>(delegateType, arg1, arg2);
    }

    /// @brief The wrapper for an invokable delegate with a context.
    /// @tparam I The instance type, which must be move-constructible.
    /// @tparam R The return type of the function being called.
    /// @tparam TArgs The argument types of the function being called.
    template<class I, class R, class... TArgs>
    struct WrapperInstance {
        I rawInstance;
        std::function<R(I*, TArgs...)> wrappedFunc;
    };

    /// @brief The wrapper for an invokable delegate without an existing context.
    /// @tparam R The return type of the function being called.
    /// @tparam TArgs The argument types of the function being called.
    template<class R, class... TArgs>
    struct WrapperStatic : Il2CppObject {
        std::function<R(TArgs...)> wrappedFunc;
    };

    /// @brief The invoker function for a delegate that has a non-trivial context.
    /// @tparam I The wrapped instance type.
    /// @tparam R The return type of the function.
    /// @tparam TArgs The argument types of the function.
    /// @param instance The wrapped instance of this context function.
    /// @param args The arguments to pass to this function.
    /// @return The return from the wrapped function.
    template<class I, class R, class... TArgs>
    R __attribute__((noinline)) invoker_func_instance(WrapperInstance<I, R, TArgs...>* instance, TArgs... args) {
        if constexpr (std::is_same_v<R, void>) {
            instance->wrappedFunc(&instance->rawInstance, args...);
        } else {
            return instance->wrappedFunc(&instance->rawInstance, args...);
        }
    }

    /// @brief The invoker function for a delegate with a wrapped type.
    /// @tparam R The return type of the function.
    /// @tparam TArgs The argument types of the function.
    /// @param instance The wrapped instance of this context function.
    /// @param args The arguments to pass to this function.
    /// @return The return from the wrapped function.
    template<class R, class... TArgs>
    R __attribute__((noinline)) invoker_func_static(WrapperStatic<R, TArgs...>* instance, TArgs... args) {
        if constexpr (std::is_same_v<R, void>) {
            instance->wrappedFunc(args...);
        } else {
            return instance->wrappedFunc(args...);
        }
    }

    /// @brief EXTREMEMLY UNSAFE ALLOCATION! THIS SHOULD BE AVOIDED UNLESS YOU KNOW WHAT YOU ARE DOING!
    /// This function allocates a GC-able object of the size provided by manipulating an existing Il2CppClass' instance_size.
    /// This is VERY DANGEROUS (and NOT THREAD SAFE!) and may cause all sorts of race conditions. Use at your own risk.
    /// @param size The size to allocat the unsafe object with.
    /// @return The returned GC-allocated instance.
    void* __AllocateUnsafe(std::size_t size);

    /// @brief Makes a delegate wrapping a context function (such as a context lambda).
    /// @tparam T The type to return.
    /// @tparam I The instance object to provide to this delegate.
    /// @tparam R The return type of the delegate.
    /// @tparam TArgs The arguments of the delegate.
    /// @param delegateClass The Il2CppClass* of the delegate to create.
    /// @param instance The (move constructible) instance reference to provide to the delegate. This instance is moved and will no longer be valid.
    /// @param f The function to invoke with the delegate.
    /// @return The created delegate.
    template<typename T = MulticastDelegate*, class I, class R, class... TArgs>
    T MakeDelegate(const Il2CppClass* delegateClass, I& instance, std::function<R(I*, TArgs...)> f) {
        auto* wrapperInstance = reinterpret_cast<WrapperInstance<I, R, TArgs...>*>(__AllocateUnsafe(sizeof(WrapperInstance<I, R, TArgs...>)));

        wrapperInstance->rawInstance = std::move(instance);
        wrapperInstance->wrappedFunc = f;
        return MakeDelegate<T>(delegateClass, wrapperInstance, &invoker_func_instance<I, R, TArgs...>);
    }

    /// @brief Makes a delegate wrapping a context function (such as a context lambda).
    /// @tparam T The type to return.
    /// @tparam I The instance object to provide to this delegate.
    /// @tparam R The return type of the delegate.
    /// @tparam TArgs The arguments of the delegate.
    /// @param delegateClass The Il2CppClass* of the delegate to create.
    /// @param instance The (move constructible) instance reference to provide to the delegate. This instance is moved and will no longer be valid.
    /// @param f The function to invoke with the delegate.
    /// @return The created delegate.
    template<typename T = MulticastDelegate*, class R, class... TArgs>
    T MakeDelegate(const Il2CppClass* delegateClass, std::function<R(TArgs...)> f) {
        auto* wrapperInstance = reinterpret_cast<WrapperStatic<R, TArgs...>*>(__AllocateUnsafe(sizeof(WrapperStatic<R, TArgs...>)));
        wrapperInstance->wrappedFunc = f;
        wrapperInstance->klass = GetClassFromName("System", "Object");
        return MakeDelegate<T>(delegateClass, wrapperInstance, &invoker_func_static<R, TArgs...>);
    }

    /// @brief Makes a delegate wrapping the provided instance method.
    /// @tparam T The type to return.
    /// @tparam I The instance object.
    /// @tparam R The return type of the delegate.
    /// @tparam TArgs The arguments of the delegate.
    /// @param delegateClass The Il2CppClass* of the delegate to create.
    /// @param instance The (move constructible) instance reference to provide to the delegate. This instance is moved and will no longer be valid.
    /// @param memberFunc A pointer to the member function on the provided instance to invoke for this delegate.
    /// @return The created delegate. 
    template<typename T = MulticastDelegate*, class I, class R, class... TArgs>
    inline T MakeDelegate(const Il2CppClass* delegateClass, I& instance, R (I::*memberFunc)(TArgs...)) {
        return MakeDelegate<T>(delegateClass, instance, std::function<R(I*, TArgs...)>(memberFunc));
    }

    // Intializes an object (using the given args) fit to be passed to the given method at the given parameter index.
    template<typename... TArgs>
    Il2CppObject* CreateParam(const MethodInfo* method, int paramIdx, TArgs&& ...args) {
        static auto& logger = getLogger();
        auto* klass = RET_0_UNLESS(logger, GetParamClass(method, paramIdx));
        return il2cpp_utils::New(klass, args...);
    }

    template<typename... TArgs>
    Il2CppObject* CreateParamUnsafe(const MethodInfo* method, int paramIdx, TArgs&& ...args) {
        static auto& logger = getLogger();
        auto* klass = RET_0_UNLESS(logger, GetParamClass(method, paramIdx));
        return il2cpp_utils::NewUnsafe(klass, args...);
    }

    /// @brief Converts a vector to an Array*
    /// @tparam T Inner type of the vector and array
    /// @param vec Vector to create the Array from
    /// @return The created Array<T>*
    template<typename T>
    Array<T>* vectorToArray(::std::vector<T>& vec) {
        il2cpp_functions::Init();
        static auto& logger = getLogger();
        Array<T>* arr = reinterpret_cast<Array<T>*>(RET_0_UNLESS(logger, il2cpp_functions::array_new(il2cpp_type_check::il2cpp_no_arg_class<T>::get(), vec.size())));
        for (size_t i = 0; i < vec.size(); i++) {
            arr->values[i] = vec[i];
        }
        return arr;
    }

    // Calls the System.RuntimeType.MakeGenericType(System.Type gt, System.Type[] types) function
    Il2CppReflectionType* MakeGenericType(Il2CppReflectionType* gt, Il2CppArray* types);

    enum struct StringType {
        Temporary,  // string is normal C# object, may be GC'd
        Manual,     // string is owned by C++, must be manually freed (use this for string constants as well)
    };

    /// @brief Creates a new C# string and registers it with GC. Copies the input string.
    /// @param inp String view to create the string from.
    /// @param type The type of string to create
    /// @return Created string
    Il2CppString* createcsstr(::std::string_view inp, StringType type = StringType::Temporary);

    // Returns if a given source object is an object of the given class
    // Created by zoller27osu
    [[nodiscard]] bool Match(const Il2CppObject* source, const Il2CppClass* klass) noexcept;

    // Asserts that a given source object is an object of the given class
    // Created by zoller27osu
    bool AssertMatch(const Il2CppObject* source, const Il2CppClass* klass);

    template<class To, class From>
    // Downcasts a class from From* to To*
    [[nodiscard]] auto down_cast(From* in) noexcept {
        static_assert(::std::is_nothrow_convertible_v<To*, From*>);
        return static_cast<To*>(in);
    }

    template<typename... TArgs>
    // Runtime Invoke, but with a list initializer for args
    Il2CppObject* RuntimeInvoke(const MethodInfo* method, Il2CppObject* reference, Il2CppException** exc, TArgs* ...args) {
        il2cpp_functions::Init();

        void* invokeParams[] = {reinterpret_cast<void*>(args)...};
        return il2cpp_functions::runtime_invoke(method, reference, invokeParams, exc);
    }

    template<typename T = void, typename... TArgs>
    void ExtractClassesNoArgs(::std::vector<const Il2CppClass*>& vec) {
        vec.push_back(classof(T));
        if constexpr (sizeof...(TArgs) != 0) {
            ExtractClassesNoArgs<TArgs...>(vec);
        }
    }

    template<typename... TArgs>
    ::std::vector<const Il2CppClass*> ExtractFromFunctionNoArgs() {
        ::std::vector<const Il2CppClass*> vec(sizeof...(TArgs));
        ExtractClassesNoArgs<TArgs...>(vec);
        return vec;
    }

    /// @brief Creates and returns a C# System.Func<TArgs..., Ret> from the provided function_ptr_t.
    /// Note that this function assumes AOT code exists for a System.Func with the provided generic arguments.
    /// @tparam Ret The return type of the function
    /// @tparam TArgs The arguments of the function
    /// @returns The created System.Func<TArgs..., Ret>. Null if it could not be created.
    template<typename T = MulticastDelegate*, typename Ret, typename... TArgs>
    T MakeFunc(function_ptr_t<Ret, TArgs...> lambda) {
        static_assert(sizeof...(TArgs) + 1 <= 16, "Cannot create a Func`<T1, T2, ..., TN> where N is > 16!");
        static_assert(!std::is_same_v<Ret, void>, "Function used in ::il2cpp_utils::MakeFunc must have a non-void return!");
        static auto& logger = getLogger();
        // Get generic class with matching number of args
        static auto* genericClass = il2cpp_utils::GetClassFromName("System", "Func`" + ::std::to_string(sizeof...(TArgs) + 1));
        // Extract all parameter types and return types
        static auto genericClasses = ExtractFromFunctionNoArgs<Ret, TArgs...>();
        // Instantiate the Func` type
        auto* instantiatedFunc = RET_DEFAULT_UNLESS(logger, il2cpp_utils::MakeGeneric(genericClass, genericClasses));
        // Create the action from the instantiated Func` type
        return il2cpp_utils::MakeDelegate<T>(instantiatedFunc, static_cast<Il2CppObject*>(nullptr), lambda);
    }

    /// @brief Creates and returns a C# System.Action<TArgs...> from the provided function_ptr_t.
    /// Note that this function assumes AOT code exists for a System.Action with the provided generic arguments.
    /// @tparam TArgs The arguments of the function
    /// @returns The created System.Action<TArgs...>. Null if it could not be created.
    template<typename T = MulticastDelegate*, typename... TArgs>
    T MakeAction(function_ptr_t<void, TArgs...> lambda) {
        static_assert(sizeof...(TArgs) <= 16, "Cannot create an Action`<T1, T2, ..., TN> where N is > 16!");
        static auto& logger = getLogger();
        if constexpr (sizeof...(TArgs) != 0) {
            // Get generic class with matching number of args
            static auto* genericClass = il2cpp_utils::GetClassFromName("System", "Action`" + ::std::to_string(sizeof...(TArgs)));
            // Extract all parameter types and return types
            static auto genericClasses = ExtractFromFunctionNoArgs<TArgs...>();
            // Instantiate the Func` type
            auto* instantiatedFunc = RET_DEFAULT_UNLESS(logger, il2cpp_utils::MakeGeneric(genericClass, genericClasses));
            // Create the action from the instantiated Func` type
            return il2cpp_utils::MakeDelegate<T>(instantiatedFunc, static_cast<Il2CppObject*>(nullptr), lambda);
        } else {
            static auto* klass = il2cpp_utils::GetClassFromName("System", "Action");
            return il2cpp_utils::MakeDelegate<T>(klass, static_cast<Il2CppObject*>(nullptr), lambda);
        }
    }
}

#pragma pack(pop)

#endif /* IL2CPP_UTILS_H */

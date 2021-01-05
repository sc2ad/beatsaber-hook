#include "typedefs.h"  // needed for some il2cpp_type_check things and MulticastDelegate for MakeDelegate
#ifndef IL2CPP_UTILS_H
#define IL2CPP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <optional>
#include <vector>

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
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeDelegate");
        /*
        * TODO: call PlatformInvoke::MarshalFunctionPointerToDelegate directly instead of copying code from it,
        * or at least use a cache like utils::NativeDelegateMethodCache::GetNativeDelegate(nativeFunctionPointer);
        */
        // Lets cache this method. Well formed delegates have only one Invoke method, so ignore param count.
        auto* invoke = il2cpp_utils::FindMethodUnsafe(delegateClass, "Invoke", -1);
        auto* method = (MethodInfo*) calloc(1, sizeof(MethodInfo));
        // Add the allocated delegate so we can free it later.
        method->methodPointer = (Il2CppMethodPointer)callback;
        method->invoker_method = NULL;
        method->parameters_count = invoke->parameters_count;
        method->slot = kInvalidIl2CppMethodSlot;
        method->is_marshaled_from_native = true;  // "a fake MethodInfo wrapping a native function pointer"
        // In the event that a function is static, this will behave as normal
        if (obj == nullptr) method->flags |= METHOD_ATTRIBUTE_STATIC;

        // TODO: figure out why passing method directly doesn't work
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
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeDelegate");
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
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeDelegate");
        auto* delegateType = RET_0_UNLESS(logger, il2cpp_functions::field_get_type(field));
        return MakeDelegate<T>(delegateType, arg1, arg2);
    }

    // Intializes an object (using the given args) fit to be passed to the given method at the given parameter index.
    template<typename... TArgs>
    Il2CppObject* CreateParam(const MethodInfo* method, int paramIdx, TArgs&& ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("CreateParam");
        auto* klass = RET_0_UNLESS(logger, GetParamClass(method, paramIdx));
        return il2cpp_utils::New(klass, args...);
    }

    template<typename... TArgs>
    Il2CppObject* CreateParamUnsafe(const MethodInfo* method, int paramIdx, TArgs&& ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("CreateParamUnsafe");
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
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("vectorToArray");
        Array<T>* arr = reinterpret_cast<Array<T>*>(RET_0_UNLESS(logger, il2cpp_functions::array_new(il2cpp_type_check::il2cpp_no_arg_class<T>::get(), vec.size())));
        for (size_t i = 0; i < vec.size(); i++) {
            arr->values[i] = vec[i];
        }
        return arr;
    }

    // Calls the System.RuntimeType.MakeGenericType(System.Type gt, System.Type[] types) function
    Il2CppReflectionType* MakeGenericType(Il2CppReflectionType* gt, Il2CppArray* types);

    enum StringType {
        Temporary,  // string is normal C# object, may be GC'd
        Manual,     // string is owned by C++, must be manually freed (use this for string constants as well)
    };

    /// @brief How to create an il2cpp object.
    enum CreationType {
        /// @brief Created object is a C# object, it may be GC'd.
        Temporary,
        /// @brief Created object is manual, it must be freed explicitly (via delete).
        Manual
    };

    /// @brief Manually creates an instance of the provided Il2CppClass*.
    /// The created instance's type initializer will NOT execute on another thread! Be warned!
    /// @param klass The Il2CppClass* to create an instance of.
    /// @return The created instance, or nullptr if it failed for any reason.
    Il2CppObject* createManual(Il2CppClass* const klass) noexcept;

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
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeFunc");
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
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeAction");
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
#endif /* IL2CPP_UTILS_H */

#include "typedefs.h"
#ifndef IL2CPP_UTILS_METHODS
#define IL2CPP_UTILS_METHODS
#include "il2cpp-functions.hpp"
#include "logging.hpp"
#include <vector>
#include "il2cpp-utils-exceptions.hpp"
#include "il2cpp-utils-classes.hpp"
#include "il2cpp-type-check.hpp"
#include "utils.h"

namespace il2cpp_utils {
    std::vector<const Il2CppType*> TypesFrom(std::vector<const Il2CppType*> types);
    std::vector<const Il2CppType*> TypesFrom(std::vector<const Il2CppClass*> classes);
    std::vector<const Il2CppType*> TypesFrom(std::vector<std::string_view> strings);

    struct FindMethodInfo {
        Il2CppClass* klass = nullptr;
        ::std::string name;
        Il2CppClass* returnType = nullptr;
        ::std::vector<Il2CppClass*> genTypes;
        ::std::vector<const Il2CppType*> argTypes;

        template <typename T, typename... TParams,
            ::std::enable_if_t<!::std::is_convertible_v<T, ::std::string_view>, int> = 0>
        FindMethodInfo(T&& classOrInstance, ::std::string_view methodName, TParams&&... paramTypes) {
            klass = ExtractClass(classOrInstance);
            name = methodName;

            if constexpr (sizeof...(TParams) > 0) {
                if constexpr (sizeof...(TParams) == 1 && (... && is_vector<::std::decay_t<TParams>>::value))
                    argTypes = TypesFrom(paramTypes...);
                else
                    argTypes = TypesFrom({paramTypes...});
            }
        }

        template <typename T, typename G, typename... TArgs,
            ::std::enable_if_t<!::std::is_convertible_v<Il2CppType*, G>, int> = 0>
        FindMethodInfo(T&& classOrInstance, ::std::string_view methodName, ::std::vector<G> genericArgs, TArgs&&... args)
            : FindMethodInfo(classOrInstance, methodName, args...)
        {
            genTypes = ClassesFrom(genericArgs);
        }

        template <typename T, typename R, typename... TArgs,
            ::std::enable_if_t<!::std::is_convertible_v<R, ::std::string_view>, int> = 0>
        FindMethodInfo(T&& classOrInstance, R returnTypeOrClass, ::std::string_view methodName, TArgs&&... args)
            : FindMethodInfo(classOrInstance, methodName, args...)
        {
            returnType = ExtractClass(returnTypeOrClass);
        }

        template <typename... TArgs>
        FindMethodInfo(::std::string_view namespaceName, ::std::string_view className, TArgs&&... args)
            : FindMethodInfo(GetClassFromName(namespaceName, className), args...) { }
    };

    template<class T>
    Il2CppObject* ToIl2CppObject(T&& arg) {
        il2cpp_functions::Init();

        using Dt = ::std::decay_t<T>;
        if constexpr (::std::is_same_v<Dt, Il2CppType*> || ::std::is_same_v<Dt, Il2CppClass*>) {
            return nullptr;
        }
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("ToIl2CppObject");
        auto* klass = RET_0_UNLESS(logger, ExtractClass(arg));
        return il2cpp_functions::value_box(klass, &arg);
    }

    template<class T>
    void* ExtractValue(T&& arg) {
        il2cpp_functions::Init();

        using Dt = ::std::decay_t<T>;
        if constexpr (::std::is_same_v<Dt, Il2CppType*> || ::std::is_same_v<Dt, Il2CppClass*>) {
            return nullptr;
        } else if constexpr (::std::is_pointer_v<Dt>) {
            if constexpr (::std::is_base_of_v<Il2CppObject, ::std::remove_pointer_t<Dt>>) {
                if (arg) {
                    auto* klass = il2cpp_functions::object_get_class(arg);
                    if (klass && klass->valuetype) {
                        // Arg is an Il2CppObject* of a value type. It needs to be unboxed.
                        return il2cpp_functions::object_unbox(arg);
                    }
                }
            }
            return arg;
        } else {
            return const_cast<Dt*>(&arg);
        }
    }

    inline auto ExtractValues() {
        return ::std::vector<void*>();
    }

    template<class T, class... TArgs>
    ::std::vector<void*> ExtractValues(T&& arg, TArgs&& ...args) {
        auto* firstVal = ExtractValue(arg);
        auto otherVals = ExtractValues(args...);
        otherVals.insert(otherVals.begin(), firstVal);  // inserts at front
        return otherVals;
    }

    #if __has_feature(cxx_exceptions)
    /// @brief Instantiates a generic MethodInfo* from the provided Il2CppClasses.
    /// This method will throw an Il2CppUtilException if it fails for any reason.
    /// @return MethodInfo* for RunMethod calls.
    const MethodInfo* MakeGenericMethod(const MethodInfo* info, ::std::vector<Il2CppClass*> types);
    /// @brief Finds the first MethodInfo* described by the given Il2CppClass*, method name, and argument count.
    /// Throws an Il2CppUtilException when: klass is null, or the method could not be found.
    /// @return The found MethodInfo*
    /// @param klass The Il2CppClass* to search for the method
    /// @param methodName The il2cpp name of the method to find
    /// @param argsCount The number of arguments to match (or -1 to not match at all)
    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, ::std::string_view methodName, int argsCount);
    /// @brief Find the first MethodInfo* on the given instance, described by the methodName, and argument count.
    /// Throws an Il2CppUtilException when: instance is null, the Il2CppClass* could not be loaded, or the method could not be found.
    /// @return The found MethodInfo*
    /// @param instance The Il2CppObject* to search for the method
    /// @param methodName The il2cpp name of the method to find
    /// @param argsCount The number of arguments to match (or -1 to not match at all)
    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, ::std::string_view methodName, int argsCount);
    /// @brief Find the first MethodInfo* on the given instance, described by the methodName, and argument count.
    /// Throws an Il2CppUtilException when: the Il2CppClass* could not be found, or the method could not be found.
    /// @return The found MethodInfo*
    /// @param instance The Il2CppObject* to search for the method
    /// @param methodName The il2cpp name of the method to find
    /// @param argsCount The number of arguments to match (or -1 to not match at all)
    const MethodInfo* FindMethodUnsafe(::std::string_view nameSpace, ::std::string_view className, ::std::string_view methodName, int argsCount);
    const MethodInfo* FindMethod(FindMethodInfo& info);
    template <typename... TArgs, ::std::enable_if_t<(... && !::std::is_convertible_v<TArgs, FindMethodInfo>), int> = 0>
    const MethodInfo* FindMethod(TArgs&&... args) {
        auto info = FindMethodInfo(args...);
        return FindMethod(info);
    }

    bool IsConvertible(const Il2CppType* to, const Il2CppType* from, bool asArgs = true);

    // Returns if a given MethodInfo's parameters match the Il2CppType vector
    bool ParameterMatch(const MethodInfo* method, ::std::vector<const Il2CppType*> argTypes);

    // Returns if a given MethodInfo's parameters match the Il2CppType vector and generic types vector
    bool ParameterMatch(const MethodInfo* method, ::std::vector<Il2CppClass*> genTypes, ::std::vector<const Il2CppType*> argTypes);

    /// @brief Calls RunMethod, but throws a RunMethodException on failure.
    /// If checkTypes is false, does not perform type checking and instead is an unsafe wrapper around runtime_invoke.
    /// @tparam TOut The output to return. Defaults to Il2CppObject*.
    /// @tparam checkTypes Whether to check types or not. Defaults to true.
    /// @tparam T The instance type (either an actual instance or an Il2CppClass*/Il2CppType*).
    /// @tparam TArgs The argument types.
    /// @param instance The instance or Il2CppClass*/Il2CppType* to invoke with.
    /// @param method The MethodInfo* to invoke.
    /// @param params The arguments to pass into the function.
    template<class TOut = Il2CppObject*, bool checkTypes = true, class T, class... TArgs>
    TOut RunMethodThrow(T&& instance, const MethodInfo* method, TArgs&& ...params) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunMethodThrow");
        if (!method) {
            throw RunMethodException("Method cannot be null!");
        }

        if constexpr (checkTypes && sizeof...(TArgs) > 0) {
            auto typeVec = ExtractTypes(params...);
            if (!ParameterMatch(method, typeVec)) {
                throw RunMethodException("Parameters do not match!");
            }
        }

        void* inst = ExtractValue(instance);
        Il2CppException* exp = nullptr;
        auto invokeParamsVec = ExtractValues(params...);
        il2cpp_functions::Init();
        auto* ret = il2cpp_functions::runtime_invoke(method, inst, invokeParamsVec.data(), &exp);
        if (exp) {
            logger.error("%s: Failed with exception: %s", il2cpp_functions::method_get_name(method),
                il2cpp_utils::ExceptionToString(exp).c_str());
            throw RunMethodException(exp);
        }
        if constexpr (checkTypes) {
            if (ret) {
                // By using this instead of ExtractType, we avoid unboxing because the ultimate type in that case would depend on the
                // method in the first place
                auto* outType = ExtractIndependentType<TOut>();
                if (outType) {
                    auto* retType = ExtractType(ret);
                    if (!IsConvertible(outType, retType, false)) {
                        logger.warning("User requested TOut %s does not match the method's return object of type %s!",
                            TypeGetSimpleName(outType), TypeGetSimpleName(retType));
                    }
                }
            }
        }
        return FromIl2CppObject<TOut>(ret);
    }
    #else
    /// @brief Instantiates a generic MethodInfo* from the provided Il2CppClasses.
    /// @return MethodInfo* for RunMethod calls, will be nullptr on failure
    const MethodInfo* MakeGenericMethod(const MethodInfo* info, ::std::vector<Il2CppClass*> types) noexcept;
    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, ::std::string_view methodName, int argsCount) noexcept;
    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, ::std::string_view methodName, int argsCount) noexcept;
    const MethodInfo* FindMethodUnsafe(::std::string_view nameSpace, ::std::string_view className, ::std::string_view methodName, int argsCount) noexcept;
    const MethodInfo* FindMethod(FindMethodInfo& info) noexcept;
    template <typename... TArgs, ::std::enable_if_t<(... && !::std::is_convertible_v<TArgs, FindMethodInfo>), int> = 0>
    const MethodInfo* FindMethod(TArgs&&... args) noexcept {
        auto info = FindMethodInfo(args...);
        return FindMethod(info);
    }

    bool IsConvertible(const Il2CppType* to, const Il2CppType* from, bool asArgs = true);
    // Returns if a given MethodInfo's parameters match the Il2CppType vector
    bool ParameterMatch(const MethodInfo* method, ::std::vector<const Il2CppType*> argTypes);

    // Returns if a given MethodInfo's parameters match the Il2CppType vector and generic types vector
    bool ParameterMatch(const MethodInfo* method, ::std::vector<Il2CppClass*> genTypes, ::std::vector<const Il2CppType*> argTypes);
    #endif

    // Function made by zoller27osu, modified by Sc2ad
    // Logs information about the given MethodInfo* as log(DEBUG)
    void LogMethod(LoggerContextObject& logger, const MethodInfo* method);

    // Created by zoller27osu
    // Calls LogMethod on all methods in the given class
    void LogMethods(LoggerContextObject& logger, Il2CppClass* klass, bool logParents = false);

    template<class TOut = Il2CppObject*, bool checkTypes = true, class T, class... TArgs>
    // Runs a MethodInfo with the specified parameters and instance, with return type TOut.
    // Assumes a static method if instance == nullptr. May fail due to exception or wrong name, hence the ::std::optional.
    ::std::optional<TOut> RunMethod(T&& instance, const MethodInfo* method, TArgs&& ...params) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunMethod");
        RET_NULLOPT_UNLESS(logger, method);

        if constexpr (checkTypes && sizeof...(TArgs) > 0) {
            auto typeVec = ExtractTypes(params...);
            RET_NULLOPT_UNLESS(logger, typeVec.size() == sizeof...(TArgs));
            RET_NULLOPT_UNLESS(logger, ParameterMatch(method, typeVec));
        }

        void* inst = ExtractValue(instance);  // null is allowed (for T = Il2CppType* or Il2CppClass*)
        Il2CppException* exp = nullptr;
        auto invokeParamsVec = ExtractValues(params...);
        il2cpp_functions::Init();
        auto* ret = il2cpp_functions::runtime_invoke(method, inst, invokeParamsVec.data(), &exp);

        // Check if the TOut that the user requested makes sense given the Il2CppObject* we actually got
        if constexpr (checkTypes) {
            if (ret) {
                // By using this instead of ExtractType, we avoid unboxing because the ultimate type in that case would depend on the
                // method in the first place
                auto* outType = ExtractIndependentType<TOut>();
                if (outType) {
                    auto* retType = ExtractType(ret);
                    if (!IsConvertible(outType, retType, false)) {
                        logger.warning("User requested TOut %s does not match the method's return object of type %s!",
                            TypeGetSimpleName(outType), TypeGetSimpleName(retType));
                    }
                }
            }
        }

        if (exp) {
            logger.error("%s: Failed with exception: %s", il2cpp_functions::method_get_name(method),
                il2cpp_utils::ExceptionToString(exp).c_str());
            return ::std::nullopt;
        }

        return FromIl2CppObject<TOut>(ret);
    }

    template<class TOut = Il2CppObject*, bool checkTypes = true, class T, class... TArgs>
    // Runs a (static) method with the specified method name, with return type TOut.
    // Checks the types of the parameters against the candidate methods.
    ::std::enable_if_t<!::std::is_convertible_v<T, ::std::string_view>, ::std::optional<TOut>>
    RunMethod(T&& classOrInstance, ::std::string_view methodName, TArgs&& ...params) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunMethod");
        if constexpr (checkTypes) {
            auto types = ExtractTypes(params...);
            if (types.size() != sizeof...(TArgs)) {
                logger.warning("ExtractTypes for method %s failed!", methodName.data());
                return ::std::nullopt;
            }
            auto* method = RET_NULLOPT_UNLESS(logger, FindMethod(classOrInstance, NoArgClass<TOut>(), methodName, types));
            return RunMethod<TOut, true>(classOrInstance, method, params...);
        }
        // TODO: We should probably change how FindMethod is called/isn't called
        // At the moment, however, if you call RunMethod with checkTypes = false, it will perform an FindMethodUnsafe instead
        // But it will return nullptr if there are multiple matches
        else {
            if constexpr (std::is_same_v<T, Il2CppClass*>) {
                auto* method = RET_NULLOPT_UNLESS(logger, FindMethodUnsafe(classOrInstance, methodName, sizeof...(TArgs)));
                return RunMethod<TOut, false>(classOrInstance, method, params...);
            } else {
                auto* method = RET_NULLOPT_UNLESS(logger, FindMethodUnsafe(ExtractClass(classOrInstance), methodName, sizeof...(TArgs)));
                return RunMethod<TOut, false>(classOrInstance, method, params...);
            }
        }
    }

    template<class TOut = Il2CppObject*, bool checkTypes = true, class... TArgs>
    // Runs a static method with the specified method name and arguments, on the class with the specified namespace and class name.
    // The method also has return type TOut.
    ::std::optional<TOut> RunMethod(::std::string_view nameSpace, ::std::string_view klassName, ::std::string_view methodName, TArgs&& ...params) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunMethod");
        auto* klass = RET_NULLOPT_UNLESS(logger, GetClassFromName(nameSpace, klassName));
        return RunMethod<TOut, checkTypes>(klass, methodName, params...);
    }

    /// @brief Instantiates a generic MethodInfo* from the provided Il2CppClasses and invokes it.
    /// @n This method will not crash.
    /// @tparam TOut The return type of the method to invoke
    /// @tparam T Instance type
    /// @tparam TArgs Parameters to RunMethod
    /// @param instance Instance to RunMethod, or null/class
    /// @param info Generic MethodInfo* to invoke
    /// @param genTypes Types to instantiate the generic MethodInfo* with
    /// @param params Parameters to RunMethod
    template<class TOut = Il2CppObject*, class T, class... TArgs>
    ::std::optional<TOut> RunGenericMethod(T&& instance, const MethodInfo* info, ::std::vector<Il2CppClass*> genTypes, TArgs&& ...params) noexcept {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunGenericMethod");
        auto* createdMethod = RET_NULLOPT_UNLESS(logger, MakeGenericMethod(info, genTypes));
        return RunMethod<TOut, false>(instance, createdMethod, params...);
    }
    
    template<class TOut = Il2CppObject*, class T, class... TArgs>
    ::std::optional<TOut> RunGenericMethod(T&& classOrInstance, ::std::string_view methodName, ::std::vector<Il2CppClass*> genTypes, TArgs&& ...params) noexcept {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunGenericMethod");
        auto types = ExtractTypes(params...);
        if (types.size() != sizeof...(TArgs)) {
            logger.warning("ExtractTypes for method %s failed!", methodName.data());
            return ::std::nullopt;
        }

        auto* info = RET_NULLOPT_UNLESS(logger, FindMethod(classOrInstance, NoArgClass<TOut>(), methodName, genTypes, types));
        return RunGenericMethod<TOut>(classOrInstance, info, genTypes, params...);
    }
    template<class TOut = Il2CppObject*, class... TArgs>
    // Runs a static generic method with the specified method name and arguments, on the class with the specified namespace and class name.
    // The method also has return type TOut.
    ::std::optional<TOut> RunGenericMethod(::std::string_view nameSpace, ::std::string_view klassName, ::std::string_view methodName, ::std::vector<Il2CppClass*> genTypes, TArgs&& ...params) noexcept {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunGenericMethod");
        auto* klass = RET_NULLOPT_UNLESS(logger, GetClassFromName(nameSpace, klassName));
        return RunGenericMethod<TOut>(klass, methodName, genTypes, params...);
    }

    template<class TOut = Il2CppObject*, class T, class... TArgs>
    // Runs a (static) method with the specified method name and number of arguments, with return type TOut.
    // DOES NOT PERFORM TYPE CHECKING.
    ::std::enable_if_t<::std::is_base_of_v<Il2CppClass, T> || ::std::is_base_of_v<Il2CppObject, T>, ::std::optional<TOut>>
    RunMethodUnsafe(T* classOrInstance, ::std::string_view methodName, TArgs&& ...params) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunMethodUnsafe");
        RET_NULLOPT_UNLESS(logger, classOrInstance);
        auto* method = RET_NULLOPT_UNLESS(logger, FindMethodUnsafe(classOrInstance, methodName, sizeof...(TArgs)));
        return RunMethod<TOut, false>(classOrInstance, method, params...);
    }

    template<class TOut = Il2CppObject*, class... TArgs>
    // Runs a static method with the specified method name and arguments, on the class with the specified namespace and class name.
    // The method also has return type TOut.
    // DOES NOT PERFORM TYPE CHECKING.
    ::std::optional<TOut> RunMethodUnsafe(::std::string_view nameSpace, ::std::string_view klassName, ::std::string_view methodName, TArgs&& ...params) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("RunMethodUnsafe");
        auto* klass = RET_NULLOPT_UNLESS(logger, GetClassFromName(nameSpace, klassName));
        return RunMethodUnsafe<TOut>(klass, methodName, params...);
    }

    template<typename TOut = Il2CppObject*, typename... TArgs>
    // Creates a new object of the given class using the given constructor parameters
    // Will only run a .ctor whose parameter types match the given arguments.
    ::std::optional<TOut> New(Il2CppClass* klass, TArgs&& ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("New");
        il2cpp_functions::Init();

        // object_new call
        Il2CppObject* obj = RET_NULLOPT_UNLESS(logger, il2cpp_functions::object_new(klass));
        // runtime_invoke constructor with right type(s) of arguments, return null if constructor errors
        RET_NULLOPT_UNLESS(logger, RunMethod(obj, ".ctor", args...));
        return FromIl2CppObject<TOut>(obj);
    }

    template<typename TOut = Il2CppObject*, typename... TArgs>
    // Creates a new object of the given class using the given constructor parameters
    // DOES NOT PERFORM ARGUMENT TYPE CHECKING! Uses the first .ctor with the right number of parameters it sees.
    ::std::optional<TOut> NewUnsafe(const Il2CppClass* klass, TArgs&& ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("NewUnsafe");
        il2cpp_functions::Init();

        // object_new call
        Il2CppObject* obj = RET_NULLOPT_UNLESS(logger, il2cpp_functions::object_new(klass));
        // runtime_invoke constructor with right number of args, return null if constructor errors
        RET_NULLOPT_UNLESS(logger, RunMethodUnsafe(obj, ".ctor", args...));
        return FromIl2CppObject<TOut>(obj);
    }

    template<typename TOut = Il2CppObject*, typename... TArgs>
    // Creates a new object of the returned type using the given constructor parameters
    // Will only run a .ctor whose parameter types match the given arguments.
    ::std::enable_if_t<(... && (!::std::is_convertible_v<Il2CppClass*, TArgs> && !::std::is_convertible_v<TArgs, ::std::string_view>)),
    ::std::optional<TOut>> New(TArgs&& ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("New");
        auto* klass = RET_NULLOPT_UNLESS(logger, (NoArgClass<TOut, true>()));
        return New<TOut>(klass, args...);
    }

    template<typename TOut = Il2CppObject*, typename... TArgs>
    // Creates a new object of the class with the given nameSpace and className using the given constructor parameters.
    // Will only run a .ctor whose parameter types match the given arguments.
    ::std::optional<TOut> New(::std::string_view nameSpace, ::std::string_view className, TArgs&& ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("New");
        auto* klass = RET_0_UNLESS(logger, GetClassFromName(nameSpace, className));
        return New<TOut>(klass, args...);
    }

    template<typename TOut = Il2CppObject*, typename... TArgs>
    // Creates a new object of the class with the given nameSpace and className using the given constructor parameters.
    // DOES NOT PERFORM ARGUMENT TYPE CHECKING! Uses the first .ctor with the right number of parameters it sees.
    ::std::optional<TOut> NewUnsafe(::std::string_view nameSpace, ::std::string_view className, TArgs* ...args) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("NewUnsafe");
        auto* klass = RET_0_UNLESS(logger, GetClassFromName(nameSpace, className));
        return NewUnsafe<TOut>(klass, args...);
    }
}

#endif

#pragma once
#include "typedefs.h"
#include "logging.hpp"
#include "il2cpp-type-check.hpp"
#include "il2cpp-functions.hpp"
#include "il2cpp-utils-methods.hpp"

namespace il2cpp_utils {

    std::string GenericClassStandardName(Il2CppGenericClass* genClass);
    // Some parts provided by zoller27osu
    // Logs information about the given Il2CppClass* as log(DEBUG)
    void LogClass(LoggerContextObject& logger, Il2CppClass* klass, bool logParents = false) noexcept;

    // Logs all classes (from every namespace) that start with the given prefix
    // WARNING: THIS FUNCTION IS VERY SLOW. ONLY USE THIS FUNCTION ONCE AND WITH A FAIRLY SPECIFIC PREFIX!
    void LogClasses(LoggerContextObject& logger, ::std::string_view classPrefix, bool logParents = false) noexcept;

    // Gets the System.Type Il2CppObject* (actually an Il2CppReflectionType*) for an Il2CppClass*
    Il2CppReflectionType* GetSystemType(const Il2CppClass* klass);
    Il2CppReflectionType* GetSystemType(const Il2CppType* typ);

    // Gets the System.Type Il2CppObject* (actually an Il2CppReflectionType*) for the class with the given namespace and name
    Il2CppReflectionType* GetSystemType(::std::string_view nameSpace, ::std::string_view className);

    // Gets the standard class name of an Il2CppClass*
    ::std::string ClassStandardName(const Il2CppClass* klass, bool generics = true);

    // Gets a C# name of a type
    const char* TypeGetSimpleName(const Il2CppType* type);

    // "Calling" this gives a compile-time warning (if warnings from this header are enabled)
    template<class T>
    [[deprecated]]void a_lack_of_no_arg_class_for([[maybe_unused]]::std::string_view s) {};

    template<typename T>
    Il2CppClass* ExtractClass(T&& arg) {
        using Dt = ::std::decay_t<T>;
        using arg_class = il2cpp_type_check::il2cpp_arg_class<Dt>;
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("ExtractClass");
        Il2CppClass* klass = arg_class::get(arg);
        logger.error("Failed to determine class! Tips: instead of nullptr, pass the Il2CppType* or Il2CppClass* of the argument instead!");
        return THROW_OR_RET_NULL(logger, klass);
    }

    template<class T, bool ResultRequired = false>
    Il2CppClass* NoArgClass() {
        // TODO: change ifndef HAS_CODEGEN to 'if compile warnings are not errors'?
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("NoArgClass");
        #ifndef HAS_CODEGEN
        using arg_class = il2cpp_type_check::il2cpp_no_arg_class<T>;
        if constexpr (!has_get<arg_class>) {
            if constexpr (ResultRequired) {
                static_assert(false_t<arg_class>, "il2cpp-type-check.hpp could not deduce what C# type your type represents");
            } else {
                a_lack_of_no_arg_class_for<T>("please tell il2cpp-type-check.hpp what C# type your type represents");
                THROW_OR_RET_NULL(logger, false);
            }
        } else
        #endif
        if constexpr (ResultRequired) {
            return THROW_OR_RET_NULL(logger, il2cpp_type_check::il2cpp_no_arg_class<T>::get());
        } else {
            return il2cpp_type_check::il2cpp_no_arg_class<T>::get();
        }
    }

    template<typename T>
    const Il2CppType* ExtractType(T&& arg) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("ExtractType");
        const Il2CppType* typ = il2cpp_type_check::il2cpp_arg_type<T>::get(arg);
        if (!typ)
            logger.error("ExtractType: failed to determine type! Tips: instead of nullptr, pass the Il2CppType* or Il2CppClass* of the argument instead!");
        return typ;
    }

    // Like ExtractType, but only returns an Il2CppType* if it can be extracted without an instance of T.
    template<class T>
    const Il2CppType* ExtractIndependentType() {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("ExtractIndependentType");
        auto* klass = RET_0_UNLESS(logger, NoArgClass<T>());
        il2cpp_functions::Init();
        return il2cpp_functions::class_get_type(klass);
    }

    inline auto ExtractTypes() {
        return ::std::vector<const Il2CppType*>();
    }

    template<typename T, typename... TArgs>
    ::std::vector<const Il2CppType*> ExtractTypes(T&& arg, TArgs&&... args) {
        auto* tFirst = ExtractType(arg);
        auto tOthers = ExtractTypes(args...);
        if (tFirst) tOthers.insert(tOthers.begin(), tFirst);
        return tOthers;
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

    // Adds the given TypeDefinitionIndex to the class hash table of a given image
    // Mainly used in LogClasses
    void AddTypeToNametoClassHashTable(const Il2CppImage* img, TypeDefinitionIndex index);

    // Adds the given nested types of the namespaze, parentName, and klass to the hastable
    // Mainly used in AddTypeToNametoClassHashTable
    void AddNestedTypesToNametoClassHashTable(Il2CppNameToTypeDefinitionIndexHashTable* hashTable, const char *namespaze, const ::std::string& parentName, Il2CppClass *klass);

    // Adds the given nested types of typeDefinition to the class hash table of a given image
    // Mainly used in AddTypeToNametoClassHashTable
    void AddNestedTypesToNametoClassHashTable(const Il2CppImage* img, const Il2CppTypeDefinition* typeDefinition);
}
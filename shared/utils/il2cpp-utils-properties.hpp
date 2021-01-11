#ifndef IL2CPP_UTILS_PROPERTIES
#define IL2CPP_UTILS_PROPERTIES

#pragma pack(push)

#include "il2cpp-functions.hpp"
#include <optional>
#include "il2cpp-utils-methods.hpp"

namespace il2cpp_utils {
    // Created by zoller27osu
    // Logs information about the given PropertyInfo* as log(DEBUG)
    void LogProperty(LoggerContextObject& logger, const PropertyInfo* field);

    // Created by zoller27osu
    // Calls LogProperty on all properties in the given class
    void LogProperties(LoggerContextObject& logger, Il2CppClass* klass, bool logParents = false);

    // Returns the PropertyInfo for the property of the given class with the given name
    // Created by zoller27osu
    const PropertyInfo* FindProperty(Il2CppClass* klass, ::std::string_view propertyName);
    // Wrapper for FindProperty taking a namespace and class name in place of an Il2CppClass*
    const PropertyInfo* FindProperty(::std::string_view nameSpace, ::std::string_view className, ::std::string_view propertyName);
    // Wrapper for FindProperty taking an instance to extract the Il2CppClass* from
    template<class T>
    const PropertyInfo* FindProperty(T&& instance, ::std::string_view propertyName) {
        static auto& logger = getLogger();
        auto* klass = RET_0_UNLESS(logger, ExtractClass(instance));
        return FindProperty(klass, propertyName);
    }

    template<class TOut = Il2CppObject*, bool checkTypes = true, class T>
    // Gets a value from the given object instance, and PropertyInfo, with return type TOut.
    // Assumes a static property if instance == nullptr
    ::std::optional<TOut> GetPropertyValue(T&& classOrInstance, const PropertyInfo* prop) {
        static auto& logger = getLogger();
        il2cpp_functions::Init();
        RET_NULLOPT_UNLESS(logger, prop);

        auto* getter = RET_NULLOPT_UNLESS(logger, il2cpp_functions::property_get_get_method(prop));
        return RunMethod<TOut, checkTypes>(classOrInstance, getter);
    }

    template<typename TOut = Il2CppObject*, bool checkTypes = true, typename T>
    // Gets the value of the property with the given name from the given class or instance, and returns it as TOut.
    ::std::optional<TOut> GetPropertyValue(T&& classOrInstance, ::std::string_view propName) {
        static auto& logger = getLogger();
        auto* prop = RET_NULLOPT_UNLESS(logger, FindProperty(classOrInstance, propName));
        return GetPropertyValue<TOut, checkTypes>(classOrInstance, prop);
    }

    template<typename TOut = Il2CppObject*, bool checkTypes = true>
    // Gets the value of the static property with the given name from the class with the given nameSpace and className.
    ::std::optional<TOut> GetPropertyValue(::std::string_view nameSpace, ::std::string_view className, ::std::string_view propName) {
        static auto& logger = getLogger();
        auto* klass = RET_0_UNLESS(logger, GetClassFromName(nameSpace, className));
        return GetPropertyValue<TOut, checkTypes>(klass, propName);
    }

    // Sets the value of a given property, given an object instance or class and PropertyInfo
    // Returns false if it fails.
    // Only static properties work with classOrInstance == nullptr
    template<bool checkTypes = true, class T, class TArg>
    bool SetPropertyValue(T& classOrInstance, const PropertyInfo* prop, TArg&& value) {
        static auto& logger = getLogger();
        il2cpp_functions::Init();
        RET_0_UNLESS(logger, prop);

        auto* setter = RET_0_UNLESS(logger, il2cpp_functions::property_get_set_method(prop));
        return (bool)RunMethod<Il2CppObject*, checkTypes>(classOrInstance, setter, value);
    }

    // Sets the value of a given property, given an object instance or class and property name
    // Returns false if it fails
    template<bool checkTypes = true, class T, class TArg>
    bool SetPropertyValue(T& classOrInstance, ::std::string_view propName, TArg&& value) {
        static auto& logger = getLogger();
        auto* prop = RET_0_UNLESS(logger, FindProperty(classOrInstance, propName));
        return SetPropertyValue<checkTypes>(classOrInstance, prop, value);
    }

    // Sets the value of a given static property, given the class' namespace and name, and the property name and value.
    // Returns false if it fails
    template<bool checkTypes = true, class TArg>
    bool SetPropertyValue(::std::string_view nameSpace, ::std::string_view className, ::std::string_view propName, TArg&& value) {
        static auto& logger = getLogger();
        auto* klass = RET_0_UNLESS(logger, GetClassFromName(nameSpace, className));
        return SetPropertyValue<checkTypes>(klass, propName, value);
    }
}

#pragma pack(pop)

#endif

#include <utility>  // for std::pair
#include "hashing.hpp"
#include "../../shared/utils/il2cpp-utils.hpp"
#include "../../shared/utils/utils.h"
#include "../../shared/utils/il2cpp-functions.hpp"
#include "../../shared/utils/alphanum.hpp"
#include "../../shared/utils/typedefs.h"
#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace std
{
    // From https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
    template<class T> void hash_combine(size_t& seed, T v) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Let a "sequence" type be any type that supports .size() and iteration and whose elements are hashable and support !=.
    // Calculates a hash for a sequence.
    template<class T> std::size_t hash_seq(T const& seq) noexcept {
        std::size_t seed = seq.size();
        for(auto i: seq) {
            hash_combine(seed, i);
        }
        return seed;
    }

    // Specializes std::hash for std::vector
    template<class T> struct hash<std::vector<T>> {
        std::size_t operator()(std::vector<T> const& seq) const noexcept {
            return hash_seq(seq);
        }
    };
}

// Please see comments in il2cpp-utils.hpp
// TODO: Make this into a static class
namespace il2cpp_utils {
    // decltype is static so this should make the hashmap use as little memory as needed regardless of il2cpp changes
    static std::unordered_map<std::pair<const Il2CppClass*, std::pair<std::string, decltype(MethodInfo::parameters_count)>>, const MethodInfo*, hash_pair_3> classesNamesToMethodsCache;

    typedef std::pair<std::string, std::vector<const Il2CppType*>> classesNamesTypesInnerPairType;
    static std::unordered_map<std::pair<const Il2CppClass*, classesNamesTypesInnerPairType>, const MethodInfo*, hash_pair_3> classesNamesTypesToMethodsCache;
    static std::unordered_map<std::pair<const Il2CppClass*, std::string>, FieldInfo*, hash_pair> classesNamesToFieldsCache;
    static std::unordered_map<std::pair<const Il2CppClass*, std::string>, const PropertyInfo*, hash_pair> classesNamesToPropertiesCache;
    // Maximum length of characters of an exception message - 1
    #define EXCEPTION_MESSAGE_SIZE 4096

    // Returns a legible string from an Il2CppException*
    std::string ExceptionToString(Il2CppException* exp) {
        il2cpp_functions::Init();

        char msg[EXCEPTION_MESSAGE_SIZE];
        il2cpp_functions::format_exception(exp, msg, EXCEPTION_MESSAGE_SIZE);
        // auto exception_message = csstrtostr(exp->message);
        // return to_utf8(exception_message);
        return msg;
    }

    std::vector<const Il2CppType*> ClassVecToTypes(std::vector<const Il2CppClass*> seq) {
        il2cpp_functions::Init();

        std::vector<const Il2CppType*> types(seq.size());
        std::transform(seq.begin(), seq.end(), types.begin(), il2cpp_functions::class_get_type_const);
        return types;
    }

    const Il2CppType* MakeRef(const Il2CppType* type) {
        if (type->byref) return type;
        il2cpp_functions::Init();
        // could use Class::GetByrefType instead of &->this_arg but it does the same thing
        return &il2cpp_functions::class_from_il2cpp_type(type)->this_arg;
    }

    const Il2CppType* UnRef(const Il2CppType* type) {
        if (!type->byref) return type;
        il2cpp_functions::Init();
        return il2cpp_functions::class_get_type(il2cpp_functions::class_from_il2cpp_type(type));
    }

    bool IsConvertible(const Il2CppType* to, const Il2CppType* from, bool asArgs) {
        RET_0_UNLESS(to);
        RET_0_UNLESS(from);
        Logger::get().debug("IsConvertible(%s, %s)", TypeGetSimpleName(to), TypeGetSimpleName(from));
        if (asArgs) {
            if (to->byref) {
                if (!from->byref) {
                    Logger::get().debug("IsConvertible: to (%s, %p) is ref/out while from (%s, %p) is not. Not convertible.",
                        TypeGetSimpleName(to), to, TypeGetSimpleName(from), from);
                    return false;
                } else {
                    Logger::get().debug("IsConvertible: to (%s, %p) and from (%s, %p) are both ret/out. May be convertible.",
                        TypeGetSimpleName(to), to, TypeGetSimpleName(from), from);
                }
            }
        }
        il2cpp_functions::Init();
        auto classTo = il2cpp_functions::class_from_il2cpp_type(to);
        auto classFrom = il2cpp_functions::class_from_il2cpp_type(from);
        Logger::get().debug("IsConvertible: class_is_assignable_from(%s, %s)",
                ClassStandardName(classTo).c_str(), ClassStandardName(classFrom).c_str());
        bool ret = (to->type == IL2CPP_TYPE_MVAR) || il2cpp_functions::class_is_assignable_from(classTo, classFrom);
        if (!ret) {
            Logger::get().debug("IsConvertible: class_is_assignable_from(%s, %s) returned %s",
                ClassStandardName(classTo).c_str(), ClassStandardName(classFrom).c_str(), ret ? "true" : "false");
            if (il2cpp_functions::class_is_enum(classTo)) {
                Logger::get().debug("IsConvertible: but classTo is enum! Comparing against class_enum_basetype.");
                ret = IsConvertible(il2cpp_functions::class_enum_basetype(classTo), from);
            }
        }
        return ret;
    }

    bool ParameterMatch(const MethodInfo* method, std::vector<Il2CppClass*> genTypes, std::vector<const Il2CppType*> argTypes) {
        il2cpp_functions::Init();
        if (method->parameters_count != argTypes.size()) {
            return false;
        }
        auto genCount = (method->is_generic && !method->is_inflated) ? method->genericContainer->type_argc : 0;
        if ((size_t)genCount != genTypes.size()) {
            Logger::get().warning("Potential method match had wrong number of generics %i (expected %i)",
                genCount, genTypes.size());
            return false;
        }
        // TODO: supply boolStrictMatch and use type_equals instead of IsConvertible if supplied?
        for (decltype(method->parameters_count) i = 0; i < method->parameters_count; i++) {
            auto* paramType = method->parameters[i].parameter_type;
            if (paramType->type == IL2CPP_TYPE_MVAR) {
                auto genIdx = paramType->data.genericParameterIndex - method->genericContainer->genericParameterStart;
                if (genIdx < 0) {
                    Logger::get().warning("Extracted invalid genIdx %i from parameter %i", genIdx, i);
                } else if (genIdx >= genCount) {
                    Logger::get().warning("ParameterMatch was not supplied enough genTypes to determine type of parameter %i "
                        "(had %i, needed %i)!", i, genCount, genIdx);
                } else {
                    auto* klass = genTypes.at(genIdx);
                    paramType = (paramType->byref) ? &klass->this_arg : &klass->byval_arg;
                }
            }
            // TODO: just because two parameter lists match doesn't necessarily mean this is the best match...
            if (!(IsConvertible(paramType, argTypes.at(i)))) {
                return false;
            }
        }
        return true;
    }

    bool ParameterMatch(const MethodInfo* method, std::vector<const Il2CppType*> argTypes) {
        return ParameterMatch(method, {}, argTypes);
    }

    static std::unordered_map<Il2CppClass*, const char*> typeMap;

    // TODO: somehow append "out/ref " to the front if type->byref? Make a TypeStandardName?
    const char* TypeGetSimpleName(const Il2CppType* type) {
        il2cpp_functions::Init();

        if (typeMap.empty()) {
            typeMap[il2cpp_functions::defaults->boolean_class] = "bool";
            typeMap[il2cpp_functions::defaults->byte_class] = "byte";
            typeMap[il2cpp_functions::defaults->sbyte_class] = "sbyte";
            typeMap[il2cpp_functions::defaults->char_class] = "char";
            typeMap[il2cpp_functions::defaults->single_class] = "float";
            typeMap[il2cpp_functions::defaults->double_class] = "double";
            typeMap[il2cpp_functions::defaults->int16_class] = "short";
            typeMap[il2cpp_functions::defaults->uint16_class] = "ushort";
            typeMap[il2cpp_functions::defaults->int32_class] = "int";
            typeMap[il2cpp_functions::defaults->uint32_class] = "uint";
            typeMap[il2cpp_functions::defaults->int64_class] = "long";
            typeMap[il2cpp_functions::defaults->uint64_class] = "ulong";
            typeMap[il2cpp_functions::defaults->object_class] = "object";
            typeMap[il2cpp_functions::defaults->string_class] = "string";
            typeMap[il2cpp_functions::defaults->void_class] = "void";
        }
        auto p = typeMap.find(il2cpp_functions::class_from_il2cpp_type(type));
        if (p != typeMap.end()) {
            return p->second;
        } else {
            return il2cpp_functions::type_get_name(type);
        }
    }

    bool IsInterface(const Il2CppClass* klass) {
        return (klass->flags & TYPE_ATTRIBUTE_INTERFACE) ||
            (klass->byval_arg.type == IL2CPP_TYPE_VAR) ||
            (klass->byval_arg.type == IL2CPP_TYPE_MVAR);
    }

    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, std::string_view methodName, int argsCount) {
        il2cpp_functions::Init();
        RET_0_UNLESS(klass);

        // Check Cache
        auto innerPair = std::pair<std::string, decltype(MethodInfo::parameters_count)>(methodName, argsCount);
        auto key = std::pair<const Il2CppClass*, decltype(innerPair)>(klass, innerPair);
        auto itr = classesNamesToMethodsCache.find(key);
        if (itr != classesNamesToMethodsCache.end()) {
            return itr->second;
        }
        // Recurses through klass's parents
        auto methodInfo = il2cpp_functions::class_get_method_from_name(klass, methodName.data(), argsCount);
        if (!methodInfo) {
            Logger::get().error("could not find method %s with %i parameters in class '%s'!", methodName.data(), argsCount,
                ClassStandardName(klass).c_str());
            LogMethods(const_cast<Il2CppClass*>(klass), true);
        }
        classesNamesToMethodsCache.emplace(key, methodInfo);
        return methodInfo;
    }

    const MethodInfo* FindMethodUnsafe(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount) {
        return FindMethodUnsafe(GetClassFromName(nameSpace, className), methodName, argsCount);
    }

    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, std::string_view methodName, int argsCount) {
        il2cpp_functions::Init();
        auto klass = RET_0_UNLESS(il2cpp_functions::object_get_class(instance));
        return FindMethodUnsafe(klass, methodName, argsCount);
    }

    std::vector<Il2CppClass*> ClassesFrom(std::vector<Il2CppClass*> classes) { return classes; }
    std::vector<Il2CppClass*> ClassesFrom(std::vector<std::string_view> strings) {
        std::vector<Il2CppClass*> classes;
        for (size_t i = 0; i < strings.size() - 1; i += 2) {
            classes.push_back(GetClassFromName(strings[i].data(), strings[i+1].data()));
        }
        return classes;
    }
    std::vector<const Il2CppType*> TypesFrom(std::vector<const Il2CppType*> types) { return types; }
    std::vector<const Il2CppType*> TypesFrom(std::vector<const Il2CppClass*> classes) { return ClassVecToTypes(classes); }
    std::vector<const Il2CppType*> TypesFrom(std::vector<std::string_view> strings) {
        std::vector<const Il2CppType*> types;
        il2cpp_functions::Init();
        for (size_t i = 0; i < strings.size() - 1; i += 2) {
            auto clazz = GetClassFromName(strings[i].data(), strings[i+1].data());
            types.push_back(il2cpp_functions::class_get_type(clazz));
        }
        return types;
    }

    // Returns the MethodInfo for the method on the given class with the given name and types of arguments
    // Created by zoller27osu
    const MethodInfo* FindMethod(FindMethodInfo& info) {
        il2cpp_functions::Init();
        auto* klass = info.klass;
        RET_0_UNLESS(klass);

        // TODO: make cache work for generics (stratify by generics count?) and differing return types?
        // Check Cache
        auto innerPair = classesNamesTypesInnerPairType(info.name, info.argTypes);
        auto key = std::pair<Il2CppClass*, classesNamesTypesInnerPairType>(klass, innerPair);
        auto itr = classesNamesTypesToMethodsCache.find(key);
        if (itr != classesNamesTypesToMethodsCache.end()) {
            return itr->second;
        }

        void* myIter = nullptr;
        const MethodInfo* methodInfo = nullptr;  // basic match
        bool multipleBasicMatches = false;
        const MethodInfo* returnMatch = nullptr;
        bool multipleReturnMatches = false;
        const MethodInfo* perfectMatch = nullptr;
        bool multiplePerfectMatches = false;
        // Does NOT automatically recurse through klass's parents
        while (const MethodInfo* current = il2cpp_functions::class_get_methods(info.klass, &myIter)) {
            if ((info.name == current->name) && ParameterMatch(current, info.genTypes, info.argTypes)) {
                il2cpp_utils::LogMethod(current);
                if (info.returnType) {
                    auto* returnClass = il2cpp_functions::class_from_il2cpp_type(current->return_type);
                    if (info.returnType == returnClass) {
                        if (perfectMatch) {
                            multiplePerfectMatches = true;
                            Logger::get().error("Multiple perfect matches???");
                        }
                        else perfectMatch = current;
                    }
                    if (il2cpp_functions::class_is_assignable_from(info.returnType, returnClass)) {
                        if (returnMatch) {
                            Logger::get().debug("Multiple return type matches.");
                            multipleReturnMatches = true;
                        }
                        else returnMatch = current;
                    }
                }
                if (methodInfo)
                    multipleBasicMatches = true;
                else
                    methodInfo = current;
            }
        }
        if (!methodInfo && klass->parent && klass->parent != klass) {
            info.klass = klass->parent;
            methodInfo = FindMethod(info);
            info.klass = klass;
        }

        if (!multiplePerfectMatches && perfectMatch) {
            // It's important that these kinds of matches aren't added to the general cache
            return perfectMatch;
        } else if (!multipleReturnMatches && returnMatch) {
            return returnMatch;
        } else if (!methodInfo || multipleBasicMatches) {
            std::stringstream ss;
            ss << ((multipleBasicMatches) ? "found multiple matches for" : "could not find");
            ss << " method " << info.name << "(";
            bool first = true;
            for (auto t : info.argTypes) {
                if (!first) ss << ", ";
                first = false;
                ss << TypeGetSimpleName(t);
            }
            ss << ") in class '" << ClassStandardName(klass) << "'!";
            Logger::get().error("%s", ss.str().c_str());
            LogMethods(klass);
            return nullptr;
        }
        classesNamesTypesToMethodsCache.emplace(key, methodInfo);
        return methodInfo;
    }

    FieldInfo* FindField(Il2CppClass* klass, std::string_view fieldName) {
        il2cpp_functions::Init();
        RET_0_UNLESS(klass);

        // Check Cache
        auto key = std::pair<Il2CppClass*, std::string>(klass, fieldName);
        auto itr = classesNamesToFieldsCache.find(key);
        if (itr != classesNamesToFieldsCache.end()) {
            return itr->second;
        }
        auto field = il2cpp_functions::class_get_field_from_name(klass, fieldName.data());
        if (!field) {
            Logger::get().error("could not find field %s in class '%s'!", fieldName.data(), ClassStandardName(klass).c_str());
            LogFields(klass);
            if (klass->parent != klass) field = FindField(klass->parent, fieldName);
        }
        classesNamesToFieldsCache.emplace(key, field);
        return field;
    }

    const PropertyInfo* FindProperty(Il2CppClass* klass, std::string_view propName) {
        il2cpp_functions::Init();
        RET_0_UNLESS(klass);

        // Check Cache
        auto key = std::pair<Il2CppClass*, std::string>(klass, propName);
        auto itr = classesNamesToPropertiesCache.find(key);
        if (itr != classesNamesToPropertiesCache.end()) {
            return itr->second;
        }
        auto prop = il2cpp_functions::class_get_property_from_name(klass, propName.data());
        if (!prop) {
            Logger::get().error("could not find property %s in class '%s'!", propName.data(), ClassStandardName(klass).c_str());
            LogProperties(klass);
            if (klass->parent != klass) prop = FindProperty(klass->parent, propName);
        }
        classesNamesToPropertiesCache.emplace(key, prop);
        return prop;
    }

    const PropertyInfo* FindProperty(std::string_view nameSpace, std::string_view className, std::string_view propertyName) {
        return FindProperty(GetClassFromName(nameSpace, className), propertyName);
    }

    Il2CppClass* GetParamClass(const MethodInfo* method, int paramIdx) {
        auto type = RET_0_UNLESS(il2cpp_functions::method_get_param(method, paramIdx));
        return il2cpp_functions::class_from_il2cpp_type(type);
    }

    Il2CppClass* GetFieldClass(FieldInfo* field) {
        auto type = RET_0_UNLESS(il2cpp_functions::field_get_type(field));
        return il2cpp_functions::class_from_il2cpp_type(type);
    }

    Il2CppReflectionType* MakeGenericType(Il2CppReflectionType* gt, Il2CppArray* types) {
        il2cpp_functions::Init();

        auto runtimeType = RET_0_UNLESS(il2cpp_functions::defaults->runtimetype_class);
        auto makeGenericMethod = RET_0_UNLESS(FindMethodUnsafe(runtimeType, "MakeGenericType", 2));

        Il2CppException* exp = nullptr;
        void* params[] = {reinterpret_cast<void*>(gt), reinterpret_cast<void*>(types)};
        auto genericType = il2cpp_functions::runtime_invoke(makeGenericMethod, nullptr, params, &exp);
        if (exp) {
            Logger::get().error("il2cpp_utils: MakeGenericType: Failed with exception: %s", ExceptionToString(exp).c_str());
            return nullptr;
        }
        return reinterpret_cast<Il2CppReflectionType*>(genericType);
    }

    const MethodInfo* MakeGenericMethod(const MethodInfo* info, std::vector<Il2CppClass*> types) noexcept {
        il2cpp_functions::Init();
        // Ensure it exists and is generic
        RET_0_UNLESS(info);
        RET_0_UNLESS(il2cpp_functions::method_is_generic(info));
        static auto* typeClass = RET_0_UNLESS(il2cpp_functions::defaults->systemtype_class);
        // Create the Il2CppReflectionMethod* from the MethodInfo* using the MethodInfo's type
        auto* infoObj = il2cpp_functions::method_get_object(info, nullptr);
        if (!infoObj) {
            Logger::get().error("il2cpp_utils: MakeGenericMethod: Failed to get MonoMethod from MethodInfo: %p", info);
            return nullptr;
        }
        // Populate generic parameters into array
        auto* arr = reinterpret_cast<Array<Il2CppReflectionType*>*>(il2cpp_functions::array_new(typeClass, types.size()));
        if (!arr) {
            Logger::get().error("il2cpp_utils: MakeGenericMethod: Failed to create array of length: %u", types.size());
            return nullptr;
        }
        int i = 0;
        for (auto* klass : types) {
            auto* typeObj = GetSystemType(klass);
            if (!typeObj) {
                Logger::get().error("il2cpp_utils: MakeGenericMethod: Failed to get type object from class: %s", il2cpp_functions::class_get_name_const(klass));
                return nullptr;
            }
            arr->values[i] = typeObj;
            i++;
        }
        // Call instance function on infoObj to MakeGeneric
        const auto* returnedInfoObj = RET_0_UNLESS(il2cpp_utils::RunMethod<Il2CppReflectionMethod*>(infoObj, "MakeGenericMethod", arr));
        if (!returnedInfoObj) {
            Logger::get().error("il2cpp_utils: MakeGenericMethod: Failed to get Il2CppReflectionMethod from MakeGenericMethod!");
            return nullptr;
        }
        // Get MethodInfo* back from generic instantiated method
        const auto* inflatedInfo = il2cpp_functions::method_get_from_reflection(returnedInfoObj);
        if (!inflatedInfo) {
            Logger::get().error("il2cpp_utils: MakeGenericMethod: Got null MethodInfo* from Il2CppReflectionMethod: %p", returnedInfoObj);
        }
        // Return method to be invoked by caller
        return inflatedInfo;
    }

    Il2CppReflectionType* GetSystemType(const Il2CppType* typ) {
        il2cpp_functions::Init();
        return reinterpret_cast<Il2CppReflectionType*>(il2cpp_functions::type_get_object(typ));
    }

    Il2CppReflectionType* GetSystemType(const Il2CppClass* klass) {
        il2cpp_functions::Init();
        RET_0_UNLESS(klass);

        auto* typ = il2cpp_functions::class_get_type_const(klass);
        return GetSystemType(typ);
    }

    Il2CppReflectionType* GetSystemType(std::string_view nameSpace, std::string_view className) {
        return GetSystemType(il2cpp_utils::GetClassFromName(nameSpace, className));
    }

    void LogMethod(const MethodInfo* method) {
        il2cpp_functions::Init();
        RET_V_UNLESS(method);

        auto flags = il2cpp_functions::method_get_flags(method, nullptr);
        std::stringstream flagStream;
        if (flags & METHOD_ATTRIBUTE_STATIC) flagStream << "static ";
        if (flags & METHOD_ATTRIBUTE_VIRTUAL) flagStream << "virtual ";
        if (flags & METHOD_ATTRIBUTE_ABSTRACT) flagStream << "abstract ";
        const auto& flagStrRef = flagStream.str();
        const char* flagStr = flagStrRef.c_str();
        auto* retType = il2cpp_functions::method_get_return_type(method);
        auto retTypeStr = TypeGetSimpleName(retType);
        auto methodName = il2cpp_functions::method_get_name(method);
        methodName = methodName ? methodName : "__noname__";
        std::stringstream paramStream;
        for (size_t i = 0; i < il2cpp_functions::method_get_param_count(method); i++) {
            if (i > 0) paramStream << ", ";
            auto* argType = il2cpp_functions::method_get_param(method, i);
            if (il2cpp_functions::type_is_byref(argType)) {
                paramStream << "out/ref ";
            }
            paramStream << TypeGetSimpleName(argType) << " ";
            auto name = il2cpp_functions::method_get_param_name(method, i);
            paramStream << (name ? name : "__noname__");
        }
        const auto& paramStrRef = paramStream.str();
        const char* paramStr = paramStrRef.c_str();
        // TODO: add <T> after methodName
        Logger::get().debug("%s%s %s(%s);", flagStr, retTypeStr, methodName, paramStr);
    }

    void LogField(FieldInfo* field) {
        il2cpp_functions::Init();
        RET_V_UNLESS(field);

        auto flags = il2cpp_functions::field_get_flags(field);
        const char* flagStr = (flags & FIELD_ATTRIBUTE_STATIC) ? "static " : "";
        auto* type = il2cpp_functions::field_get_type(field);
        auto typeStr = TypeGetSimpleName(type);
        auto name = il2cpp_functions::field_get_name(field);
        name = name ? name : "__noname__";
        auto offset = il2cpp_functions::field_get_offset(field);

        Logger::get().debug("%s%s %s; // 0x%lx, flags: 0x%.4X", flagStr, typeStr, name, offset, flags);
    }

    void LogFields(Il2CppClass* klass, bool logParents) {
        il2cpp_functions::Init();
        RET_V_UNLESS(klass);

        void* myIter = nullptr;
        FieldInfo* field;
        if (klass->name) il2cpp_functions::Class_Init(klass);
        if (logParents) Logger::get().info("class name: %s", ClassStandardName(klass).c_str());

        Logger::get().debug("field_count: %i", klass->field_count);
        while ((field = il2cpp_functions::class_get_fields(klass, &myIter))) {
            LogField(field);
        }
        usleep(100);
        if (logParents && klass->parent && klass->parent != klass) {
            LogFields(klass->parent, logParents);
        }
    }

    void LogProperty(const PropertyInfo* prop) {
        il2cpp_functions::Init();
        RET_V_UNLESS(prop);

        auto flags = il2cpp_functions::property_get_flags(prop);
        const char* flagStr = (flags & FIELD_ATTRIBUTE_STATIC) ? "static " : "";
        auto name = il2cpp_functions::property_get_name(prop);
        name = name ? name : "__noname__";
        auto* getter = il2cpp_functions::property_get_get_method(prop);
        auto getterName = getter ? il2cpp_functions::method_get_name(getter) : "";
        auto* setter = il2cpp_functions::property_get_set_method(prop);
        auto setterName = setter ? il2cpp_functions::method_get_name(setter) : "";
        const Il2CppType* type = nullptr;
        if (getter) {
            type = il2cpp_functions::method_get_return_type(getter);
        } else if (setter) {
            type = il2cpp_functions::method_get_param(setter, 0);
        }
        auto typeStr = type ? TypeGetSimpleName(type) : "?type?";

        Logger::get().debug("%s%s %s { %s; %s; }; // flags: 0x%.4X", flagStr, typeStr, name, getterName, setterName, flags);
    }

    void LogProperties(Il2CppClass* klass, bool logParents) {
        il2cpp_functions::Init();
        RET_V_UNLESS(klass);

        void* myIter = nullptr;
        const PropertyInfo* prop;
        if (klass->name) il2cpp_functions::Class_Init(klass);
        if (logParents) Logger::get().info("class name: %s", ClassStandardName(klass).c_str());

        Logger::get().debug("property_count: %i", klass->property_count);
        while ((prop = il2cpp_functions::class_get_properties(klass, &myIter))) {
            LogProperty(prop);
        }
        usleep(100);
        if (logParents && klass->parent && klass->parent != klass) {
            LogProperties(klass->parent, logParents);
        }
    }

    void GenericsToStringHelper(Il2CppGenericClass* genClass, std::ostream& os) {
        auto genContext = &genClass->context;
        auto* genInst = genContext->class_inst;
        if (!genInst) {
            genInst = genContext->method_inst;
            if (genInst) Logger::get().warning("Missing class_inst! Trying method_inst?");
        }
        if (genInst) {
            os << "<";
            for (size_t i = 0; i < genInst->type_argc; i++) {
                auto typ = genInst->type_argv[i];
                if (i > 0) os << ", ";
                const char* typName = TypeGetSimpleName(typ);
                os << typName;
            }
            os << ">";
        } else {
            Logger::get().warning("context->class_inst missing for genClass!");
        }
    }

    std::string ClassStandardName(const Il2CppClass* klass, bool generics) {
        il2cpp_functions::Init();
        std::stringstream ss;
        const char* namespaze = il2cpp_functions::class_get_namespace(klass);
        auto* declaring = il2cpp_functions::class_get_declaring_type(klass);
        bool hasNamespace = (namespaze && namespaze[0] != '\0');
        if (!hasNamespace && declaring) {
            ss << ClassStandardName(declaring) << "/";
        } else {
            ss << namespaze << "::";
        }
        ss << il2cpp_functions::class_get_name(klass);

        if (generics) {
            il2cpp_functions::class_is_generic(klass);
            auto* genClass = klass->generic_class;
            if (genClass) {
                GenericsToStringHelper(genClass, ss);
            }
        }
        return ss.str();
    }

    std::string GenericClassStandardName(Il2CppGenericClass* genClass) {
        if (genClass->cached_class) {
            return ClassStandardName(genClass->cached_class);
        }
        if (genClass->typeDefinitionIndex != kTypeDefinitionIndexInvalid) {
            il2cpp_functions::Init();
            auto* klass = il2cpp_functions::GenericClass_GetClass(genClass);
            return ClassStandardName(klass);
        }
        return "?";
    }

    void LogMethods(Il2CppClass* klass, bool logParents) {
        RET_V_UNLESS(klass);

        if (klass->name) {
            il2cpp_functions::Init();
            il2cpp_functions::Class_Init(klass);
        }
        if (klass->method_count && !(klass->methods)) {
            Logger::get().warning("Class is valid and claims to have methods but ->methods is null! class name: %s", ClassStandardName(klass).c_str());
            return;
        }
        if (logParents) Logger::get().info("class name: %s", ClassStandardName(klass).c_str());

        Logger::get().debug("method_count: %i", klass->method_count);
        for (int i = 0; i < klass->method_count; i++) {
            if (klass->methods[i]) {
                Logger::get().debug("Method %i:", i);
                LogMethod(klass->methods[i]);
            } else {
                Logger::get().warning("Method: %i Does not exist!", i);
            }
        }
        usleep(100);  // 0.0001s
        if (logParents && klass->parent && (klass->parent != klass)) {
            LogMethods(klass->parent, logParents);
        }
    }

    static int indent = -1;
    static int maxIndent;
    std::unordered_set<Il2CppClass*> loggedClasses;
    void LogClass(Il2CppClass* klass, bool logParents) {
        il2cpp_functions::Init();
        RET_V_UNLESS(klass);

        if (loggedClasses.count(klass)) {
            Logger::get().debug("Already logged %p!", klass);
            return;
        }
        loggedClasses.insert(klass);

        RET_V_UNLESS(klass->klass == klass);  // otherwise, klass is likely NOT an Il2CppClass*!
        RET_V_UNLESS(klass->name);  // ditto

        indent++;
        // Note: il2cpp stops at GenericMetadata::MaximumRuntimeGenericDepth (which is 8)
        maxIndent = std::max(indent, maxIndent);

        bool methodInit = false;
        if (klass->name) {
            // Note: unless vm/Class.cpp is wrong, Class::Init always returns true
            il2cpp_functions::Class_Init(klass);
            if (klass->initialized_and_no_error) {
                methodInit = true;
            }
        }

        Logger::get().debug("%i ======================CLASS INFO FOR CLASS: %s======================", indent, ClassStandardName(klass).c_str());
        void* myIter = nullptr;
        if (!methodInit) {
            // log results of Class::Init
            Logger::get().warning("klass->initialized: %i, init_pending: %i, has_initialization_error: %i, initializationExceptionGCHandle: %Xll",
                    klass->initialized, klass->init_pending, klass->has_initialization_error, klass->initializationExceptionGCHandle);
            auto* m1 = il2cpp_functions::class_get_methods(klass, &myIter);  // attempt again to initialize the method data
            if (klass->method_count && !klass->methods) {
                Logger::get().error("Class::Init and class_get_methods failed to initialize klass->methods! class_get_methods returned: %p",
                    m1);
                if (m1) LogMethod(m1);
            }
        }

        Logger::get().debug("Pointer: %p", klass);
        Logger::get().debug("Type Token: %i", il2cpp_functions::class_get_type_token(klass));
        auto typeDefIdx = klass->generic_class ? klass->generic_class->typeDefinitionIndex : il2cpp_functions::MetadataCache_GetIndexForTypeDefinition(klass);
        Logger::get().debug("TypeDefinitionIndex: %i", typeDefIdx);
        // Repair the typeDefinition value if it was null but we found one
        if (!klass->typeDefinition && typeDefIdx > 0) klass->typeDefinition = il2cpp_functions::MetadataCache_GetTypeDefinitionFromIndex(typeDefIdx);
        Logger::get().debug("Type definition: %p", klass->typeDefinition);

        Logger::get().debug("Assembly Name: %s", il2cpp_functions::class_get_assemblyname(klass));

        auto* typ = il2cpp_functions::class_get_type(klass);
        if (typ) {
            Logger::get().debug("Type name: %s", il2cpp_functions::type_get_name(typ));
            if (auto* reflName = il2cpp_functions::Type_GetName(typ, IL2CPP_TYPE_NAME_FORMAT_REFLECTION)) {
                Logger::get().debug("Type reflection name: %s", reflName);
                il2cpp_functions::free(reflName);
            }
            Logger::get().debug("Fully qualifed type name: %s", il2cpp_functions::type_get_assembly_qualified_name(typ));
        }
        Logger::get().debug("Rank: %i", il2cpp_functions::class_get_rank(klass));
        Logger::get().debug("Flags: 0x%.8X", il2cpp_functions::class_get_flags(klass));
        Logger::get().debug("Event Count: %i", klass->event_count);
        Logger::get().debug("Method Count: %i", klass->method_count);
        Logger::get().debug("Is Generic: %i", il2cpp_functions::class_is_generic(klass));
        Logger::get().debug("Is Abstract: %i", il2cpp_functions::class_is_abstract(klass));

        // Some methods, such as GenericClass::GetClass, may not initialize all fields in Il2CppClass, and thus not meet all implicit contracts defined by the comments in Il2CppClass's struct definition.
        // But unless we're blind, the only method that sets is_generic on non-methods is MetadataCache::FromTypeDefinition. That method also contains the only assignment of genericContainerIndex.
        // Therefore, this code makes only the following assumptions:
        // 1. If is_generic is set, then genericContainerIndex was also intentionally set (even if it's 0) and is not -1 (invalid)
        // 2. Even if is_generic wasn't set, a positive genericContainerIndex was intentionally set that way and is a valid index.
        if (klass->is_generic || klass->genericContainerIndex > 0) {
            auto* genContainer = il2cpp_functions::MetadataCache_GetGenericContainerFromIndex(klass->genericContainerIndex);
            Logger::get().debug("genContainer: idx %i, ownerIndex: %i, is_method: %i", klass->genericContainerIndex, genContainer->ownerIndex, genContainer->is_method);
            if (genContainer->ownerIndex != typeDefIdx) {
                Logger::get().error("genContainer ownerIndex mismatch!");
            }
            for (int i = 0; i < genContainer->type_argc; i++) {
                auto genParamIdx = genContainer->genericParameterStart + i;
                auto* genParam = il2cpp_functions::MetadataCache_GetGenericParameterFromIndex(genParamIdx);
                if (genParam) {
                    Logger::get().debug("genParam #%i, idx %i: ownerIdx %i, name %s, num %i, flags (see "
                        "IL2CPP_GENERIC_PARAMETER_ATTRIBUTE_X in il2cpp-tabledefs.h) 0x%.2X", i, genParamIdx, genParam->ownerIndex,
                        il2cpp_functions::MetadataCache_GetStringFromIndex(genParam->nameIndex), genParam->num, genParam->flags);
                } else {
                    Logger::get().warning("genParam %i, idx %i: null", i, genParamIdx);
                }
            }
        } else {
            Logger::get().debug("genericContainerIndex: %i", klass->genericContainerIndex);
        }

        Logger::get().debug("%i =========METHODS=========", indent);
        LogMethods(klass);
        Logger::get().debug("%i =======END METHODS=======", indent);

        auto* declaring = il2cpp_functions::class_get_declaring_type(klass);
        Logger::get().debug("declaring type: %p (%s)", declaring, declaring ? ClassStandardName(declaring).c_str() : "");
        if (declaring && logParents) LogClass(declaring, logParents);
        auto* element = il2cpp_functions::class_get_element_class(klass);
        Logger::get().debug("element class: %p ('%s', self = %p)", element, element ? ClassStandardName(element).c_str() : "", klass);
        if (element && element != klass && logParents) LogClass(element, logParents);

        Logger::get().debug("%i =======PROPERTIES=======", indent);
        LogProperties(klass);
        Logger::get().debug("%i =====END PROPERTIES=====", indent);
        Logger::get().debug("%i =========FIELDS=========", indent);
        LogFields(klass);
        Logger::get().debug("%i =======END FIELDS=======", indent);

        auto* parent = il2cpp_functions::class_get_parent(klass);
        Logger::get().debug("parent: %p (%s)", parent, parent ? ClassStandardName(parent).c_str() : "");
        if (parent && logParents) LogClass(parent, logParents);
        Logger::get().debug("%i, ==================================================================================", indent);
        indent--;
    }

    static std::unordered_map<Il2CppClass*, std::map<std::string, Il2CppGenericClass*, doj::alphanum_less<std::string>>> classToGenericClassMap;
    void BuildGenericsMap() {
        il2cpp_functions::Init();
        auto* metadataReg = RET_V_UNLESS(*il2cpp_functions::s_Il2CppMetadataRegistrationPtr);
        Logger::get().debug("metadataReg: %p, offset = %lX", metadataReg, ((intptr_t)metadataReg) - getRealOffset(0));

        int uncached_class_count = 0;
        for (int i=0; i < metadataReg->genericClassesCount; i++) {
            Il2CppGenericClass* genClass = metadataReg->genericClasses[i];
            if (!genClass) continue;
            if (!(genClass->cached_class)) {
                uncached_class_count++;
            }
            std::string genClassName = GenericClassStandardName(genClass);

            auto* typeDefClass = il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex(genClass->typeDefinitionIndex);
            if (!typeDefClass) continue;

            classToGenericClassMap[typeDefClass][genClassName.c_str()] = genClass;
        }
        Logger::get().debug("uncached_class_count: %i (%f proportion of total)", uncached_class_count, uncached_class_count * 1.0 / metadataReg->genericClassesCount);
    }

    void LogClasses(std::string_view classPrefix, bool logParents) {
        il2cpp_functions::Init();
        BuildGenericsMap();

        // Begin prefix matching
        std::map<std::string, Il2CppClass*, doj::alphanum_less<std::string>> matches;
        // Get il2cpp domain
        auto* dom = il2cpp_functions::domain_get();
        // Get all il2cpp assemblies
        size_t size;
        auto** assembs = il2cpp_functions::domain_get_assemblies(dom, &size);

        for (size_t i = 0; i < size; ++i) {
            // Get image for each assembly
            if (!assembs[i]) {
                Logger::get().warning("Assembly %zu was null! Skipping.", i);
                continue;
            }
            Logger::get().debug("Scanning assembly \"%s\"", assembs[i]->aname.name);
            auto* img = il2cpp_functions::assembly_get_image(assembs[i]);
            if (!img) {
                Logger::get().warning("Assembly's image was null! Skipping.");
                continue;
            }

            if (img->nameToClassHashTable == nullptr) {
                Logger::get().debug("Assembly's nameToClassHashTable is empty. Populating it instead.");

                img->nameToClassHashTable = new Il2CppNameToTypeDefinitionIndexHashTable();
                for (uint32_t index = 0; index < img->typeCount; index++) {
                    TypeDefinitionIndex typeIndex = img->typeStart + index;
                    AddTypeToNametoClassHashTable(img, typeIndex);
                }

                for (uint32_t index = 0; index < img->exportedTypeCount; index++) {
                    auto typeIndex = il2cpp_functions::MetadataCache_GetExportedTypeFromIndex(img->exportedTypeStart + index);
                    if (typeIndex != kTypeIndexInvalid)
                        AddTypeToNametoClassHashTable(img, typeIndex);
                }
            }

            for (auto itr = img->nameToClassHashTable->begin(); itr != img->nameToClassHashTable->end(); ++itr) {
                // ->first is a KeyWrapper(pair(namespaceName, className))
                // ->second is TypeDefinitionIndex
                if (strncmp(classPrefix.data(), itr->first.key.second, classPrefix.length()) == 0) {
                    // Starts with!
                    // Convert TypeDefinitionIndex --> class
                    auto klazz = il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex(itr->second);
                    matches[ClassStandardName(klazz)] = klazz;
                }
            }
        }

        usleep(1000);  // 0.001s
        Logger::get().debug("LogClasses:");
        for ( const auto &pair : matches ) {
            LogClass(pair.second, logParents);
            indent = -1;
            for ( const auto &genPair : classToGenericClassMap[pair.second] ) {
                Logger::get().debug("%s", genPair.first.c_str());
            }
            usleep(1000);  // 0.001s
        }
        Logger::get().debug("LogClasses(\"%s\") is complete.", classPrefix.data());
        Logger::get().debug("maxIndent: %i", maxIndent);
    }

    void AddTypeToNametoClassHashTable(const Il2CppImage* img, TypeDefinitionIndex index) {
        il2cpp_functions::Init();
        const Il2CppTypeDefinition* typeDefinition = il2cpp_functions::MetadataCache_GetTypeDefinitionFromIndex(index);
        // don't add nested types
        if (typeDefinition->declaringTypeIndex != kTypeIndexInvalid)
            return;

        if (img != il2cpp_functions::get_corlib())
            AddNestedTypesToNametoClassHashTable(img, typeDefinition);

        img->nameToClassHashTable->insert(std::make_pair(std::make_pair(il2cpp_functions::MetadataCache_GetStringFromIndex(typeDefinition->namespaceIndex), il2cpp_functions::MetadataCache_GetStringFromIndex(typeDefinition->nameIndex)), index));
    }

    void AddNestedTypesToNametoClassHashTable(const Il2CppImage* img, const Il2CppTypeDefinition* typeDefinition) {
        il2cpp_functions::Init();
        for (int i = 0; i < typeDefinition->nested_type_count; ++i) {
            Il2CppClass *klass = il2cpp_functions::MetadataCache_GetNestedTypeFromIndex(typeDefinition->nestedTypesStart + i);
            AddNestedTypesToNametoClassHashTable(img->nameToClassHashTable, il2cpp_functions::MetadataCache_GetStringFromIndex(typeDefinition->namespaceIndex), il2cpp_functions::MetadataCache_GetStringFromIndex(typeDefinition->nameIndex), klass);
        }
    }

    void AddNestedTypesToNametoClassHashTable(Il2CppNameToTypeDefinitionIndexHashTable* hashTable, const char *namespaze, const std::string& parentName, Il2CppClass *klass) {
        il2cpp_functions::Init();
        std::string name = parentName + "/" + klass->name;
        char *pName = (char*)calloc(name.size() + 1, sizeof(char));
        strcpy(pName, name.c_str());

        hashTable->insert(std::make_pair(std::make_pair(namespaze, (const char*)pName), il2cpp_functions::MetadataCache_GetIndexForTypeDefinition(klass)));

        void *iter = NULL;
        while (Il2CppClass *nestedClass = il2cpp_functions::class_get_nested_types(klass, &iter))
            AddNestedTypesToNametoClassHashTable(hashTable, namespaze, name, nestedClass);
    }

    Il2CppString* createcsstr(std::string_view inp, StringType type) {
        il2cpp_functions::Init();
        switch (type) {
            case Manual: {
                il2cpp_functions::CheckS_GlobalMetadata();
                auto mallocSize = sizeof(Il2CppString) + sizeof(Il2CppChar) * inp.length();
                auto* str = RET_0_UNLESS(reinterpret_cast<Il2CppString*>(calloc(1, mallocSize)));
                reinterpret_cast<Il2CppObject*>(str)->klass = il2cpp_functions::defaults->string_class;
                reinterpret_cast<Il2CppObject*>(str)->monitor = nullptr;
                setcsstr(str, to_utf16(inp));
                return str;
            }
            default:
                // Get ASCII Encoding
                static auto enc = RET_0_UNLESS(GetPropertyValue("System.Text", "Encoding", "ASCII"));
                // Create new string, created from the literal char*, not to be confused with a copy of this data
                auto* obj = RET_0_UNLESS(RunMethod<Il2CppString*>("System", "String", "CreateStringFromEncoding", (uint8_t*)inp.data(), (int)inp.length(), enc));
                return obj;
        }
    }

    [[nodiscard]] bool Match(const Il2CppObject* source, const Il2CppClass* klass) noexcept {
        return (source->klass == klass);
    }

    bool AssertMatch(const Il2CppObject* source, Il2CppClass* klass) {
        il2cpp_functions::Init();
        if (!Match(source, klass)) {
            Logger::get().critical("il2cpp_utils: AssertMatch: source with class '%s' does not match class '%s'!",
                ClassStandardName(source->klass).c_str(), ClassStandardName(klass).c_str());
            SAFE_ABORT();
        }
        return true;
    }

    // Contains the set of created delegates
    std::unordered_set<Delegate*> delegates;

    void ClearDelegates() {
        for (auto itr : delegates) {
            free(itr->original_method_info);
        }
        delegates.clear();
    }

    void ClearDelegate(Delegate* delegate) {
        auto itr = delegates.find(delegate);
        if (itr != delegates.end()) {
            free((*itr)->original_method_info);
            delegates.erase(itr);
        }
    }

    void AddAllocatedDelegate(Delegate* delegate) {
        delegates.insert(delegate);
    }
}

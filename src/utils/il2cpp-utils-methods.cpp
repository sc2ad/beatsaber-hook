#include "../../shared/utils/il2cpp-utils-methods.hpp"
#include "../../shared/utils/hashing.hpp"
#include <sstream>

namespace std {
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

namespace il2cpp_utils {
    static std::unordered_map<std::pair<const Il2CppClass*, std::pair<std::string, decltype(MethodInfo::parameters_count)>>, const MethodInfo*, hash_pair_3> classesNamesToMethodsCache;
    typedef std::pair<std::string, std::vector<const Il2CppType*>> classesNamesTypesInnerPairType;
    static std::unordered_map<std::pair<const Il2CppClass*, classesNamesTypesInnerPairType>, const MethodInfo*, hash_pair_3> classesNamesTypesToMethodsCache;

    #if __has_feature(cxx_exceptions)
    const MethodInfo* MakeGenericMethod(const MethodInfo* info, std::vector<Il2CppClass*> types)
    #else
    const MethodInfo* MakeGenericMethod(const MethodInfo* info, std::vector<Il2CppClass*> types) noexcept
    #endif
    {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeGenericMethod");
        il2cpp_functions::Init();
        // Ensure it exists and is generic
        THROW_OR_RET_NULL(logger, info);
        THROW_OR_RET_NULL(logger, il2cpp_functions::method_is_generic(info));
        static auto* typeClass = THROW_OR_RET_NULL(logger, il2cpp_functions::defaults->systemtype_class);
        // Create the Il2CppReflectionMethod* from the MethodInfo* using the MethodInfo's type
        auto* infoObj = il2cpp_functions::method_get_object(info, nullptr);
        if (!infoObj) {
            logger.error("Failed to get MonoMethod from MethodInfo: %p", info);
            THROW_OR_RET_NULL(logger, infoObj);
        }
        // Populate generic parameters into array
        auto* arr = reinterpret_cast<Array<Il2CppReflectionType*>*>(il2cpp_functions::array_new(typeClass, types.size()));
        if (!arr) {
            logger.error("Failed to create array of length: %u", types.size());
            THROW_OR_RET_NULL(logger, arr);
        }
        int i = 0;
        for (auto* klass : types) {
            auto* typeObj = GetSystemType(klass);
            if (!typeObj) {
                logger.error("Failed to get type object from class: %s", il2cpp_functions::class_get_name_const(klass));
                THROW_OR_RET_NULL(logger, typeObj);
            }
            arr->values[i] = typeObj;
            i++;
        }
        // Call instance function on infoObj to MakeGeneric
        // Does not need to perform type checking, since if this does not match, it will fail more miserably.
        auto res = il2cpp_utils::RunMethod<Il2CppReflectionMethod*, false>(infoObj, "MakeGenericMethod", arr);
        const auto* returnedInfoObj = RET_0_UNLESS(logger, res);
        if (!returnedInfoObj) {
            logger.error("Failed to get Il2CppReflectionMethod from MakeGenericMethod!");
            THROW_OR_RET_NULL(logger, returnedInfoObj);
        }
        // Get MethodInfo* back from generic instantiated method
        const auto* inflatedInfo = il2cpp_functions::method_get_from_reflection(returnedInfoObj);
        if (!inflatedInfo) {
            logger.error("Got null MethodInfo* from Il2CppReflectionMethod: %p", returnedInfoObj);
            THROW_OR_RET_NULL(logger, inflatedInfo);
        }
        // Return method to be invoked by caller
        return inflatedInfo;
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, std::string_view methodName, int argsCount)
    #else
    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, std::string_view methodName, int argsCount) noexcept
    #endif
    {
        il2cpp_functions::Init();
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("FindMethodUnsafe");
        THROW_OR_RET_NULL(logger, klass);

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
            logger.error("could not find method %s with %i parameters in class '%s'!", methodName.data(), argsCount, ClassStandardName(klass).c_str());
            LogMethods(logger, const_cast<Il2CppClass*>(klass), true);
            THROW_OR_RET_NULL(logger, methodInfo);
        }
        classesNamesToMethodsCache.emplace(key, methodInfo);
        return methodInfo;
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethodUnsafe(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount)
    #else
    const MethodInfo* FindMethodUnsafe(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount) noexcept
    #endif
    {
        return FindMethodUnsafe(GetClassFromName(nameSpace, className), methodName, argsCount);
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, std::string_view methodName, int argsCount)
    #else
    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, std::string_view methodName, int argsCount) noexcept
    #endif
    {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("FindMethodUnsafe");
        il2cpp_functions::Init();
        auto klass = THROW_OR_RET_NULL(logger, il2cpp_functions::object_get_class(instance));
        return FindMethodUnsafe(klass, methodName, argsCount);
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethod(FindMethodInfo& info)
    #else
    const MethodInfo* FindMethod(FindMethodInfo& info) noexcept
    #endif
    {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("FindMethod");
        il2cpp_functions::Init();
        auto* klass = info.klass;
        THROW_OR_RET_NULL(logger, klass);

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
                if (info.returnType) {
                    auto* returnClass = il2cpp_functions::class_from_il2cpp_type(current->return_type);
                    if (info.returnType == returnClass) {
                        if (perfectMatch) {
                            multiplePerfectMatches = true;
                            logger.error("Multiple perfect matches???");
                        }
                        else perfectMatch = current;
                    }
                    if (il2cpp_functions::class_is_assignable_from(info.returnType, returnClass)) {
                        if (returnMatch) {
                            logger.debug("Multiple return type matches.");
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
            logger.error("%s", ss.str().c_str());
            LogMethods(logger, klass);
            THROW_OR_RET_NULL(logger, !methodInfo || multipleBasicMatches);
        }
        classesNamesTypesToMethodsCache.emplace(key, methodInfo);
        return methodInfo;
    }

    void LogMethods(LoggerContextObject& logger, Il2CppClass* klass, bool logParents) {
        RET_V_UNLESS(logger, klass);

        if (klass->name) {
            il2cpp_functions::Init();
            il2cpp_functions::Class_Init(klass);
        }
        if (klass->method_count && !(klass->methods)) {
            logger.warning("Class is valid and claims to have methods but ->methods is null! class name: %s", ClassStandardName(klass).c_str());
            return;
        }
        if (logParents) logger.info("class name: %s", ClassStandardName(klass).c_str());

        logger.debug("method_count: %i", klass->method_count);
        for (int i = 0; i < klass->method_count; i++) {
            if (klass->methods[i]) {
                logger.debug("Method %i:", i);
                LogMethod(logger, klass->methods[i]);
            } else {
                logger.warning("Method: %i Does not exist!", i);
            }
        }
        usleep(100);  // 0.0001s
        if (logParents && klass->parent && (klass->parent != klass)) {
            LogMethods(logger, klass->parent, logParents);
        }
    }

    void LogMethod(LoggerContextObject& logger, const MethodInfo* method) {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, method);

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
        logger.debug("%s%s %s(%s);", flagStr, retTypeStr, methodName, paramStr);
    }

    bool IsConvertible(const Il2CppType* to, const Il2CppType* from, bool asArgs) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("IsConvertible");
        RET_0_UNLESS(logger, to);
        RET_0_UNLESS(logger, from);
        if (asArgs) {
            if (to->byref) {
                if (!from->byref) {
                    logger.debug("to (%s, %p) is ref/out while from (%s, %p) is not. Not convertible.",
                        TypeGetSimpleName(to), to, TypeGetSimpleName(from), from);
                    return false;
                } else {
                    logger.debug("to (%s, %p) and from (%s, %p) are both ret/out. May be convertible.",
                        TypeGetSimpleName(to), to, TypeGetSimpleName(from), from);
                }
            }
        }
        il2cpp_functions::Init();
        auto classTo = il2cpp_functions::class_from_il2cpp_type(to);
        auto classFrom = il2cpp_functions::class_from_il2cpp_type(from);
        bool ret = (to->type == IL2CPP_TYPE_MVAR) || il2cpp_functions::class_is_assignable_from(classTo, classFrom);
        if (!ret) {
            logger.debug("class_is_assignable_from(%s, %s) returned %s",
                ClassStandardName(classTo).c_str(), ClassStandardName(classFrom).c_str(), ret ? "true" : "false");
            if (il2cpp_functions::class_is_enum(classTo)) {
                logger.debug("but classTo is enum! Comparing against class_enum_basetype.");
                ret = IsConvertible(il2cpp_functions::class_enum_basetype(classTo), from, asArgs);
            }
        }
        return ret;
    }
}
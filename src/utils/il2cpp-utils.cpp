#include <utility>  // for std::pair
#include "../../shared/utils/hashing.hpp"
#include "../../shared/utils/il2cpp-utils.hpp"
#include "../../shared/utils/utils.h"
#include "../../shared/utils/il2cpp-functions.hpp"
#include "../../shared/utils/typedefs.h"
#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

// Please see comments in il2cpp-utils.hpp
// TODO: Make this into a static class
namespace il2cpp_utils {
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

    bool ParameterMatch(const MethodInfo* method, std::vector<Il2CppClass*> genTypes, std::vector<const Il2CppType*> argTypes) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("ParameterMatch");
        il2cpp_functions::Init();
        if (method->parameters_count != argTypes.size()) {
            return false;
        }
        auto genCount = (method->is_generic && !method->is_inflated) ? method->genericContainer->type_argc : 0;
        if ((size_t)genCount != genTypes.size()) {
            logger.warning("Potential method match had wrong number of generics %i (expected %i)",
                genCount, genTypes.size());
            return false;
        }
        // TODO: supply boolStrictMatch and use type_equals instead of IsConvertible if supplied?
        for (decltype(method->parameters_count) i = 0; i < method->parameters_count; i++) {
            auto* paramType = method->parameters[i].parameter_type;
            if (paramType->type == IL2CPP_TYPE_MVAR) {
                auto genIdx = paramType->data.genericParameterIndex - method->genericContainer->genericParameterStart;
                if (genIdx < 0) {
                    logger.warning("Extracted invalid genIdx %i from parameter %i", genIdx, i);
                } else if (genIdx >= genCount) {
                    logger.warning("ParameterMatch was not supplied enough genTypes to determine type of parameter %i "
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

    Il2CppClass* GetParamClass(const MethodInfo* method, int paramIdx) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("GetParamClass");
        auto type = RET_0_UNLESS(logger, il2cpp_functions::method_get_param(method, paramIdx));
        return il2cpp_functions::class_from_il2cpp_type(type);
    }

    Il2CppReflectionType* MakeGenericType(Il2CppReflectionType* gt, Il2CppArray* types) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("MakeGenericType");
        il2cpp_functions::Init();

        auto runtimeType = RET_0_UNLESS(logger, il2cpp_functions::defaults->runtimetype_class);
        auto makeGenericMethod = RET_0_UNLESS(logger, FindMethodUnsafe(runtimeType, "MakeGenericType", 2));

        Il2CppException* exp = nullptr;
        void* params[] = {reinterpret_cast<void*>(gt), reinterpret_cast<void*>(types)};
        auto genericType = il2cpp_functions::runtime_invoke(makeGenericMethod, nullptr, params, &exp);
        if (exp) {
            logger.error("il2cpp_utils: MakeGenericType: Failed with exception: %s", ExceptionToString(exp).c_str());
            return nullptr;
        }
        return reinterpret_cast<Il2CppReflectionType*>(genericType);
    }

    Il2CppReflectionType* GetSystemType(const Il2CppType* typ) {
        il2cpp_functions::Init();
        return reinterpret_cast<Il2CppReflectionType*>(il2cpp_functions::type_get_object(typ));
    }

    Il2CppReflectionType* GetSystemType(const Il2CppClass* klass) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("GetSystemType");
        il2cpp_functions::Init();
        RET_0_UNLESS(logger, klass);

        auto* typ = il2cpp_functions::class_get_type_const(klass);
        return GetSystemType(typ);
    }

    Il2CppReflectionType* GetSystemType(std::string_view nameSpace, std::string_view className) {
        return GetSystemType(il2cpp_utils::GetClassFromName(nameSpace, className));
    }

    void GenericsToStringHelper(Il2CppGenericClass* genClass, std::ostream& os) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("GenericsToStringHelper");
        auto genContext = &genClass->context;
        auto* genInst = genContext->class_inst;
        if (!genInst) {
            genInst = genContext->method_inst;
            if (genInst) logger.warning("Missing class_inst! Trying method_inst?");
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
            logger.warning("context->class_inst missing for genClass!");
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

    Il2CppObject* createManual(const Il2CppClass* klass) noexcept {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("createManual");
        if (!klass->initialized) {
            logger.error("Cannot create an object that does not have an initialized class: %p", klass);
            return nullptr;
        }
        // Allocate object
        auto* obj = reinterpret_cast<Il2CppObject*>(calloc(1, klass->instance_size));
        obj->klass = const_cast<Il2CppClass*>(klass);
        // Call cctor, we don't bother making a new thread for the type initializer. BE WARNED!
        if (klass->has_cctor && !klass->cctor_finished && !klass->cctor_started) {
            obj->klass->cctor_started = true;
            auto* m = RET_0_UNLESS(logger, FindMethodUnsafe(klass, ".cctor", 0));
            RET_0_UNLESS(logger, il2cpp_utils::RunStaticMethodUnsafe(m));
            obj->klass->cctor_finished = true;
        }
        return obj;
    }

    Il2CppString* createcsstr(std::string_view inp, StringType type) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("createcsstr");
        il2cpp_functions::Init();
        switch (type) {
            case StringType::Manual: {
                il2cpp_functions::CheckS_GlobalMetadata();
                auto mallocSize = sizeof(Il2CppString) + sizeof(Il2CppChar) * inp.length();
                auto* str = RET_0_UNLESS(logger, reinterpret_cast<Il2CppString*>(calloc(1, mallocSize)));
                reinterpret_cast<Il2CppObject*>(str)->klass = il2cpp_functions::defaults->string_class;
                reinterpret_cast<Il2CppObject*>(str)->monitor = nullptr;
                setcsstr(str, to_utf16(inp));
                return str;
            }
            default:
                // Get ASCII Encoding
                static auto enc = RET_0_UNLESS(logger, GetPropertyValue("System.Text", "Encoding", "ASCII"));
                // Create new string, created from the literal char*, not to be confused with a copy of this data
                auto* obj = RET_0_UNLESS(logger, RunMethod<Il2CppString*>("System", "String", "CreateStringFromEncoding", (uint8_t*)inp.data(), (int)inp.length(), enc));
                return obj;
        }
    }

    [[nodiscard]] bool Match(const Il2CppObject* source, const Il2CppClass* klass) noexcept {
        return (source->klass == klass);
    }

    bool AssertMatch(const Il2CppObject* source, Il2CppClass* klass) {
        static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("AssertMatch");
        il2cpp_functions::Init();
        if (!Match(source, klass)) {
            logger.critical("source with class '%s' does not match class '%s'!",
                ClassStandardName(source->klass).c_str(), ClassStandardName(klass).c_str());
            SAFE_ABORT();
        }
        return true;
    }

    // Contains the set of created delegates
    std::unordered_set<void*> delegates;

    void ClearDelegates() {
        for (auto itr : delegates) {
            free(reinterpret_cast<Delegate*>(itr)->original_method_info);
        }
        delegates.clear();
    }

    void ClearDelegate(void* delegate) {
        auto itr = delegates.find(delegate);
        if (itr != delegates.end()) {
            free(reinterpret_cast<Delegate*>(*itr)->original_method_info);
            delegates.erase(itr);
        }
    }

    void AddAllocatedDelegate(void* delegate) {
        delegates.insert(delegate);
    }
}

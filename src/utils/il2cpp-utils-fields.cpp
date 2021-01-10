#include "../../shared/utils/il2cpp-utils-fields.hpp"
#include "../../shared/utils/hashing.hpp"
#include "../../shared/utils/utils.h"
#include <unordered_map>
#include "../../shared/utils/typedefs.h"

namespace il2cpp_utils {
    static std::unordered_map<std::pair<const Il2CppClass*, std::string>, FieldInfo*, hash_pair> classesNamesToFieldsCache;

    FieldInfo* FindField(Il2CppClass* klass, std::string_view fieldName) {
        static auto logger = getLogger().WithContext("FindField");
        il2cpp_functions::Init();
        RET_0_UNLESS(logger, klass);

        // Check Cache
        auto key = std::pair<Il2CppClass*, std::string>(klass, fieldName);
        auto itr = classesNamesToFieldsCache.find(key);
        if (itr != classesNamesToFieldsCache.end()) {
            return itr->second;
        }
        auto field = il2cpp_functions::class_get_field_from_name(klass, fieldName.data());
        if (!field) {
            logger.error("could not find field %s in class '%s'!", fieldName.data(), ClassStandardName(klass).c_str());
            LogFields(logger, klass);
            if (klass->parent != klass) field = FindField(klass->parent, fieldName);
        }
        classesNamesToFieldsCache.emplace(key, field);
        return field;
    }

    Il2CppClass* GetFieldClass(FieldInfo* field) {
        static auto logger = getLogger().WithContext("GetFieldClass");
        auto type = RET_0_UNLESS(logger, il2cpp_functions::field_get_type(field));
        return il2cpp_functions::class_from_il2cpp_type(type);
    }

    void LogField(LoggerContextObject& logger, FieldInfo* field) {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, field);

        auto flags = il2cpp_functions::field_get_flags(field);
        const char* flagStr = (flags & FIELD_ATTRIBUTE_STATIC) ? "static " : "";
        auto* type = il2cpp_functions::field_get_type(field);
        auto typeStr = TypeGetSimpleName(type);
        auto name = il2cpp_functions::field_get_name(field);
        name = name ? name : "__noname__";
        auto offset = il2cpp_functions::field_get_offset(field);

        logger.debug("%s%s %s; // 0x%lx, flags: 0x%.4X", flagStr, typeStr, name, offset, flags);
    }

    void LogFields(LoggerContextObject& logger, Il2CppClass* klass, bool logParents) {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, klass);

        void* myIter = nullptr;
        FieldInfo* field;
        if (klass->name) il2cpp_functions::Class_Init(klass);
        if (logParents) logger.info("class name: %s", ClassStandardName(klass).c_str());

        logger.debug("field_count: %i", klass->field_count);
        while ((field = il2cpp_functions::class_get_fields(klass, &myIter))) {
            LogField(logger, field);
        }
        usleep(100);
        if (logParents && klass->parent && klass->parent != klass) {
            LogFields(logger, klass->parent, logParents);
        }
    }
}
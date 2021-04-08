#include "../../shared/utils/il2cpp-utils-properties.hpp"
#include "../../shared/utils/utils.h"
#include <unordered_map>
#include "../../shared/utils/hashing.hpp"
#include "../../shared/utils/typedefs.h"

namespace il2cpp_utils {
    static std::unordered_map<std::pair<const Il2CppClass*, std::string>, const PropertyInfo*, hash_pair> classesNamesToPropertiesCache;
    static std::mutex classPropertiesLock;

    const PropertyInfo* FindProperty(Il2CppClass* klass, std::string_view propName) {
        static auto logger = getLogger().WithContext("FindProperty");
        il2cpp_functions::Init();
        RET_0_UNLESS(logger, klass);

        // Check Cache
        auto key = std::pair<Il2CppClass*, std::string>(klass, propName);
        classPropertiesLock.lock();
        auto itr = classesNamesToPropertiesCache.find(key);
        if (itr != classesNamesToPropertiesCache.end()) {
            classPropertiesLock.unlock();
            return itr->second;
        }
        classPropertiesLock.unlock();
        auto prop = il2cpp_functions::class_get_property_from_name(klass, propName.data());
        if (!prop) {
            logger.error("could not find property %s in class '%s'!", propName.data(), ClassStandardName(klass).c_str());
            LogProperties(logger, klass);
            if (klass->parent != klass) prop = FindProperty(klass->parent, propName);
        }
        classPropertiesLock.lock();
        classesNamesToPropertiesCache.emplace(key, prop);
        classPropertiesLock.unlock();
        return prop;
    }

    const PropertyInfo* FindProperty(std::string_view nameSpace, std::string_view className, std::string_view propertyName) {
        return FindProperty(GetClassFromName(nameSpace, className), propertyName);
    }

    void LogProperty(LoggerContextObject& logger, const PropertyInfo* prop) {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, prop);

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

        logger.debug("%s%s %s { %s; %s; }; // flags: 0x%.4X", flagStr, typeStr, name, getterName, setterName, flags);
    }

    void LogProperties(LoggerContextObject& logger, Il2CppClass* klass, bool logParents) {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, klass);

        void* myIter = nullptr;
        const PropertyInfo* prop;
        if (klass->name) il2cpp_functions::Class_Init(klass);
        if (logParents) logger.info("class name: %s", ClassStandardName(klass).c_str());

        logger.debug("property_count: %i", klass->property_count);
        while ((prop = il2cpp_functions::class_get_properties(klass, &myIter))) {
            LogProperty(logger, prop);
        }
        usleep(100);
        if (logParents && klass->parent && klass->parent != klass) {
            LogProperties(logger, klass->parent, logParents);
        }
    }
}
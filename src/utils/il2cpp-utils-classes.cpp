#include "../../shared/utils/il2cpp-utils-classes.hpp"
#include "../../shared/utils/il2cpp-utils-methods.hpp"
#include "../../shared/utils/il2cpp-utils-properties.hpp"
#include "../../shared/utils/il2cpp-utils-fields.hpp"
#include <map>
#include <unordered_map>
#include "../../shared/utils/alphanum.hpp"
#include "../../shared/utils/typedefs.h"

namespace il2cpp_utils {
    static int indent = -1;
    static int maxIndent;
    std::unordered_set<Il2CppClass*> loggedClasses;

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

    void LogClass(LoggerContextObject& logger, Il2CppClass* klass, bool logParents) noexcept {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, klass);

        if (loggedClasses.count(klass)) {
            logger.debug("Already logged %p!", klass);
            return;
        }
        loggedClasses.insert(klass);

        RET_V_UNLESS(logger, klass->klass == klass);  // otherwise, klass is likely NOT an Il2CppClass*!
        RET_V_UNLESS(logger, klass->name);  // ditto

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

        logger.debug("%i ======================CLASS INFO FOR CLASS: %s======================", indent, ClassStandardName(klass).c_str());
        void* myIter = nullptr;
        if (!methodInit) {
            // log results of Class::Init
            logger.warning("klass->initialized: %i, init_pending: %i, has_initialization_error: %i, initializationExceptionGCHandle: %Xll",
                    klass->initialized, klass->init_pending, klass->has_initialization_error, klass->initializationExceptionGCHandle);
            auto* m1 = il2cpp_functions::class_get_methods(klass, &myIter);  // attempt again to initialize the method data
            if (klass->method_count && !klass->methods) {
                logger.error("Class::Init and class_get_methods failed to initialize klass->methods! class_get_methods returned: %p",
                    m1);
                if (m1) LogMethod(logger, m1);
            }
        }

        logger.debug("Pointer: %p", klass);
        logger.debug("Type Token: %i", il2cpp_functions::class_get_type_token(klass));
        auto typeDefIdx = klass->generic_class ? klass->generic_class->typeDefinitionIndex : il2cpp_functions::MetadataCache_GetIndexForTypeDefinition(klass);
        logger.debug("TypeDefinitionIndex: %i", typeDefIdx);
        // Repair the typeDefinition value if it was null but we found one
        if (!klass->typeDefinition && typeDefIdx > 0) klass->typeDefinition = il2cpp_functions::MetadataCache_GetTypeDefinitionFromIndex(typeDefIdx);
        logger.debug("Type definition: %p", klass->typeDefinition);

        logger.debug("Assembly Name: %s", il2cpp_functions::class_get_assemblyname(klass));

        auto* typ = il2cpp_functions::class_get_type(klass);
        if (typ) {
            logger.debug("Type name: %s", il2cpp_functions::type_get_name(typ));
            if (auto* reflName = il2cpp_functions::Type_GetName(typ, IL2CPP_TYPE_NAME_FORMAT_REFLECTION)) {
                logger.debug("Type reflection name: %s", reflName);
                il2cpp_functions::free(reflName);
            }
            logger.debug("Fully qualifed type name: %s", il2cpp_functions::type_get_assembly_qualified_name(typ));
        }
        logger.debug("Rank: %i", il2cpp_functions::class_get_rank(klass));
        logger.debug("Flags: 0x%.8X", il2cpp_functions::class_get_flags(klass));
        logger.debug("Event Count: %i", klass->event_count);
        logger.debug("Method Count: %i", klass->method_count);
        logger.debug("Is Generic: %i", il2cpp_functions::class_is_generic(klass));
        logger.debug("Is Abstract: %i", il2cpp_functions::class_is_abstract(klass));

        // Some methods, such as GenericClass::GetClass, may not initialize all fields in Il2CppClass, and thus not meet all implicit contracts defined by the comments in Il2CppClass's struct definition.
        // But unless we're blind, the only method that sets is_generic on non-methods is MetadataCache::FromTypeDefinition. That method also contains the only assignment of genericContainerIndex.
        // Therefore, this code makes only the following assumptions:
        // 1. If is_generic is set, then genericContainerIndex was also intentionally set (even if it's 0) and is not -1 (invalid)
        // 2. Even if is_generic wasn't set, a positive genericContainerIndex was intentionally set that way and is a valid index.
        if (klass->is_generic || klass->genericContainerIndex > 0) {
            auto* genContainer = il2cpp_functions::MetadataCache_GetGenericContainerFromIndex(klass->genericContainerIndex);
            logger.debug("genContainer: idx %i, ownerIndex: %i, is_method: %i", klass->genericContainerIndex, genContainer->ownerIndex, genContainer->is_method);
            if (genContainer->ownerIndex != typeDefIdx) {
                logger.error("genContainer ownerIndex mismatch!");
            }
            for (int i = 0; i < genContainer->type_argc; i++) {
                auto genParamIdx = genContainer->genericParameterStart + i;
                auto* genParam = il2cpp_functions::MetadataCache_GetGenericParameterFromIndex(genParamIdx);
                if (genParam) {
                    logger.debug("genParam #%i, idx %i: ownerIdx %i, name %s, num %i, flags (see "
                        "IL2CPP_GENERIC_PARAMETER_ATTRIBUTE_X in il2cpp-tabledefs.h) 0x%.2X", i, genParamIdx, genParam->ownerIndex,
                        il2cpp_functions::MetadataCache_GetStringFromIndex(genParam->nameIndex), genParam->num, genParam->flags);
                } else {
                    logger.warning("genParam %i, idx %i: null", i, genParamIdx);
                }
            }
        } else {
            logger.debug("genericContainerIndex: %i", klass->genericContainerIndex);
        }

        logger.debug("%i =========METHODS=========", indent);
        LogMethods(logger, klass);
        logger.debug("%i =======END METHODS=======", indent);

        auto* declaring = il2cpp_functions::class_get_declaring_type(klass);
        logger.debug("declaring type: %p (%s)", declaring, declaring ? ClassStandardName(declaring).c_str() : "");
        if (declaring && logParents) LogClass(logger, declaring, logParents);
        auto* element = il2cpp_functions::class_get_element_class(klass);
        logger.debug("element class: %p ('%s', self = %p)", element, element ? ClassStandardName(element).c_str() : "", klass);
        if (element && element != klass && logParents) LogClass(logger, element, logParents);

        logger.debug("%i =======PROPERTIES=======", indent);
        LogProperties(logger, klass);
        logger.debug("%i =====END PROPERTIES=====", indent);
        logger.debug("%i =========FIELDS=========", indent);
        LogFields(logger, klass);
        logger.debug("%i =======END FIELDS=======", indent);

        auto* parent = il2cpp_functions::class_get_parent(klass);
        logger.debug("parent: %p (%s)", parent, parent ? ClassStandardName(parent).c_str() : "");
        if (parent && logParents) LogClass(logger, parent, logParents);
        logger.debug("%i, ==================================================================================", indent);
        indent--;
    }

    static std::unordered_map<Il2CppClass*, std::map<std::string, Il2CppGenericClass*, doj::alphanum_less<std::string>>> classToGenericClassMap;
    void BuildGenericsMap() {
        static auto logger = getLogger().WithContext("BuildGenericsMap");
        il2cpp_functions::Init();
        auto* metadataReg = RET_V_UNLESS(logger, *il2cpp_functions::s_Il2CppMetadataRegistrationPtr);
        logger.debug("metadataReg: %p, offset = %lX", metadataReg, ((uintptr_t)metadataReg) - getRealOffset(0));

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
        logger.debug("uncached_class_count: %i (%f proportion of total)", uncached_class_count, uncached_class_count * 1.0 / metadataReg->genericClassesCount);
    }

    void LogClasses(LoggerContextObject& logger, std::string_view classPrefix, bool logParents) noexcept {
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
                logger.warning("Assembly %zu was null! Skipping.", i);
                continue;
            }
            logger.debug("Scanning assembly \"%s\"", assembs[i]->aname.name);
            auto* img = il2cpp_functions::assembly_get_image(assembs[i]);
            if (!img) {
                logger.warning("Assembly's image was null! Skipping.");
                continue;
            }

            if (img->nameToClassHashTable == nullptr) {
                logger.debug("Assembly's nameToClassHashTable is empty. Populating it instead.");

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
        logger.debug("LogClasses:");
        for ( const auto &pair : matches ) {
            LogClass(logger, pair.second, logParents);
            indent = -1;
            for ( const auto &genPair : classToGenericClassMap[pair.second] ) {
                logger.debug("%s", genPair.first.c_str());
            }
            usleep(1000);  // 0.001s
        }
        logger.debug("LogClasses(\"%s\") is complete.", classPrefix.data());
        logger.debug("maxIndent: %i", maxIndent);
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
}
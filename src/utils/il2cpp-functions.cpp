#include <unistd.h>

#include "../../shared/utils/hooking.hpp"
#include "../../shared/utils/il2cpp-functions.hpp"
#include "../../shared/utils/instruction-parsing.hpp"
#include "../../shared/utils/logging.hpp"
#include "modloader/shared/modloader.hpp"

// All the fields...
#ifdef UNITY_2019
int (*il2cpp_functions::init)(const char* domain_name);
int (*il2cpp_functions::init_utf16)(const Il2CppChar * domain_name);
#else
void (*il2cpp_functions::init)(const char* domain_name);
void (*il2cpp_functions::init_utf16)(const Il2CppChar * domain_name);
#endif
void (*il2cpp_functions::shutdown)();
void (*il2cpp_functions::set_config_dir)(const char *config_path);
void (*il2cpp_functions::set_data_dir)(const char *data_path);
void (*il2cpp_functions::set_temp_dir)(const char *temp_path);
void (*il2cpp_functions::set_commandline_arguments)(int argc, const char* const argv[], const char* basedir);
void (*il2cpp_functions::set_commandline_arguments_utf16)(int argc, const Il2CppChar * const argv[], const char* basedir);
void (*il2cpp_functions::set_config_utf16)(const Il2CppChar * executablePath);
void (*il2cpp_functions::set_config)(const char* executablePath);
void (*il2cpp_functions::set_memory_callbacks)(Il2CppMemoryCallbacks * callbacks);
const Il2CppImage* (*il2cpp_functions::get_corlib)();
void (*il2cpp_functions::add_internal_call)(const char* name, Il2CppMethodPointer method);
Il2CppMethodPointer (*il2cpp_functions::resolve_icall)(const char* name);
void* (*il2cpp_functions::alloc)(size_t size);
void (*il2cpp_functions::free)(void* ptr);
Il2CppClass* (*il2cpp_functions::array_class_get)(Il2CppClass * element_class, uint32_t rank);
uint32_t (*il2cpp_functions::array_length)(Il2CppArray * array);
uint32_t (*il2cpp_functions::array_get_byte_length)(Il2CppArray * array);
Il2CppArray* (*il2cpp_functions::array_new)(Il2CppClass * elementTypeInfo, il2cpp_array_size_t length);
Il2CppArray* (*il2cpp_functions::array_new_specific)(Il2CppClass * arrayTypeInfo, il2cpp_array_size_t length);
Il2CppArray* (*il2cpp_functions::array_new_full)(Il2CppClass * array_class, il2cpp_array_size_t * lengths, il2cpp_array_size_t * lower_bounds);
Il2CppClass* (*il2cpp_functions::bounded_array_class_get)(Il2CppClass * element_class, uint32_t rank, bool bounded);
int (*il2cpp_functions::array_element_size)(const Il2CppClass * array_class);
const Il2CppImage* (*il2cpp_functions::assembly_get_image)(const Il2CppAssembly * assembly);
#ifdef UNITY_2019
void (*il2cpp_functions::class_for_each)(void(*klassReportFunc)(Il2CppClass* klass, void* userData), void* userData);
#endif
const Il2CppType* (*il2cpp_functions::class_enum_basetype)(Il2CppClass * klass);
bool (*il2cpp_functions::class_is_generic)(const Il2CppClass * klass);
bool (*il2cpp_functions::class_is_inflated)(const Il2CppClass * klass);
bool (*il2cpp_functions::class_is_assignable_from)(Il2CppClass * klass, Il2CppClass * oklass);
bool (*il2cpp_functions::class_is_subclass_of)(Il2CppClass * klass, Il2CppClass * klassc, bool check_interfaces);
bool (*il2cpp_functions::class_has_parent)(Il2CppClass * klass, Il2CppClass * klassc);
Il2CppClass* (*il2cpp_functions::class_from_il2cpp_type)(const Il2CppType * type);
Il2CppClass* (*il2cpp_functions::class_from_name)(const Il2CppImage * image, const char* namespaze, const char *name);
Il2CppClass* (*il2cpp_functions::class_from_system_type)(Il2CppReflectionType * type);
Il2CppClass* (*il2cpp_functions::class_get_element_class)(Il2CppClass * klass);
const EventInfo* (*il2cpp_functions::class_get_events)(Il2CppClass * klass, void* *iter);
FieldInfo* (*il2cpp_functions::class_get_fields)(Il2CppClass * klass, void* *iter);
Il2CppClass* (*il2cpp_functions::class_get_nested_types)(Il2CppClass * klass, void* *iter);
Il2CppClass* (*il2cpp_functions::class_get_interfaces)(Il2CppClass * klass, void* *iter);
const PropertyInfo* (*il2cpp_functions::class_get_properties)(Il2CppClass * klass, void* *iter);
const PropertyInfo* (*il2cpp_functions::class_get_property_from_name)(Il2CppClass * klass, const char *name);
FieldInfo* (*il2cpp_functions::class_get_field_from_name)(Il2CppClass * klass, const char *name);
const MethodInfo* (*il2cpp_functions::class_get_methods)(Il2CppClass * klass, void* *iter);
const MethodInfo* (*il2cpp_functions::class_get_method_from_name)(const Il2CppClass * klass, const char* name, int argsCount);
const char* (*il2cpp_functions::class_get_name)(const Il2CppClass * klass);
#ifdef UNITY_2019
void (*il2cpp_functions::type_get_name_chunked)(const Il2CppType * type, void(*chunkReportFunc)(void* data, void* userData), void* userData);
#endif
const char* (*il2cpp_functions::class_get_namespace)(const Il2CppClass * klass);
Il2CppClass* (*il2cpp_functions::class_get_parent)(Il2CppClass * klass);
Il2CppClass* (*il2cpp_functions::class_get_declaring_type)(const Il2CppClass * klass);
int32_t (*il2cpp_functions::class_instance_size)(Il2CppClass * klass);
size_t (*il2cpp_functions::class_num_fields)(const Il2CppClass * enumKlass);
bool (*il2cpp_functions::class_is_valuetype)(const Il2CppClass * klass);
int32_t (*il2cpp_functions::class_value_size)(Il2CppClass * klass, uint32_t * align);
bool (*il2cpp_functions::class_is_blittable)(const Il2CppClass * klass);
int (*il2cpp_functions::class_get_flags)(const Il2CppClass * klass);
bool (*il2cpp_functions::class_is_abstract)(const Il2CppClass * klass);
bool (*il2cpp_functions::class_is_interface)(const Il2CppClass * klass);
int (*il2cpp_functions::class_array_element_size)(const Il2CppClass * klass);
Il2CppClass* (*il2cpp_functions::class_from_type)(const Il2CppType * type);
const Il2CppType* (*il2cpp_functions::class_get_type)(Il2CppClass * klass);
uint32_t (*il2cpp_functions::class_get_type_token)(Il2CppClass * klass);
bool (*il2cpp_functions::class_has_attribute)(Il2CppClass * klass, Il2CppClass * attr_class);
bool (*il2cpp_functions::class_has_references)(Il2CppClass * klass);
bool (*il2cpp_functions::class_is_enum)(const Il2CppClass * klass);
const Il2CppImage* (*il2cpp_functions::class_get_image)(Il2CppClass * klass);
const char* (*il2cpp_functions::class_get_assemblyname)(const Il2CppClass * klass);
int (*il2cpp_functions::class_get_rank)(const Il2CppClass * klass);
#ifdef UNITY_2019
uint32_t (*il2cpp_functions::class_get_data_size)(const Il2CppClass * klass);
void* (*il2cpp_functions::class_get_static_field_data)(const Il2CppClass * klass);
#endif
size_t (*il2cpp_functions::class_get_bitmap_size)(const Il2CppClass * klass);
void (*il2cpp_functions::class_get_bitmap)(Il2CppClass * klass, size_t * bitmap);
bool (*il2cpp_functions::stats_dump_to_file)(const char *path);
uint64_t (*il2cpp_functions::stats_get_value)(Il2CppStat stat);
Il2CppDomain* (*il2cpp_functions::domain_get)();
const Il2CppAssembly* (*il2cpp_functions::domain_assembly_open)(Il2CppDomain * domain, const char* name);
const Il2CppAssembly** (*il2cpp_functions::domain_get_assemblies)(const Il2CppDomain * domain, size_t * size);
#ifdef UNITY_2019
void (*il2cpp_functions::raise_exception)(Il2CppException*);
#endif
Il2CppException* (*il2cpp_functions::exception_from_name_msg)(const Il2CppImage * image, const char *name_space, const char *name, const char *msg);
Il2CppException* (*il2cpp_functions::get_exception_argument_null)(const char *arg);
void (*il2cpp_functions::format_exception)(const Il2CppException * ex, char* message, int message_size);
void (*il2cpp_functions::format_stack_trace)(const Il2CppException * ex, char* output, int output_size);
void (*il2cpp_functions::unhandled_exception)(Il2CppException*);
int (*il2cpp_functions::field_get_flags)(FieldInfo * field);
const char* (*il2cpp_functions::field_get_name)(FieldInfo * field);
Il2CppClass* (*il2cpp_functions::field_get_parent)(FieldInfo * field);
size_t (*il2cpp_functions::field_get_offset)(FieldInfo * field);
const Il2CppType* (*il2cpp_functions::field_get_type)(FieldInfo * field);
void (*il2cpp_functions::field_get_value)(Il2CppObject * obj, FieldInfo * field, void *value);
Il2CppObject* (*il2cpp_functions::field_get_value_object)(FieldInfo * field, Il2CppObject * obj);
bool (*il2cpp_functions::field_has_attribute)(FieldInfo * field, Il2CppClass * attr_class);
void (*il2cpp_functions::field_set_value)(Il2CppObject * obj, FieldInfo * field, void *value);
void (*il2cpp_functions::field_static_get_value)(FieldInfo * field, void *value);
void (*il2cpp_functions::field_static_set_value)(FieldInfo * field, void *value);
void (*il2cpp_functions::field_set_value_object)(Il2CppObject * instance, FieldInfo * field, Il2CppObject * value);
#ifdef UNITY_2019
bool (*il2cpp_functions::field_is_literal)(FieldInfo * field);
#endif
void (*il2cpp_functions::gc_collect)(int maxGenerations);
int32_t (*il2cpp_functions::gc_collect_a_little)();
void (*il2cpp_functions::gc_disable)();
void (*il2cpp_functions::gc_enable)();
bool (*il2cpp_functions::gc_is_disabled)();
#ifdef UNITY_2019
int64_t (*il2cpp_functions::gc_get_max_time_slice_ns)();
void (*il2cpp_functions::gc_set_max_time_slice_ns)(int64_t maxTimeSlice);
bool (*il2cpp_functions::gc_is_incremental)();
#endif
int64_t (*il2cpp_functions::gc_get_used_size)();
int64_t (*il2cpp_functions::gc_get_heap_size)();
void (*il2cpp_functions::gc_wbarrier_set_field)(Il2CppObject * obj, void **targetAddress, void *object);
#ifdef UNITY_2019
bool (*il2cpp_functions::gc_has_strict_wbarriers)();
void (*il2cpp_functions::gc_set_external_allocation_tracker)(void(*func)(void*, size_t, int));
void (*il2cpp_functions::gc_set_external_wbarrier_tracker)(void(*func)(void**));
void (*il2cpp_functions::gc_foreach_heap)(void(*func)(void* data, void* userData), void* userData);
void (*il2cpp_functions::stop_gc_world)();
void (*il2cpp_functions::start_gc_world)();
#endif
uint32_t (*il2cpp_functions::gchandle_new)(Il2CppObject * obj, bool pinned);
uint32_t (*il2cpp_functions::gchandle_new_weakref)(Il2CppObject * obj, bool track_resurrection);
Il2CppObject* (*il2cpp_functions::gchandle_get_target)(uint32_t gchandle);
void (*il2cpp_functions::gchandle_free)(uint32_t gchandle);
#ifdef UNITY_2019
void (*il2cpp_functions::gchandle_foreach_get_target)(void(*func)(void* data, void* userData), void* userData);
uint32_t (*il2cpp_functions::object_header_size)();
uint32_t (*il2cpp_functions::array_object_header_size)();
uint32_t (*il2cpp_functions::offset_of_array_length_in_array_object_header)();
uint32_t (*il2cpp_functions::offset_of_array_bounds_in_array_object_header)();
uint32_t (*il2cpp_functions::allocation_granularity)();
#endif
void* (*il2cpp_functions::unity_liveness_calculation_begin)(Il2CppClass * filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata, il2cpp_WorldChangedCallback onWorldStarted, il2cpp_WorldChangedCallback onWorldStopped);
void (*il2cpp_functions::unity_liveness_calculation_end)(void* state);
void (*il2cpp_functions::unity_liveness_calculation_from_root)(Il2CppObject * root, void* state);
void (*il2cpp_functions::unity_liveness_calculation_from_statics)(void* state);
const Il2CppType* (*il2cpp_functions::method_get_return_type)(const MethodInfo * method);
Il2CppClass* (*il2cpp_functions::method_get_declaring_type)(const MethodInfo * method);
const char* (*il2cpp_functions::method_get_name)(const MethodInfo * method);
const MethodInfo* (*il2cpp_functions::method_get_from_reflection)(const Il2CppReflectionMethod * method);
Il2CppReflectionMethod* (*il2cpp_functions::method_get_object)(const MethodInfo * method, Il2CppClass * refclass);
bool (*il2cpp_functions::method_is_generic)(const MethodInfo * method);
bool (*il2cpp_functions::method_is_inflated)(const MethodInfo * method);
bool (*il2cpp_functions::method_is_instance)(const MethodInfo * method);
uint32_t (*il2cpp_functions::method_get_param_count)(const MethodInfo * method);
const Il2CppType* (*il2cpp_functions::method_get_param)(const MethodInfo * method, uint32_t index);
Il2CppClass* (*il2cpp_functions::method_get_class)(const MethodInfo * method);
bool (*il2cpp_functions::method_has_attribute)(const MethodInfo * method, Il2CppClass * attr_class);
uint32_t (*il2cpp_functions::method_get_flags)(const MethodInfo * method, uint32_t * iflags);
uint32_t (*il2cpp_functions::method_get_token)(const MethodInfo * method);
const char* (*il2cpp_functions::method_get_param_name)(const MethodInfo * method, uint32_t index);

// ONLY IF THE PROFILER EXISTS FOR UNITY_2019
void (*il2cpp_functions::profiler_install)(Il2CppProfiler * prof, Il2CppProfileFunc shutdown_callback);
void (*il2cpp_functions::profiler_set_events)(Il2CppProfileFlags events);
void (*il2cpp_functions::profiler_install_enter_leave)(Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc fleave);
void (*il2cpp_functions::profiler_install_allocation)(Il2CppProfileAllocFunc callback);
void (*il2cpp_functions::profiler_install_gc)(Il2CppProfileGCFunc callback, Il2CppProfileGCResizeFunc heap_resize_callback);
void (*il2cpp_functions::profiler_install_fileio)(Il2CppProfileFileIOFunc callback);
void (*il2cpp_functions::profiler_install_thread)(Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end);

uint32_t (*il2cpp_functions::property_get_flags)(const PropertyInfo * prop);
const MethodInfo* (*il2cpp_functions::property_get_get_method)(const PropertyInfo * prop);
const MethodInfo* (*il2cpp_functions::property_get_set_method)(const PropertyInfo * prop);
const char* (*il2cpp_functions::property_get_name)(const PropertyInfo * prop);
Il2CppClass* (*il2cpp_functions::property_get_parent)(const PropertyInfo * prop);
Il2CppClass* (*il2cpp_functions::object_get_class)(Il2CppObject * obj);
uint32_t (*il2cpp_functions::object_get_size)(Il2CppObject * obj);
const MethodInfo* (*il2cpp_functions::object_get_virtual_method)(Il2CppObject * obj, const MethodInfo * method);
Il2CppObject* (*il2cpp_functions::object_new)(const Il2CppClass * klass);
// Always returns (void*)(obj + 1)
void* (*il2cpp_functions::object_unbox)(Il2CppObject * obj);
// If klass is not a ValueType, returns (Il2CppObject*)(*il2cpp_functions::data), else boxes
Il2CppObject* (*il2cpp_functions::value_box)(Il2CppClass * klass, void* data);
void (*il2cpp_functions::monitor_enter)(Il2CppObject * obj);
bool (*il2cpp_functions::monitor_try_enter)(Il2CppObject * obj, uint32_t timeout);
void (*il2cpp_functions::monitor_exit)(Il2CppObject * obj);
void (*il2cpp_functions::monitor_pulse)(Il2CppObject * obj);
void (*il2cpp_functions::monitor_pulse_all)(Il2CppObject * obj);
void (*il2cpp_functions::monitor_wait)(Il2CppObject * obj);
bool (*il2cpp_functions::monitor_try_wait)(Il2CppObject * obj, uint32_t timeout);
Il2CppObject* (*il2cpp_functions::runtime_invoke)(const MethodInfo * method, void *obj, void **params, Il2CppException **exc);
Il2CppObject* (*il2cpp_functions::runtime_invoke_convert_args)(const MethodInfo * method, void *obj, Il2CppObject **params, int paramCount, Il2CppException **exc);
void (*il2cpp_functions::runtime_class_init)(Il2CppClass * klass);
void (*il2cpp_functions::runtime_object_init)(Il2CppObject * obj);
void (*il2cpp_functions::runtime_object_init_exception)(Il2CppObject * obj, Il2CppException** exc);
void (*il2cpp_functions::runtime_unhandled_exception_policy_set)(Il2CppRuntimeUnhandledExceptionPolicy value);
int32_t (*il2cpp_functions::string_length)(Il2CppString * str);
Il2CppChar* (*il2cpp_functions::string_chars)(Il2CppString * str);
Il2CppString* (*il2cpp_functions::string_new)(const char* str);
Il2CppString* (*il2cpp_functions::string_new_len)(const char* str, uint32_t length);
Il2CppString* (*il2cpp_functions::string_new_utf16)(const Il2CppChar * text, int32_t len);
Il2CppString* (*il2cpp_functions::string_new_wrapper)(const char* str);
Il2CppString* (*il2cpp_functions::string_intern)(Il2CppString * str);
Il2CppString* (*il2cpp_functions::string_is_interned)(Il2CppString * str);
Il2CppThread* (*il2cpp_functions::thread_current)();
Il2CppThread* (*il2cpp_functions::thread_attach)(Il2CppDomain * domain);
void (*il2cpp_functions::thread_detach)(Il2CppThread * thread);
Il2CppThread** (*il2cpp_functions::thread_get_all_attached_threads)(size_t * size);
bool (*il2cpp_functions::is_vm_thread)(Il2CppThread * thread);
void (*il2cpp_functions::current_thread_walk_frame_stack)(Il2CppFrameWalkFunc func, void* user_data);
void (*il2cpp_functions::thread_walk_frame_stack)(Il2CppThread * thread, Il2CppFrameWalkFunc func, void* user_data);
bool (*il2cpp_functions::current_thread_get_top_frame)(Il2CppStackFrameInfo * frame);
bool (*il2cpp_functions::thread_get_top_frame)(Il2CppThread * thread, Il2CppStackFrameInfo * frame);
bool (*il2cpp_functions::current_thread_get_frame_at)(int32_t offset, Il2CppStackFrameInfo * frame);
bool (*il2cpp_functions::thread_get_frame_at)(Il2CppThread * thread, int32_t offset, Il2CppStackFrameInfo * frame);
int32_t (*il2cpp_functions::current_thread_get_stack_depth)();
int32_t (*il2cpp_functions::thread_get_stack_depth)(Il2CppThread * thread);
#ifdef UNITY_2019
void (*il2cpp_functions::override_stack_backtrace)(Il2CppBacktraceFunc stackBacktraceFunc);
#endif
Il2CppObject* (*il2cpp_functions::type_get_object)(const Il2CppType * type);
int (*il2cpp_functions::type_get_type)(const Il2CppType * type);
Il2CppClass* (*il2cpp_functions::type_get_class_or_element_class)(const Il2CppType * type);
char* (*il2cpp_functions::type_get_name)(const Il2CppType * type);
bool (*il2cpp_functions::type_is_byref)(const Il2CppType * type);
uint32_t (*il2cpp_functions::type_get_attrs)(const Il2CppType * type);
bool (*il2cpp_functions::type_equals)(const Il2CppType * type, const Il2CppType * otherType);
char* (*il2cpp_functions::type_get_assembly_qualified_name)(const Il2CppType * type);
#ifdef UNITY_2019
bool (*il2cpp_functions::type_is_static)(const Il2CppType * type);
bool (*il2cpp_functions::type_is_pointer_type)(const Il2CppType * type);
#endif
const Il2CppAssembly* (*il2cpp_functions::image_get_assembly)(const Il2CppImage * image);
const char* (*il2cpp_functions::image_get_name)(const Il2CppImage * image);
const char* (*il2cpp_functions::image_get_filename)(const Il2CppImage * image);
const MethodInfo* (*il2cpp_functions::image_get_entry_point)(const Il2CppImage * image);
size_t (*il2cpp_functions::image_get_class_count)(const Il2CppImage * image);
const Il2CppClass* (*il2cpp_functions::image_get_class)(const Il2CppImage * image, size_t index);
Il2CppManagedMemorySnapshot* (*il2cpp_functions::capture_memory_snapshot)();
void (*il2cpp_functions::free_captured_memory_snapshot)(Il2CppManagedMemorySnapshot * snapshot);
void (*il2cpp_functions::set_find_plugin_callback)(Il2CppSetFindPlugInCallback method);
void (*il2cpp_functions::register_log_callback)(Il2CppLogCallback method);
void (*il2cpp_functions::debugger_set_agent_options)(const char* options);
bool (*il2cpp_functions::is_debugger_attached)();
#ifdef UNITY_2019
void (*il2cpp_functions::register_debugger_agent_transport)(Il2CppDebuggerTransport * debuggerTransport);
bool (*il2cpp_functions::debug_get_method_info)(const MethodInfo*, Il2CppMethodDebugInfo * methodDebugInfo);
#endif
void (*il2cpp_functions::unity_install_unitytls_interface)(const void* unitytlsInterfaceStruct);
Il2CppCustomAttrInfo* (*il2cpp_functions::custom_attrs_from_class)(Il2CppClass * klass);
Il2CppCustomAttrInfo* (*il2cpp_functions::custom_attrs_from_method)(const MethodInfo * method);
Il2CppObject* (*il2cpp_functions::custom_attrs_get_attr)(Il2CppCustomAttrInfo * ainfo, Il2CppClass * attr_klass);
bool (*il2cpp_functions::custom_attrs_has_attr)(Il2CppCustomAttrInfo * ainfo, Il2CppClass * attr_klass);
Il2CppArray* (*il2cpp_functions::custom_attrs_construct)(Il2CppCustomAttrInfo * cinfo);
void (*il2cpp_functions::custom_attrs_free)(Il2CppCustomAttrInfo * ainfo);
#ifdef UNITY_2019
void (*il2cpp_functions::class_set_userdata)(Il2CppClass * klass, void* userdata);
int (*il2cpp_functions::class_get_userdata_offset)();
#endif

// MANUALLY DEFINED CONST DEFINITIONS
const Il2CppType* (*il2cpp_functions::class_get_type_const)(const Il2CppClass * klass);
const char* (*il2cpp_functions::class_get_name_const)(const Il2CppClass * klass);

// SELECT NON-API LIBIL2CPP FUNCTIONS:
bool (*il2cpp_functions::Class_Init)(Il2CppClass* klass);

Il2CppClass* (*il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex)(TypeDefinitionIndex index);
Il2CppClass* (*il2cpp_functions::MetadataCache_GetTypeInfoFromTypeIndex)(TypeIndex index);

#ifdef UNITY_2019
std::string (*il2cpp_functions::_Type_GetName_)(const Il2CppType *type, Il2CppTypeNameFormat format);
#else
gnu_string (*il2cpp_functions::_Type_GetName_)(const Il2CppType *type, Il2CppTypeNameFormat format);
#endif
void (*il2cpp_functions::GC_free)(void* addr);
void (*il2cpp_functions::GarbageCollector_SetWriteBarrier)(void** ptr);
void* (*il2cpp_functions::GarbageCollector_AllocateFixed)(size_t sz, void* desc);

Il2CppClass* (*il2cpp_functions::Class_FromIl2CppType)(Il2CppType* typ);
Il2CppClass* (*il2cpp_functions::Class_GetPtrClass)(Il2CppClass* elementClass);
Il2CppClass* (*il2cpp_functions::GenericClass_GetClass)(Il2CppGenericClass* gclass);
AssemblyVector* (*il2cpp_functions::Assembly_GetAllAssemblies)();

const Il2CppMetadataRegistration** il2cpp_functions::s_Il2CppMetadataRegistrationPtr;
const void** il2cpp_functions::s_GlobalMetadataPtr;
const Il2CppGlobalMetadataHeader** il2cpp_functions::s_GlobalMetadataHeaderPtr;

std::remove_pointer_t<decltype(il2cpp_functions::s_GlobalMetadataPtr)> il2cpp_functions::s_GlobalMetadata;
std::remove_pointer_t<decltype(il2cpp_functions::s_GlobalMetadataHeaderPtr)> il2cpp_functions::s_GlobalMetadataHeader;
const Il2CppDefaults* il2cpp_functions::defaults;
bool il2cpp_functions::initialized;

// copies of the highly-inlinable functions
const Il2CppTypeDefinition* il2cpp_functions::MetadataCache_GetTypeDefinitionFromIndex(TypeDefinitionIndex index) {
    CheckS_GlobalMetadata();
    if (index == kTypeDefinitionIndexInvalid) return NULL;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) < s_GlobalMetadataHeader->typeDefinitionsCount / sizeof(Il2CppTypeDefinition));
    auto typeDefinitions = (const Il2CppTypeDefinition*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->typeDefinitionsOffset);
    return typeDefinitions + index;
}

const char* il2cpp_functions::MetadataCache_GetStringFromIndex(StringIndex index) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(index <= s_GlobalMetadataHeader->stringCount);
    const char* strings = ((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->stringOffset) + index;
    return strings;
}

const Il2CppGenericContainer* il2cpp_functions::MetadataCache_GetGenericContainerFromIndex(GenericContainerIndex index) {
    CheckS_GlobalMetadata();
    if (index == kGenericContainerIndexInvalid) return NULL;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->genericContainersCount / sizeof(Il2CppGenericContainer));
    const Il2CppGenericContainer* genericContainers = (const Il2CppGenericContainer*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->genericContainersOffset);
    return genericContainers + index;
}

const Il2CppGenericParameter* il2cpp_functions::MetadataCache_GetGenericParameterFromIndex(GenericParameterIndex index) {
    CheckS_GlobalMetadata();
    if (index == kGenericParameterIndexInvalid) return NULL;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->genericParametersCount / sizeof(Il2CppGenericParameter));
    const Il2CppGenericParameter* genericParameters = (const Il2CppGenericParameter*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->genericParametersOffset);
    return genericParameters + index;
}

TypeDefinitionIndex il2cpp_functions::MetadataCache_GetExportedTypeFromIndex(TypeDefinitionIndex index) {
    CheckS_GlobalMetadata();
    if (index == kTypeDefinitionIndexInvalid) return kTypeDefinitionIndexInvalid;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) < s_GlobalMetadataHeader->exportedTypeDefinitionsCount / sizeof(TypeDefinitionIndex));
    auto exportedTypes = (TypeDefinitionIndex*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->exportedTypeDefinitionsOffset);
    return *(exportedTypes + index);
}

Il2CppClass* il2cpp_functions::MetadataCache_GetNestedTypeFromIndex(NestedTypeIndex index) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->nestedTypesCount / sizeof(TypeDefinitionIndex));
    auto nestedTypeIndices = (const TypeDefinitionIndex*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->nestedTypesOffset);

    return il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex(nestedTypeIndices[index]);
}

TypeDefinitionIndex il2cpp_functions::MetadataCache_GetIndexForTypeDefinition(const Il2CppClass* typeDefinition) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(typeDefinition->typeDefinition);
    const Il2CppTypeDefinition* typeDefinitions = (const Il2CppTypeDefinition*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->typeDefinitionsOffset);

    IL2CPP_ASSERT(typeDefinition->typeDefinition >= typeDefinitions && typeDefinition->typeDefinition < typeDefinitions + s_GlobalMetadataHeader->typeDefinitionsCount);

    ptrdiff_t index = typeDefinition->typeDefinition - typeDefinitions;
    IL2CPP_ASSERT(index <= std::numeric_limits<TypeDefinitionIndex>::max());
    return static_cast<TypeDefinitionIndex>(index);
}

char* il2cpp_functions::Type_GetName(const Il2CppType *type, Il2CppTypeNameFormat format) {
    if (!il2cpp_functions::_Type_GetName_) return nullptr;
    // TODO debug the ref/lifetime weirdness with _Type_GetName_ to avoid the need for explicit allocation
    const auto str = il2cpp_functions::_Type_GetName_(type, format);
    char* buffer = static_cast<char*>(il2cpp_functions::alloc(str.length() + 1));
    memcpy(buffer, str.c_str(), str.length() + 1);
    return buffer;
}

static bool find_GC_free(const int32_t* Runtime_Shutdown) {
    bool multipleMatches;
    auto sigMatch = findUniquePatternInLibil2cpp(multipleMatches, "f8 5f bc a9 f6 57 01 a9 f4 4f 02 a9 "
        "fd 7b 03 a9 fd c3 00 91 a0 08 00 b4 f3 03 00 aa ?? ?? ?? ?? 69 82 56 d3 4a ?? ?? 91 4b 0d 09 8b 49 55 40 f9 "
        "0a 9c 9c 52 0a 04 a0 72 00 cc 74 92 68 fe 56 d3 6c 01 0a 8b 0a 03 84 52", "GC_free");
    if (sigMatch && !multipleMatches) {
        il2cpp_functions::GC_free = (decltype(il2cpp_functions::GC_free))sigMatch;
    } else {
        static auto logger = il2cpp_functions::getFuncLogger().WithContext("find_GC_free");
        // xref tracing __WILL__ fail if Runtime_Shutdown is hooked by __ANY__ lib/mod, such as another bs-hook's file logger.
        Instruction Runtime_Shutdown_inst(Runtime_Shutdown);
        auto blr = RET_0_UNLESS(logger, Runtime_Shutdown_inst.findNth(1, std::mem_fn(&Instruction::isIndirectBranch)));
        auto j2GC_FF = RET_0_UNLESS(logger, blr->findNthCall(5));  // BL(R)
        if (blr != &Runtime_Shutdown_inst) delete blr;
        Instruction GC_FreeFixed(RET_0_UNLESS(logger, j2GC_FF->label));
        il2cpp_functions::GC_free = (decltype(il2cpp_functions::GC_free))RET_0_UNLESS(logger, GC_FreeFixed.label);
        if (j2GC_FF != blr) delete j2GC_FF;
    }
    return true;
}

static bool find_GC_SetWriteBarrier(const int32_t* set_wbarrier_field) {
    if (!set_wbarrier_field) {
        return false;
    }
    static auto logger = il2cpp_functions::getFuncLogger().WithContext("find_GC_SetWriteBarrier");
    Instruction gc_wbarrier_start(set_wbarrier_field);
    auto* swb = RET_0_UNLESS(logger, gc_wbarrier_start.findNthDirectBranchWithoutLink(1));
    il2cpp_functions::GarbageCollector_SetWriteBarrier = (decltype(il2cpp_functions::GarbageCollector_SetWriteBarrier))RET_0_UNLESS(logger, swb->label);
    if (swb != &gc_wbarrier_start) delete swb;
    return true;
}

void* (*wrapped_gc_malloc_uncollectable)(size_t sz, long long type);

void* __wrapper_gc_malloc_uncollectable(size_t sz, [[maybe_unused]] void* desc) {
    // 2 determined from ghidra dump of caller
    return wrapped_gc_malloc_uncollectable(sz, 2);
}

static bool trace_GC_AllocFixed(Instruction* DomainGetCurrent) {
    static auto logger = il2cpp_functions::getFuncLogger().WithContext("trace_GC_AllocFixed");
    // Domain::GetCurrent has a single bl to GarbageCollector::AllocateFixed
    // MetadataCache::InitializeGCSafe is 3rd bl after first b.ne, which is the 6th b(.lt, .ne), t(bz, nz), c(bz, nz)
    Instruction domainGetCurrentImpl(RET_0_UNLESS(logger, DomainGetCurrent->label));
    auto* gcAlloc = RET_0_UNLESS(logger, domainGetCurrentImpl.findNthCall(1));
    il2cpp_functions::GarbageCollector_AllocateFixed = (decltype(il2cpp_functions::GarbageCollector_AllocateFixed))RET_0_UNLESS(logger, gcAlloc->label);
    if (gcAlloc != &domainGetCurrentImpl) delete gcAlloc;
    return true;
}

static bool find_GC_AllocFixed(Instruction* DomainGetCurrent) {
    if (!trace_GC_AllocFixed(DomainGetCurrent)) {
        bool multipleMatches;
        auto sigMatch = findUniquePatternInLibil2cpp(multipleMatches, "f5 0f 1d f8 f4 4f 01 a9 fd 7b 02 a9"
            "fd 83 00 91 ?? ?? ?? ?? ?? ?? ?? ?? 1f 00 20 f1 f3 03 01 2a", "GC_Malloc_Uncollectable");

        if (sigMatch && !multipleMatches) {
            // We need to make a wrapper method instead and set that.
            wrapped_gc_malloc_uncollectable = (decltype(wrapped_gc_malloc_uncollectable))sigMatch;
            il2cpp_functions::GarbageCollector_AllocateFixed = &__wrapper_gc_malloc_uncollectable;
        } else {
            return false;
        }
    }
    return true;
}

LoggerContextObject& il2cpp_functions::getFuncLogger() {
    static auto logger = Logger::get().WithContext("il2cpp_functions");
    return logger;
}

// closes log on application shutdown
// Address is unused, so left as 0
// MAKE_HOOK(shutdown_hook, nullptr, void) {
//     Logger::closeAll();
//     shutdown_hook();
// }

// Autogenerated; modified by zoller27osu
// Initializes all of the IL2CPP functions via dlopen and dlsym for use.
void il2cpp_functions::Init() {
    if (initialized) {
        return;
    }
    static auto logger = getFuncLogger().WithContext("Init");
    logger.info("il2cpp_functions: Init: Initializing all IL2CPP Functions...");
    dlerror();  // clears existing errors
    auto path = Modloader::getLibIl2CppPath();
    void *imagehandle = dlopen(path.c_str(), RTLD_GLOBAL | RTLD_LAZY);
    if (!imagehandle) {
        logger.error("Failed to dlopen %s: %s!", path.c_str(), dlerror());
        return;
    }

    // Please verify that these have more than 1 instruction to their name before attempting to hook them!
    *(void**)(&init) = dlsym(imagehandle, "il2cpp_init");
    logger.info("Loaded: il2cpp_init, error: %s", dlerror());
    *(void**)(&shutdown) = dlsym(imagehandle, "il2cpp_shutdown");
    logger.info("Loaded: il2cpp_shutdown, error: %s", dlerror());
    *(void**)(&init_utf16) = dlsym(imagehandle, "il2cpp_init_utf16");
    logger.info("Loaded: il2cpp_init_utf16");
    *(void**)(&set_config_dir) = dlsym(imagehandle, "il2cpp_set_config_dir");
    logger.info("Loaded: il2cpp_set_config_dir");
    *(void**)(&set_data_dir) = dlsym(imagehandle, "il2cpp_set_data_dir");
    logger.info("Loaded: il2cpp_set_data_dir");
    *(void**)(&set_temp_dir) = dlsym(imagehandle, "il2cpp_set_temp_dir");
    logger.info("Loaded: il2cpp_set_temp_dir");
    *(void**)(&set_commandline_arguments) = dlsym(imagehandle, "il2cpp_set_commandline_arguments");
    logger.info("Loaded: il2cpp_set_commandline_arguments");
    *(void**)(&set_commandline_arguments_utf16) = dlsym(imagehandle, "il2cpp_set_commandline_arguments_utf16");
    logger.info("Loaded: il2cpp_set_commandline_arguments_utf16");
    *(void**)(&set_config_utf16) = dlsym(imagehandle, "il2cpp_set_config_utf16");
    logger.info("Loaded: il2cpp_set_config_utf16");
    *(void**)(&set_config) = dlsym(imagehandle, "il2cpp_set_config");
    logger.info("Loaded: il2cpp_set_config");
    *(void**)(&set_memory_callbacks) = dlsym(imagehandle, "il2cpp_set_memory_callbacks");
    logger.info("Loaded: il2cpp_set_memory_callbacks");
    *(void**)(&get_corlib) = dlsym(imagehandle, "il2cpp_get_corlib");
    logger.info("Loaded: il2cpp_get_corlib");
    *(void**)(&add_internal_call) = dlsym(imagehandle, "il2cpp_add_internal_call");
    logger.info("Loaded: il2cpp_add_internal_call");
    *(void**)(&resolve_icall) = dlsym(imagehandle, "il2cpp_resolve_icall");
    logger.info("Loaded: il2cpp_resolve_icall");
    *(void**)(&alloc) = dlsym(imagehandle, "il2cpp_alloc");
    logger.info("Loaded: il2cpp_alloc");
    *(void**)(&free) = dlsym(imagehandle, "il2cpp_free");
    logger.info("Loaded: il2cpp_free");
    *(void**)(&array_class_get) = dlsym(imagehandle, "il2cpp_array_class_get");
    logger.info("Loaded: il2cpp_array_class_get");
    *(void**)(&array_length) = dlsym(imagehandle, "il2cpp_array_length");
    logger.info("Loaded: il2cpp_array_length");
    *(void**)(&array_get_byte_length) = dlsym(imagehandle, "il2cpp_array_get_byte_length");
    logger.info("Loaded: il2cpp_array_get_byte_length");
    *(void**)(&array_new) = dlsym(imagehandle, "il2cpp_array_new");
    logger.info("Loaded: il2cpp_array_new");
    *(void**)(&array_new_specific) = dlsym(imagehandle, "il2cpp_array_new_specific");
    logger.info("Loaded: il2cpp_array_new_specific");
    *(void**)(&array_new_full) = dlsym(imagehandle, "il2cpp_array_new_full");
    logger.info("Loaded: il2cpp_array_new_full");
    *(void**)(&bounded_array_class_get) = dlsym(imagehandle, "il2cpp_bounded_array_class_get");
    logger.info("Loaded: il2cpp_bounded_array_class_get");
    *(void**)(&array_element_size) = dlsym(imagehandle, "il2cpp_array_element_size");
    logger.info("Loaded: il2cpp_array_element_size");
    *(void**)(&assembly_get_image) = dlsym(imagehandle, "il2cpp_assembly_get_image");
    logger.info("Loaded: il2cpp_assembly_get_image");
    #ifdef UNITY_2019
    *(void**)(&class_for_each) = dlsym(imagehandle, "il2cpp_class_for_each");
    logger.info("Loaded: il2cpp_class_for_each");
    #endif
    *(void**)(&class_enum_basetype) = dlsym(imagehandle, "il2cpp_class_enum_basetype");
    logger.info("Loaded: il2cpp_class_enum_basetype");
    *(void**)(&class_is_generic) = dlsym(imagehandle, "il2cpp_class_is_generic");
    logger.info("Loaded: il2cpp_class_is_generic");
    *(void**)(&class_is_inflated) = dlsym(imagehandle, "il2cpp_class_is_inflated");
    logger.info("Loaded: il2cpp_class_is_inflated");
    *(void**)(&class_is_assignable_from) = dlsym(imagehandle, "il2cpp_class_is_assignable_from");
    logger.info("Loaded: il2cpp_class_is_assignable_from");
    *(void**)(&class_is_subclass_of) = dlsym(imagehandle, "il2cpp_class_is_subclass_of");
    logger.info("Loaded: il2cpp_class_is_subclass_of");
    *(void**)(&class_has_parent) = dlsym(imagehandle, "il2cpp_class_has_parent");
    logger.info("Loaded: il2cpp_class_has_parent");
    *(void**)(&class_from_il2cpp_type) = dlsym(imagehandle, "il2cpp_class_from_il2cpp_type");
    logger.info("Loaded: il2cpp_class_from_il2cpp_type");
    *(void**)(&class_from_name) = dlsym(imagehandle, "il2cpp_class_from_name");
    logger.info("Loaded: il2cpp_class_from_name");
    *(void**)(&class_from_system_type) = dlsym(imagehandle, "il2cpp_class_from_system_type");
    logger.info("Loaded: il2cpp_class_from_system_type");
    *(void**)(&class_get_element_class) = dlsym(imagehandle, "il2cpp_class_get_element_class");
    logger.info("Loaded: il2cpp_class_get_element_class");
    *(void**)(&class_get_events) = dlsym(imagehandle, "il2cpp_class_get_events");
    logger.info("Loaded: il2cpp_class_get_events");
    *(void**)(&class_get_fields) = dlsym(imagehandle, "il2cpp_class_get_fields");
    logger.info("Loaded: il2cpp_class_get_fields");
    *(void**)(&class_get_nested_types) = dlsym(imagehandle, "il2cpp_class_get_nested_types");
    logger.info("Loaded: il2cpp_class_get_nested_types");
    *(void**)(&class_get_interfaces) = dlsym(imagehandle, "il2cpp_class_get_interfaces");
    logger.info("Loaded: il2cpp_class_get_interfaces");
    *(void**)(&class_get_properties) = dlsym(imagehandle, "il2cpp_class_get_properties");
    logger.info("Loaded: il2cpp_class_get_properties");
    *(void**)(&class_get_property_from_name) = dlsym(imagehandle, "il2cpp_class_get_property_from_name");
    logger.info("Loaded: il2cpp_class_get_property_from_name");
    *(void**)(&class_get_field_from_name) = dlsym(imagehandle, "il2cpp_class_get_field_from_name");
    logger.info("Loaded: il2cpp_class_get_field_from_name");
    *(void**)(&class_get_methods) = dlsym(imagehandle, "il2cpp_class_get_methods");
    logger.info("Loaded: il2cpp_class_get_methods");
    *(void**)(&class_get_method_from_name) = dlsym(imagehandle, "il2cpp_class_get_method_from_name");
    logger.info("Loaded: il2cpp_class_get_method_from_name");
    *(void**)(&class_get_name) = dlsym(imagehandle, "il2cpp_class_get_name");
    logger.info("Loaded: il2cpp_class_get_name");
    #ifdef UNITY_2019
    *(void**)(&type_get_name_chunked) = dlsym(imagehandle, "il2cpp_type_get_name_chunked");
    logger.info("Loaded: il2cpp_type_get_name_chunked");
    #endif
    *(void**)(&class_get_namespace) = dlsym(imagehandle, "il2cpp_class_get_namespace");
    logger.info("Loaded: il2cpp_class_get_namespace");
    *(void**)(&class_get_parent) = dlsym(imagehandle, "il2cpp_class_get_parent");
    logger.info("Loaded: il2cpp_class_get_parent");
    *(void**)(&class_get_declaring_type) = dlsym(imagehandle, "il2cpp_class_get_declaring_type");
    logger.info("Loaded: il2cpp_class_get_declaring_type");
    *(void**)(&class_instance_size) = dlsym(imagehandle, "il2cpp_class_instance_size");
    logger.info("Loaded: il2cpp_class_instance_size");
    *(void**)(&class_num_fields) = dlsym(imagehandle, "il2cpp_class_num_fields");
    logger.info("Loaded: il2cpp_class_num_fields");
    *(void**)(&class_is_valuetype) = dlsym(imagehandle, "il2cpp_class_is_valuetype");
    logger.info("Loaded: il2cpp_class_is_valuetype");
    *(void**)(&class_value_size) = dlsym(imagehandle, "il2cpp_class_value_size");
    logger.info("Loaded: il2cpp_class_value_size");
    *(void**)(&class_is_blittable) = dlsym(imagehandle, "il2cpp_class_is_blittable");
    logger.info("Loaded: il2cpp_class_is_blittable");
    *(void**)(&class_get_flags) = dlsym(imagehandle, "il2cpp_class_get_flags");
    logger.info("Loaded: il2cpp_class_get_flags");
    *(void**)(&class_is_abstract) = dlsym(imagehandle, "il2cpp_class_is_abstract");
    logger.info("Loaded: il2cpp_class_is_abstract");
    *(void**)(&class_is_interface) = dlsym(imagehandle, "il2cpp_class_is_interface");
    logger.info("Loaded: il2cpp_class_is_interface");
    *(void**)(&class_array_element_size) = dlsym(imagehandle, "il2cpp_class_array_element_size");
    logger.info("Loaded: il2cpp_class_array_element_size");
    *(void**)(&class_from_type) = dlsym(imagehandle, "il2cpp_class_from_type");
    logger.info("Loaded: il2cpp_class_from_type");
    *(void**)(&class_get_type) = dlsym(imagehandle, "il2cpp_class_get_type");
    logger.info("Loaded: il2cpp_class_get_type");
    *(void**)(&class_get_type_token) = dlsym(imagehandle, "il2cpp_class_get_type_token");
    logger.info("Loaded: il2cpp_class_get_type_token");
    *(void**)(&class_has_attribute) = dlsym(imagehandle, "il2cpp_class_has_attribute");
    logger.info("Loaded: il2cpp_class_has_attribute");
    *(void**)(&class_has_references) = dlsym(imagehandle, "il2cpp_class_has_references");
    logger.info("Loaded: il2cpp_class_has_references");
    *(void**)(&class_is_enum) = dlsym(imagehandle, "il2cpp_class_is_enum");
    logger.info("Loaded: il2cpp_class_is_enum");
    *(void**)(&class_get_image) = dlsym(imagehandle, "il2cpp_class_get_image");
    logger.info("Loaded: il2cpp_class_get_image");
    *(void**)(&class_get_assemblyname) = dlsym(imagehandle, "il2cpp_class_get_assemblyname");
    logger.info("Loaded: il2cpp_class_get_assemblyname");
    *(void**)(&class_get_rank) = dlsym(imagehandle, "il2cpp_class_get_rank");
    logger.info("Loaded: il2cpp_class_get_rank");
    #ifdef UNITY_2019
    *(void**)(&class_get_data_size) = dlsym(imagehandle, "il2cpp_class_get_data_size");
    logger.info("Loaded: il2cpp_class_get_data_size");
    *(void**)(&class_get_static_field_data) = dlsym(imagehandle, "il2cpp_class_get_static_field_data");
    logger.info("Loaded: il2cpp_class_get_static_field_data");
    #endif
    *(void**)(&class_get_bitmap_size) = dlsym(imagehandle, "il2cpp_class_get_bitmap_size");
    logger.info("Loaded: il2cpp_class_get_bitmap_size");
    *(void**)(&class_get_bitmap) = dlsym(imagehandle, "il2cpp_class_get_bitmap");
    logger.info("Loaded: il2cpp_class_get_bitmap");
    *(void**)(&stats_dump_to_file) = dlsym(imagehandle, "il2cpp_stats_dump_to_file");
    logger.info("Loaded: il2cpp_stats_dump_to_file");
    *(void**)(&stats_get_value) = dlsym(imagehandle, "il2cpp_stats_get_value");
    logger.info("Loaded: il2cpp_stats_get_value");
    *(void**)(&domain_get) = dlsym(imagehandle, "il2cpp_domain_get");
    logger.info("Loaded: il2cpp_domain_get");
    *(void**)(&domain_assembly_open) = dlsym(imagehandle, "il2cpp_domain_assembly_open");
    logger.info("Loaded: il2cpp_domain_assembly_open");
    *(void**)(&domain_get_assemblies) = dlsym(imagehandle, "il2cpp_domain_get_assemblies");
    logger.info("Loaded: il2cpp_domain_get_assemblies");
    #ifdef UNITY_2019
    *(void**)(&raise_exception) = dlsym(imagehandle, "il2cpp_raise_exception");
    logger.info("Loaded: il2cpp_raise_exception");
    #endif
    *(void**)(&exception_from_name_msg) = dlsym(imagehandle, "il2cpp_exception_from_name_msg");
    logger.info("Loaded: il2cpp_exception_from_name_msg");
    *(void**)(&get_exception_argument_null) = dlsym(imagehandle, "il2cpp_get_exception_argument_null");
    logger.info("Loaded: il2cpp_get_exception_argument_null");
    *(void**)(&format_exception) = dlsym(imagehandle, "il2cpp_format_exception");
    logger.info("Loaded: il2cpp_format_exception");
    *(void**)(&format_stack_trace) = dlsym(imagehandle, "il2cpp_format_stack_trace");
    logger.info("Loaded: il2cpp_format_stack_trace");
    *(void**)(&unhandled_exception) = dlsym(imagehandle, "il2cpp_unhandled_exception");
    logger.info("Loaded: il2cpp_unhandled_exception");
    *(void**)(&field_get_flags) = dlsym(imagehandle, "il2cpp_field_get_flags");
    logger.info("Loaded: il2cpp_field_get_flags");
    *(void**)(&field_get_name) = dlsym(imagehandle, "il2cpp_field_get_name");
    logger.info("Loaded: il2cpp_field_get_name");
    *(void**)(&field_get_parent) = dlsym(imagehandle, "il2cpp_field_get_parent");
    logger.info("Loaded: il2cpp_field_get_parent");
    *(void**)(&field_get_offset) = dlsym(imagehandle, "il2cpp_field_get_offset");
    logger.info("Loaded: il2cpp_field_get_offset");
    *(void**)(&field_get_type) = dlsym(imagehandle, "il2cpp_field_get_type");
    logger.info("Loaded: il2cpp_field_get_type");
    *(void**)(&field_get_value) = dlsym(imagehandle, "il2cpp_field_get_value");
    logger.info("Loaded: il2cpp_field_get_value");
    *(void**)(&field_get_value_object) = dlsym(imagehandle, "il2cpp_field_get_value_object");
    logger.info("Loaded: il2cpp_field_get_value_object");
    *(void**)(&field_has_attribute) = dlsym(imagehandle, "il2cpp_field_has_attribute");
    logger.info("Loaded: il2cpp_field_has_attribute");
    *(void**)(&field_set_value) = dlsym(imagehandle, "il2cpp_field_set_value");
    logger.info("Loaded: il2cpp_field_set_value");
    *(void**)(&field_static_get_value) = dlsym(imagehandle, "il2cpp_field_static_get_value");
    logger.info("Loaded: il2cpp_field_static_get_value");
    *(void**)(&field_static_set_value) = dlsym(imagehandle, "il2cpp_field_static_set_value");
    logger.info("Loaded: il2cpp_field_static_set_value");
    *(void**)(&field_set_value_object) = dlsym(imagehandle, "il2cpp_field_set_value_object");
    logger.info("Loaded: il2cpp_field_set_value_object");
    #ifdef UNITY_2019
    *(void**)(&field_is_literal) = dlsym(imagehandle, "il2cpp_field_is_literal");
    logger.info("Loaded: il2cpp_field_is_literal");
    #endif
    *(void**)(&gc_collect) = dlsym(imagehandle, "il2cpp_gc_collect");
    logger.info("Loaded: il2cpp_gc_collect");
    *(void**)(&gc_collect_a_little) = dlsym(imagehandle, "il2cpp_gc_collect_a_little");
    logger.info("Loaded: il2cpp_gc_collect_a_little");
    *(void**)(&gc_disable) = dlsym(imagehandle, "il2cpp_gc_disable");
    logger.info("Loaded: il2cpp_gc_disable");
    *(void**)(&gc_enable) = dlsym(imagehandle, "il2cpp_gc_enable");
    logger.info("Loaded: il2cpp_gc_enable");
    *(void**)(&gc_is_disabled) = dlsym(imagehandle, "il2cpp_gc_is_disabled");
    logger.info("Loaded: il2cpp_gc_is_disabled");
    #ifdef UNITY_2019
    *(void**)(&gc_get_max_time_slice_ns) = dlsym(imagehandle, "il2cpp_gc_get_max_time_slice_ns");
    logger.info("Loaded: il2cpp_gc_get_max_time_slice_ns");
    *(void**)(&gc_set_max_time_slice_ns) = dlsym(imagehandle, "il2cpp_gc_set_max_time_slice_ns");
    logger.info("Loaded: il2cpp_gc_set_max_time_slice_ns");
    *(void**)(&gc_is_incremental) = dlsym(imagehandle, "il2cpp_gc_is_incremental");
    logger.info("Loaded: il2cpp_gc_is_incremental");
    #endif
    *(void**)(&gc_get_used_size) = dlsym(imagehandle, "il2cpp_gc_get_used_size");
    logger.info("Loaded: il2cpp_gc_get_used_size");
    *(void**)(&gc_get_heap_size) = dlsym(imagehandle, "il2cpp_gc_get_heap_size");
    logger.info("Loaded: il2cpp_gc_get_heap_size");
    *(void**)(&gc_wbarrier_set_field) = dlsym(imagehandle, "il2cpp_gc_wbarrier_set_field");
    logger.info("Loaded: il2cpp_gc_wbarrier_set_field");
    #ifdef UNITY_2019
    *(void**)(&gc_has_strict_wbarriers) = dlsym(imagehandle, "il2cpp_gc_has_strict_wbarriers");
    logger.info("Loaded: il2cpp_gc_has_strict_wbarriers");
    *(void**)(&gc_set_external_allocation_tracker) = dlsym(imagehandle, "il2cpp_gc_set_external_allocation_tracker");
    logger.info("Loaded: il2cpp_gc_set_external_allocation_tracker");
    *(void**)(&gc_set_external_wbarrier_tracker) = dlsym(imagehandle, "il2cpp_gc_set_external_wbarrier_tracker");
    logger.info("Loaded: il2cpp_gc_set_external_wbarrier_tracker");
    *(void**)(&gc_foreach_heap) = dlsym(imagehandle, "il2cpp_gc_foreach_heap");
    logger.info("Loaded: il2cpp_gc_foreach_heap");
    *(void**)(&stop_gc_world) = dlsym(imagehandle, "il2cpp_stop_gc_world");
    logger.info("Loaded: il2cpp_stop_gc_world");
    *(void**)(&start_gc_world) = dlsym(imagehandle, "il2cpp_start_gc_world");
    logger.info("Loaded: il2cpp_start_gc_world");
    #endif
    *(void**)(&gchandle_new) = dlsym(imagehandle, "il2cpp_gchandle_new");
    logger.info("Loaded: il2cpp_gchandle_new");
    *(void**)(&gchandle_new_weakref) = dlsym(imagehandle, "il2cpp_gchandle_new_weakref");
    logger.info("Loaded: il2cpp_gchandle_new_weakref");
    *(void**)(&gchandle_get_target) = dlsym(imagehandle, "il2cpp_gchandle_get_target");
    logger.info("Loaded: il2cpp_gchandle_get_target");
    *(void**)(&gchandle_free) = dlsym(imagehandle, "il2cpp_gchandle_free");
    logger.info("Loaded: il2cpp_gchandle_free");
    #ifdef UNITY_2019
    *(void**)(&gchandle_foreach_get_target) = dlsym(imagehandle, "il2cpp_gchandle_foreach_get_target");
    logger.info("Loaded: il2cpp_gchandle_foreach_get_target");
    *(void**)(&object_header_size) = dlsym(imagehandle, "il2cpp_object_header_size");
    logger.info("Loaded: il2cpp_object_header_size");
    *(void**)(&array_object_header_size) = dlsym(imagehandle, "il2cpp_array_object_header_size");
    logger.info("Loaded: il2cpp_array_object_header_size");
    *(void**)(&offset_of_array_length_in_array_object_header) = dlsym(imagehandle, "il2cpp_offset_of_array_length_in_array_object_header");
    logger.info("Loaded: il2cpp_offset_of_array_length_in_array_object_header");
    *(void**)(&offset_of_array_bounds_in_array_object_header) = dlsym(imagehandle, "il2cpp_offset_of_array_bounds_in_array_object_header");
    logger.info("Loaded: il2cpp_offset_of_array_bounds_in_array_object_header");
    *(void**)(&allocation_granularity) = dlsym(imagehandle, "il2cpp_allocation_granularity");
    logger.info("Loaded: il2cpp_allocation_granularity");
    #endif
    *(void**)(&unity_liveness_calculation_begin) = dlsym(imagehandle, "il2cpp_unity_liveness_calculation_begin");
    logger.info("Loaded: il2cpp_unity_liveness_calculation_begin");
    *(void**)(&unity_liveness_calculation_end) = dlsym(imagehandle, "il2cpp_unity_liveness_calculation_end");
    logger.info("Loaded: il2cpp_unity_liveness_calculation_end");
    *(void**)(&unity_liveness_calculation_from_root) = dlsym(imagehandle, "il2cpp_unity_liveness_calculation_from_root");
    logger.info("Loaded: il2cpp_unity_liveness_calculation_from_root");
    *(void**)(&unity_liveness_calculation_from_statics) = dlsym(imagehandle, "il2cpp_unity_liveness_calculation_from_statics");
    logger.info("Loaded: il2cpp_unity_liveness_calculation_from_statics");
    *(void**)(&method_get_return_type) = dlsym(imagehandle, "il2cpp_method_get_return_type");
    logger.info("Loaded: il2cpp_method_get_return_type");
    *(void**)(&method_get_declaring_type) = dlsym(imagehandle, "il2cpp_method_get_declaring_type");
    logger.info("Loaded: il2cpp_method_get_declaring_type");
    *(void**)(&method_get_name) = dlsym(imagehandle, "il2cpp_method_get_name");
    logger.info("Loaded: il2cpp_method_get_name");
    *(void**)(&method_get_from_reflection) = dlsym(imagehandle, "il2cpp_method_get_from_reflection");
    logger.info("Loaded: il2cpp_method_get_from_reflection");
    *(void**)(&method_get_object) = dlsym(imagehandle, "il2cpp_method_get_object");
    logger.info("Loaded: il2cpp_method_get_object");
    *(void**)(&method_is_generic) = dlsym(imagehandle, "il2cpp_method_is_generic");
    logger.info("Loaded: il2cpp_method_is_generic");
    *(void**)(&method_is_inflated) = dlsym(imagehandle, "il2cpp_method_is_inflated");
    logger.info("Loaded: il2cpp_method_is_inflated");
    *(void**)(&method_is_instance) = dlsym(imagehandle, "il2cpp_method_is_instance");
    logger.info("Loaded: il2cpp_method_is_instance");
    *(void**)(&method_get_param_count) = dlsym(imagehandle, "il2cpp_method_get_param_count");
    logger.info("Loaded: il2cpp_method_get_param_count");
    *(void**)(&method_get_param) = dlsym(imagehandle, "il2cpp_method_get_param");
    logger.info("Loaded: il2cpp_method_get_param");
    *(void**)(&method_get_class) = dlsym(imagehandle, "il2cpp_method_get_class");
    logger.info("Loaded: il2cpp_method_get_class");
    *(void**)(&method_has_attribute) = dlsym(imagehandle, "il2cpp_method_has_attribute");
    logger.info("Loaded: il2cpp_method_has_attribute");
    *(void**)(&method_get_flags) = dlsym(imagehandle, "il2cpp_method_get_flags");
    logger.info("Loaded: il2cpp_method_get_flags");
    *(void**)(&method_get_token) = dlsym(imagehandle, "il2cpp_method_get_token");
    logger.info("Loaded: il2cpp_method_get_token");
    *(void**)(&method_get_param_name) = dlsym(imagehandle, "il2cpp_method_get_param_name");
    logger.info("Loaded: il2cpp_method_get_param_name");
    *(void**)(&profiler_install) = dlsym(imagehandle, "il2cpp_profiler_install");
    logger.info("Loaded: il2cpp_profiler_install");
    *(void**)(&profiler_set_events) = dlsym(imagehandle, "il2cpp_profiler_set_events");
    logger.info("Loaded: il2cpp_profiler_set_events");
    *(void**)(&profiler_install_enter_leave) = dlsym(imagehandle, "il2cpp_profiler_install_enter_leave");
    logger.info("Loaded: il2cpp_profiler_install_enter_leave");
    *(void**)(&profiler_install_allocation) = dlsym(imagehandle, "il2cpp_profiler_install_allocation");
    logger.info("Loaded: il2cpp_profiler_install_allocation");
    *(void**)(&profiler_install_gc) = dlsym(imagehandle, "il2cpp_profiler_install_gc");
    logger.info("Loaded: il2cpp_profiler_install_gc");
    *(void**)(&profiler_install_fileio) = dlsym(imagehandle, "il2cpp_profiler_install_fileio");
    logger.info("Loaded: il2cpp_profiler_install_fileio");
    *(void**)(&profiler_install_thread) = dlsym(imagehandle, "il2cpp_profiler_install_thread");
    logger.info("Loaded: il2cpp_profiler_install_thread");
    *(void**)(&property_get_flags) = dlsym(imagehandle, "il2cpp_property_get_flags");
    logger.info("Loaded: il2cpp_property_get_flags");
    *(void**)(&property_get_get_method) = dlsym(imagehandle, "il2cpp_property_get_get_method");
    logger.info("Loaded: il2cpp_property_get_get_method");
    *(void**)(&property_get_set_method) = dlsym(imagehandle, "il2cpp_property_get_set_method");
    logger.info("Loaded: il2cpp_property_get_set_method");
    *(void**)(&property_get_name) = dlsym(imagehandle, "il2cpp_property_get_name");
    logger.info("Loaded: il2cpp_property_get_name");
    *(void**)(&property_get_parent) = dlsym(imagehandle, "il2cpp_property_get_parent");
    logger.info("Loaded: il2cpp_property_get_parent");
    *(void**)(&object_get_class) = dlsym(imagehandle, "il2cpp_object_get_class");
    logger.info("Loaded: il2cpp_object_get_class");
    *(void**)(&object_get_size) = dlsym(imagehandle, "il2cpp_object_get_size");
    logger.info("Loaded: il2cpp_object_get_size");
    *(void**)(&object_get_virtual_method) = dlsym(imagehandle, "il2cpp_object_get_virtual_method");
    logger.info("Loaded: il2cpp_object_get_virtual_method");
    *(void**)(&object_new) = dlsym(imagehandle, "il2cpp_object_new");
    logger.info("Loaded: il2cpp_object_new");
    *(void**)(&object_unbox) = dlsym(imagehandle, "il2cpp_object_unbox");
    logger.info("Loaded: il2cpp_object_unbox");
    *(void**)(&value_box) = dlsym(imagehandle, "il2cpp_value_box");
    logger.info("Loaded: il2cpp_value_box");
    *(void**)(&monitor_enter) = dlsym(imagehandle, "il2cpp_monitor_enter");
    logger.info("Loaded: il2cpp_monitor_enter");
    *(void**)(&monitor_try_enter) = dlsym(imagehandle, "il2cpp_monitor_try_enter");
    logger.info("Loaded: il2cpp_monitor_try_enter");
    *(void**)(&monitor_exit) = dlsym(imagehandle, "il2cpp_monitor_exit");
    logger.info("Loaded: il2cpp_monitor_exit");
    *(void**)(&monitor_pulse) = dlsym(imagehandle, "il2cpp_monitor_pulse");
    logger.info("Loaded: il2cpp_monitor_pulse");
    *(void**)(&monitor_pulse_all) = dlsym(imagehandle, "il2cpp_monitor_pulse_all");
    logger.info("Loaded: il2cpp_monitor_pulse_all");
    *(void**)(&monitor_wait) = dlsym(imagehandle, "il2cpp_monitor_wait");
    logger.info("Loaded: il2cpp_monitor_wait");
    *(void**)(&monitor_try_wait) = dlsym(imagehandle, "il2cpp_monitor_try_wait");
    logger.info("Loaded: il2cpp_monitor_try_wait");
    *(void**)(&runtime_invoke) = dlsym(imagehandle, "il2cpp_runtime_invoke");
    logger.info("Loaded: il2cpp_runtime_invoke");
    *(void**)(&runtime_invoke_convert_args) = dlsym(imagehandle, "il2cpp_runtime_invoke_convert_args");
    logger.info("Loaded: il2cpp_runtime_invoke_convert_args");
    *(void**)(&runtime_class_init) = dlsym(imagehandle, "il2cpp_runtime_class_init");
    logger.info("Loaded: il2cpp_runtime_class_init");
    *(void**)(&runtime_object_init) = dlsym(imagehandle, "il2cpp_runtime_object_init");
    logger.info("Loaded: il2cpp_runtime_object_init");
    *(void**)(&runtime_object_init_exception) = dlsym(imagehandle, "il2cpp_runtime_object_init_exception");
    logger.info("Loaded: il2cpp_runtime_object_init_exception");
    *(void**)(&runtime_unhandled_exception_policy_set) = dlsym(imagehandle, "il2cpp_runtime_unhandled_exception_policy_set");
    logger.info("Loaded: il2cpp_runtime_unhandled_exception_policy_set");
    *(void**)(&string_length) = dlsym(imagehandle, "il2cpp_string_length");
    logger.info("Loaded: il2cpp_string_length");
    *(void**)(&string_chars) = dlsym(imagehandle, "il2cpp_string_chars");
    logger.info("Loaded: il2cpp_string_chars");
    *(void**)(&string_new) = dlsym(imagehandle, "il2cpp_string_new");
    logger.info("Loaded: il2cpp_string_new");
    *(void**)(&string_new_len) = dlsym(imagehandle, "il2cpp_string_new_len");
    logger.info("Loaded: il2cpp_string_new_len");
    *(void**)(&string_new_utf16) = dlsym(imagehandle, "il2cpp_string_new_utf16");
    logger.info("Loaded: il2cpp_string_new_utf16");
    *(void**)(&string_new_wrapper) = dlsym(imagehandle, "il2cpp_string_new_wrapper");
    logger.info("Loaded: il2cpp_string_new_wrapper");
    *(void**)(&string_intern) = dlsym(imagehandle, "il2cpp_string_intern");
    logger.info("Loaded: il2cpp_string_intern");
    *(void**)(&string_is_interned) = dlsym(imagehandle, "il2cpp_string_is_interned");
    logger.info("Loaded: il2cpp_string_is_interned");
    *(void**)(&thread_current) = dlsym(imagehandle, "il2cpp_thread_current");
    logger.info("Loaded: il2cpp_thread_current");
    *(void**)(&thread_attach) = dlsym(imagehandle, "il2cpp_thread_attach");
    logger.info("Loaded: il2cpp_thread_attach");
    *(void**)(&thread_detach) = dlsym(imagehandle, "il2cpp_thread_detach");
    logger.info("Loaded: il2cpp_thread_detach");
    *(void**)(&thread_get_all_attached_threads) = dlsym(imagehandle, "il2cpp_thread_get_all_attached_threads");
    logger.info("Loaded: il2cpp_thread_get_all_attached_threads");
    *(void**)(&is_vm_thread) = dlsym(imagehandle, "il2cpp_is_vm_thread");
    logger.info("Loaded: il2cpp_is_vm_thread");
    *(void**)(&current_thread_walk_frame_stack) = dlsym(imagehandle, "il2cpp_current_thread_walk_frame_stack");
    logger.info("Loaded: il2cpp_current_thread_walk_frame_stack");
    *(void**)(&thread_walk_frame_stack) = dlsym(imagehandle, "il2cpp_thread_walk_frame_stack");
    logger.info("Loaded: il2cpp_thread_walk_frame_stack");
    *(void**)(&current_thread_get_top_frame) = dlsym(imagehandle, "il2cpp_current_thread_get_top_frame");
    logger.info("Loaded: il2cpp_current_thread_get_top_frame");
    *(void**)(&thread_get_top_frame) = dlsym(imagehandle, "il2cpp_thread_get_top_frame");
    logger.info("Loaded: il2cpp_thread_get_top_frame");
    *(void**)(&current_thread_get_frame_at) = dlsym(imagehandle, "il2cpp_current_thread_get_frame_at");
    logger.info("Loaded: il2cpp_current_thread_get_frame_at");
    *(void**)(&thread_get_frame_at) = dlsym(imagehandle, "il2cpp_thread_get_frame_at");
    logger.info("Loaded: il2cpp_thread_get_frame_at");
    *(void**)(&current_thread_get_stack_depth) = dlsym(imagehandle, "il2cpp_current_thread_get_stack_depth");
    logger.info("Loaded: il2cpp_current_thread_get_stack_depth");
    *(void**)(&thread_get_stack_depth) = dlsym(imagehandle, "il2cpp_thread_get_stack_depth");
    logger.info("Loaded: il2cpp_thread_get_stack_depth");
    #ifdef UNITY_2019
    *(void**)(&override_stack_backtrace) = dlsym(imagehandle, "il2cpp_override_stack_backtrace");
    logger.info("Loaded: il2cpp_override_stack_backtrace");
    #endif
    *(void**)(&type_get_object) = dlsym(imagehandle, "il2cpp_type_get_object");
    logger.info("Loaded: il2cpp_type_get_object");
    *(void**)(&type_get_type) = dlsym(imagehandle, "il2cpp_type_get_type");
    logger.info("Loaded: il2cpp_type_get_type");
    *(void**)(&type_get_class_or_element_class) = dlsym(imagehandle, "il2cpp_type_get_class_or_element_class");
    logger.info("Loaded: il2cpp_type_get_class_or_element_class");
    *(void**)(&type_get_name) = dlsym(imagehandle, "il2cpp_type_get_name");
    logger.info("Loaded: il2cpp_type_get_name");
    *(void**)(&type_is_byref) = dlsym(imagehandle, "il2cpp_type_is_byref");
    logger.info("Loaded: il2cpp_type_is_byref");
    *(void**)(&type_get_attrs) = dlsym(imagehandle, "il2cpp_type_get_attrs");
    logger.info("Loaded: il2cpp_type_get_attrs");
    *(void**)(&type_equals) = dlsym(imagehandle, "il2cpp_type_equals");
    logger.info("Loaded: il2cpp_type_equals");
    *(void**)(&type_get_assembly_qualified_name) = dlsym(imagehandle, "il2cpp_type_get_assembly_qualified_name");
    logger.info("Loaded: il2cpp_type_get_assembly_qualified_name");
    #ifdef UNITY_2019
    *(void**)(&type_is_static) = dlsym(imagehandle, "il2cpp_type_is_static");
    logger.info("Loaded: il2cpp_type_is_static");
    *(void**)(&type_is_pointer_type) = dlsym(imagehandle, "il2cpp_type_is_pointer_type");
    logger.info("Loaded: il2cpp_type_is_pointer_type");
    #endif
    *(void**)(&image_get_assembly) = dlsym(imagehandle, "il2cpp_image_get_assembly");
    logger.info("Loaded: il2cpp_image_get_assembly");
    *(void**)(&image_get_name) = dlsym(imagehandle, "il2cpp_image_get_name");
    logger.info("Loaded: il2cpp_image_get_name");
    *(void**)(&image_get_filename) = dlsym(imagehandle, "il2cpp_image_get_filename");
    logger.info("Loaded: il2cpp_image_get_filename");
    *(void**)(&image_get_entry_point) = dlsym(imagehandle, "il2cpp_image_get_entry_point");
    logger.info("Loaded: il2cpp_image_get_entry_point");
    *(void**)(&image_get_class_count) = dlsym(imagehandle, "il2cpp_image_get_class_count");
    logger.info("Loaded: il2cpp_image_get_class_count");
    *(void**)(&image_get_class) = dlsym(imagehandle, "il2cpp_image_get_class");
    logger.info("Loaded: il2cpp_image_get_class");
    *(void**)(&capture_memory_snapshot) = dlsym(imagehandle, "il2cpp_capture_memory_snapshot");
    logger.info("Loaded: il2cpp_capture_memory_snapshot");
    *(void**)(&free_captured_memory_snapshot) = dlsym(imagehandle, "il2cpp_free_captured_memory_snapshot");
    logger.info("Loaded: il2cpp_free_captured_memory_snapshot");
    *(void**)(&set_find_plugin_callback) = dlsym(imagehandle, "il2cpp_set_find_plugin_callback");
    logger.info("Loaded: il2cpp_set_find_plugin_callback");
    *(void**)(&register_log_callback) = dlsym(imagehandle, "il2cpp_register_log_callback");
    logger.info("Loaded: il2cpp_register_log_callback");
    *(void**)(&debugger_set_agent_options) = dlsym(imagehandle, "il2cpp_debugger_set_agent_options");
    logger.info("Loaded: il2cpp_debugger_set_agent_options");
    *(void**)(&is_debugger_attached) = dlsym(imagehandle, "il2cpp_is_debugger_attached");
    logger.info("Loaded: il2cpp_is_debugger_attached");
    #ifdef UNITY_2019
    *(void**)(&register_debugger_agent_transport) = dlsym(imagehandle, "il2cpp_register_debugger_agent_transport");
    logger.info("Loaded: il2cpp_register_debugger_agent_transport");
    *(void**)(&debug_get_method_info) = dlsym(imagehandle, "il2cpp_debug_get_method_info");
    logger.info("Loaded: il2cpp_debug_get_method_info");
    #endif
    *(void**)(&unity_install_unitytls_interface) = dlsym(imagehandle, "il2cpp_unity_install_unitytls_interface");
    logger.info("Loaded: il2cpp_unity_install_unitytls_interface");
    *(void**)(&custom_attrs_from_class) = dlsym(imagehandle, "il2cpp_custom_attrs_from_class");
    logger.info("Loaded: il2cpp_custom_attrs_from_class");
    *(void**)(&custom_attrs_from_method) = dlsym(imagehandle, "il2cpp_custom_attrs_from_method");
    logger.info("Loaded: il2cpp_custom_attrs_from_method");
    *(void**)(&custom_attrs_get_attr) = dlsym(imagehandle, "il2cpp_custom_attrs_get_attr");
    logger.info("Loaded: il2cpp_custom_attrs_get_attr");
    *(void**)(&custom_attrs_has_attr) = dlsym(imagehandle, "il2cpp_custom_attrs_has_attr");
    logger.info("Loaded: il2cpp_custom_attrs_has_attr");
    *(void**)(&custom_attrs_construct) = dlsym(imagehandle, "il2cpp_custom_attrs_construct");
    logger.info("Loaded: il2cpp_custom_attrs_construct");
    *(void**)(&custom_attrs_free) = dlsym(imagehandle, "il2cpp_custom_attrs_free");
    logger.info("Loaded: il2cpp_custom_attrs_free");
    #ifdef UNITY_2019
    *(void**)(&class_set_userdata) = dlsym(imagehandle, "il2cpp_class_set_userdata");
    logger.info("Loaded: il2cpp_class_set_userdata");
    *(void**)(&class_get_userdata_offset) = dlsym(imagehandle, "il2cpp_class_get_userdata_offset");
    logger.info("Loaded: il2cpp_class_get_userdata_offset");
    #endif

    // MANUALLY DEFINED CONST DEFINITIONS
    *(void**)(&class_get_type_const) = dlsym(imagehandle, "il2cpp_class_get_type");
    logger.info("Loaded: il2cpp_class_get_type CONST VERSION!");
    *(void**)(&il2cpp_functions::class_get_name_const) = dlsym(imagehandle, "il2cpp_class_get_name");
    logger.info("Loaded: il2cpp_class_get_name CONST VERSION!");

    const char* err = dlerror();
    if (err) {
        logger.critical("A dlsym failed! dlerror: %s", err);
    }

    // XREF TRACES
    // Class::Init. 0x846A68 in 1.5, 0x9EC0A4 in 1.7.0, 0xA6D1B8 in 1.8.0b1
    Instruction ans((const int32_t*)HookTracker::GetOrig(array_new_specific));
    Instruction Array_NewSpecific(CRASH_UNLESS(ans.label));
    logger.debug("Array::NewSpecific offset: %lX", ((uintptr_t)Array_NewSpecific.addr) - getRealOffset(0));
    auto j2Cl_I = CRASH_UNLESS(Array_NewSpecific.findNthCall(1));  // also the 113th call in Runtime::Init
    Class_Init = (decltype(Class_Init))CRASH_UNLESS(j2Cl_I->label);
    logger.debug("Class::Init found? offset: %lX", ((uintptr_t)Class_Init) - getRealOffset(0));
    if (j2Cl_I != &Array_NewSpecific) delete j2Cl_I;
    usleep(1000);  // 0.001s

    // MetadataCache::GetTypeInfoFromTypeIndex. offset 0x84F764 in 1.5, 0x9F5250 in 1.7.0, 0xA7A79C in 1.8.0b1
    Instruction caha((const int32_t*)HookTracker::GetOrig(custom_attrs_has_attr));
    auto mchab = CRASH_UNLESS(caha.findNthDirectBranchWithoutLink(1));
    Instruction MetadataCache_HasAttribute(CRASH_UNLESS(mchab->label));
    if (mchab != &caha) delete mchab;
    auto j2MC_GTIFTI = CRASH_UNLESS(MetadataCache_HasAttribute.findNthCall(1));
    MetadataCache_GetTypeInfoFromTypeIndex = (decltype(MetadataCache_GetTypeInfoFromTypeIndex))CRASH_UNLESS(j2MC_GTIFTI->label);
    logger.debug("MetadataCache::GetTypeInfoFromTypeIndex found? offset: %lX",
        ((uintptr_t)MetadataCache_GetTypeInfoFromTypeIndex) - getRealOffset(0));
    if (j2MC_GTIFTI != &MetadataCache_HasAttribute) delete j2MC_GTIFTI;
    usleep(1000);  // 0.001s

    // MetadataCache::GetTypeInfoFromTypeDefinitionIndex. offset 0x84FBA4 in 1.5, 0x9F5690 in 1.7.0, 0xA75958 in 1.8.0b1
    Instruction tgcoec((const int32_t*)HookTracker::GetOrig(type_get_class_or_element_class));
    auto tgoecb = CRASH_UNLESS(tgcoec.findNthDirectBranchWithoutLink(1));
    Instruction Type_GetClassOrElementClass(CRASH_UNLESS(tgoecb->label));
    if (tgoecb != &tgcoec) delete tgoecb;
    auto j2MC_GTIFTDI = CRASH_UNLESS(Type_GetClassOrElementClass.findNthDirectBranchWithoutLink(5));
    MetadataCache_GetTypeInfoFromTypeDefinitionIndex =
        (decltype(MetadataCache_GetTypeInfoFromTypeDefinitionIndex))CRASH_UNLESS(j2MC_GTIFTDI->label);
    logger.debug("MetadataCache::GetTypeInfoFromTypeDefinitionIndex found? offset: %lX",
        ((uintptr_t)MetadataCache_GetTypeInfoFromTypeDefinitionIndex) - getRealOffset(0));
    if (j2MC_GTIFTDI != &Type_GetClassOrElementClass) delete j2MC_GTIFTDI;
    usleep(1000);  // 0.001s

    // Type::GetName. offset 0x8735DC in 1.5, 0xA1A458 in 1.7.0, 0xA7B634 in 1.8.0b1
    Instruction tanq((const int32_t*)HookTracker::GetOrig(type_get_assembly_qualified_name));
    auto j2T_GN = CRASH_UNLESS(tanq.findNthCall(1));
    _Type_GetName_ = (decltype(_Type_GetName_))CRASH_UNLESS(j2T_GN->label);
    logger.debug("Type::GetName found? offset: %lX", ((uintptr_t)_Type_GetName_) - getRealOffset(0));
    if (j2T_GN != &tanq) delete j2T_GN;
    usleep(1000);  // 0.001s

    // GenericClass::GetClass. offset 0x88DF64 in 1.5, 0xA34F20 in 1.7.0, 0xA6E4EC in 1.8.0b1
    Instruction cfit((const int32_t*)HookTracker::GetOrig(class_from_il2cpp_type));
    auto b = CRASH_UNLESS(cfit.findNthDirectBranchWithoutLink(1));
    Class_FromIl2CppType = (decltype(Class_FromIl2CppType))CRASH_UNLESS(b->label);
    if (b != &cfit) delete b;
    auto caseStart = CRASH_UNLESS(EvalSwitch(Class_FromIl2CppType, 1, 1, IL2CPP_TYPE_GENERICINST));
    auto j2GC_GC = CRASH_UNLESS(caseStart->findNthDirectBranchWithoutLink(1));
    delete caseStart;
    logger.debug("j2GC_GC: %s", j2GC_GC->toString().c_str());
    GenericClass_GetClass = (decltype(GenericClass_GetClass))CRASH_UNLESS(j2GC_GC->label);
    logger.debug("GenericClass::GetClass found? offset: %lX", ((uintptr_t)GenericClass_GetClass) - getRealOffset(0));
    if (j2GC_GC != caseStart) delete j2GC_GC;
    usleep(1000);  // 0.001s

    // Class::GetPtrClass.
    auto ptrCase = CRASH_UNLESS(EvalSwitch(Class_FromIl2CppType, 1, 1, IL2CPP_TYPE_PTR));
    auto j2C_GPC = CRASH_UNLESS(ptrCase->findNthDirectBranchWithoutLink(1));
    delete ptrCase;
    logger.debug("j2C_GPC: %s", j2C_GPC->toString().c_str());
    Class_GetPtrClass = (decltype(Class_GetPtrClass))CRASH_UNLESS(j2C_GPC->label);
    logger.debug("Class::GetPtrClass(Il2CppClass*) found? offset: %lX", ((uintptr_t)Class_GetPtrClass) - getRealOffset(0));
    if (j2C_GPC != ptrCase) delete j2C_GPC;
    usleep(1000);  // 0.001s

    // Assembly::GetAllAssemblies
    Instruction dga((const int32_t*)HookTracker::GetOrig(domain_get_assemblies));
    auto* j2A_GAA = CRASH_UNLESS(dga.findNthCall(1));    
    Assembly_GetAllAssemblies = (decltype(Assembly_GetAllAssemblies))CRASH_UNLESS(j2A_GAA->label);
    logger.debug("Assembly::GetAllAssemblies found? offset: %lX", ((uintptr_t)Assembly_GetAllAssemblies) - getRealOffset(0));
    if (j2A_GAA != &dga) delete j2A_GAA;
    usleep(1000);  // 0.001s


    CRASH_UNLESS(shutdown);
    // GC_free
    Instruction sd((const int32_t*)HookTracker::GetOrig(shutdown));
    auto sdb = CRASH_UNLESS(sd.findNthDirectBranchWithoutLink(1));
    auto* Runtime_Shutdown = CRASH_UNLESS(sdb->label);
    if (sdb != &sd) delete sdb;

    if (find_GC_free(Runtime_Shutdown)) {
        logger.debug("gc::GarbageCollector::FreeFixed found? offset: %lX", ((uintptr_t)GC_free) - getRealOffset(0));
        usleep(1000);  // 0.001s
    }

    // GarbageCollector::SetWriteBarrier(void*)
    if (find_GC_SetWriteBarrier((const int32_t*)HookTracker::GetOrig(gc_wbarrier_set_field))) {
        logger.debug("GarbageCollector::SetWriteBarrier found? offset: %lX", ((uintptr_t)GarbageCollector_SetWriteBarrier) - getRealOffset(0));
        usleep(1000);  // 0.001s
    }

    // GarbageCollector::AllocateFixed(size_t, void*)
    Instruction localDomainGet((const int32_t*)HookTracker::GetOrig(domain_get));
    auto* inst = CRASH_UNLESS(localDomainGet.findNthDirectBranchWithoutLink(1));
    if (find_GC_AllocFixed(inst)) {
        logger.debug("GarbageCollector::AllocateFixed found? offset: %lX", ((uintptr_t)GarbageCollector_AllocateFixed) - getRealOffset(0));
        usleep(1000);  // 0.001s
    }
    if (inst != &localDomainGet) delete inst;

    // il2cpp_defaults
    Instruction iu16((const int32_t*)HookTracker::GetOrig(init_utf16));
    auto j2R_I = CRASH_UNLESS(iu16.findNthCall(3));
    Instruction Runtime_Init(CRASH_UNLESS(j2R_I->label));
    if (j2R_I != &iu16) delete j2R_I;
    // alternatively, could just get the 1st ADRP in Runtime::Init with dest reg x20 (or the 9th ADRP)
    // We DO need to skip at least one ret, though.
    auto ldr = CRASH_UNLESS(Runtime_Init.findNth(6, std::mem_fn(&Instruction::isLoad), 1));  // the load for the malloc that precedes our adrp
    il2cpp_functions::defaults = (decltype(il2cpp_functions::defaults))ExtractAddress(ldr->addr, 1, 1);
    logger.debug("il2cpp_defaults found? offset: %lX", ((uintptr_t)defaults) - getRealOffset(0));
    if (ldr != &Runtime_Init) delete ldr;
    usleep(1000);  // 0.001s

    // FIELDS
    // Extract locations of s_GlobalMetadataHeader, s_Il2CppMetadataRegistration, & s_GlobalMetadata
    // TODO: refactor to reduce instruction re-parsing?
    il2cpp_functions::s_GlobalMetadataHeaderPtr = (decltype(il2cpp_functions::s_GlobalMetadataHeaderPtr))CRASH_UNLESS(
        ExtractAddress(il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex, 3, 1));
    usleep(1000);  // 0.001s
    il2cpp_functions::s_Il2CppMetadataRegistrationPtr = (decltype(il2cpp_functions::s_Il2CppMetadataRegistrationPtr))CRASH_UNLESS(
        ExtractAddress(il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex, 4, 1));
    usleep(1000);  // 0.001s
    il2cpp_functions::s_GlobalMetadataPtr = (decltype(il2cpp_functions::s_GlobalMetadataPtr))CRASH_UNLESS(
        ExtractAddress(il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex, 5, 1));
    usleep(1000);  // 0.001s
    logger.debug("All global constants found!");

    // NOTE: Runtime.Shutdown is NOT CALLED even for exceptions!
    // There is practically no use in hooking this becuase of that.
    // Runtime.Shutdown (for file loggers)
    // if (Runtime_Shutdown) {
    //     logger.info("hook installing to: %p (offset %lX)", Runtime_Shutdown, ((uintptr_t)Runtime_Shutdown) - getRealOffset(0));
    //     INSTALL_HOOK_DIRECT(logger, shutdown_hook, (void*)Runtime_Shutdown);
    // } else {
    //     logger.critical("Failed to parse il2cpp_shutdown's implementation address! Could not install shutdown hook for closing file logs.");
    // }

    dlclose(imagehandle);
    initialized = true;
    logger.info("il2cpp_functions: Init: Successfully loaded all il2cpp functions!");
    usleep(100);  // 0.0001s
}

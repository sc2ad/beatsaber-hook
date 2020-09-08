#include "il2cpp-functions.hpp"
#ifndef IL2CPP_TYPE_CHECK_H
#define IL2CPP_TYPE_CHECK_H

#if __has_include(<concepts>)
#include <concepts>
template<typename T>
constexpr bool has_no_arg_get = requires(const T& t) {
    t.get();
};
#elif __has_include(<experimental/type_traits>)
#include <experimental/type_traits>
template<typename T>
using no_arg_get_t = decltype( T::get() );

template<typename T>
constexpr bool has_no_arg_get = std::experimental::is_detected_v<no_arg_get_t, T>;
#else
#error No libraries for the implementation of has_no_arg_get available!
#endif

#include "utils.h"

namespace il2cpp_utils {
    // Returns the first matching class from the given namespace and typeName by searching through all assemblies that are loaded.
    Il2CppClass* GetClassFromName(std::string_view name_space, std::string_view type_name);

    // Function made by zoller27osu, modified by Sc2ad
    // PLEASE don't use, there are easier ways to get generics (see CreateParam, CreateFieldValue)
    Il2CppClass* MakeGeneric(const Il2CppClass* klass, std::vector<const Il2CppClass*> args);
    Il2CppClass* MakeGeneric(const Il2CppClass* klass, const Il2CppType** types, uint32_t numTypes);

    // Framework provided by DaNike
    namespace il2cpp_type_check {
        // You should extend this class for any nested/inner type of a generic type, and also put a `using declaring_type = Blah;`
        class NestedType {};
        // To fix "no member named 'get' in il2cpp_type_check::il2cpp_arg_class<Blah>", just define il2cpp_no_arg_class<Blah>!
        // The macros like DEFINE_IL2CPP_ARG_TYPE make this easy! The only reason to define an il2cpp_arg_class struct is if,
        // given a value of Blah, the class returned should depend on the value, and not in the same way as Blah's base class.
        template<typename T, class Enable = void>
        struct il2cpp_no_arg_class { };

        #if __has_feature(cxx_rtti)
        template<typename T>
        struct il2cpp_no_arg_class<T, typename std::enable_if_t<std::is_base_of_v<NestedType, T>>> {
            // TODO: make this work on any class with a `using declaring_type`, then remove NestedType
            static inline Il2CppClass* get() {
                il2cpp_functions::Init();
                Il2CppClass* declaring = il2cpp_no_arg_class<typename T::declaring_type>::get();
                Il2CppClass* classWithNested = declaring;
                // TODO: 32bit
                // if (declaring->generic_class) {
                //     // Class::GetNestedTypes refuses to work on generic instances, so get the generic template instead
                //     classWithNested = CRASH_UNLESS(il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex(declaring->generic_class->typeDefinitionIndex));
                // }
                std::string typeName = type_name<T>();
                auto idx = typeName.find_last_of(':');
                if (idx >= 0) typeName = typeName.substr(idx+1);

                // log(INFO, "type_name: %s", typeName.c_str());
                void* myIter = nullptr;
                Il2CppClass* found = nullptr;
                while (Il2CppClass* nested = il2cpp_functions::class_get_nested_types(classWithNested, &myIter)) {
                    // log(INFO, "nested->name: %s", nested->name);
                    if (typeName == nested->name) {
                        found = nested;
                        break;
                    }
                }
                CRASH_UNLESS(found);
                if (declaring->generic_class) {
                    const Il2CppGenericInst* genInst = declaring->generic_class->context.class_inst;
                    found = CRASH_UNLESS(il2cpp_utils::MakeGeneric(found, genInst->type_argv, genInst->type_argc));
                }
                return found;
            }
        };
        #endif

        template<typename T>
        struct il2cpp_arg_class {
            static inline Il2CppClass* get([[maybe_unused]] T arg) {
                return il2cpp_no_arg_class<T>::get();
            }
        };

        #define DEFINE_IL2CPP_ARG_TYPE(type, nameSpace, className) \
        template<> \
        struct ::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<type> { \
            static inline Il2CppClass* get() { \
                return il2cpp_utils::GetClassFromName(nameSpace, className); \
            } \
        }

        DEFINE_IL2CPP_ARG_TYPE(int8_t, "System", "SByte");
        DEFINE_IL2CPP_ARG_TYPE(uint8_t, "System", "Byte");
        DEFINE_IL2CPP_ARG_TYPE(int16_t, "System", "Int16");  // "short"
        DEFINE_IL2CPP_ARG_TYPE(uint16_t, "System", "UInt16");  // "ushort"
        DEFINE_IL2CPP_ARG_TYPE(int32_t, "System", "Int32");  // "int"
        DEFINE_IL2CPP_ARG_TYPE(uint32_t, "System", "UInt32");  // "uint"
        DEFINE_IL2CPP_ARG_TYPE(int64_t, "System", "Int64");  // "long"
        DEFINE_IL2CPP_ARG_TYPE(uint64_t, "System", "UInt64");  // "ulong"

        DEFINE_IL2CPP_ARG_TYPE(float, "System", "Single");
        DEFINE_IL2CPP_ARG_TYPE(double, "System", "Double");

        DEFINE_IL2CPP_ARG_TYPE(bool, "System", "Boolean");
        DEFINE_IL2CPP_ARG_TYPE(Il2CppChar, "System", "Char");

        #ifdef NEED_UNSAFE_CSHARP
        DEFINE_IL2CPP_ARG_TYPE(void, "System", "Void");
        DEFINE_IL2CPP_ARG_TYPE(Il2CppObject*, "System", "Object");
        #endif
        DEFINE_IL2CPP_ARG_TYPE(Il2CppString*, "System", "String");
        DEFINE_IL2CPP_ARG_TYPE(Il2CppReflectionType*, "System", "Type");
        DEFINE_IL2CPP_ARG_TYPE(Il2CppReflectionRuntimeType*, "System", "RuntimeType");

        DEFINE_IL2CPP_ARG_TYPE(long double, "System", "Decimal");
        DEFINE_IL2CPP_ARG_TYPE(Color, "UnityEngine", "Color");
        DEFINE_IL2CPP_ARG_TYPE(Vector2, "UnityEngine", "Vector2");
        DEFINE_IL2CPP_ARG_TYPE(Vector3, "UnityEngine", "Vector3");
        DEFINE_IL2CPP_ARG_TYPE(Vector4, "UnityEngine", "Vector4");
        DEFINE_IL2CPP_ARG_TYPE(Quaternion, "UnityEngine", "Quaternion");
        DEFINE_IL2CPP_ARG_TYPE(Rect, "UnityEngine", "Rect");
        DEFINE_IL2CPP_ARG_TYPE(Scene, "UnityEngine.SceneManagement", "Scene");

        template<typename TArg>
        struct il2cpp_no_arg_class<Array<TArg>*> {
            static inline Il2CppClass* get() {
                il2cpp_functions::Init();
                if constexpr (std::is_same_v<std::decay_t<TArg>, Il2CppObject*>) {
                    return il2cpp_functions::array_class_get(il2cpp_utils::GetClassFromName("System", "Object"), 1);
                } else {
                    Il2CppClass* eClass = RET_0_UNLESS(il2cpp_no_arg_class<TArg>::get());
                    return il2cpp_functions::array_class_get(eClass, 1);
                }
            }
        };

        template<>
        struct il2cpp_arg_class<Il2CppClass*> {
            static inline Il2CppClass* get(Il2CppClass* arg) {
                return arg;
            }
        };

        template<>
        struct il2cpp_arg_class<Il2CppType*> {
            static inline Il2CppClass* get(Il2CppType* arg) {
                RET_0_UNLESS(arg);
                il2cpp_functions::Init();
                return il2cpp_functions::class_from_il2cpp_type(arg);
            }
        };

        // TODO: is_il2cpp_object type trait?

        template<typename T>
        struct il2cpp_arg_class<T*> {
            static inline Il2CppClass* get(T* arg) {
                using ptr_arg_class = il2cpp_no_arg_class<T*>;
                if constexpr (has_no_arg_get<ptr_arg_class>) {
                    if (!arg)
                        return ptr_arg_class::get();
                    // otherwise, falls through to Il2CppObject* handler
                }
                // TODO: 32bit
                // else if constexpr (has_no_arg_get<element_arg_class>) {
                //     Il2CppClass* elementClass = element_arg_class::get();
                //     return il2cpp_functions::Class_GetPtrClass(elementClass);
                // }
                il2cpp_functions::Init();
                return RET_0_UNLESS(il2cpp_functions::object_get_class(reinterpret_cast<Il2CppObject*>(arg)));
            }
        };

        template<template<typename... ST> class S>
        struct il2cpp_gen_struct_no_arg_class;

        template<template<typename... ST> class S>
        struct il2cpp_gen_class_no_arg_class;

        template<typename... TArgs, template<typename... ST> class S>
        struct il2cpp_no_arg_class<S<TArgs...>> {
            static inline Il2CppClass* get() {
                auto* klass = il2cpp_gen_struct_no_arg_class<S>::get();
                return il2cpp_utils::MakeGeneric(klass, {il2cpp_no_arg_class<TArgs>::get()...});
            }
        };

        template<typename... TArgs, template<typename... ST> class S>
        struct il2cpp_no_arg_class<S<TArgs...>*> {
            static inline Il2CppClass* get() {
                Il2CppClass* genTemplate;
                bool isStruct = false;
                if constexpr (has_no_arg_get<il2cpp_gen_class_no_arg_class<S>>) {
                    genTemplate = il2cpp_gen_class_no_arg_class<S>::get();
                } else if constexpr (has_no_arg_get<il2cpp_gen_struct_no_arg_class<S>>) {
                    genTemplate = il2cpp_gen_struct_no_arg_class<S>::get();
                    isStruct = true;
                } else {
                    static_assert(false_t<S<TArgs...>>);
                }
                auto* genInst = il2cpp_utils::MakeGeneric(genTemplate, {il2cpp_no_arg_class<TArgs>::get()...});
                // TODO: 32bit
                // if (isStruct) {
                //     return il2cpp_functions::Class_GetPtrClass(genInst);
                // }
                return genInst;
            }
        };

        #define DEFINE_IL2CPP_ARG_TYPE_GENERIC_STRUCT(templateType, nameSpace, className) \
        template<> \
        struct ::il2cpp_utils::il2cpp_type_check::il2cpp_gen_struct_no_arg_class<templateType> { \
            static inline Il2CppClass* get() { \
                return il2cpp_utils::GetClassFromName(nameSpace, className); \
            } \
        }

        #define DEFINE_IL2CPP_ARG_TYPE_GENERIC_CLASS(templateType, nameSpace, className) \
        template<> \
        struct ::il2cpp_utils::il2cpp_type_check::il2cpp_gen_class_no_arg_class<templateType> { \
            static inline Il2CppClass* get() { \
                return il2cpp_utils::GetClassFromName(nameSpace, className); \
            } \
        }

        template<typename T>
        struct il2cpp_arg_type { };

        template<typename T>
        struct il2cpp_arg_type<T&> {
            static inline const Il2CppType* get(T& arg) {
                // A method can store a result back to a non-const ref! Make the type byref!
                Il2CppClass* klass = il2cpp_arg_class<T>::get(arg);
                return &klass->this_arg;
            }
        };

        template<typename T>
        struct il2cpp_arg_type<const T&> {
            static inline const Il2CppType* get(const T& arg) {
                // A method cannot store a result back to a const ref. It is not a C# ref.
                const Il2CppClass* klass = il2cpp_arg_class<T>::get(arg);
                return &klass->byval_arg;
            }
        };

        template<typename T>
        struct il2cpp_arg_type<T&&> {
            static inline const Il2CppType* get(T&& arg) {
                Il2CppClass* klass = il2cpp_arg_class<T>::get(arg);
                return &klass->byval_arg;
            }
        };

        template<typename T>
        struct il2cpp_arg_ptr {
            static inline void* get(T const& arg) {
                return reinterpret_cast<void*>(&arg);
            }
        };
        template<typename T>
        struct il2cpp_arg_ptr<T*> {
            static inline void* get(T* arg) {
                return reinterpret_cast<void*>(arg);
            }
        };
    }
}
#endif /* #ifndef IL2CPP_TYPE_CHECK_H */

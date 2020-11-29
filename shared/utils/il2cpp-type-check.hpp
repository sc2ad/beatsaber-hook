#include "il2cpp-functions.hpp"
#ifndef IL2CPP_TYPE_CHECK_H
#define IL2CPP_TYPE_CHECK_H

#if __has_include(<concepts>)
#include <concepts>
template<typename T>
constexpr bool has_get = requires(const T& t) {
    t.get;
};
#elif __has_include(<experimental/type_traits>)
#include <experimental/type_traits>
template<typename T>
using get_t = decltype( &T::get );

template<typename T>
constexpr bool has_get = std::experimental::is_detected_v<get_t, T>;
#else
#error No libraries for the implementation of has_get available!
#endif

#include "utils.h"
#include <string_view>
#include <vector>

#ifndef classof
// Returns the Il2CppClass* of the provided type T.
// Should be a pointer type if it is a reference type, otherwise it should be a value type.
#define classof(...) (::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<__VA_ARGS__>::get())
#endif

#ifndef csTypeOf
// Returns the Il2CppReflectionType* of the provided type T.
// Should be a pointer type if it is a reference type, otherwise it should be a value type.
#define csTypeOf(...) (::il2cpp_utils::GetSystemType(classof(__VA_ARGS__)))
#endif

namespace il2cpp_utils {
    // Returns the first matching class from the given namespace and typeName by searching through all assemblies that are loaded.
    Il2CppClass* GetClassFromName(std::string_view name_space, std::string_view type_name);

    // Function made by zoller27osu, modified by Sc2ad
    // PLEASE don't use, there are easier ways to get generics (see CreateParam, CreateFieldValue)
    Il2CppClass* MakeGeneric(const Il2CppClass* klass, std::vector<const Il2CppClass*> args);
    Il2CppClass* MakeGeneric(const Il2CppClass* klass, const Il2CppType** types, uint32_t numTypes);

    // Framework provided by DaNike
    namespace il2cpp_type_check {
        // You should extend this class for any nested/inner type of a generic type, and also put a
        // `using declaring_type = Foo<T>*;` (minus the * if it is a value type)
        // and a `static constexpr std::string_view NESTED_NAME = "Bar";`
        class NestedType {};
        // To fix "no member named 'get' in il2cpp_type_check::il2cpp_arg_class<Blah>", just define il2cpp_no_arg_class<Blah>!
        // The macros like DEFINE_IL2CPP_ARG_TYPE make this easy! The only reason to define an il2cpp_arg_class struct is if,
        // given a value of Blah, the class returned should depend on the value, and not in the same way as Blah's base class.
        template<typename T, class Enable = void>
        struct il2cpp_no_arg_class { };

        template<typename T>
        struct il2cpp_no_arg_class<T*, typename std::enable_if_t<has_get<il2cpp_no_arg_class<T>>>> {
            static inline Il2CppClass* get() {
                il2cpp_functions::Init();
                static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("il2cpp_type_check").WithContext("il2cpp_no_arg_class");
                auto* klass = RET_0_UNLESS(logger, il2cpp_no_arg_class<T>::get());
                RET_0_UNLESS(logger, il2cpp_functions::class_is_valuetype(klass));
                return il2cpp_functions::Class_GetPtrClass(klass);
            }
        };

        template<typename T>
        struct il2cpp_no_arg_class<T, typename std::enable_if_t<std::is_base_of_v<NestedType, T>>> {
            // TODO: make this work on any class with a `using declaring_type`, then remove NestedType
            static inline Il2CppClass* get() {
                il2cpp_functions::Init();
                Il2CppClass* declaring = il2cpp_no_arg_class<typename T::declaring_type>::get();
                Il2CppClass* classWithNested = declaring;
                if (declaring->generic_class) {
                    // Class::GetNestedTypes refuses to work on generic instances, so get the generic template instead
                    classWithNested = CRASH_UNLESS(il2cpp_functions::MetadataCache_GetTypeInfoFromTypeDefinitionIndex(declaring->generic_class->typeDefinitionIndex));
                }
                #if __has_feature(cxx_rtti)
                std::string typeName = type_name<T>();
                auto idx = typeName.find_last_of(':');
                if (idx >= 0) typeName = typeName.substr(idx+1);
                #else
                std::string typeName(T::NESTED_NAME);
                #endif

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

        template<typename T>
        struct il2cpp_arg_class {
            static inline Il2CppClass* get([[maybe_unused]] T arg) {
                if constexpr (has_get<il2cpp_no_arg_class<T>>) {
                    return il2cpp_no_arg_class<T>::get();
                }
                return nullptr;
            }
        };

        #define DEFINE_IL2CPP_DEFAULT_TYPE(type, fieldName) \
        template<> \
        struct ::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<type> { \
            static inline Il2CppClass* get() { \
                il2cpp_functions::Init(); \
                return il2cpp_functions::defaults->fieldName##_class; \
            } \
        }

        #define DEFINE_IL2CPP_ARG_TYPE(type, nameSpace, className) \
        template<> \
        struct ::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<type> { \
            static inline Il2CppClass* get() { \
                return il2cpp_utils::GetClassFromName(nameSpace, className); \
            } \
        }

        DEFINE_IL2CPP_DEFAULT_TYPE(int8_t, sbyte);
        DEFINE_IL2CPP_DEFAULT_TYPE(uint8_t, byte);
        DEFINE_IL2CPP_DEFAULT_TYPE(int16_t, int16);  // "short"
        DEFINE_IL2CPP_DEFAULT_TYPE(uint16_t, uint16);  // "ushort"
        DEFINE_IL2CPP_DEFAULT_TYPE(int32_t, int32);  // "int"
        DEFINE_IL2CPP_DEFAULT_TYPE(uint32_t, uint32);  // "uint"
        DEFINE_IL2CPP_DEFAULT_TYPE(int64_t, int64);  // "long"
        DEFINE_IL2CPP_DEFAULT_TYPE(uint64_t, uint64);  // "ulong"

        DEFINE_IL2CPP_DEFAULT_TYPE(float, single);
        DEFINE_IL2CPP_DEFAULT_TYPE(double, double);

        DEFINE_IL2CPP_DEFAULT_TYPE(bool, boolean);
        DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppChar, char);

        DEFINE_IL2CPP_DEFAULT_TYPE(void, void);
        DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppObject*, object);
        DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppString*, string);
        DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppArray*, array);
        DEFINE_IL2CPP_DEFAULT_TYPE(Il2CppReflectionType*, systemtype);

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
                    il2cpp_functions::CheckS_GlobalMetadata();
                    return il2cpp_functions::array_class_get(il2cpp_functions::defaults->object_class, 1);
                } else {
                    static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("il2cpp_type_check").WithContext("il2cpp_no_arg_class<Array<TArg>*>");
                    Il2CppClass* eClass = RET_0_UNLESS(logger, il2cpp_no_arg_class<TArg>::get());
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
                static auto logger = Logger::get().WithContext("il2cpp_utils").WithContext("il2cpp_type_check").WithContext("il2cpp_arg_class");
                RET_0_UNLESS(logger, arg);
                il2cpp_functions::Init();
                return il2cpp_functions::class_from_il2cpp_type(arg);
            }
        };

        // TODO: is_il2cpp_object type trait?

        template<typename T>
        struct il2cpp_arg_class<T*> {
            static inline Il2CppClass* get(T* arg) {
                using element_arg_class = il2cpp_no_arg_class<T>;
                if constexpr (has_get<element_arg_class>) {
                    Il2CppClass* elementClass = element_arg_class::get();
                    il2cpp_functions::Init();
                    return il2cpp_functions::Class_GetPtrClass(elementClass);
                }
                if (arg) {
                    il2cpp_functions::Init();
                    auto* klass = il2cpp_functions::object_get_class(reinterpret_cast<Il2CppObject*>(arg));
                    if (klass && klass->klass == klass) return klass;
                }
                using ptr_arg_class = il2cpp_no_arg_class<T*>;
                if constexpr (has_get<ptr_arg_class>) {
                    return ptr_arg_class::get();
                }
                return nullptr;
            }
        };

        template<template<typename... ST> class S>
        struct il2cpp_gen_struct_no_arg_class;

        template<template<typename... ST> class S>
        struct il2cpp_gen_class_no_arg_class;

        template<typename... TArgs, template<typename... ST> class S>
        struct il2cpp_no_arg_class<S<TArgs...>, typename std::enable_if_t<has_get<il2cpp_gen_struct_no_arg_class<S>>>> {
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
                if constexpr (has_get<il2cpp_gen_class_no_arg_class<S>>) {
                    genTemplate = il2cpp_gen_class_no_arg_class<S>::get();
                } else if constexpr (has_get<il2cpp_gen_struct_no_arg_class<S>>) {
                    genTemplate = il2cpp_gen_struct_no_arg_class<S>::get();
                    isStruct = true;
                } else {
                    static_assert(false_t<S<TArgs...>>);
                }
                auto* genInst = il2cpp_utils::MakeGeneric(genTemplate, {il2cpp_no_arg_class<TArgs>::get()...});
                if (isStruct) {
                    il2cpp_functions::Init();
                    return il2cpp_functions::Class_GetPtrClass(genInst);
                }
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

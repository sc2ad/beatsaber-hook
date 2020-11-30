#pragma once
#include <type_traits>

namespace mod {
    struct tag_type {};
}

namespace modloader {
    using init_t = void(mod::tag_type);
    using lib_main_t = void(mod::tag_type);
    using preload_t = void(mod::tag_type);
    using load_t = void(mod::tag_type);
}

namespace detail {
    template<typename ...All>
    struct type_list {
        template<template<typename> typename A>
        using map = type_list<>;
        template<template<typename...> typename A>
        using apply = A<>;
    };
    template<typename F, typename ...Rest>
    struct type_list<F, Rest...> {
        using first = F;
        using rest = type_list<Rest...>;
        template<template<typename> typename A>
        using map = type_list<A<F>, A<Rest>...>;
        template<template<typename...> typename A>
        using apply = A<F, Rest...>;
    };

    template<typename> struct fptr;
    template<typename R, typename ...A>
    struct fptr<R(A...)> {
        using ret_type = R;
        using args_types = type_list<A...>;
    };

    template<typename T>
    struct type_only {
        /*implicit*/ operator T();
    };

    template<template<typename...> typename A, typename ...With>
    struct bind_with {
        template<typename ...B>
        using type = A<With..., B...>;
    };

    template<typename A, typename B>
    using alias = A;
}

#define EXEC(...) __VA_ARGS__
#define MEMBERS(M) EXEC(M(init)) EXEC(M(preload)) EXEC(M(lib_main)) EXEC(M(load))

template<typename T = void>
class checker {
    struct spec_a {};
    struct spec_b : public spec_a {};

#define CHECKER_DEFINE_MEMBERS(name) \
private: \
    template<typename ...ArgT> \
    struct _check_##name##_helper { \
        using tag = ::mod::tag_type; \
        using f = ::detail::fptr<::modloader::name##_t>; \
        static_assert(std::is_same_v<f::args_types::first, tag>, "First argument mismatch"); \
        static constexpr bool check() { \
            return std::is_same_v<decltype(name(::std::declval<::detail::alias<tag, T>>(), \
                ::std::declval<::detail::type_only<ArgT>>()...)), f::ret_type>; \
        } \
    }; \
    static constexpr bool _check_##name(spec_b const&) { \
        return ::detail::fptr<::modloader::name##_t>::args_types:: \
            rest::template apply<_check_##name##_helper>::check(); \
    } \
    static constexpr bool _check_##name(spec_a const&) \
    { return false; } \
public: \
    static constexpr bool check_##name() \
    { return _check_##name(spec_b{}); }

    MEMBERS(CHECKER_DEFINE_MEMBERS);
#undef CHECKER_DEFINE_MEMBERS

    static constexpr bool has_any() {
#define HAS_ANY_OPERATOR(name) check_##name() ||
        return MEMBERS(HAS_ANY_OPERATOR) false;
#undef HAS_ANY_OPERATOR
    }

};

#define CHECK(name) static_assert(checker<>::check_##name(), \
                                    #name " does not exist or is wrong!");
#define CHECK_MOD_INIT CHECK(init)
#define CHECK_MOD_PRELOAD CHECK(preload)
#define CHECK_MOD_LIB_MAIN CHECK(lib_main)
#define CHECK_MOD_LOAD CHECK(load)

#define CHECK_MOD_ANY static_assert(checker<>::has_any(), \
                                "You do not define any of the required functions");
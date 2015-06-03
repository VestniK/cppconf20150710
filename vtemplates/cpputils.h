#pragma once

#include <type_traits>

// std::experimental::apply and dependencies
#if defined(HAVE_EXPERIMENTAL_TUPLE)
#include <experimental/tuple>
#else
#include <utility>
#endif

namespace cpputils {
#if defined(__cpp_lib_experimental_apply)
using apply = std::experimental::apply;
#else
namespace __internals {
template <typename F, typename Tuple, std::size_t... I>
constexpr decltype(auto) apply(F &&f, Tuple &&t, std::index_sequence<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}
} // namespace __internals

template <class F, class Tuple>
constexpr decltype(auto) apply(F &&f, Tuple &&t) {
    return __internals::apply(
        std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
}
#endif

namespace __internals {
template<typename T>
struct const_memfn_helper: std::false_type {};

template<typename T, class U>
struct const_memfn_helper<T U::* const>: std::is_function<T> {};
}

template<typename T>
struct is_const_member_function_pointer: __internals::const_memfn_helper<
    typename std::remove_cv<T>::type
> {};

}


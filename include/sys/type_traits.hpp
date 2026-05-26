// sys/type_traits.hpp — minimal type traits.
#pragma once

namespace sys {

template <class T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant;
    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template <class T> struct remove_reference { using type = T; };
template <class T> struct remove_reference<T&> { using type = T; };
template <class T> struct remove_reference<T&&> { using type = T; };
template <class T> using remove_reference_t = typename remove_reference<T>::type;

template <class T> struct remove_const { using type = T; };
template <class T> struct remove_const<const T> { using type = T; };
template <class T> using remove_const_t = typename remove_const<T>::type;

template <class T> struct remove_volatile { using type = T; };
template <class T> struct remove_volatile<volatile T> { using type = T; };

template <class T>
struct remove_cv { using type = typename remove_const<typename remove_volatile<T>::type>::type; };
template <class T> using remove_cv_t = typename remove_cv<T>::type;

template <class T> struct is_lvalue_reference : false_type {};
template <class T> struct is_lvalue_reference<T&> : true_type {};

template <class T, class U> struct is_same : false_type {};
template <class T> struct is_same<T, T> : true_type {};
template <class T, class U> inline constexpr bool is_same_v = is_same<T, U>::value;

template <bool B, class T = void> struct enable_if {};
template <class T> struct enable_if<true, T> { using type = T; };
template <bool B, class T = void> using enable_if_t = typename enable_if<B, T>::type;

template <bool B, class T, class F> struct conditional { using type = T; };
template <class T, class F> struct conditional<false, T, F> { using type = F; };
template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

template <class T> struct is_array : false_type {};
template <class T> struct is_array<T[]> : true_type {};
template <class T, decltype(sizeof(0)) N> struct is_array<T[N]> : true_type {};

template <class T> struct is_pointer : false_type {};
template <class T> struct is_pointer<T*> : true_type {};

template <class T> struct remove_extent { using type = T; };
template <class T> struct remove_extent<T[]> { using type = T; };
template <class T, decltype(sizeof(0)) N> struct remove_extent<T[N]> { using type = T; };
template <class T> using remove_extent_t = typename remove_extent<T>::type;

template <class T>
struct decay {
private:
    using U = typename remove_reference<T>::type;
public:
    using type = typename remove_cv<U>::type;
};
template <class T> using decay_t = typename decay<T>::type;

template <class T>
typename remove_reference<T>::type&& declval_impl();  // never defined; declval-like
template <class T>
typename remove_reference<T>::type&& declval() noexcept;  // never defined; for SFINAE.

namespace detail {
template <class T> struct is_integral_base : false_type {};
template <> struct is_integral_base<bool> : true_type {};
template <> struct is_integral_base<char> : true_type {};
template <> struct is_integral_base<signed char> : true_type {};
template <> struct is_integral_base<unsigned char> : true_type {};
template <> struct is_integral_base<wchar_t> : true_type {};
template <> struct is_integral_base<char16_t> : true_type {};
template <> struct is_integral_base<char32_t> : true_type {};
template <> struct is_integral_base<short> : true_type {};
template <> struct is_integral_base<unsigned short> : true_type {};
template <> struct is_integral_base<int> : true_type {};
template <> struct is_integral_base<unsigned int> : true_type {};
template <> struct is_integral_base<long> : true_type {};
template <> struct is_integral_base<unsigned long> : true_type {};
template <> struct is_integral_base<long long> : true_type {};
template <> struct is_integral_base<unsigned long long> : true_type {};
}  // namespace detail

template <class T>
struct is_integral : detail::is_integral_base<remove_cv_t<T>> {};
template <class T>
inline constexpr bool is_integral_v = is_integral<T>::value;

}  // namespace sys

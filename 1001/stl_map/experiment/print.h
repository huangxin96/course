#include <type_traits>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>

namespace _print_details {
    template <class T, class = void>
    struct _printer {
        static void print(T const &t) {
            std::cout << t;
        }
    };

    template <class T>
    using _rmcvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template <class T, class U = void, class = void>
    struct _enable_if_tuple {
        using not_type = U;
    };

    template <class T, class U>
    struct _enable_if_tuple<T, U, std::void_t<decltype(std::tuple_size<T>::value)>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_map {
        using not_type = U;
    };

    template <class T, class U>
    struct _enable_if_map<T, U, std::enable_if_t<std::is_same_v<typename T::value_type, std::pair<typename T::key_type const, typename T::mapped_type>>>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_iterable {
        using not_type = U;
    };

    template <class T, class U>
    struct _enable_if_iterable<T, U, std::void_t<typename std::iterator_traits<decltype(std::begin(std::declval<T const &>()))>::value_type>> {
        using type = U;
    };

    /* template <class T, class U> */
    /* struct _enable_if_iterable<T, U, std::void_t<typename std::iterator_traits<typename T::const_iterator>::value_type>> { */
    /*     using type = U; */
    /* }; */

    template <class T, class U = void, class = void>
    struct _enable_if_char {
        using not_type = U;
    };

    template <class U>
    struct _enable_if_char<char, U, void> {
        using not_type = U;
    };

    template <class U>
    struct _enable_if_char<wchar_t, U, void> {
        using not_type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_string {
        using not_type = U;
    };

    template <class T, class Alloc, class Traits, class U>
    struct _enable_if_string<std::basic_string<T, Alloc, Traits>, U, void> {
        using type = U;
    };

    template <class T, class Traits, class U>
    struct _enable_if_string<std::basic_string_view<T, Traits>, U, void> {
        using type = U;
    };

    template <class T, class U>
    struct _enable_if_string<T, U, std::enable_if_t<std::is_pointer_v<std::decay_t<T>> && std::is_same_v<std::remove_const_t<std::remove_pointer_t<std::decay_t<T>>>, char>>> {
        using type = U;
    };

    template <class T>
    struct _printer<T, typename _enable_if_iterable<T, typename _enable_if_string<T, typename _enable_if_tuple<T, typename _enable_if_map<T>::not_type>::not_type>::not_type>::type> {
        static void print(T const &t) {
            std::cout << "{";
            bool once = false;
            for (auto const &v: t) {
                if (once) {
                    std::cout << ", ";
                } else {
                    once = true;
                }
                _printer<_rmcvref_t<decltype(v)>>::print(v);
            }
            std::cout << "}";
        }
    };

    template <class T>
    struct _printer<T, typename _enable_if_tuple<T, typename _enable_if_iterable<T>::not_type>::type> {
        template <std::size_t ...Is>
        static void _unrolled_print(T const &t, std::index_sequence<Is...>) {
            std::cout << "{";
            ((_printer<_rmcvref_t<std::tuple_element_t<Is, T>>>::print(std::get<Is>(t)), std::cout << ", "), ...);
            if constexpr (sizeof...(Is) != 0) _printer<_rmcvref_t<std::tuple_element_t<sizeof...(Is), T>>>::print(std::get<sizeof...(Is)>(t));
            std::cout << "}";
        }

        static void print(T const &t) {
            _unrolled_print(t, std::make_index_sequence<std::max(static_cast<std::size_t>(1), std::tuple_size_v<T>) - 1>{});
        }
    };

    template <class T>
    struct _printer<T, typename _enable_if_map<T>::type> {
        static void print(T const &t) {
            std::cout << "{";
            bool once = false;
            for (auto const &[k, v]: t) {
                if (once) {
                    std::cout << ", ";
                } else {
                    once = true;
                }
                _printer<_rmcvref_t<decltype(k)>>::print(k);
                std::cout << ": ";
                _printer<_rmcvref_t<decltype(v)>>::print(v);
            }
            std::cout << "}";
        }
    };

    template <class T>
    struct _printer<T, typename _enable_if_string<T>::type> {
        static void print(T const &t) {
            std::cout << std::quoted(t);
        }
    };

    template <class T>
    struct _printer<T, typename _enable_if_char<T>::type> {
        static void print(T const &t) {
            std::cout << std::quoted(t, T('\''));
        }
    };

    template <class T0, class ...Ts>
    void print(T0 const &t0, Ts const &...ts) {
        _printer<_rmcvref_t<T0>>::print(t0);
        ((std::cout << " " << _printer<_rmcvref_t<Ts>>::print(ts)), ...);
        std::cout << "\n";
    }
}

using _print_details::print;

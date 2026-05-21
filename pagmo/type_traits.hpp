/* Copyright 2017-2021 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */

#ifndef PAGMO_TYPE_TRAITS_HPP
#define PAGMO_TYPE_TRAITS_HPP

#include <cstddef>
#include <initializer_list>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <pagmo/threading.hpp>

namespace pagmo
{

namespace detail
{

// http://en.cppreference.com/w/cpp/experimental/is_detected
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

// http://en.cppreference.com/w/cpp/experimental/nonesuch
struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const &) = delete;
    void operator=(nonesuch const &) = delete;
};

// NOTE: we used to have custom implementations
// of these utilities in pre-C++17, but now we don't
// need them any more.

template <typename T, typename F, std::size_t... Is>
void apply_to_each_item(T &&t, const F &f, std::index_sequence<Is...>)
{
    (void)std::initializer_list<int>{0, (void(f(std::get<Is>(std::forward<T>(t)))), 0)...};
}

// Tuple for_each(). Execute the functor f on each element of the input Tuple.
// https://isocpp.org/blog/2015/01/for-each-arg-eric-niebler
// https://www.reddit.com/r/cpp/comments/2tffv3/for_each_argumentsean_parent/
// https://www.reddit.com/r/cpp/comments/33b06v/for_each_in_tuple/
template <class Tuple, class F>
void tuple_for_each(Tuple &&t, const F &f)
{
    apply_to_each_item(std::forward<Tuple>(t), f,
                       std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>{});
}

} // namespace detail

/// Implementation of \p std::is_detected.
/**
 * Implementation of \p std::is_detected, from C++17. See: http://en.cppreference.com/w/cpp/experimental/is_detected.
 */
template <template <class...> class Op, class... Args>
using is_detected =
#if defined(PAGMO_DOXYGEN_INVOKED)
    implementation_defined;
#else
    typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;
#endif

/// Implementation of \p std::detected_t.
/**
 * Implementation of \p std::detected_t, from C++17. See: http://en.cppreference.com/w/cpp/experimental/is_detected.
 */
template <template <class...> class Op, class... Args>
using detected_t =
#if defined(PAGMO_DOXYGEN_INVOKED)
    implementation_defined;
#else
    typename detail::detector<detail::nonesuch, void, Op, Args...>::type;
#endif

} // namespace pagmo

#endif

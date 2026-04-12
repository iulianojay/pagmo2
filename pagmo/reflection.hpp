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

#ifndef PAGMO_REFLECTION_HPP
#define PAGMO_REFLECTION_HPP

#include <cassert>
#include <concepts>
#include <functional>
#include <meta>
#include <string>
#include <type_traits>

namespace pagmo
{
namespace detail
{

template <typename E>
    requires std::is_enum_v<E>
constexpr std::string enum_to_string(E value)
{
    std::string result = "<unnamed>";
    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E)))
    {
        if (value == [:e:]) {
            result = std::meta::identifier_of(e);
        }
    }
    return result;
}

template <typename E>
    requires std::is_enum_v<E>
constexpr std::optional<E> string_to_enum(std::string_view name)
{
    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E)))
    {
        if (name == std::meta::identifier_of(e)) {
            return [:e:];
        }
    }

    return std::nullopt;
}

// fixed_string is structural (all-public), usable as an NTTP.
template <std::size_t N>
struct fixed_string {
    char data[N]{};
    constexpr fixed_string(const char (&s)[N])
    {
        std::copy_n(s, N, data);
    }
    constexpr operator std::string_view() const
    {
        return {data, N - 1};
    }
};
template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

template <fixed_string name, typename T>
consteval bool has_member_named()
{
    for (auto m : std::meta::members_of(^^T, std::meta::access_context::current())) {
        if (std::meta::is_function(m) && std::meta::has_identifier(m)
            && std::meta::identifier_of(m) == std::string_view(name))
            return true;
    }
    return false;
}

template <fixed_string name, typename T>
constexpr auto reflect_function_impl(T &t)
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return [&t]() mutable { return t.[:m:](); };
        }
    }
}

template <fixed_string name, typename T>
constexpr auto reflect_function_impl(T &&t)
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return [t]() mutable { return t.[:m:](); };
        }
    }
}

template <typename Default_T, fixed_string name, typename T>
constexpr auto reflect_function(T &t)
{
    if constexpr (has_member_named<name, T>()) {
        return reflect_function_impl<name>(t);
    } else {
        return reflect_function_impl<name>(Default_T{}); // Fallback to default type if member not found in T
    }
}

template <typename Default_T, fixed_string... names, typename T>
constexpr auto reflect_functions(T &t)
{
    return std::tuple{reflect_function<Default_T, names>(t)...};
}

} // namespace detail
} // namespace pagmo

#endif
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

#include <array>
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

// static_string is structural (all-public), usable as an NTTP.
template <std::size_t N>
struct static_string {
    char data[N]{};
    constexpr static_string(const char (&s)[N])
    {
        std::copy_n(s, N, data);
    }
    constexpr operator std::string_view() const
    {
        return {data, N - 1};
    }
};
template <std::size_t N>
static_string(const char (&)[N]) -> static_string<N>;

template <typename T, static_string name>
consteval bool has_function_named()
{
    for (auto m : std::meta::members_of(^^T, std::meta::access_context::current())) {
        if (std::meta::is_function(m) && std::meta::has_identifier(m)
            && std::meta::identifier_of(m) == std::string_view(name))
            return true;
    }
    return false;
}

template <typename T, static_string name>
consteval bool has_function_template_named()
{
    for (auto m : std::meta::members_of(^^T, std::meta::access_context::current())) {
        if ((std::meta::is_function(m) || std::meta::is_function_template(m)) && std::meta::has_identifier(m)
            && std::meta::identifier_of(m) == std::string_view(name))
            return true;
    }
    return false;
}

template <static_string name, typename T>
constexpr auto wire_function_impl(T &t)
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return std::bind(&[:m:], std::ref(t));
        }
    }
}

template <static_string name, typename T>
constexpr auto wire_function_impl(T &&t)
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return std::bind(&[:m:], std::move(t));
        }
    }
}

template <typename Default_T, static_string name, typename T>
constexpr auto wire_function(T &t)
{
    if constexpr (has_function_named<T, name>()) {
        return wire_function_impl<name>(t);
    } else {
        return wire_function_impl<name>(Default_T{}); // Fallback to default type if member not found in T
    }
}

template <typename Default_T, static_string... names, typename T>
constexpr auto wire_functions(T &t)
{
    return std::tuple{wire_function<Default_T, names>(t)...};
}

template <typename T, static_string name>
consteval std::meta::info get_function_metadata_impl()
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return m;
        }
    }
}

template <typename T, typename Default_T, static_string name>
consteval std::meta::info get_function_metadata()
{
    if constexpr (has_function_named<T, name>()) {
        return get_function_metadata_impl<T, name>();
    } else {
        return get_function_metadata_impl<Default_T, name>(); // Fallback to default type if member not found in T
    }
}

template <typename T, typename Default_T, static_string... names>
consteval auto get_functions_metadata()
{
    return std::array<std::meta::info, sizeof...(names)>{get_function_metadata<T, Default_T, names>()...};
}

} // namespace detail
} // namespace pagmo

#endif
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

#ifndef PAGMO_DETAIL_BASE_SR_POLICY_HPP
#define PAGMO_DETAIL_BASE_SR_POLICY_HPP

#include <concepts>
#include <type_traits>
#include <variant>

#include <pagmo/concepts.hpp>
#include <pagmo/detail/visibility.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/type_traits.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
namespace pagmo
{

namespace detail
{

class PAGMO_DLL_PUBLIC base_sr_policy
{
    void verify_fp_ctor() const;

    // Dispatching for the generic ctor
    // via two private constructors: one
    // for absolute migration rate, one
    // for fractional migration rate.
    struct ptag {
    };
    // Absolute migration rate.
    template <typename T>
        requires(std::is_integral_v<T>)
    explicit base_sr_policy(ptag, T n) : m_migr_rate(numeric_cast<pop_size_t>(n))
    {
    }
    // Fractional migration rate.
    template <typename T>
        requires(std::is_floating_point_v<T>)
    explicit base_sr_policy(ptag, T x) : m_migr_rate(static_cast<double>(x))
    {
        verify_fp_ctor();
    }

public:
    // Constructor from fractional or absolute migration policy.
    template <typename T>
        requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
    explicit base_sr_policy(T x) : base_sr_policy(ptag{}, x)
    {
    }

private:
    friend class cereal::access;
    // Serialization support.
    template <typename Archive>
    void serialize(Archive &ar)
    {
        detail::archive(ar, m_migr_rate);
    }

public:
    const std::variant<pop_size_t, double> &get_migr_rate() const;

protected:
    std::variant<pop_size_t, double> m_migr_rate;
};

} // namespace detail

} // namespace pagmo

#endif

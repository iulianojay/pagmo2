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

#ifndef PAGMO_CONCEPTS_HPP
#define PAGMO_CONCEPTS_HPP

#include <concepts>
#include <type_traits>

#include <pagmo/threading.hpp>

namespace pagmo
{

/// Remove reference and cv qualifiers from type \p T.
template <typename T>
using RemoveConstVolatileRef = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
concept IsConstVolatileRef = !std::is_same_v<T, RemoveConstVolatileRef<T>>;

template <typename T>
concept IsNotConstVolatileRef = std::is_same_v<T, RemoveConstVolatileRef<T>>;

template <typename T, typename U>
concept IsDifferentBaseType = !std::is_same_v<T, RemoveConstVolatileRef<U>>;

/// Detect \p set_seed() method.
/**
 * This concept will be satisfied if \p T provides a method with
 * the following signature:
 * @code{.unparsed}
 * void set_seed(unsigned);
 * @endcode
 * The \p set_seed() method is part of the interface for the definition of problems and algorithms
 * (see pagmo::problem and pagmo::algorithm).
 */
template <typename T>
concept HasSetSeed = requires(T &t) {
    { t.set_seed(1u) } -> std::same_as<void>;
};

/// Detect \p has_set_seed() method.
/**
 * This concept will be satisfied if \p T provides a method with
 * the following signature:
 * @code{.unparsed}
 * bool has_set_seed() const;
 * @endcode
 * The \p has_set_seed() method is part of the interface for the definition of problems and algorithms
 * (see pagmo::problem and pagmo::algorithm).
 */
template <typename T>
concept OverrideHasSetSeed = requires(const T &t) {
    { t.has_set_seed() } -> std::same_as<bool>;
};

/// Detect \p get_name() method.
/**
 * This concept will be satisfied if \p T provides a method with
 * the following signature:
 * @code{.unparsed}
 * std::string get_name() const;
 * @endcode
 * The \p get_name() method is part of the interface for the definition of problems and algorithms
 * (see pagmo::problem and pagmo::algorithm).
 */
template <typename T>
concept HasGetName = requires(const T &t) {
    { t.get_name() } -> std::same_as<std::string>;
};

/// Detect \p get_extra_info() method.
/**
 * This concept will be satisfied if \p T provides a method with
 * the following signature:
 * @code{.unparsed}
 * std::string get_extra_info() const;
 * @endcode
 * The \p get_extra_info() method is part of the interface for the definition of problems and algorithms
 * (see pagmo::problem and pagmo::algorithm).
 */
template <typename T>
concept HasGetExtraInfo = requires(const T &t) {
    { t.get_extra_info() } -> std::same_as<std::string>;
};

/// Detect \p get_thread_safety() method.
/**
 * This concept will be satisfied if \p T provides a method with
 * the following signature:
 * @code{.unparsed}
 * pagmo::thread_safety get_thread_safety() const;
 * @endcode
 * The \p get_thread_safety() method is part of the interface for the definition of problems and algorithms
 * (see pagmo::problem and pagmo::algorithm).
 */
template <typename T>
concept HasGetThreadSafety = requires(const T &t) {
    { t.get_thread_safety() } -> std::same_as<thread_safety>;
};

} // namespace pagmo

#endif
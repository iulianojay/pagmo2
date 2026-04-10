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

#ifndef PAGMO_S_POLICY_HPP
#define PAGMO_S_POLICY_HPP

#include <cassert>
#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <pagmo/concepts.hpp>
#include <pagmo/config.hpp>
#include <pagmo/detail/pagmo.fwd.hpp>
#include <pagmo/detail/support_xeus_cling.hpp>
#include <pagmo/detail/type_name.hpp>
#include <pagmo/detail/typeid_name_extract.hpp>
#include <pagmo/detail/visibility.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/type_traits.hpp>
#include <pagmo/types.hpp>

// Declares the compile-time cereal name binding and polymorphic caster relation.
// Safe to place in headers — creates no static initializer objects.
#define PAGMO_S11N_S_POLICY_EXPORT_KEY(s)                                                                              \
    namespace cereal                                                                                                   \
    {                                                                                                                  \
    namespace detail                                                                                                   \
    {                                                                                                                  \
    template <>                                                                                                        \
    struct binding_name<pagmo::detail::s_pol_inner<s>> {                                                               \
        static constexpr char const *name()                                                                            \
        {                                                                                                              \
            return "pagmo::detail::s_pol_inner<" #s ">";                                                               \
        }                                                                                                              \
    };                                                                                                                 \
    }                                                                                                                  \
    } /* end namespaces */                                                                                             \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(pagmo::detail::s_pol_inner_base, pagmo::detail::s_pol_inner<s>)

// Creates the static registration initializer. Must be used in ONE .cpp only
// (pagmo/s11n_registrations.cpp) to avoid duplicate StaticObject singletons on macOS.
#define PAGMO_S11N_S_POLICY_IMPLEMENT(s) CEREAL_BIND_TO_ARCHIVES(pagmo::detail::s_pol_inner<s>)

#define PAGMO_S11N_S_POLICY_EXPORT(s)                                                                                  \
    PAGMO_S11N_S_POLICY_EXPORT_KEY(s)                                                                                  \
    PAGMO_S11N_S_POLICY_IMPLEMENT(s)

namespace pagmo
{

// Check if T has a select() member function conforming to the UDSP requirements.
template <typename T>
concept HasSelect
    = requires(const T &t, const individuals_group_t &inds, const vector_double::size_type &nx,
               const vector_double::size_type &nix, const vector_double::size_type &nobj,
               const vector_double::size_type &nec, const vector_double::size_type &nic, const vector_double &tol) {
          { t.select(inds, nx, nix, nobj, nec, nic, tol) } -> std::same_as<individuals_group_t>;
      };

namespace detail
{

// Specialise this to true in order to disable all the UDSP checks and mark a type
// as a UDSP regardless of the features provided by it.
// NOTE: this is needed when implementing the machinery for Python s_policies.
// NOTE: leave this as an implementation detail for now.
template <typename>
struct disable_udsp_checks : std::false_type {
};

} // namespace detail

// Detect UDSPs
template <typename T>
concept IsUdSPolicy = requires(T) {
    requires IsNotConstVolatileRef<T>;
    requires std::is_default_constructible_v<T>;
    requires std::is_copy_constructible_v<T>;
    requires std::is_move_constructible_v<T>;
    requires std::is_destructible_v<T>;
    requires HasSelect<T>;
    requires !detail::disable_udsp_checks<T>::value;
};

namespace detail
{

struct PAGMO_DLL_PUBLIC_INLINE_CLASS s_pol_inner_base {
    virtual ~s_pol_inner_base() {}
    virtual std::unique_ptr<s_pol_inner_base> clone() const = 0;
    virtual individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                       const vector_double::size_type &, const vector_double::size_type &,
                                       const vector_double::size_type &, const vector_double::size_type &,
                                       const vector_double &) const
        = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_extra_info() const = 0;
    virtual std::type_index get_type_index() const = 0;
    virtual const void *get_ptr() const = 0;
    virtual void *get_ptr() = 0;

private:
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive &)
    {
    }
};

template <typename T>
struct PAGMO_DLL_PUBLIC_INLINE_CLASS s_pol_inner final : s_pol_inner_base {
    // We just need the def ctor, delete everything else.
    s_pol_inner() = default;
    s_pol_inner(const s_pol_inner &) = delete;
    s_pol_inner(s_pol_inner &&) = delete;
    s_pol_inner &operator=(const s_pol_inner &) = delete;
    s_pol_inner &operator=(s_pol_inner &&) = delete;
    // Constructors from T.
    explicit s_pol_inner(const T &x) : m_value(x) {}
    explicit s_pol_inner(T &&x) : m_value(std::move(x)) {}
    // The clone method, used in the copy constructor of s_policy.
    std::unique_ptr<s_pol_inner_base> clone() const final
    {
        return std::make_unique<s_pol_inner>(m_value);
    }
    // The mandatory select() method.
    individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &nx,
                               const vector_double::size_type &nix, const vector_double::size_type &nobj,
                               const vector_double::size_type &nec, const vector_double::size_type &nic,
                               const vector_double &tol) const final
    {
        return m_value.select(inds, nx, nix, nobj, nec, nic, tol);
    }
    // Optional methods.
    std::string get_name() const final
    {
        return get_name_impl(m_value);
    }
    std::string get_extra_info() const final
    {
        return get_extra_info_impl(m_value);
    }
    template <typename U>
        requires(HasGetName<U>)
    static std::string get_name_impl(const U &value)
    {
        return value.get_name();
    }
    template <typename U>
        requires(!HasGetName<U>)
    static std::string get_name_impl(const U &)
    {
        return detail::type_name<U>();
    }
    template <typename U>
        requires(HasGetExtraInfo<U>)
    static std::string get_extra_info_impl(const U &value)
    {
        return value.get_extra_info();
    }
    template <typename U>
        requires(!HasGetExtraInfo<U>)
    static std::string get_extra_info_impl(const U &)
    {
        return "";
    }
    // Get the type at runtime.
    std::type_index get_type_index() const final
    {
        return std::type_index(typeid(T));
    }
    // Raw getters for the internal instance.
    const void *get_ptr() const final
    {
        return &m_value;
    }
    void *get_ptr() final
    {
        return &m_value;
    }

private:
    friend class cereal::access;
    // Serialization
    template <typename Archive>
    void serialize(Archive &ar)
    {
        detail::archive(ar, cereal::base_class<s_pol_inner_base>(this), m_value);
    }

public:
    T m_value;
};

} // namespace detail

} // namespace pagmo

// Disable Boost.Serialization tracking for the implementation
// details of s_policy.

namespace pagmo
{

// S_policy concept definitions
template <typename T>
concept SpolGenericCtorEnabler = IsDifferentBaseType<s_policy, T> && IsUdSPolicy<RemoveConstVolatileRef<T>>;

// Selection policy.
class PAGMO_DLL_PUBLIC s_policy
{
    // Implementation of the generic ctor.
    void generic_ctor_impl();

public:
    // Default constructor.
    s_policy();
    // Constructor from a UDSP.
    template <typename T>
        requires SpolGenericCtorEnabler<T>
    explicit s_policy(T &&x)
        : m_ptr(std::make_unique<detail::s_pol_inner<RemoveConstVolatileRef<T>>>(std::forward<T>(x)))
    {
        generic_ctor_impl();
    }
    // Copy constructor.
    s_policy(const s_policy &);
    // Move constructor.
    s_policy(s_policy &&) noexcept;
    // Move assignment operator
    s_policy &operator=(s_policy &&) noexcept;
    // Copy assignment operator
    s_policy &operator=(const s_policy &);
    // Assignment from a UDSP.
    template <typename T>
        requires SpolGenericCtorEnabler<T>
    s_policy &operator=(T &&x)
    {
        return (*this) = s_policy(std::forward<T>(x));
    }

    // Extraction and related.
    template <typename T>
    const T *extract() const noexcept
    {
#if defined(PAGMO_PREFER_TYPEID_NAME_EXTRACT)
        return detail::typeid_name_extract<T>(*this);
#else
        auto p = dynamic_cast<const detail::s_pol_inner<T> *>(ptr());
        return p == nullptr ? nullptr : &(p->m_value);
#endif
    }
    template <typename T>
    T *extract() noexcept
    {
#if defined(PAGMO_PREFER_TYPEID_NAME_EXTRACT)
        return detail::typeid_name_extract<T>(*this);
#else
        auto p = dynamic_cast<detail::s_pol_inner<T> *>(ptr());
        return p == nullptr ? nullptr : &(p->m_value);
#endif
    }
    template <typename T>
    bool is() const noexcept
    {
        return extract<T>() != nullptr;
    }

    // Select.
    individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double &) const;

    // Name.
    std::string get_name() const
    {
        return m_name;
    }
    // Extra info.
    std::string get_extra_info() const;

    // Check if the s_policy is valid.
    bool is_valid() const;

    // Get the type at runtime.
    std::type_index get_type_index() const;

    // Get a const pointer to the UDSP.
    const void *get_ptr() const;

    // Get a mutable pointer to the UDSP.
    void *get_ptr();

private:
    friend class cereal::access;
    // Serialisation support.
    template <typename Archive>
    void save(Archive &ar) const
    {
        detail::to_archive(ar, m_ptr, m_name);
    }
    template <typename Archive>
    void load(Archive &ar)
    {
        try {
            detail::from_archive(ar, m_ptr, m_name);
        } catch (...) {
            *this = s_policy{};
            throw;
        }
    }

    // Just two small helpers to make sure that whenever we require
    // access to the pointer it actually points to something.
    detail::s_pol_inner_base const *ptr() const
    {
        assert(m_ptr.get() != nullptr);
        return m_ptr.get();
    }
    detail::s_pol_inner_base *ptr()
    {
        assert(m_ptr.get() != nullptr);
        return m_ptr.get();
    }
    // Helper to check the inputs and outputs of the select() function.
    PAGMO_DLL_LOCAL void verify_select_input(const individuals_group_t &, const vector_double::size_type &,
                                             const vector_double::size_type &, const vector_double::size_type &,
                                             const vector_double::size_type &, const vector_double::size_type &,
                                             const vector_double &) const;
    PAGMO_DLL_LOCAL void verify_select_output(const individuals_group_t &, vector_double::size_type,
                                              vector_double::size_type) const;

    // Pointer to the inner base s_pol.
    std::unique_ptr<detail::s_pol_inner_base> m_ptr;
    // Various properties determined at construction time
    // from the udsp. These will be constant for the lifetime
    // of s_policy, but we cannot mark them as such because we want to be
    // able to assign and deserialise s_policies.
    std::string m_name;
};

#if !defined(PAGMO_DOXYGEN_INVOKED)

// Stream operator.
PAGMO_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const s_policy &);

#endif

} // namespace pagmo

// Add some repr support for CLING
PAGMO_IMPLEMENT_XEUS_CLING_REPR(s_policy)

#endif

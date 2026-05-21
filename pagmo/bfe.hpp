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

#ifndef PAGMO_BFE_HPP
#define PAGMO_BFE_HPP

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
#include <pagmo/problem.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/type_traits.hpp>
#include <pagmo/types.hpp>

// Declares the compile-time cereal name binding, polymorphic caster relation, and
// archive binding. CEREAL_BIND_TO_ARCHIVES is included here so every DSO that
// includes this header self-registers the type — required for macOS two-level namespace.
#define PAGMO_S11N_BFE_EXPORT_KEY(b)                                                                                   \
    namespace cereal                                                                                                   \
    {                                                                                                                  \
    namespace detail                                                                                                   \
    {                                                                                                                  \
    template <>                                                                                                        \
    struct binding_name<pagmo::detail::bfe_inner<b>> {                                                                 \
        static constexpr char const *name()                                                                            \
        {                                                                                                              \
            return "pagmo::detail::bfe_inner<" #b ">";                                                                 \
        }                                                                                                              \
    };                                                                                                                 \
    }                                                                                                                  \
    } /* end namespaces */                                                                                             \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(pagmo::detail::bfe_inner_base, pagmo::detail::bfe_inner<b>)                   \
    CEREAL_BIND_TO_ARCHIVES(pagmo::detail::bfe_inner<b>)

// Also called from pagmo/s11n_registrations.cpp (idempotent — safe to call multiple times).
#define PAGMO_S11N_BFE_IMPLEMENT(b)

#define PAGMO_S11N_BFE_EXPORT(b)                                                                                       \
    PAGMO_S11N_BFE_EXPORT_KEY(b)                                                                                       \
    PAGMO_S11N_BFE_IMPLEMENT(b)

namespace pagmo
{

// Check if T has a call operator conforming to the UDBFE requirements.
template <typename T>
concept HasBfeCallOperator = requires(const T &t, const problem &prob, const vector_double &dvs) {
    { t(prob, dvs) } -> std::same_as<vector_double>;
};

namespace detail
{

// Specialise this to true in order to disable all the UDBFE checks and mark a type
// as a UDBFE regardless of the features provided by it.
// NOTE: this is needed when implementing the machinery for Python batch evaluators.
// NOTE: leave this as an implementation detail for now.
template <typename>
struct disable_udbfe_checks : std::false_type {
};

} // namespace detail

// Check if T is a UDBFE.
template <typename T>
concept IsUdBfe = requires(T) {
    requires IsNotConstVolatileRef<T>;
    requires std::is_default_constructible_v<T>;
    requires std::is_copy_constructible_v<T>;
    requires std::is_move_constructible_v<T>;
    requires std::is_destructible_v<T>;
    requires HasBfeCallOperator<T>;
    requires !detail::disable_udbfe_checks<T>::value;
    // Exclude lambda closure types by detecting their unique characteristics:
    // They can be converted to function pointers but are themselves class types
    requires !(std::is_class_v<T>
               && std::is_convertible_v<T, vector_double (*)(const problem &, const vector_double &)>);
};

namespace detail
{

struct PAGMO_DLL_PUBLIC_INLINE_CLASS bfe_inner_base {
    virtual ~bfe_inner_base() {}
    virtual std::unique_ptr<bfe_inner_base> clone() const = 0;
    virtual vector_double operator()(const problem &, const vector_double &) const = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_extra_info() const = 0;
    virtual thread_safety get_thread_safety() const = 0;
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
struct PAGMO_DLL_PUBLIC_INLINE_CLASS bfe_inner final : bfe_inner_base {
    // We just need the def ctor, delete everything else.
    bfe_inner() = default;
    bfe_inner(const bfe_inner &) = delete;
    bfe_inner(bfe_inner &&) = delete;
    bfe_inner &operator=(const bfe_inner &) = delete;
    bfe_inner &operator=(bfe_inner &&) = delete;
    // Constructors from T (copy and move variants).
    explicit bfe_inner(const T &x) : m_value(x) {}
    explicit bfe_inner(T &&x) : m_value(std::move(x)) {}
    // The clone method, used in the copy constructor of bfe.
    std::unique_ptr<bfe_inner_base> clone() const final
    {
        return std::make_unique<bfe_inner>(m_value);
    }
    // Mandatory methods.
    vector_double operator()(const problem &p, const vector_double &dvs) const final
    {
        return m_value(p, dvs);
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
    thread_safety get_thread_safety() const final
    {
        return get_thread_safety_impl(m_value);
    }
    // Implementation of the optional methods.
    template <typename U>
        requires HasGetName<U>
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
        requires HasGetExtraInfo<U>
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
    template <typename U>
        requires HasGetThreadSafety<U>
    static thread_safety get_thread_safety_impl(const U &value)
    {
        return value.get_thread_safety();
    }
    template <typename U>
        requires(!HasGetThreadSafety<U>)
    static thread_safety get_thread_safety_impl(const U &)
    {
        return thread_safety::basic;
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
    // Serialization.
    template <typename Archive>
    void serialize(Archive &ar)
    {
        detail::archive(ar, cereal::base_class<bfe_inner_base>(this), m_value);
    }

public:
    T m_value;
};

} // namespace detail

} // namespace pagmo

// Disable Boost.Serialization tracking for the implementation
// details of algorithm.

namespace pagmo
{

// BFE concept definitions
template <typename T>
concept BfeGenericCtorEnabler
    = (IsDifferentBaseType<bfe, T> && IsUdBfe<RemoveConstVolatileRef<T>>)
      || std::is_same_v<vector_double(const problem &, const vector_double &), RemoveConstVolatileRef<T>>;

class PAGMO_DLL_PUBLIC bfe
{
    // Dispatching for the generic ctor. We have a special case if T is
    // a function type, in which case we will manually do the conversion to
    // function pointer and delegate to the other overload.
    template <typename T>
    explicit bfe(T &&x, std::true_type)
        : bfe(static_cast<vector_double (*)(const problem &, const vector_double &)>(std::forward<T>(x)),
              std::false_type{})
    {
    }
    template <typename T>
    explicit bfe(T &&x, std::false_type)
        : m_ptr(std::make_unique<detail::bfe_inner<RemoveConstVolatileRef<T>>>(std::forward<T>(x)))
    {
    }
    // Implementation of the generic ctor.
    void generic_ctor_impl();

public:
    // Default ctor.
    bfe();
    // Constructor from a UDBFE.
    template <typename T>
        requires BfeGenericCtorEnabler<T>
    explicit bfe(T &&x) : bfe(std::forward<T>(x), std::is_function<RemoveConstVolatileRef<T>>{})
    {
        generic_ctor_impl();
    }
    // Copy constructor.
    bfe(const bfe &);
    // Move constructor.
    bfe(bfe &&) noexcept;
    // Move assignment operator
    bfe &operator=(bfe &&) noexcept;
    // Copy assignment operator
    bfe &operator=(const bfe &);
    // Assignment from a UDBFE.
    template <typename T>
        requires BfeGenericCtorEnabler<T>
    bfe &operator=(T &&x)
    {
        return (*this) = bfe(std::forward<T>(x));
    }

    // Extraction and related.
    template <typename T>
    const T *extract() const noexcept
    {
#if defined(PAGMO_PREFER_TYPEID_NAME_EXTRACT)
        return detail::typeid_name_extract<T>(*this);
#else
        auto p = dynamic_cast<const detail::bfe_inner<T> *>(ptr());
        return p == nullptr ? nullptr : &(p->m_value);
#endif
    }
    template <typename T>
    T *extract() noexcept
    {
#if defined(PAGMO_PREFER_TYPEID_NAME_EXTRACT)
        return detail::typeid_name_extract<T>(*this);
#else
        auto p = dynamic_cast<detail::bfe_inner<T> *>(ptr());
        return p == nullptr ? nullptr : &(p->m_value);
#endif
    }
    template <typename T>
    bool is() const noexcept
    {
        return extract<T>() != nullptr;
    }

    // Call operator.
    vector_double operator()(const problem &, const vector_double &) const;

    // Name.
    std::string get_name() const
    {
        return m_name;
    }
    // Extra info.
    std::string get_extra_info() const;

    // Thread safety level.
    thread_safety get_thread_safety() const
    {
        return m_thread_safety;
    }

    // Check if the bfe is valid.
    bool is_valid() const;

    // Get the type at runtime.
    std::type_index get_type_index() const;

    // Get a const pointer to the UDBFE.
    const void *get_ptr() const;

    // Get a mutable pointer to the UDBFE.
    void *get_ptr();

private:
    friend class cereal::access;
    template <typename Archive>
    void save(Archive &ar) const
    {
        detail::to_archive(ar, m_ptr, m_name, m_thread_safety);
    }
    template <typename Archive>
    void load(Archive &ar)
    {
        try {
            detail::from_archive(ar, m_ptr, m_name, m_thread_safety);
        } catch (...) {
            *this = bfe{};
            throw;
        }
    }

    // Just two small helpers to make sure that whenever we require
    // access to the pointer it actually points to something.
    detail::bfe_inner_base const *ptr() const
    {
        assert(m_ptr.get() != nullptr);
        return m_ptr.get();
    }
    detail::bfe_inner_base *ptr()
    {
        assert(m_ptr.get() != nullptr);
        return m_ptr.get();
    }

    // Pointer to the inner base bfe
    std::unique_ptr<detail::bfe_inner_base> m_ptr;
    // Various properties determined at construction time
    // from the udbfe. These will be constant for the lifetime
    // of bfe, but we cannot mark them as such because we want to be
    // able to assign and deserialise bfes.
    std::string m_name;
    // Thread safety.
    thread_safety m_thread_safety;
};

#if !defined(PAGMO_DOXYGEN_INVOKED)

// Stream operator.
PAGMO_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const bfe &);

#endif

} // namespace pagmo

// Add some repr support for CLING
PAGMO_IMPLEMENT_XEUS_CLING_REPR(bfe)

#endif

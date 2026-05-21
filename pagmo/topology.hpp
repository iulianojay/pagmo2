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

#ifndef PAGMO_TOPOLOGY_HPP
#define PAGMO_TOPOLOGY_HPP

#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <graaflib/graph.h>

#include <pagmo/concepts.hpp>
#include <pagmo/config.hpp>
#include <pagmo/detail/pagmo.fwd.hpp>
#include <pagmo/detail/support_xeus_cling.hpp>
#include <pagmo/detail/type_name.hpp>
#include <pagmo/detail/typeid_name_extract.hpp>
#include <pagmo/detail/visibility.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/type_traits.hpp>
#include <pagmo/types.hpp>

// Declares the compile-time cereal name binding, polymorphic caster relation, and
// archive binding. CEREAL_BIND_TO_ARCHIVES is included here so every DSO that
// includes this header self-registers the type — required for macOS two-level namespace.
#define PAGMO_S11N_TOPOLOGY_EXPORT_KEY(topo)                                                                           \
    namespace cereal                                                                                                   \
    {                                                                                                                  \
    namespace detail                                                                                                   \
    {                                                                                                                  \
    template <>                                                                                                        \
    struct binding_name<pagmo::detail::topo_inner<topo>> {                                                             \
        static constexpr char const *name()                                                                            \
        {                                                                                                              \
            return "pagmo::detail::topo_inner<" #topo ">";                                                             \
        }                                                                                                              \
    };                                                                                                                 \
    }                                                                                                                  \
    } /* end namespaces */                                                                                             \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(pagmo::detail::topo_inner_base, pagmo::detail::topo_inner<topo>)              \
    CEREAL_BIND_TO_ARCHIVES(pagmo::detail::topo_inner<topo>)

// Also called from pagmo/s11n_registrations.cpp (idempotent — safe to call multiple times).
#define PAGMO_S11N_TOPOLOGY_IMPLEMENT(topo)

#define PAGMO_S11N_TOPOLOGY_EXPORT(topo)                                                                               \
    PAGMO_S11N_TOPOLOGY_EXPORT_KEY(topo)                                                                               \
    PAGMO_S11N_TOPOLOGY_IMPLEMENT(topo)

namespace pagmo
{

// Detect the get_connections() method.
template <typename T>
concept HasGetConnections = requires(const T &t) {
    { t.get_connections(std::size_t(0)) } -> std::same_as<std::pair<std::vector<std::size_t>, vector_double>>;
};

// Detect the push_back() method.
template <typename T>
concept HasPushBack = requires(T &t) {
    { t.push_back() } -> std::same_as<void>;
};

#if !defined(PAGMO_DOXYGEN_INVOKED)

// A directed graph type used as export format for topologies
// and also as the underlying graph type for base_bgl_topology.
// Vertices carry no data (int dummy), edges store migration probability (double).
using graph_t = graaf::directed_graph<int, double>;

#endif

// Detect the to_graph() method.
template <typename T>
concept HasToGraph = requires(const T &t) {
    { t.to_graph() } -> std::same_as<graph_t>;
};

namespace detail
{

// Specialise this to true in order to disable all the UDT checks and mark a type
// as a UDT regardless of the features provided by it.
// NOTE: this is needed when implementing the machinery for Python topos.
// NOTE: leave this as an implementation detail for now.
template <typename>
struct disable_udt_checks : std::false_type {
};

} // namespace detail

// Detect user-defined topologies (UDT).
template <typename T>
concept IsUdTopology = requires(T) {
    requires IsNotConstVolatileRef<T>;
    requires std::is_default_constructible_v<T>;
    requires std::is_copy_constructible_v<T>;
    requires std::is_move_constructible_v<T>;
    requires std::is_destructible_v<T>;
    requires HasGetConnections<T>;
    requires HasPushBack<T>;
    requires !detail::disable_udt_checks<T>::value;
};
namespace detail
{

struct PAGMO_DLL_PUBLIC_INLINE_CLASS topo_inner_base {
    virtual ~topo_inner_base() {}
    virtual std::unique_ptr<topo_inner_base> clone() const = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_extra_info() const = 0;
    virtual std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const = 0;
    virtual void push_back() = 0;
    virtual graph_t to_graph() const = 0;
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
struct PAGMO_DLL_PUBLIC_INLINE_CLASS topo_inner final : topo_inner_base {
    // We just need the def ctor, delete everything else.
    topo_inner() = default;
    topo_inner(const topo_inner &) = delete;
    topo_inner(topo_inner &&) = delete;
    topo_inner &operator=(const topo_inner &) = delete;
    topo_inner &operator=(topo_inner &&) = delete;
    // Constructors from T (copy and move variants).
    explicit topo_inner(const T &x) : m_value(x) {}
    explicit topo_inner(T &&x) : m_value(std::move(x)) {}
    // The clone method, used in the copy constructor of topology.
    std::unique_ptr<topo_inner_base> clone() const final
    {
        return std::make_unique<topo_inner>(m_value);
    }
    // The mandatory methods.
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t n) const final
    {
        return m_value.get_connections(n);
    }
    void push_back() final
    {
        m_value.push_back();
    }
    // Optional methods.
    graph_t to_graph() const final
    {
        return to_graph_impl(m_value);
    }
    std::string get_name() const final
    {
        return get_name_impl(m_value);
    }
    std::string get_extra_info() const final
    {
        return get_extra_info_impl(m_value);
    }
    // Implementation of the optional methods.
    template <typename U>
        requires(HasToGraph<U>)
    static graph_t to_graph_impl(const U &value)
    {
        return value.to_graph();
    }
    template <typename U>
        requires(!HasToGraph<U>)
    [[noreturn]] static graph_t to_graph_impl(const U &value)
    {
        pagmo_throw(not_implemented_error,
                    "The to_graph() method has been invoked, but it is not implemented in a UDT of type '"
                        + get_name_impl(value) + "'");
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
        detail::archive(ar, cereal::base_class<topo_inner_base>(this), m_value);
    }

public:
    T m_value;
};

} // namespace detail

} // namespace pagmo

// Disable Boost.Serialization tracking for the implementation
// details of topology.

namespace pagmo
{

// Topology concept definitions
template <typename T>
concept TopoGenericCtorEnabler
    = !std::same_as<topology, RemoveConstVolatileRef<T>> && IsUdTopology<RemoveConstVolatileRef<T>>;

// Topology class.
class PAGMO_DLL_PUBLIC topology
{
public:
    // Default constructor.
    topology();

private:
    void generic_ctor_impl();

public:
    // Generic constructor.
    template <typename T>
        requires(TopoGenericCtorEnabler<T>)
    explicit topology(T &&x)
        : m_ptr(std::make_unique<detail::topo_inner<RemoveConstVolatileRef<T>>>(std::forward<T>(x)))
    {
        generic_ctor_impl();
    }
    // Copy ctor.
    topology(const topology &);
    // Move ctor.
    topology(topology &&) noexcept;
    // Move assignment.
    topology &operator=(topology &&) noexcept;
    // Copy assignment.
    topology &operator=(const topology &);
    // Generic assignment.
    template <typename T>
        requires(TopoGenericCtorEnabler<T>)
    topology &operator=(T &&x)
    {
        return (*this) = topology(std::forward<T>(x));
    }

    // Extract.
    template <typename T>
    const T *extract() const noexcept
    {
#if defined(PAGMO_PREFER_TYPEID_NAME_EXTRACT)
        return detail::typeid_name_extract<T>(*this);
#else
        auto p = dynamic_cast<const detail::topo_inner<T> *>(ptr());
        return p == nullptr ? nullptr : &(p->m_value);
#endif
    }
    template <typename T>
    T *extract() noexcept
    {
#if defined(PAGMO_PREFER_TYPEID_NAME_EXTRACT)
        return detail::typeid_name_extract<T>(*this);
#else
        auto p = dynamic_cast<detail::topo_inner<T> *>(ptr());
        return p == nullptr ? nullptr : &(p->m_value);
#endif
    }
    template <typename T>
    bool is() const noexcept
    {
        return extract<T>() != nullptr;
    }

    // Name.
    std::string get_name() const
    {
        return m_name;
    }

    // Extra info.
    std::string get_extra_info() const;

    // Check if the topology is valid.
    bool is_valid() const;

    // Get the connections to a vertex.
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const;

    // Add a vertex.
    void push_back();
    // Add multiple vertices.
    void push_back(unsigned);

    // Convert to graph.
    graph_t to_graph() const;

    // Get the type at runtime.
    std::type_index get_type_index() const;

    // Get a const pointer to the UDT.
    const void *get_ptr() const;

    // Get a mutable pointer to the UDT.
    void *get_ptr();

private:
    friend class cereal::access;
    // Serialization.
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
            *this = topology{};
            throw;
        }
    }

    // Two small helpers to make sure that whenever we require
    // access to the pointer it actually points to something.
    detail::topo_inner_base const *ptr() const
    {
        assert(m_ptr.get() != nullptr);
        return m_ptr.get();
    }
    detail::topo_inner_base *ptr()
    {
        assert(m_ptr.get() != nullptr);
        return m_ptr.get();
    }

    std::unique_ptr<detail::topo_inner_base> m_ptr;
    // Various topology properties determined at construction time
    // from the concrete topology. These will be constant for the lifetime
    // of topology, but we cannot mark them as such because of serialization.
    std::string m_name;
};

#if !defined(PAGMO_DOXYGEN_INVOKED)

// Streaming operator for topology.
PAGMO_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const topology &);

#endif

namespace detail
{

// A small helper for checking the weight of an edge in a topology.
PAGMO_DLL_PUBLIC void topology_check_edge_weight(double);

} // namespace detail

} // namespace pagmo

// Add some repr support for CLING
PAGMO_IMPLEMENT_XEUS_CLING_REPR(topology)

#endif

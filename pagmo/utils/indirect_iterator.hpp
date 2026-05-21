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

#ifndef PAGMO_UTILS_INDIRECT_ITERATOR_HPP
#define PAGMO_UTILS_INDIRECT_ITERATOR_HPP

namespace pagmo
{
namespace detail
{

// Lightweight indirect iterator: wraps an iterator over owning pointers / smart-pointers
// and dereferences through them, yielding a reference to the pointed-to object.
template <typename Iter>
class indirect_iterator
{
    Iter m_it;

public:
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using value_type = std::remove_reference_t<decltype(**std::declval<Iter>())>;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;

    indirect_iterator() = default;
    explicit indirect_iterator(Iter it) : m_it(std::move(it)) {}

    reference operator*() const
    {
        return **m_it;
    }
    pointer operator->() const
    {
        return std::to_address(*m_it);
    }

    indirect_iterator &operator++()
    {
        ++m_it;
        return *this;
    }
    indirect_iterator operator++(int)
    {
        auto t = *this;
        ++m_it;
        return t;
    }
    indirect_iterator &operator--()
    {
        --m_it;
        return *this;
    }
    indirect_iterator operator--(int)
    {
        auto t = *this;
        --m_it;
        return t;
    }

    bool operator==(const indirect_iterator &o) const
    {
        return m_it == o.m_it;
    }
    bool operator!=(const indirect_iterator &o) const
    {
        return m_it != o.m_it;
    }

    // Mixed comparison: allow comparing indirect_iterator<Iter> with indirect_iterator<OtherIter>
    // (e.g. iterator vs const_iterator) when their base iterators are equality-comparable.
    template <typename OtherIter>
    bool operator==(const indirect_iterator<OtherIter> &o) const
    {
        return m_it == o.base();
    }
    template <typename OtherIter>
    bool operator!=(const indirect_iterator<OtherIter> &o) const
    {
        return m_it != o.base();
    }

    // Expose the underlying iterator for mixed comparisons.
    Iter base() const
    {
        return m_it;
    }

    reference operator[](difference_type n) const
    {
        return *m_it[n];
    }
    indirect_iterator operator+(difference_type n) const
    {
        return indirect_iterator(m_it + n);
    }
    indirect_iterator operator-(difference_type n) const
    {
        return indirect_iterator(m_it - n);
    }
    indirect_iterator &operator+=(difference_type n)
    {
        m_it += n;
        return *this;
    }
    indirect_iterator &operator-=(difference_type n)
    {
        m_it -= n;
        return *this;
    }
    difference_type operator-(const indirect_iterator &o) const
    {
        return m_it - o.m_it;
    }
    bool operator<(const indirect_iterator &o) const
    {
        return m_it < o.m_it;
    }
    bool operator>(const indirect_iterator &o) const
    {
        return m_it > o.m_it;
    }
    bool operator<=(const indirect_iterator &o) const
    {
        return m_it <= o.m_it;
    }
    bool operator>=(const indirect_iterator &o) const
    {
        return m_it >= o.m_it;
    }
};

template <typename Iter>
auto operator+(typename indirect_iterator<Iter>::difference_type n, const indirect_iterator<Iter> &it)
{
    return it + n;
}

} // namespace detail
} // namespace pagmo

#endif
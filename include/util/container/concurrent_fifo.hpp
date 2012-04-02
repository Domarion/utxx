/**
 * The unbound and bound fifo queue class suitable for concurrent use in
 * multi-producer/multi-consumer model.  The class doesn't
 * own enqueued data.
 *
 *-----------------------------------------------------------------------------
 * Copyright (c) 2009 Serge Aleynikov <serge@aleynikov.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *-----------------------------------------------------------------------------
 * Created: 2010-02-03
 * $Id$
 */

#ifndef _UTIL_CONCURRENT_FIFO_HPP_
#define _UTIL_CONCURRENT_FIFO_HPP_

#include <util/container/detail/base_fifo.hpp>

namespace util {
namespace container {

//-----------------------------------------------------------------------------
/// @class bound_lock_free_queue
//-----------------------------------------------------------------------------

template <typename T, int Size>
class bound_lock_free_queue
    : public detail::lock_free_queue<T, detail::bound_allocator<T,Size> > {
    typedef detail::lock_free_queue<T, detail::bound_allocator<T,Size> > base_t;
    detail::bound_allocator<T,Size> m_allocator;
public:
    bound_lock_free_queue() : base_t(m_allocator) {}
};

//-----------------------------------------------------------------------------
/// @class unbound_lock_free_queue
//-----------------------------------------------------------------------------

template <typename T>
class unbound_lock_free_queue
    : public detail::lock_free_queue<T, detail::unbound_cached_allocator<T> > {
    typedef detail::lock_free_queue<T, detail::unbound_cached_allocator<T> > base_t;
    detail::unbound_cached_allocator<T> m_allocator;
public:
    unbound_lock_free_queue() : base_t(m_allocator) {}
};

//-----------------------------------------------------------------------------
/// @class blocking_unbound_fifo
//-----------------------------------------------------------------------------

template <typename T, int Size, typename EventT = synch::futex>
class blocking_bound_fifo
    : public detail::blocking_lock_free_queue<
        T, detail::bound_allocator<T,Size>, true, EventT> {
    typedef detail::blocking_lock_free_queue<
        T, detail::bound_allocator<T,Size>, true, EventT> base_t;
    detail::bound_allocator<T,Size> m_allocator;
public:
    blocking_bound_fifo() : base_t(m_allocator) {}
};

//-----------------------------------------------------------------------------
/// @class blocking_unbound_fifo
//-----------------------------------------------------------------------------

template <typename T, typename EventT = synch::futex>
class blocking_unbound_fifo
    : public detail::blocking_lock_free_queue<
        T, detail::unbound_cached_allocator<T>, false, EventT> {
    typedef detail::blocking_lock_free_queue<
        T, detail::unbound_cached_allocator<T>, false, EventT> base_t;
    detail::unbound_cached_allocator<T> m_allocator;
public:
    blocking_unbound_fifo() : base_t(m_allocator) {}
};

} // namespace container
} // namespace util

#endif // _UTIL_CONCURRENT_FIFO_HPP_

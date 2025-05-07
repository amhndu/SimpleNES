//  Adapted from Boost.Lockfree to work without other boost dependencies
//
//  lock-free single-producer/single-consumer ringbuffer
//  this algorithm is implemented in various projects (linux kernel)
//
//  Copyright (C) 2009-2013, 2022 Tim Blechmann, Amish K. Naidu
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>
#include <atomic>
#include <type_traits>
#include <vector>

namespace spsc
{

using std::size_t;

// RingBuffer assumes value is trivially destructible
// Only works with single producer and single consumer threads.
//
// Thread safety:
//   * During push, write-index is stored with memory order release *after* storage[write_index] is written to, ensuring
//   the storage writes are visible in pop due to Release-Acquire ordering
//   * write-index only moves forward *upto* the `read_index_`, so during a pop, it is safe to extract values from the
//   storage, since push can only affect the empty area
//   * read-index is stored using release ordering to ensure it is only updated after the full pop operation is finished
template<typename T>
class RingBuffer
{
    static_assert(std::is_trivially_destructible<T>::value,
                  "expecting a simple (trivially_destructible) type in the ring buffer");

private:
    const size_t        max_size;
    std::atomic<size_t> write_index_;
    std::atomic<size_t> read_index_;

    std::vector<T>      storage;

    RingBuffer(RingBuffer const&)            = delete;
    RingBuffer& operator=(RingBuffer const&) = delete;

public:
    explicit RingBuffer(int capacity)
      : max_size(capacity)
      , write_index_(0)
      , read_index_(0)
    {
        storage.resize(capacity);
    }

    static size_t next_index(size_t arg, size_t max_size)
    {
        size_t ret = arg + 1;
        while (ret >= max_size)
            ret -= max_size;
        return ret;
    }

    /** Push a value into the ring-buffer
     *
     * \returns If push was successful (it could fail if the queue was full)
     * \note Must be called from a single writer thread
     * */
    bool push(T const& t)
    {
        const size_t write_index = write_index_.load(std::memory_order_relaxed); // only written from push thread
        const size_t next        = next_index(write_index, max_size);

        if (next == read_index_.load(std::memory_order_acquire))
            return false; /* RingBuffer is full */

        storage[write_index] = t;

        write_index_.store(next, std::memory_order_release);

        return true;
    }

    /** Pop values into a output buffer
     *
     * \returns Number of values read/popped
     * \note Must be called from a single reader thread
     * */
    size_t pop(T* output_buffer, size_t output_count)
    {
        const size_t write_index = write_index_.load(std::memory_order_acquire);
        const size_t read_index  = read_index_.load(std::memory_order_relaxed); // only written from pop thread

        size_t       avail;
        if (write_index >= read_index)
        {
            avail = write_index - read_index;
        }
        else
        {
            avail = write_index + max_size - read_index;
        }

        if (avail == 0)
        {
            return 0;
        }

        output_count          = std::min(output_count, avail);

        size_t new_read_index = read_index + output_count;

        if (read_index + output_count >= max_size)
        {
            // copy data in two sections
            const size_t count0 = max_size - read_index;
            const size_t count1 = output_count - count0;

            std::copy(storage.begin() + read_index, storage.end(), output_buffer);
            std::copy(storage.begin(), storage.begin() + count1, output_buffer + count0);

            new_read_index -= max_size;
        }
        else
        {
            std::copy(storage.begin() + read_index, storage.begin() + (read_index + output_count), output_buffer);
            if (new_read_index == max_size)
            {
                new_read_index = 0;
            }
        }

        read_index_.store(new_read_index, std::memory_order_release);
        return output_count;
    }

    /** reset the RingBuffer
     *
     * \note Not thread-safe
     * */
    void reset()
    {
        write_index_.store(0, std::memory_order_relaxed);
        read_index_.store(0, std::memory_order_release);
    }

    /** Check if the RingBuffer is empty
     *
     * \return true, if the RingBuffer is empty, false otherwise
     * \note Due to the concurrent nature of the RingBuffer the result may be inaccurate.
     * */
    bool        empty() { return size() == 0; }

    /** Get the current RingBuffer size
     *
     * \return Size of the buffer
     * \note Due to the concurrent nature of the RingBuffer the result may be inaccurate
     * */
    std::size_t size()
    {
        const size_t write_index = write_index_.load(std::memory_order_relaxed);
        const size_t read_index  = read_index_.load(std::memory_order_relaxed);

        size_t       avail;
        if (write_index >= read_index)
        {
            avail = write_index - read_index;
        }
        else
        {
            avail = write_index + max_size - read_index;
        }

        return avail;
    }

    std::size_t capacity() { return max_size; }
};

} /* namespace lockfree */

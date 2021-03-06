#ifndef HAND_ROLLED_SBO_COW_INCLUDED__
#define HAND_ROLLED_SBO_COW_INCLUDED__

#include <printable_types.hpp>
#include <iostream>
#include <atomic>
#include <array>

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


#if defined(_MSC_VER) && _MSC_VER == 1800
#define noexcept
#define alignof __alignof
#endif

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 9
/*
This implementation is taken from libc++, since GCC <= 4.9 is missing it.

Copyright (c) 2010-2014 Howard Hinnant, Joerg Sonnenberger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
namespace std {
inline void* align(size_t alignment, size_t size, void*& ptr, size_t& space)
{
    void* r = nullptr;
    if (size <= space)
    {
        char* p1 = static_cast<char*>(ptr);
        char* p2 = reinterpret_cast<char*>(reinterpret_cast<size_t>(p1 + (alignment - 1)) & -alignment);
        size_t d = static_cast<size_t>(p2 - p1);
        if (d <= space - size)
        {
            r = p2;
            ptr = r;
            space -= d;
        }
    }
    return r;
}
}
#endif

class printable_sbo_cow
{
public:
    // Contructors
    printable_sbo_cow () :
        handle_ (nullptr),
        buffer_ {}
    {}

    template <typename T>
    printable_sbo_cow (T value) :
        handle_ (nullptr),
        buffer_ {}
    { handle_ = clone_impl(std::move(value), buffer_); }

    printable_sbo_cow (const printable_sbo_cow & rhs) :
        handle_ (
            !rhs.handle_ || heap_allocated(rhs.handle_, rhs.buffer_) ?
            rhs.handle_ :
            handle_ptr(
                char_ptr(&buffer_) + handle_offset(rhs.handle_, rhs.buffer_)
            )
        ),
        buffer_ (rhs.buffer_)
    {
        if (handle_)
            handle_->add_ref();
    }

    printable_sbo_cow (printable_sbo_cow && rhs) noexcept :
        handle_ (nullptr),
        buffer_ {}
    { swap(rhs.handle_, rhs.buffer_); }

    // Assignment
    template <typename T>
    printable_sbo_cow & operator= (T value)
    {
        reset();
        handle_ = clone_impl(std::move(value), buffer_);
        return *this;
    }

    printable_sbo_cow & operator= (const printable_sbo_cow & rhs)
    {
        printable_sbo_cow temp(rhs);
        swap(temp.handle_, temp.buffer_);
        if (handle_)
            handle_->add_ref();
        return *this;
    }

    printable_sbo_cow & operator= (printable_sbo_cow && rhs) noexcept
    {
        printable_sbo_cow temp(std::move(rhs));
        swap(temp.handle_, temp.buffer_);
        return *this;
    }

    ~printable_sbo_cow ()
    { reset(); }

    // Public interface
    void print () const
    {
        assert(handle_);
        read().print();
    }

private:
    typedef std::array<unsigned char, 24> buffer;

    struct handle_base
    {
        virtual ~handle_base () {}
        virtual handle_base * clone_into (buffer & buf) const = 0;
        virtual bool unique () const = 0;
        virtual void add_ref () = 0;
        virtual void destroy () = 0;

        // Public interface
        virtual void print () const = 0;
    };

    template <typename T, bool HeapAllocated>
    struct handle :
        handle_base
    {
        template <typename U = T>
        handle (T value,
                typename std::enable_if<
                    std::is_reference<U>::value
                >::type * = 0) :
            value_ (value),
            ref_count_ (1)
        {}

        template <typename U = T>
        handle (T value,
                typename std::enable_if<
                    !std::is_reference<U>::value,
                    int
                >::type * = 0) noexcept :
            value_ (std::move(value)),
            ref_count_ (1)
        {}

        virtual handle_base * clone_into (buffer & buf) const
        { return clone_impl(value_, buf); }

        virtual bool unique () const
        { return ref_count_ == 1u; }

        virtual void add_ref ()
        { ++ref_count_; }

        virtual void destroy ()
        {
            if (ref_count_ == 1u) {
                if (HeapAllocated)
                    delete this;
                else
                    this->~handle();
            } else {
                --ref_count_;
            }
        }

        // Public interface
        virtual void print () const
        { value_.print(); }

        T value_;
        std::atomic_size_t ref_count_;
    };

    template <typename T, bool HeapAllocated>
    struct handle<std::reference_wrapper<T>, HeapAllocated> :
        handle<T &, HeapAllocated>
    {
        handle (std::reference_wrapper<T> ref) :
            handle<T &, HeapAllocated> (ref.get())
        {}
    };

    template <typename T>
    static handle_base * clone_impl (T value, buffer & buf)
    {
        handle_base * retval = nullptr;
        typedef typename std::remove_reference<T>::type handle_t;
        void * buf_ptr = &buf;
        std::size_t buf_size = sizeof(buf);
        const std::size_t alignment = alignof(handle<handle_t, false>);
        const std::size_t size = sizeof(handle<handle_t, false>);
        const bool memcopyable =
            std::is_trivially_destructible<typename std::remove_cv<T>::type>();
        buf_ptr =
            memcopyable ?
            std::align(alignment, size, buf_ptr, buf_size) :
            nullptr;
        if (buf_ptr) {
            new (buf_ptr) handle<handle_t, false>(std::move(value));
            retval = static_cast<handle_base *>(buf_ptr);
        } else {
            retval = new handle<handle_t, true>(std::move(value));
        }
        return retval;
    }

    static bool heap_allocated (const handle_base * h, const buffer & b)
    {
        return
            h < handle_ptr(char_ptr(&b)) ||
            handle_ptr(char_ptr(&b) + sizeof(b)) <= h;
    }

    void swap (handle_base * & rhs_handle, buffer & rhs_buffer)
    {
        const bool this_heap_allocated = heap_allocated(handle_, buffer_);
        const bool rhs_heap_allocated = heap_allocated(rhs_handle, rhs_buffer);

        if (this_heap_allocated && rhs_heap_allocated) {
            std::swap(handle_, rhs_handle);
        } else if (this_heap_allocated) {
            const std::ptrdiff_t offset = handle_offset(rhs_handle, rhs_buffer);
            rhs_handle = handle_;
            buffer_ = rhs_buffer;
            handle_ = handle_ptr(char_ptr(&buffer_) + offset);
        } else if (rhs_heap_allocated) {
            const std::ptrdiff_t offset = handle_offset(handle_, buffer_);
            handle_ = rhs_handle;
            rhs_buffer = buffer_;
            rhs_handle = handle_ptr(char_ptr(&rhs_buffer) + offset);
        } else {
            const std::ptrdiff_t this_offset =
                handle_offset(handle_, buffer_);
            const std::ptrdiff_t rhs_offset =
                handle_offset(rhs_handle, rhs_buffer);
            std::swap(buffer_, rhs_buffer);
            handle_ = handle_ptr(char_ptr(&buffer_) + rhs_offset);
            rhs_handle = handle_ptr(char_ptr(&rhs_buffer) + this_offset);
        }
    }

    void reset()
    {
        if (handle_)
            handle_->destroy();
    }

    const handle_base & read () const
    { return *handle_; }

    handle_base & write ()
    {
        if (!handle_->unique())
            handle_ = handle_->clone_into(buffer_);
        return *handle_;
    }

    template <typename T>
    static unsigned char * char_ptr (T * ptr)
    {
        return static_cast<unsigned char *>(
            static_cast<void *>(
                const_cast<typename std::remove_const<T>::type *>(ptr)
            )
        );
    }

    static handle_base * handle_ptr (unsigned char * ptr)
    { return static_cast<handle_base *>(static_cast<void *>(ptr)); }

    static std::ptrdiff_t handle_offset (const handle_base * h,
                                         const buffer & b)
    {
        assert(h);
        unsigned char * const char_handle = char_ptr(h);
        unsigned char * const char_buffer = char_ptr(&b);
        return char_handle - char_buffer;
    }

    handle_base * handle_;
    buffer buffer_;
};

#endif

#ifndef HAND_ROLLED_SBO_COW_INCLUDED__
#define HAND_ROLLED_SBO_COW_INCLUDED__

#include <printable_types.hpp>

#include <array>
#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


#ifdef _MSC_VER
#define noexcept
#define alignof __alignof
#endif

class printable_sbo_cow
{
public:
    // Contructors
    printable_sbo_cow () :
        handle_ (nullptr),
        buffer_ {},
        ref_count_ (0)
    {}

    template <typename T>
    printable_sbo_cow (T value) :
        handle_ (nullptr),
        buffer_ {},
        ref_count_ (1)
    { handle_ = clone_impl(std::forward<T>(value), buffer_); }

    printable_sbo_cow (const printable_sbo_cow & rhs) :
        handle_ (
            !rhs.handle_ || rhs.handle_->heap_allocated() ?
            rhs.handle_ :
            handle_ptr(
                char_ptr(&buffer_) + handle_offset(rhs.handle_, rhs.buffer_)
            )
        ),
        buffer_ (rhs.buffer_),
        ref_count_ (static_cast<std::size_t>(rhs.ref_count_))
    {
        if (handle_)
            ++ref_count_;
    }

    printable_sbo_cow (printable_sbo_cow && rhs) noexcept :
        handle_ (nullptr),
        buffer_ {},
        ref_count_ (0)
    { swap(rhs.handle_, rhs.buffer_, rhs.ref_count_); }

    // Assignment
    template <typename T>
    printable_sbo_cow & operator= (T value)
    {
        reset();
        handle_ = clone_impl(std::forward<T>(value), buffer_);
        ref_count_ = 1;
        return *this;
    }

    printable_sbo_cow & operator= (const printable_sbo_cow & rhs)
    {
        printable_sbo_cow temp(rhs);
        swap(temp.handle_, temp.buffer_, temp.ref_count_);
        if (handle_)
            ++ref_count_;
        return *this;
    }

    printable_sbo_cow & operator= (printable_sbo_cow && rhs) noexcept
    {
        printable_sbo_cow temp(std::move(rhs));
        swap(temp.handle_, temp.buffer_, temp.ref_count_);
        return *this;
    }

    ~printable_sbo_cow ()
    { reset(); }

    // Public interface
    void print () const
    {
        assert(handle_);
        handle_->print();
    }

private:
    typedef std::array<unsigned char, 24> buffer;

    struct handle_base
    {
        virtual ~handle_base () {}
        virtual handle_base * clone_into (buffer & buf) const = 0;
        virtual bool heap_allocated () const = 0;
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
            value_ (value)
        {}

        template <typename U = T>
        handle (T value,
                typename std::enable_if<
                    !std::is_reference<U>::value,
                    int
                >::type * = 0) noexcept :
            value_ (std::move(value))
        {}

        virtual handle_base * clone_into (buffer & buf) const
        { return clone_impl(value_, buf); }

        virtual bool heap_allocated () const
        { return HeapAllocated; }

        virtual void destroy ()
        {
            if (HeapAllocated)
                delete this;
            else
                this->~handle();
        }

        // Public interface
        virtual void print () const
        { value_.print(); }

        T value_;
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
        buf_ptr = std::align(alignment, size, buf_ptr, buf_size);
        if (buf_ptr) {
            new (buf_ptr) handle<handle_t, false>(std::forward<T>(value));
            retval = static_cast<handle_base *>(buf_ptr);
        } else {
            retval = new handle<handle_t, true>(std::forward<T>(value));
        }
        return retval;
    }

    void swap (handle_base * & rhs_handle,
               buffer & rhs_buffer,
               std::atomic_size_t & rhs_ref_count)
    {
        const bool this_heap_allocated =
            !handle_ || handle_->heap_allocated();
        const bool rhs_heap_allocated =
            !rhs_handle || rhs_handle->heap_allocated();

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
            handle_ = handle_ptr(char_ptr(&buffer_) + this_offset);
            rhs_handle = handle_ptr(char_ptr(&rhs_buffer) + rhs_offset);
        }

        ref_count_ = rhs_ref_count.exchange(ref_count_);
    }

    void reset()
    {
        if (handle_ && ref_count_ == 1u)
            handle_->destroy();
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
    std::atomic_size_t ref_count_;
};

#endif

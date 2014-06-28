#ifndef HAND_ROLLED_SBO_COW_INCLUDED__
#define HAND_ROLLED_SBO_COW_INCLUDED__

#include <printable_types.hpp>

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


#ifdef _MSC_VER
#define noexcept
#endif

class any_printable_cow
{
public:
    // Contructors
    any_printable_cow () :
        handle_ (nullptr),
        buffer_ {}
    {}

    template <typename T>
    any_printable_cow (T value) :
        handle_ (nullptr),
        buffer_ {}
    { handle_ = clone_impl(std::forward<T>(value), buffer_); }

    any_printable_cow (const any_printable_cow & rhs) :
        handle_ (nullptr),
        buffer_ {}
    {
        if (rhs.handle_)
            handle_ = rhs.handle_->clone_into(buffer_);
    }

    any_printable_cow (any_printable_cow && rhs) noexcept :
        handle_ (nullptr),
        buffer_ (rhs.buffer_)
    { std::swap(rhs.handle_, handle_); }

    // Assignment
    template <typename T>
    any_printable_cow & operator= (T value)
    {
        handle_ = clone_impl(std::forward<T>(value), buffer_);
        return *this;
    }

    any_printable_cow & operator= (const any_printable_cow & rhs)
    {
        any_printable_cow temp(rhs);
        std::swap(temp.handle_, handle_);
        std::swap(temp.buffer_, buffer_);
        return *this;
    }

    any_printable_cow & operator= (any_printable_cow && rhs) noexcept
    {
        any_printable_cow temp(std::move(rhs));
        std::swap(temp.handle_, handle_);
        std::swap(temp.buffer_, buffer_);
        return *this;
    }

    ~any_printable_cow ()
    {
        if (handle_)
            handle_->destroy();
    }

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
        buf_ptr = std::align(
            alignof(handle<handle_t, false>),
            sizeof(handle<handle_t, false>),
            buf_ptr,
            buf_size
        );
        if (buf_ptr) {
            new (buf_ptr) handle<handle_t, false>(std::forward<T>(value));
            retval = static_cast<handle_base *>(buf_ptr);
        } else {
            retval = new handle<handle_t, true>(std::forward<T>(value));
        }
        return retval;
    }

    handle_base * handle_;
    buffer buffer_;
};

#endif

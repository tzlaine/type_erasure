#ifndef HAND_ROLLED_VTABLE_INCLUDED__
#define HAND_ROLLED_VTABLE_INCLUDED__

#include <printable_types.hpp>

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


class any_printable
{
public:
    // Contructors
    any_printable () :
        handle_ (nullptr),
        buffer_ {}
    {}

    template <typename T>
    any_printable (T value) :
        handle_ (nullptr),
        buffer_ {}
    { handle_ = clone_impl(std::forward<T>(value), buffer_); }

    any_printable (const any_printable & rhs) :
        handle_ (nullptr),
        buffer_ {}
    {
        if (rhs.handle_)
            handle_ = rhs.handle_->clone_into(buffer_);
    }

    any_printable (any_printable && rhs) noexcept :
        handle_ (nullptr),
        buffer_ (rhs.buffer_)
    { std::swap(rhs.handle_, handle_); }

    // Assignment
    template <typename T>
    any_printable & operator= (T value)
    {
        handle_ = clone_impl(std::forward<T>(value), buffer_);
        return *this;
    }

    any_printable & operator= (const any_printable & rhs)
    {
        any_printable temp(rhs);
        std::swap(temp.handle_, handle_);
        std::swap(temp.buffer_, buffer_);
        return *this;
    }

    any_printable & operator= (any_printable && rhs) noexcept
    {
        any_printable temp(std::move(rhs));
        std::swap(temp.handle_, handle_);
        std::swap(temp.buffer_, buffer_);
        return *this;
    }

    ~any_printable ()
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
                >::type* = 0) :
            value_ (value)
        {}

        template <typename U = T>
        handle (T value,
                typename std::enable_if<
                    !std::is_reference<U>::value,
                    int
                >::type* = 0) noexcept :
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
        const bool too_big = sizeof(buf) < sizeof(handle<handle_t, false>);
        if (too_big) {
            retval = new handle<handle_t, true>(std::forward<T>(value));
        } else {
            new (&buf) handle<handle_t, false>(std::forward<T>(value));
            retval =
                static_cast<handle_base *>(static_cast<void *>(&buf));
        }
        return retval;
    }

    handle_base * handle_;
    buffer buffer_;
};

#endif

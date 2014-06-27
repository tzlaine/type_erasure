#ifndef HAND_ROLLED_INCLUDED__
#define HAND_ROLLED_INCLUDED__

#include <printable_types.hpp>

#include <algorithm>
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

// Variation 1: Accept std::reference_wrapper<>.
#ifndef ACCEPT_REFERENCE_WRAPPER
#define ACCEPT_REFERENCE_WRAPPER 1
#endif

class any_printable
{
public:
    // Contructors
    any_printable () = default;

    template <typename T>
    any_printable (T value) :
        handle_ (
            new handle<typename std::remove_reference<T>::type>(
                std::forward<T>(value)
            )
        )
    {}

    any_printable (const any_printable & rhs) :
        handle_ (rhs.handle_->clone())
    {}

    any_printable (any_printable && rhs) noexcept :
        handle_ (std::move(rhs.handle_))
    {}

    // Assignment
    template <typename T>
    any_printable& operator= (T value)
    {
        any_printable temp(std::forward<T>(value));
        std::swap(temp, *this);
        return *this;
    }

    any_printable & operator= (const any_printable & rhs)
    {
        any_printable temp(rhs);
        std::swap(temp, *this);
        return *this;
    }

    any_printable & operator= (any_printable && rhs) noexcept
    {
        handle_ = std::move(rhs.handle_);
        return *this;
    }

    // Public interface
    void print () const
    {
        assert(handle_);
        handle_->print();
    }

private:
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual handle_base* clone () const = 0;

        // Public interface
        virtual void print () const = 0;
    };

    template <typename T>
    struct handle :
        handle_base
    {
#if ACCEPT_REFERENCE_WRAPPER
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
#else
        handle (T value) :
            value_ (std::move(value))
        {}
#endif

        virtual handle_base* clone () const
        { return new handle(value_); }

        // Public interface
        virtual void print () const
        { value_.print(); }

        T value_;
    };

#if ACCEPT_REFERENCE_WRAPPER
    template <typename T>
    struct handle<std::reference_wrapper<T>> :
        handle<T &>
    {
        handle (std::reference_wrapper<T> ref) :
            handle<T &> (ref.get())
        {}
    };
#endif

    std::unique_ptr<handle_base> handle_;
};

#endif

#ifndef HAND_ROLLED_COW_INCLUDED__
#define HAND_ROLLED_COW_INCLUDED__

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


class any_printable_cow
{
public:
    // Contructors
    any_printable_cow () = default;

    template <typename T>
    any_printable_cow (T value) :
        handle_ (
            std::make_shared<handle<typename std::remove_reference<T>::type>>(
                std::forward<T>(value)
            )
        )
    {
#if INSTRUMENT_COPIES
        ++allocations();
#endif
    }

    // Assignment
    template <typename T>
    any_printable_cow& operator= (T value)
    {
        if (handle_.unique())
            *handle_ = std::forward<T>(value);
        else if (!handle_) {
            handle_ = std::make_shared<T>(std::forward<T>(value));
#if INSTRUMENT_COPIES
            ++allocations();
#endif
        }
        return *this;
    }

    // Public interface
    void print () const
    {
        assert(handle_);
        read().print();
    }

#if 0 // non-const example
    void foo ()
    {
        assert(handle_);
        write().foo();
    }
#endif

private:
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual std::shared_ptr<handle_base> clone () const = 0;

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
            value_ (value)
        {}
#endif

        virtual std::shared_ptr<handle_base> clone () const
        {
#if INSTRUMENT_COPIES
            ++allocations();
#endif
            return std::make_shared<handle>(value_);
        }

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

    const handle_base & read () const
    { return *handle_; }

    handle_base & write ()
    {
        if (!handle_.unique())
            handle_ = handle_->clone();
        return *handle_;
    }

    std::shared_ptr<handle_base> handle_;
};

#endif

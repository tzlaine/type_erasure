#ifndef HAND_ROLLED_COW_INCLUDED__
#define HAND_ROLLED_COW_INCLUDED__

#include <printable_types.hpp>
#include <iostream>

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


#if defined(_MSC_VER) && _MSC_VER == 1800
#define noexcept
#endif


class printable_cow
{
public:
    // Contructors
    printable_cow () = default;

    template <typename T>
    printable_cow (T value) :
        handle_ (
            std::make_shared<handle<typename std::remove_reference<T>::type>>(
                std::move(value)
            )
        )
    {}

    // Assignment
    template <typename T>
    printable_cow & operator= (T value)
    {
        printable_cow temp(std::move(value));
        std::swap(temp.handle_, handle_);
        return *this;
    }

    // Public interface
    void print () const
    {
        assert(handle_);
        read().print();
    }

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

        virtual std::shared_ptr<handle_base> clone () const
        { return std::make_shared<handle>(value_); }

        // Public interface
        virtual void print () const
        { value_.print(); }

        T value_;
    };

    template <typename T>
    struct handle<std::reference_wrapper<T>> :
        handle<T &>
    {
        handle (std::reference_wrapper<T> ref) :
            handle<T &> (ref.get())
        {}
    };

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

#ifndef HAND_ROLLED_INCLUDED__
#define HAND_ROLLED_INCLUDED__

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


// Variation 1: Accept std::reference_wrapper<>.
#ifndef ACCEPT_REFERENCE_WRAPPER
#define ACCEPT_REFERENCE_WRAPPER 1
#endif

#ifndef INSTRUMENT_COPIES
#define INSTRUMENT_COPIES 1
#endif

#if INSTRUMENT_COPIES
std::size_t& allocations ()
{
    static std::size_t allocations_ = 0;
    return allocations_;
}

void reset_allocations ()
{ allocations() = 0; }
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
    {
#if INSTRUMENT_COPIES
        ++allocations();
#endif
    }

    any_printable (const any_printable & rhs) :
        handle_ (rhs.handle_->clone())
    {}

    any_printable (any_printable && rhs) noexcept = default;

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

    any_printable & operator= (any_printable && rhs) noexcept = default;

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
            value_ (value)
        {}

        handle (T && value) noexcept :
            value_ (std::move(value))
        {}
#endif

        virtual handle_base* clone () const
        {
#if INSTRUMENT_COPIES
            ++allocations();
#endif
            return new handle(value_);
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

    std::unique_ptr<handle_base> handle_;
};


struct hi_printable
{
    void print () const
    { std::cout << "Hello, world!\n"; }
};

struct bye_printable
{
    void print () const
    { std::cout << "Bye, now!\n"; }
};

struct large_printable :
    std::vector<std::string>
{
    large_printable () :
        std::vector<std::string> (1000, std::string(1000, ' '))
    { ++allocations(); }

    large_printable (const large_printable & rhs) :
        std::vector<std::string> (rhs)
    { ++allocations(); }

    large_printable & operator= (const large_printable & rhs)
    {
        ++allocations();
        static_cast<std::vector<std::string> &>(*this) = rhs;
        return *this;
    }

    large_printable (large_printable && rhs) = default;
    large_printable & operator= (large_printable && rhs) = default;

    void print () const
    { std::cout << "I'm expensive to copy.\n"; }
};


/* Limitations:
   1 - Each member functions must be repeated in 3 places.
   2 - Macros, which could be used to address this, are evil.
   3 - How do you define an any_fooable type, where foo() is a free function?
   4 - How do you define an any_barable type, where bar is an operator?
   5 - How do you apply the small buffer optimization to handle small types without making allocations?
*/

#endif

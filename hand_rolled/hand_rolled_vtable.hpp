#ifndef HAND_ROLLED_INCLUDED__
#define HAND_ROLLED_INCLUDED__

#include <printable_types.hpp>

#include <array>
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

class any_printable
{
private:
    template <typename ValueType>
    static void * clone_impl (void * value)
    { return new ValueType(*static_cast<ValueType*>(value)); }

    using clone_wrapper_type = void * (*) (void *);

    template <typename ValueType>
    static void delete_impl (void * value)
    { return delete static_cast<ValueType*>(value); }

    using delete_wrapper_type = void (*) (void *);

    template <typename ValueType>
    struct print_wrapper
    {
        static void exec (void * value)
        { static_cast<ValueType*>(value)->print(); }
    };

    template <typename ValueType>
    struct print_wrapper<std::reference_wrapper<ValueType>>
    {
        static void exec (void * value)
        { static_cast<std::reference_wrapper<ValueType>*>(value)->get().print(); }
    };

    using print_wrapper_type = void (*) (void *);

public:
    // Contructors
    any_printable ();

    template <typename T>
    any_printable (T value) :
        vtable_ {
            (void_function_type)(&clone_impl<T>),
            (void_function_type)(&delete_impl<T>),
            (void_function_type)(&print_wrapper<T>::exec)
        },
        get_value_ptr_ (&get_value_ptr<sizeof(value_) < sizeof(T)>),
        value_ (0)
    {
        if (sizeof(T) <= sizeof(value_))
            new (&value_) T(std::move(value));
        else
            value_ = new T(std::move(value));
    }

    any_printable (const any_printable & rhs) :
        vtable_ (rhs.vtable_),
        get_value_ptr_ (rhs.get_value_ptr_),
        value_ (rhs.clone())
    {}

    any_printable (any_printable && rhs) noexcept;

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
        any_printable temp(std::move(rhs));
        std::swap(temp.vtable_, vtable_);
        std::swap(temp.get_value_ptr_, get_value_ptr_);
        std::swap(temp.value_, value_);
        return *this;
    }

    ~any_printable ()
    {
        if (heap_allocated()) {
            delete_wrapper_type delete_impl = (delete_wrapper_type)(vtable_[1]);
            delete_impl(get_value_ptr_(this));
        }
    }

    // Public interface
    void print () const
    {
        print_wrapper_type print_impl = (print_wrapper_type)(vtable_[2]);
        print_impl(get_value_ptr_(this));
    }

private:
    using get_value_ptr_type = void * (*) (const any_printable *);

    template <bool FromHeap>
    static void * get_value_ptr (const any_printable * _this)
    { return const_cast<void *>(_this->value_); }

    bool heap_allocated () const
    { return get_value_ptr_ == &get_value_ptr<true>; }

    void * clone () const
    {
        if (heap_allocated()) {
            clone_wrapper_type clone_impl = (clone_wrapper_type)(vtable_[0]);
            return clone_impl(get_value_ptr_(this));
        } else {
            return value_;
        }
    }

    using void_function_type = void (*) ();

    std::array<void_function_type, 3> vtable_;
    get_value_ptr_type get_value_ptr_;
    void * value_;
};

template <>
void * any_printable::get_value_ptr<false> (const any_printable * _this)
{ return const_cast<void *>(static_cast<const void *>(&_this->value_)); }

any_printable::any_printable () :
    get_value_ptr_ (get_value_ptr<false>),
    value_ (0)
{}

any_printable::any_printable (any_printable && rhs) noexcept :
    vtable_ (rhs.vtable_),
    get_value_ptr_ (rhs.get_value_ptr_),
    value_ (rhs.value_)
{ rhs.get_value_ptr_ = &get_value_ptr<false>; }



/* Limitations:
   1 - Each member functions must be repeated in 3 places.
   2 - Macros, which could be used to address this, are evil.
   3 - How do you define an any_fooable type, where foo() is a free function?
   4 - How do you define an any_barable type, where bar is an operator?
   5 - How do you apply the small buffer optimization to handle small types without making allocations?
*/

#endif

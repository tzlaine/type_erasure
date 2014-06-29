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


#ifdef _MSC_VER
#define noexcept
#endif

class printable_vtable
{
private:
    template <typename ValueType>
    static void * clone_impl (void * value)
    { return new ValueType(*static_cast<ValueType*>(value)); }

    using clone_wrapper_type = void * (*) (void *);

    template <typename ValueType>
    static void delete_impl (void * value)
    { delete static_cast<ValueType*>(value); }

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
    printable_vtable ();

    template <typename T>
    printable_vtable (T value) :
        vtable_ ({
            (void_function_type)(&clone_impl<T>),
            (void_function_type)(&delete_impl<T>),
            (void_function_type)(&print_wrapper<T>::exec)
        }),
        get_value_ptr_ (&get_value_ptr<sizeof(value_) < sizeof(T)>),
        value_ (0)
    {
        if (sizeof(T) <= sizeof(value_))
            new (&value_) T(std::move(value));
        else
            value_ = new T(std::move(value));
    }

    printable_vtable (const printable_vtable & rhs) :
        vtable_ (rhs.vtable_),
        get_value_ptr_ (rhs.get_value_ptr_),
        value_ (rhs.clone())
    {}

    printable_vtable (printable_vtable && rhs) noexcept;

    // Assignment
    template <typename T>
    printable_vtable & operator= (T value)
    {
        printable_vtable temp(std::forward<T>(value));
        std::swap(temp, *this);
        return *this;
    }

    printable_vtable & operator= (const printable_vtable & rhs)
    {
        printable_vtable temp(rhs);
        std::swap(temp, *this);
        return *this;
    }

    printable_vtable & operator= (printable_vtable && rhs) noexcept
    {
        printable_vtable temp(std::move(rhs));
        std::swap(temp.vtable_, vtable_);
        std::swap(temp.get_value_ptr_, get_value_ptr_);
        std::swap(temp.value_, value_);
        return *this;
    }

    ~printable_vtable ()
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
    using get_value_ptr_type = void * (*) (const printable_vtable *);

    template <bool FromHeap>
    static void * get_value_ptr (const printable_vtable * _this)
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
void * printable_vtable::get_value_ptr<false> (const printable_vtable * _this)
{ return const_cast<void *>(static_cast<const void *>(&_this->value_)); }

printable_vtable::printable_vtable () :
    get_value_ptr_ (get_value_ptr<false>),
    value_ (0)
{}

printable_vtable::printable_vtable (printable_vtable && rhs) noexcept :
    vtable_ (rhs.vtable_),
    get_value_ptr_ (rhs.get_value_ptr_),
    value_ (rhs.value_)
{ rhs.get_value_ptr_ = &get_value_ptr<false>; }

#endif

#ifndef ANYTHING_INCLUDED__
#define ANYTHING_INCLUDED__

#include <memory>


#ifndef ANYTHING_WITH_A_VALUE
#define ANYTHING_WITH_A_VALUE 0
#endif

// sample(anything_declaration)
struct anything
{
    anything () = default;
    anything (const anything & rhs);
    anything & operator= (const anything & rhs);
    template <typename T> anything (T t);
    template <typename T> anything & operator= (T t);

// end-sample
#if ANYTHING_WITH_A_VALUE
    int value () const
    { return handle_->value(); }
#endif
// sample(anything_declaration)
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual handle_base* clone () const = 0;
// end-sample
#if ANYTHING_WITH_A_VALUE
        virtual int value () const = 0;
#endif
// sample(anything_declaration)
    };

    template <typename T>
    struct handle : handle_base
    {
        handle (T value);
        virtual handle_base* clone () const;
// end-sample
#if ANYTHING_WITH_A_VALUE
        virtual int value () const
        { return value_.value(); }
#endif
// sample(anything_declaration)
        T value_;
    };

    std::unique_ptr<handle_base> handle_;
};
// end-sample

// sample(anything_definitions)
template <typename T>
anything::anything (T t) :
    handle_ (new handle<typename std::remove_reference<T>::type>(
        std::forward<T>(t)
    ))
{}

anything::anything (const anything & rhs) : handle_ (rhs.handle_->clone()) {}

template <typename T>
anything & anything::operator= (T t)
{
    anything temp(std::forward<T>(t));
    std::swap(temp, *this);
    return *this;
}

anything & anything::operator= (const anything & rhs)
{
    anything temp(rhs);
    std::swap(temp, *this);
    return *this;
}
// end-sample

// sample(anything_handle_definitions)
template <typename T> 
anything::handle<T>::handle (T value) :
    value_ (std::move(value))
{}

template <typename T> 
anything::handle_base* anything::handle<T>::clone () const
{ return new handle(value_); }
// end-sample


#endif

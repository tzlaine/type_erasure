#ifndef COPY_ON_WRITE_HPP_INCLUDED__
#define COPY_ON_WRITE_HPP_INCLUDED__

#include <cassert>
#include <memory>
#include <mutex>


template <typename T>
struct copy_on_write
{
    typedef T value_type;

    copy_on_write ()
    {
        std::call_once(flag_s, make_default);
        impl_ = default_s;
    }

    copy_on_write (T value) :
        impl_ (std::make_shared<T>(std::move(value)))
    {}

    copy_on_write& operator= (T value)
    {
        if (impl_.unique())
            *impl_ = std::move(value);
        else if (!impl_)
            impl_ = std::make_shared<T>(std::move(value));
    }

    const value_type & read () const
    {
        assert(impl_ && "Attempted to use a moved-from copy_on_write object.");
        return *impl_;
    }

    operator const value_type & () const
    { return read(); }

    const value_type & operator* () const
    { return read(); }
 
    const value_type * operator-> () const
    { return &read(); }

    bool unique () const
    { return !impl_ || impl_.unique(); }

    value_type & write ()
    {
        assert(impl_ && "Attempted to use a moved-from copy_on_write object.");
        if (!impl_.unique())
            impl_ = std::make_shared<T>(*impl_);
        return *impl_;
    }

    friend inline bool identity (const copy_on_write & lhs,
                                 const copy_on_write & rhs)
    { return lhs.impl_ == rhs.impl_; }

    friend inline bool operator== (const copy_on_write & lhs,
                                   const copy_on_write & rhs)
    { return identity(lhs, rhs) || lhs.impl_ && rhs.impl_ && *lhs == *rhs; }

    friend inline bool operator!= (const copy_on_write & lhs,
                                   const copy_on_write & rhs)
    { return !(lhs == rhs); }

    friend inline bool operator< (const copy_on_write & lhs,
                                  const copy_on_write & rhs)
    { return rhs.impl_ && (!lhs.impl_ || *lhs < *rhs); }

    friend inline bool operator> (const copy_on_write & lhs,
                                  const copy_on_write & rhs)
    { return rhs < lhs; }

    friend inline bool operator<= (const copy_on_write & lhs,
                                   const copy_on_write & rhs)
    { return !(rhs < lhs); }

    friend inline bool operator>= (const copy_on_write & lhs,
                                   const copy_on_write & rhs)
    { return !(lhs < rhs); }

private:
    typedef std::shared_ptr<T> impl;

    static void make_default ()
    { default_s = std::make_shared<T>(); }

    impl impl_;

    static impl default_s;
    static std::once_flag flag_s;
};

template <typename T>
typename copy_on_write<T>::impl copy_on_write<T>::default_s;

template <typename T>
std::once_flag copy_on_write<T>::flag_s;

#endif

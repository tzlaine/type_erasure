// sample(loggable_erased_type)
#ifndef LOGGABLE_INTERFACE_INCLUDED__
#define LOGGABLE_INTERFACE_INCLUDED__

# include <iostream>

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#if defined(_MSC_VER) && _MSC_VER == 1800
#define noexcept
#endif

struct loggable {
public:
    // Lots of boilerplate code here.
// end-sample
    // Contructors
    loggable () = default;

    template <typename T_T__>
    loggable (T_T__ value) :
        handle_ (
            std::make_shared<
                handle<typename std::remove_reference<T_T__>::type>
            >(std::forward<T_T__>(value))
        )
    {}

    // Assignment
    template <typename T_T__>
    loggable & operator= (T_T__ value)
    {
        if (handle_.unique())
            *handle_ = std::forward<T_T__>(value);
        else if (!handle_)
            handle_ = std::make_shared<T_T__>(std::forward<T_T__>(value));
        return *this;
    }

// sample(loggable_erased_type)
    std :: ostream & log ( std :: ostream & os) const
    { assert(handle_); return handle_->log(os ); }

private:
    // Even more boilerplate code here.
// end-sample
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual std::shared_ptr<handle_base> clone () const = 0;

        virtual std :: ostream & log ( std :: ostream & os ) const = 0;
    };

    template <typename T_T__>
    struct handle :
        handle_base
    {
        template <typename U_U__ = T_T__>
        handle (T_T__ value,
                typename std::enable_if<
                    std::is_reference<U_U__>::value
                >::type * = 0) :
            value_ (value)
        {}

        template <typename U_U__ = T_T__>
        handle (T_T__ value,
                typename std::enable_if<
                    !std::is_reference<U_U__>::value,
                    int
                >::type * = 0) noexcept :
            value_ (std::move(value))
        {}

        virtual std::shared_ptr<handle_base> clone () const
        { return std::make_shared<handle>(value_); }

        std :: ostream & log ( std :: ostream & os ) const
        { return value_.log(os ); }

        T_T__ value_;
    };

    template <typename T_T__>
    struct handle<std::reference_wrapper<T_T__>> :
        handle<T_T__ &>
    {
        handle (std::reference_wrapper<T_T__> ref) :
            handle<T_T__ &> (ref.get())
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
// sample(loggable_erased_type)
};

#endif
// end-sample

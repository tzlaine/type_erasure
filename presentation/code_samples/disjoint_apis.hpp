# include "layout_geometry.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


#if defined(_MSC_VER) && _MSC_VER == 1800
#define noexcept
#endif

// sample(widget_api)
struct widget
{
    // some boilerplate ...
// end-sample
public:
    // Contructors
    widget () = default;

    template <typename T_T__>
    widget (T_T__ value) :
        handle_ (
            std::make_shared<
                handle<typename std::remove_reference<T_T__>::type>
            >(std::forward<T_T__>(value))
        )
    {}

    // Assignment
    template <typename T_T__>
    widget & operator= (T_T__ value)
    {
        if (handle_.unique())
            *handle_ = std::forward<T_T__>(value);
        else if (!handle_)
            handle_ = std::make_shared<T_T__>(std::forward<T_T__>(value));
        return *this;
    }

#if 0
// sample(widget_api)
    void render () const;
// end-sample
#endif
    void render ( ) const
    { assert(handle_); handle_->render( ); }

private:
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual std::shared_ptr<handle_base> clone () const = 0;

        virtual void render ( ) const = 0;
    };

    template <typename T>
    struct handle :
        handle_base
    {
// sample(handle_ref_wrapper_ctors)
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
// end-sample

        virtual std::shared_ptr<handle_base> clone () const
        { return std::make_shared<handle>(value_); }

        void render ( ) const
        { value_.render( ); }

        T value_;
    };

// sample(ref_wrapper_specialization)
    template <typename T>
    struct handle<std::reference_wrapper<T>> :
        handle<T &>
    {
        handle (std::reference_wrapper<T> ref) :
            handle<T &> (ref.get())
        {}
    };
// end-sample

    const handle_base & read () const
    { return *handle_; }

    handle_base & write ()
    {
        if (!handle_.unique())
            handle_ = handle_->clone();
        return *handle_;
    }

    std::shared_ptr<handle_base> handle_;
// sample(widget_api)
    // more boilerplate ...
};
// end-sample



// sample(layoutable_api)
struct layoutable
{
    // boilerplate ...
// end-sample
public:
    // Contructors
    layoutable () = default;

    template <typename T_T__>
    layoutable (T_T__ value) :
        handle_ (
            std::make_shared<
                handle<typename std::remove_reference<T_T__>::type>
            >(std::forward<T_T__>(value))
        )
    {}

    // Assignment
    template <typename T_T__>
    layoutable & operator= (T_T__ value)
    {
        if (handle_.unique())
            *handle_ = std::forward<T_T__>(value);
        else if (!handle_)
            handle_ = std::make_shared<T_T__>(std::forward<T_T__>(value));
        return *this;
    }

#if 0
// sample(layoutable_api)
    layout_geometry geometry () const;
// end-sample
#endif
    layout_geometry geometry ( ) const
    { assert(handle_); return handle_->geometry( ); }

private:
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual std::shared_ptr<handle_base> clone () const = 0;

        virtual layout_geometry geometry ( ) const = 0;
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

        layout_geometry geometry ( ) const
        { return value_.geometry( ); }

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
// sample(layoutable_api)
    // boilerplate ...
};
// end-sample

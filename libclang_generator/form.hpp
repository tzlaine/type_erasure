%struct_prefix%
{
public:
    // Contructors
    %struct_name% () = default;

    template <typename T_T__>
    %struct_name% (T_T__ value) :
        handle_ (
            std::make_shared<
                handle<typename std::remove_reference<T_T__>::type>
            >(std::forward<T_T__>(value))
        )
    {}

    // Assignment
    template <typename T_T__>
    %struct_name% & operator= (T_T__ value)
    {
        if (handle_.unique())
            *handle_ = std::forward<T_T__>(value);
        else if (!handle_)
            handle_ = std::make_shared<T_T__>(std::forward<T_T__>(value));
        return *this;
    }

    %nonvirtual_members%

private:
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual std::shared_ptr<handle_base> clone () const = 0;

        %pure_virtual_members%
    };

    template <typename T_T__>
    struct handle :
        handle_base
    {
        template <typename U_U__ = T_T__>
        handle (T_T__ value,
                typename std::enable_if<
                    std::is_reference<U_U__>::value
                >::type* = 0) :
            value_ (value)
        {}

        template <typename U_U__ = T_T__>
        handle (T_T__ value,
                typename std::enable_if<
                    !std::is_reference<U_U__>::value,
                    int
                >::type* = 0) noexcept :
            value_ (std::move(value))
        {}

        virtual std::shared_ptr<handle_base> clone () const
        { return std::make_shared<handle>(value_); }

        %virtual_members%

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
};

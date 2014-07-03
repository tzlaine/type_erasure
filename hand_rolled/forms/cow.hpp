%struct_prefix%
{
public:
    // Contructors
    %struct_name% () = default;

    template <typename T>
    %struct_name% (T value) :
        handle_ (
            std::make_shared<handle<typename std::remove_reference<T>::type>>(
                std::forward<T>(value)
            )
        )
    {}

    // Assignment
    template <typename T>
    %struct_name% & operator= (T value)
    {
        %struct_name% temp(std::forward<T>(value));
        std::swap(temp.handle_, handle_);
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

        %virtual_members%

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

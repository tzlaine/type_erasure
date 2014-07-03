%struct_prefix%
{
public:
    // Contructors
    %struct_name% () = default;

    template <typename T>
    %struct_name% (T value) :
        handle_ (
            new handle<typename std::remove_reference<T>::type>(
                std::forward<T>(value)
            )
        )
    {}

    %struct_name% (const %struct_name% & rhs) :
        handle_ (rhs.handle_->clone())
    {}

    %struct_name% (%struct_name% && rhs) noexcept :
        handle_ (std::move(rhs.handle_))
    {}

    // Assignment
    template <typename T>
    %struct_name% & operator= (T value)
    {
        %struct_name% temp(std::forward<T>(value));
        std::swap(temp, *this);
        return *this;
    }

    %struct_name% & operator= (const %struct_name% & rhs)
    {
        %struct_name% temp(rhs);
        std::swap(temp, *this);
        return *this;
    }

    %struct_name% & operator= (%struct_name% && rhs) noexcept
    {
        handle_ = std::move(rhs.handle_);
        return *this;
    }

    %nonvirtual_members%

private:
    struct handle_base
    {
        virtual ~handle_base () {}
        virtual handle_base * clone () const = 0;

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

        virtual handle_base * clone () const
        { return new handle(value_); }

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

    std::unique_ptr<handle_base> handle_;
};

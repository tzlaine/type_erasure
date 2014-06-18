
namespace some_ns {

    struct any_foo
    {
        explicit any_foo (int i);
        void print () const;
        void print ();

        struct ignored_nested_struct
        {
            int i;
        };
    };

    bool operator< (any_foo lhs, any_foo rhs);

    namespace some_other_ns {

        class any_bar
        {
            explicit any_bar (int i);
            std::size_t index (std::size_t i) const;
            void print (double d);
        };

        bool operator< (any_bar lhs, any_bar rhs);

    }

    template <typename T>
    struct any_foo_template
    {
        explicit any_foo_template (int i);
        void print () const;
        void print ();
    };

    template <typename T>
    bool operator< (any_foo_template<T> lhs, any_foo_template<T> rhs);

    template <typename T>
    class any_bar_template
    {
        explicit any_bar_template (int i);
        T get () const;
        T get ();
    };

    template <typename T>
    bool operator< (any_bar_template<T> lhs, any_bar_template<T> rhs);

}

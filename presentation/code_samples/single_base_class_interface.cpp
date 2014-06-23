template <typename T>
void uses_foo (T * t)
{ t->foo(); }

template <typename T>
void uses_bar (T * t)
{ t->bar(); }

// sample(single_base_class_interface)
struct base
{ virtual int foo () const = 0; };

struct derived : base
{
    virtual int foo () const
    { return 42; }

    float bar () const
    { return 3.0f; }
};

void some_function () {
    base * b_pointer = new derived;
    uses_foo(b_pointer);    // Yay!
    // uses_bar(b_pointer); // Aww...
}
// end-sample

int main ()
{
    return 0;
}

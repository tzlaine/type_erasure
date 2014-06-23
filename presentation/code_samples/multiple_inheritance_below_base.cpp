template <typename T>
void uses_foo (T * t)
{ t->foo(); }

template <typename T>
void uses_bar (T * t)
{ t->bar(); }

// sample(multiple_inheritance_below_base)
struct base { virtual ~base() {} };
struct has_foo : virtual base { virtual int foo () { return 42; } };
struct has_bar : virtual base { virtual float bar () { return 3.0f; } };
struct derived : has_foo, has_bar {};

void some_function ()
{
    base * b_pointer = new derived;
    uses_foo(dynamic_cast<has_foo*>(b_pointer)); // Wait, but...
    uses_bar(dynamic_cast<has_bar*>(b_pointer)); // Aww...
}
// end-sample

int main ()
{
    return 0;
}

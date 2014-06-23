struct has_foo;
void requires_foo (has_foo *) {}

// sample(example_of_polymorphism)
struct base { virtual ~base () {} };
struct has_foo : base { virtual int foo () { return 42; } };
struct derived : has_foo {};

void some_function ()
{
    base * b_pointer = new derived;
    requires_foo(static_cast<has_foo*>(b_pointer)); // <-- this here
}
// end-sample

int main ()
{
    return 0;
}

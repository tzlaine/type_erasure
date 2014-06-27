struct derived;
void requires_foo (derived *) {}

// sample(example_of_polymorphism)
struct base { virtual ~base () {} };
struct derived : base { virtual int foo () { return 42; } };

void some_function () {
    base * b_pointer = new derived;
    requires_foo(static_cast<derived*>(b_pointer)); // <-- this here
}
// end-sample

int main ()
{
    return 0;
}

struct foo_t {};
foo_t * foo_factory ()
{ return new foo_t; }
void foo_user (foo_t * f)
{}

// sample(pointer_factory_1)
foo_t * foo_factory ();

foo_t * foo = foo_factory();
// end-sample

// sample(pointer_factory_2)
foo_t * foo_factory ();
void foo_user (foo_t * f);

void some_function ()
{
    foo_t * foo = foo_factory();
    foo_user(foo);

    // What can I say about foo here?

    /* more code ... */
}
// end-sample

int main ()
{
    return 0;
}

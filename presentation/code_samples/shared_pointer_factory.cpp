#include <memory>

struct foo_t {};
std::shared_ptr<foo_t> foo_factory ()
{ return std::make_shared<foo_t>(); }

// sample(shared_pointer_factory)
std::shared_ptr<foo_t> foo_factory ();

std::shared_ptr<foo_t> foo = foo_factory();
// end-sample

int main ()
{
    return 0;
}

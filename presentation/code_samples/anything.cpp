#include "anything.hpp"

#include <string>

struct undefaultable
{
    undefaultable () = delete;
    explicit undefaultable (int i) {}
};

struct uncopyable
{
    uncopyable () = default;
    uncopyable (const uncopyable &) = delete;
};

struct unassignable
{
    unassignable () = default;
    unassignable & operator= (const uncopyable &) = delete;
};

int main ()
{
// sample(anything_in_action)
struct foo {};
anything a;

int i = 1; 
a = i;

a = 2.0;
a = std::string("3");
a = foo();

int * i_ptr = &i;
a = i_ptr;
// end-sample

a = undefaultable(1);
// does not compile: a = uncopyable();
a = unassignable();

    return 0;
}

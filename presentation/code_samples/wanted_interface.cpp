#define ANYTHING_WITH_A_VALUE 1
#include "anything.hpp"

typedef anything magic_type;

// sample(wanted_interface)
// No coupling!
// No virtual functions, kind of!
struct foo { int value () const; }; 
struct bar { int value () const; };

// No templates!
int value_of (magic_type obj)
{ return obj.value(); }

void some_function () {
    // Value semantics!
    if (value_of(foo()) == value_of(bar())) {
        /*...*/
    }
}
// end-sample

int foo::value () const { return 0; }
int bar::value () const { return 1; }

int main ()
{
    return 0;
}

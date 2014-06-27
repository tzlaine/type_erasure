#include "anything.hpp"

#include <array>
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
int i = 1; 
int * i_ptr = &i;

anything a;
a = i;
a = i_ptr;

a = 2.0;
a = std::string("3");

struct foo {};
a = foo();
// end-sample

a = undefaultable(1);
// does not compile: a = uncopyable();
a = unassignable();

{
// sample(type_erasure_with_sbo)
std::array<int, 1024> big_array;
anything small = 1;                 // No allocation to store an int.
anything ref = std::ref(big_array); // No allocation to store a std::ref().
anything large = big_array;         // Allocation required here.
// end-sample
}

{
// sample(type_erasure_with_sbo_and_cow)
std::array<int, 1024> big_array;
anything small = 1;                 // No allocation to store an int.
anything ref = std::ref(big_array); // No allocation to store a std::ref().
anything large = big_array;         // Allocation required here.
anything copied = large;            // No allocations for copies.
// end-sample
}

    return 0;
}

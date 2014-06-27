#include "loggable_erased_type.hpp"


struct my_loggable
{
    std::ostream & log (std::ostream & os) const
    { return os << "Yay, it works!\n"; }
};

int main ()
{
    loggable l = my_loggable();

    l.log(std::cout);

    return 0;
}

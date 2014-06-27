// sample(loggable_archetype)
#ifndef LOGGABLE_INTERFACE_INCLUDED__
#define LOGGABLE_INTERFACE_INCLUDED__

#include <iostream>


struct loggable
{
    std::ostream & log (std::ostream & os) const;
};

#endif
// end-sample

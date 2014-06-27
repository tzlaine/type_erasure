#include "disjoint_apis.hpp"


struct button
{
    void render () const {}
    layout_geometry geometry () const { return layout_geometry(); }
    void set_label () {}
    void set_image () {}
    // etc.
};

int main ()
{
// sample(type_erasure_with_cow)
widget w_1(widget{button()}); // Only 1 allocation here.
widget w_2 = w_1;             // No copy.
// end-sample

    return 0;
}

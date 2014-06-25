#include "disjoint_apis.hpp"

#include <copy_on_write.hpp>


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
// sample(type_erasure_plus_ref_wrapper)
button b;
widget ref_widget(std::ref(b));   // Underlying object
widget cref_widget(std::cref(b)); // not copied.
// end-sample

// sample(type_erasure_plus_cow_wrapper_usage)
copy_on_write<widget> w_1(widget{button()});
copy_on_write<widget> w_2 = w_1;    // No copy.
widget & mutable_w_2 = w_2.write(); // Copy happens here.
// end-sample

    return 0;
}

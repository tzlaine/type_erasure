#include "disjoint_apis.hpp"


// sample(disjoint_apis_in_use)
struct button
{
    void render () const;
    layout_geometry geometry () const;
    void set_label ();
    void set_image ();
    // etc.
};

void do_layout (layoutable l) {
    layout_geometry geometry = l.geometry();
    /* use geometry... */
}
void render_widget (widget w)
{ w.render(); }

void some_function () {
    button b;
    do_layout(b);
    render_widget(b);
}
// end-sample

void button::render () const
{}

layout_geometry button::geometry () const
{ return layout_geometry(); }

void button::set_label ()
{}

void button::set_image ()
{}

int main ()
{
    return 0;
}

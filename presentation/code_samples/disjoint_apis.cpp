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

void do_layout (layoutable l);
void render_widget (widget w);

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

void do_layout (layoutable l)
{ l.geometry(); }

void render_widget (widget w)
{ w.render(); }

int main ()
{
    return 0;
}

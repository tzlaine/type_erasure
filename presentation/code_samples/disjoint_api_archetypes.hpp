#include "layout_geometry.hpp"

struct widget
{
    void render () const;
};

struct layoutable
{
    layout_geometry geometry () const;
};

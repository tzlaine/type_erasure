namespace _1 {

// sample(different_heirarchies_1)
struct int_foo
{
    virtual int foo () { return 42; }
    virtual void log () const;
};
struct float_foo
{
    virtual float foo () { return 3.0f; }
    virtual void log () const;
};
// end-sample

// sample(different_heirarchies_2)
void log_to_terminal (const int_foo & loggee);
void log_to_terminal (const float_foo & loggee);
// end-sample

}

namespace _2 {

// sample(different_heirarchies_3)
struct log_base { virtual void log () const; };
struct int_foo : log_base {/*...*/}; 
struct float_foo : log_base {/*...*/}; 
void log_to_terminal (const log_base & loggee);
// end-sample

}

int main ()
{
    return 0;
}

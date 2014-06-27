#include <iostream>
#include <vector>


#ifndef INSTRUMENT_COPIES
#define INSTRUMENT_COPIES 1
#endif

#if INSTRUMENT_COPIES
inline std::size_t& allocations ()
{
    static std::size_t allocations_ = 0;
    return allocations_;
}

inline void reset_allocations ()
{ allocations() = 0; }

void * operator new (std::size_t size)
{
    ++allocations();
    return malloc(size);
}

void * operator new[] (std::size_t size)
{
    ++allocations();
    return malloc(size);
}
#endif

struct hi_printable
{
    void print () const
    { std::cout << "Hello, world!\n"; }
};

struct bye_printable
{
    void print () const
    { std::cout << "Bye, now!\n"; }
};

struct large_printable :
    std::vector<double>
{
    large_printable () :
        std::vector<double> (1024 * 1024)
    {}

    large_printable (const large_printable & rhs) :
        std::vector<double> (rhs)
    {}

    large_printable & operator= (const large_printable & rhs)
    {
        static_cast<std::vector<double> &>(*this) = rhs;
        return *this;
    }

    large_printable (large_printable && rhs) :
        std::vector<double> (std::move(static_cast<std::vector<double> &>(rhs)))
    {}

    large_printable & operator= (large_printable && rhs)
    {
        static_cast<std::vector<double> &>(*this) = std::move(rhs);
        return *this;
    }

    void print () const
    { std::cout << "I'm expensive to copy.\n"; }
};

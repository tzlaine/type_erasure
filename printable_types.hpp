#ifndef PRINTABLE_TYPES_INCLUDED__
#define PRINTABLE_TYPES_INCLUDED__

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

struct large_printable    
{
    large_printable () :
        data_ (1024 * 1024)
    {}

    large_printable (const large_printable & rhs) :
        data_ (rhs.data_)
    {}

    large_printable & operator= (const large_printable & rhs)
    {
        data_ = rhs.data_;
        return *this;
    }

    large_printable (large_printable && rhs) :
        data_ (std::move(rhs.data_))
    {}

    large_printable & operator= (large_printable && rhs)
    {
        data_ = std::move(rhs.data_);
        return *this;
    }

    void print () const
    { std::cout << "I'm expensive to copy.\n"; }

    std::vector<double> data_;
    double and_more_data_[1024];
};

#endif

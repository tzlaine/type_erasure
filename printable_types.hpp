#include <iostream>
#include <string>
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

inline void * operator new (std::size_t size)
{
    ++allocations();
    return malloc(size);
}

inline void * operator new[] (std::size_t size)
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
    std::vector<std::string>
{
    large_printable () :
        std::vector<std::string> (1000, std::string(1000, ' '))
    {}

    large_printable (const large_printable & rhs) :
        std::vector<std::string> (rhs)
    {}

    large_printable & operator= (const large_printable & rhs)
    {
        static_cast<std::vector<std::string> &>(*this) = rhs;
        return *this;
    }

    large_printable (large_printable && rhs) = default;
    large_printable & operator= (large_printable && rhs) = default;

    void print () const
    { std::cout << "I'm expensive to copy.\n"; }
};

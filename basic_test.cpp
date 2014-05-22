#include "hand_rolled.hpp"

#define ACCEPT_REFERENCE_WRAPPER 1
#define INSTRUMENT_COPIES 1


#define BOOST_TEST_MODULE TypeErasure

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(hand_rolled)
{
#define ECHO(expr)                                                      \
    do {                                                                \
        std::cout << #expr << ";\nap.print() = ";                       \
        expr;                                                           \
        ap.print();                                                     \
        std::cout << "allocations: " << allocations() << "\n\n";        \
        reset_allocations();                                            \
    } while (false)

    ECHO(hi_printable hi; any_printable ap(hi));
    ECHO(large_printable large; any_printable ap(large));
    ECHO(bye_printable bye; any_printable ap(bye));

    ECHO(hi_printable hi; any_printable ap = hi);
    ECHO(large_printable large; any_printable ap = large);
    ECHO(bye_printable bye; any_printable ap = bye);

    ECHO(hi_printable hi; any_printable tmp = hi; any_printable ap = tmp);
    ECHO(large_printable large; any_printable tmp = large; any_printable ap = tmp);
    ECHO(bye_printable bye; any_printable tmp = bye; any_printable ap = tmp);

    ECHO(hi_printable hi; any_printable ap; ap = hi);
    ECHO(large_printable large; any_printable ap; ap = large);
    ECHO(bye_printable bye; any_printable ap; ap = bye);

    ECHO(const hi_printable hi; any_printable ap(hi));
    ECHO(const large_printable large; any_printable ap(large));
    ECHO(const bye_printable bye; any_printable ap(bye));

    ECHO(const hi_printable hi; any_printable ap = hi);
    ECHO(const large_printable large; any_printable ap = large);
    ECHO(const bye_printable bye; any_printable ap = bye);

    ECHO(const hi_printable hi; any_printable tmp = hi; any_printable ap = tmp);
    ECHO(const large_printable large; any_printable tmp = large; any_printable ap = tmp);
    ECHO(const bye_printable bye; any_printable tmp = bye; any_printable ap = tmp);

    ECHO(const hi_printable hi; any_printable ap; ap = hi);
    ECHO(const large_printable large; any_printable ap; ap = large);
    ECHO(const bye_printable bye; any_printable ap; ap = bye);

    ECHO(any_printable ap(hi_printable{}));
    ECHO(any_printable ap(large_printable{}));
    ECHO(any_printable ap(bye_printable{}));

    ECHO(any_printable ap = hi_printable{});
    ECHO(any_printable ap = large_printable{});
    ECHO(any_printable ap = bye_printable{});

#if ACCEPT_REFERENCE_WRAPPER
    ECHO(hi_printable hi; any_printable ap(std::ref(hi)));
    ECHO(large_printable large; any_printable ap(std::ref(large)));
    ECHO(bye_printable bye; any_printable ap(std::ref(bye)));

    ECHO(hi_printable hi; any_printable ap(std::cref(hi)));
    ECHO(large_printable large; any_printable ap(std::cref(large)));
    ECHO(bye_printable bye; any_printable ap(std::cref(bye)));
#endif

#undef ECHO

    hi_printable hi;
    large_printable large;
    bye_printable bye;
    const hi_printable const_hi;
    const large_printable const_large;
    const bye_printable const_bye;
    std::vector<any_printable> several_printables = {
        hi,
        large,
        bye,
#if ACCEPT_REFERENCE_WRAPPER
        std::ref(hi),
        std::ref(large),
        std::cref(bye),
#endif
        const_hi,
        const_large,
        const_bye
    };

    for (const auto & printable : several_printables) {
        printable.print();
    }
}

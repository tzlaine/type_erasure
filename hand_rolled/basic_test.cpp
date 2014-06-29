#include <copy_on_write.hpp>

#include "hand_rolled.hpp"


#define BOOST_TEST_MODULE TypeErasure

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(hand_rolled)
{
#if INSTRUMENT_COPIES
    reset_allocations();
#endif

    std::cout << "sizeof(printable) = " << sizeof(printable) << "\n";

#define ECHO(expr)                                                      \
    do {                                                                \
        std::cout << #expr << ";\nap.print() = ";                       \
        expr;                                                           \
        ap.print();                                                     \
        std::cout << "allocations: " << allocations() << "\n\n";        \
        reset_allocations();                                            \
    } while (false)

    ECHO(hi_printable hi; printable ap(hi));
    ECHO(large_printable large; printable ap(large));
    ECHO(bye_printable bye; printable ap(bye));

    ECHO(hi_printable hi; printable ap = hi);
    ECHO(large_printable large; printable ap = large);
    ECHO(bye_printable bye; printable ap = bye);

    ECHO(hi_printable hi; printable tmp = hi; printable ap = tmp);
    ECHO(large_printable large; printable tmp = large; printable ap = tmp);
    ECHO(bye_printable bye; printable tmp = bye; printable ap = tmp);

    ECHO(hi_printable hi; printable ap; ap = hi);
    ECHO(large_printable large; printable ap; ap = large);
    ECHO(bye_printable bye; printable ap; ap = bye);

    ECHO(const hi_printable hi{}; printable ap(hi));
    ECHO(const large_printable large{}; printable ap(large));
    ECHO(const bye_printable bye{}; printable ap(bye));

    ECHO(const hi_printable hi{}; printable ap = hi);
    ECHO(const large_printable large{}; printable ap = large);
    ECHO(const bye_printable bye{}; printable ap = bye);

    ECHO(const hi_printable hi{}; printable tmp = hi; printable ap = tmp);
    ECHO(const large_printable large{}; printable tmp = large; printable ap = tmp);
    ECHO(const bye_printable bye{}; printable tmp = bye; printable ap = tmp);

    ECHO(const hi_printable hi{}; printable ap; ap = hi);
    ECHO(const large_printable large{}; printable ap; ap = large);
    ECHO(const bye_printable bye{}; printable ap; ap = bye);

    ECHO(printable ap(hi_printable{}));
    ECHO(printable ap(large_printable{}));
    ECHO(printable ap(bye_printable{}));

    ECHO(printable ap = hi_printable{});
    ECHO(printable ap = large_printable{});
    ECHO(printable ap = bye_printable{});

    ECHO(hi_printable hi; printable ap(std::ref(hi)));
    ECHO(large_printable large; printable ap(std::ref(large)));
    ECHO(bye_printable bye; printable ap(std::ref(bye)));

    ECHO(hi_printable hi; printable ap(std::cref(hi)));
    ECHO(large_printable large; printable ap(std::cref(large)));
    ECHO(bye_printable bye; printable ap(std::cref(bye)));

#undef ECHO
}

BOOST_AUTO_TEST_CASE(hand_rolled_vector)
{
    std::cout << "copied vector<printable>{hi_printable, large_printable}" << "\n";

    std::vector<printable> several_printables = {
        hi_printable{},
        large_printable{}
    };

    for (const auto & printable : several_printables) {
        printable.print();
    }

    std::vector<printable> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}

BOOST_AUTO_TEST_CASE(hand_rolled_vector_copy_on_write)
{
    std::cout << "copied vector<COW<printable>>{hi_printable, large_printable}" << "\n";

    std::vector<copy_on_write<printable>> several_printables = {
        {hi_printable{}},
        {large_printable{}}
    };

    for (const auto & printable : several_printables) {
        printable->print();
    }

    std::vector<copy_on_write<printable>> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}

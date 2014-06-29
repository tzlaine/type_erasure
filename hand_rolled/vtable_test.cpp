#include <copy_on_write.hpp>

#include "hand_rolled_vtable.hpp"


#define BOOST_TEST_MODULE TypeErasure

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(hand_rolled)
{
#if INSTRUMENT_COPIES
    reset_allocations();
#endif

    std::cout << "sizeof(printable_vtable) = " << sizeof(printable_vtable) << "\n";

#define ECHO(expr)                                                      \
    do {                                                                \
        std::cout << #expr << ";\nap.print() = ";                       \
        expr;                                                           \
        ap.print();                                                     \
        std::cout << "allocations: " << allocations() << "\n\n";        \
        reset_allocations();                                            \
    } while (false)

    ECHO(hi_printable hi; printable_vtable ap(hi));
    ECHO(large_printable large; printable_vtable ap(large));
    ECHO(bye_printable bye; printable_vtable ap(bye));

    ECHO(hi_printable hi; printable_vtable ap = hi);
    ECHO(large_printable large; printable_vtable ap = large);
    ECHO(bye_printable bye; printable_vtable ap = bye);

    ECHO(hi_printable hi; printable_vtable tmp = hi; printable_vtable ap = tmp);
    ECHO(large_printable large; printable_vtable tmp = large; printable_vtable ap = tmp);
    ECHO(bye_printable bye; printable_vtable tmp = bye; printable_vtable ap = tmp);

    ECHO(hi_printable hi; printable_vtable ap; ap = hi);
    ECHO(large_printable large; printable_vtable ap; ap = large);
    ECHO(bye_printable bye; printable_vtable ap; ap = bye);

    ECHO(const hi_printable hi{}; printable_vtable ap(hi));
    ECHO(const large_printable large{}; printable_vtable ap(large));
    ECHO(const bye_printable bye{}; printable_vtable ap(bye));

    ECHO(const hi_printable hi{}; printable_vtable ap = hi);
    ECHO(const large_printable large{}; printable_vtable ap = large);
    ECHO(const bye_printable bye{}; printable_vtable ap = bye);

    ECHO(const hi_printable hi{}; printable_vtable tmp = hi; printable_vtable ap = tmp);
    ECHO(const large_printable large{}; printable_vtable tmp = large; printable_vtable ap = tmp);
    ECHO(const bye_printable bye{}; printable_vtable tmp = bye; printable_vtable ap = tmp);

    ECHO(const hi_printable hi{}; printable_vtable ap; ap = hi);
    ECHO(const large_printable large{}; printable_vtable ap; ap = large);
    ECHO(const bye_printable bye{}; printable_vtable ap; ap = bye);

    ECHO(printable_vtable ap(hi_printable{}));
    ECHO(printable_vtable ap(large_printable{}));
    ECHO(printable_vtable ap(bye_printable{}));

    ECHO(printable_vtable ap = hi_printable{});
    ECHO(printable_vtable ap = large_printable{});
    ECHO(printable_vtable ap = bye_printable{});

    ECHO(hi_printable hi; printable_vtable ap(std::ref(hi)));
    ECHO(large_printable large; printable_vtable ap(std::ref(large)));
    ECHO(bye_printable bye; printable_vtable ap(std::ref(bye)));

    ECHO(hi_printable hi; printable_vtable ap(std::cref(hi)));
    ECHO(large_printable large; printable_vtable ap(std::cref(large)));
    ECHO(bye_printable bye; printable_vtable ap(std::cref(bye)));

#undef ECHO
}

BOOST_AUTO_TEST_CASE(hand_rolled_vector)
{
    std::cout << "copied vector<printable_vtable>{hi_printable, large_printable}" << "\n";

    std::vector<printable_vtable> several_printables = {
        hi_printable{},
        large_printable{}
    };

    for (const auto & printable : several_printables) {
        printable.print();
    }

    std::vector<printable_vtable> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}

BOOST_AUTO_TEST_CASE(hand_rolled_vector_copy_on_write)
{
    std::cout << "copied vector<COW<printable_vtable>>{hi_printable, large_printable}" << "\n";

    std::vector<copy_on_write<printable_vtable>> several_printables = {
        {hi_printable{}},
        {large_printable{}}
    };

    for (const auto & printable : several_printables) {
        printable->print();
    }

    std::vector<copy_on_write<printable_vtable>> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}

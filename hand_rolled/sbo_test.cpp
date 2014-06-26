#define ACCEPT_REFERENCE_WRAPPER 1

#include <copy_on_write.hpp>

#include "hand_rolled_sbo.hpp"


#define BOOST_TEST_MODULE TypeErasure

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(hand_rolled)
{
#if INSTRUMENT_COPIES
    reset_allocations();
#endif

    std::cout << "sizeof(any_printable) = " << sizeof(any_printable) << "\n";
#if 0 // TODO
    std::cout << "sizeof(any_printable_cow) = " << sizeof(any_printable) << "\n";
#endif

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

    ECHO(const hi_printable hi{}; any_printable ap(hi));
    ECHO(const large_printable large{}; any_printable ap(large));
    ECHO(const bye_printable bye{}; any_printable ap(bye));

    ECHO(const hi_printable hi{}; any_printable ap = hi);
    ECHO(const large_printable large{}; any_printable ap = large);
    ECHO(const bye_printable bye{}; any_printable ap = bye);

    ECHO(const hi_printable hi{}; any_printable tmp = hi; any_printable ap = tmp);
    ECHO(const large_printable large{}; any_printable tmp = large; any_printable ap = tmp);
    ECHO(const bye_printable bye{}; any_printable tmp = bye; any_printable ap = tmp);

    ECHO(const hi_printable hi{}; any_printable ap; ap = hi);
    ECHO(const large_printable large{}; any_printable ap; ap = large);
    ECHO(const bye_printable bye{}; any_printable ap; ap = bye);

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
}

BOOST_AUTO_TEST_CASE(hand_rolled_vector)
{
    std::cout << "copied vector<any_printable>{hi_printable, large_printable}" << "\n";

    std::vector<any_printable> several_printables = {
        hi_printable{},
        large_printable{}
    };

    for (const auto & printable : several_printables) {
        printable.print();
    }

    std::vector<any_printable> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}

BOOST_AUTO_TEST_CASE(hand_rolled_vector_copy_on_write)
{
    std::cout << "copied vector<COW<any_printable>>{hi_printable, large_printable}" << "\n";

    std::vector<copy_on_write<any_printable>> several_printables = {
        {hi_printable{}},
        {large_printable{}}
    };

    for (const auto & printable : several_printables) {
        printable->print();
    }

    std::vector<copy_on_write<any_printable>> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}

#if 0 // TODO
BOOST_AUTO_TEST_CASE(hand_rolled_cow_vector)
{
    std::cout << "copied vector<any_printable[COW]>{hi_printable, large_printable}" << "\n";

    std::vector<any_printable_cow> several_printables = {
        {hi_printable{}},
        {large_printable{}}
    };

    for (const auto & printable : several_printables) {
        printable.print();
    }

    std::vector<any_printable_cow> several_printables_copy = several_printables;

    std::cout << "allocations: " << allocations() << "\n\n";
    reset_allocations();
}
#endif

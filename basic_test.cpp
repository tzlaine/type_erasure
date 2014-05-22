#include "hand_rolled.hpp"

#include <vector>

#define ACCEPT_REFERENCE_WRAPPER 1


#define BOOST_TEST_MODULE TypeErasure

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(hand_rolled)
{
    hi_printable hi;
    bye_printable bye;

    const hi_printable const_hi{};
    const bye_printable const_bye{};

    any_printable ap_1(hi);
    ap_1.print(); // Prints Hello, world!

    any_printable ap_2(bye);
    ap_2.print(); // Prints Bye, now!

#if ACCEPT_REFERENCE_WRAPPER
    any_printable ap_3(std::ref(hi));
    ap_3.print(); // Prints Hello, world!

    any_printable ap_4(std::cref(bye));
    ap_4.print(); // Prints Bye, now!
#endif

    any_printable ap_5 = const_hi;
    ap_5.print(); // Prints Hello, world!

    any_printable ap_6 = const_bye;
    ap_6.print(); // Prints Bye, now!

    std::vector<any_printable> several_printables = {
        hi,
        bye,
#if ACCEPT_REFERENCE_WRAPPER
        std::ref(hi),
        std::cref(bye),
#endif
        const_hi,
        const_bye
    };

    std::cout << "\n\n********************\n\n";
    for (const auto & printable : several_printables) {
        printable.print();
    }
    std::cout << "\n********************\n";
}

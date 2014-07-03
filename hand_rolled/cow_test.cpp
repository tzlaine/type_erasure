#include <copy_on_write.hpp>

#include "hand_rolled_cow.hpp"


int main ()
{
#if INSTRUMENT_COPIES
    reset_allocations();
#endif

    std::cout << "sizeof(printable_cow) = " << sizeof(printable_cow) << "\n";

#define ECHO(expr)                                                      \
    do {                                                                \
        std::cout << #expr << ";\nap.print() = ";                       \
        expr;                                                           \
        ap.print();                                                     \
        std::cout << "allocations: " << allocations() << "\n\n";        \
        reset_allocations();                                            \
    } while (false)

    ECHO(hi_printable hi; printable_cow ap(hi));
    ECHO(large_printable large; printable_cow ap(large));
    ECHO(bye_printable bye; printable_cow ap(bye));

    ECHO(hi_printable hi; printable_cow ap = hi);
    ECHO(large_printable large; printable_cow ap = large);
    ECHO(bye_printable bye; printable_cow ap = bye);

    ECHO(hi_printable hi; printable_cow tmp = hi; printable_cow ap = tmp);
    ECHO(large_printable large; printable_cow tmp = large; printable_cow ap = tmp);
    ECHO(bye_printable bye; printable_cow tmp = bye; printable_cow ap = tmp);

    ECHO(hi_printable hi; printable_cow ap; ap = hi);
    ECHO(large_printable large; printable_cow ap; ap = large);
    ECHO(bye_printable bye; printable_cow ap; ap = bye);

    ECHO(const hi_printable hi{}; printable_cow ap(hi));
    ECHO(const large_printable large{}; printable_cow ap(large));
    ECHO(const bye_printable bye{}; printable_cow ap(bye));

    ECHO(const hi_printable hi{}; printable_cow ap = hi);
    ECHO(const large_printable large{}; printable_cow ap = large);
    ECHO(const bye_printable bye{}; printable_cow ap = bye);

    ECHO(const hi_printable hi{}; printable_cow tmp = hi; printable_cow ap = tmp);
    ECHO(const large_printable large{}; printable_cow tmp = large; printable_cow ap = tmp);
    ECHO(const bye_printable bye{}; printable_cow tmp = bye; printable_cow ap = tmp);

    ECHO(const hi_printable hi{}; printable_cow ap; ap = hi);
    ECHO(const large_printable large{}; printable_cow ap; ap = large);
    ECHO(const bye_printable bye{}; printable_cow ap; ap = bye);

    ECHO(printable_cow ap(hi_printable{}));
    ECHO(printable_cow ap(large_printable{}));
    ECHO(printable_cow ap(bye_printable{}));

    ECHO(printable_cow ap = hi_printable{});
    ECHO(printable_cow ap = large_printable{});
    ECHO(printable_cow ap = bye_printable{});

    ECHO(hi_printable hi; printable_cow ap(std::ref(hi)));
    ECHO(large_printable large; printable_cow ap(std::ref(large)));
    ECHO(bye_printable bye; printable_cow ap(std::ref(bye)));

    ECHO(hi_printable hi; printable_cow ap(std::cref(hi)));
    ECHO(large_printable large; printable_cow ap(std::cref(large)));
    ECHO(bye_printable bye; printable_cow ap(std::cref(bye)));

#undef ECHO

    {
        std::cout << "copied vector<printable_cow>{hi_printable, large_printable}" << "\n";

        std::vector<printable_cow> several_printables = {
            hi_printable{},
            large_printable{}
        };

        for (const auto & printable : several_printables) {
            printable.print();
        }

        std::vector<printable_cow> several_printables_copy = several_printables;

        std::cout << "allocations: " << allocations() << "\n\n";
        reset_allocations();
    }

    {
        std::cout << "copied vector<COW<printable_cow>>{hi_printable, large_printable}" << "\n";

        std::vector<copy_on_write<printable_cow>> several_printables = {
            {hi_printable{}},
            {large_printable{}}
        };

        for (const auto & printable : several_printables) {
            printable->print();
        }

        std::vector<copy_on_write<printable_cow>> several_printables_copy = several_printables;

        std::cout << "allocations: " << allocations() << "\n\n";
        reset_allocations();
    }

    return 0;
}

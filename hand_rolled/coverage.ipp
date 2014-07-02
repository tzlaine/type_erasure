// -*- C++ -*-
#include <copy_on_write.hpp>

namespace value {

    hi_printable small_rval ()
    { return hi_printable(); }

    const hi_printable small_const_rval ()
    { return hi_printable(); }

    hi_printable & small_lval ()
    {
        static hi_printable retval;
        return retval;
    }

    const hi_printable & small_const_lval ()
    {
        static const hi_printable retval = {};
        return retval;
    }

    large_printable large_rval ()
    { return large_printable(); }

    const large_printable large_const_rval ()
    { return large_printable(); }

    large_printable & large_lval ()
    {
        static large_printable retval;
        return retval;
    }

    const large_printable & large_const_lval ()
    {
        static const large_printable retval;
        return retval;
    }

}

namespace erased_value {

    erased_type small_rval ()
    { return erased_type(hi_printable()); }

    const erased_type small_const_rval ()
    { return erased_type(hi_printable()); }

    erased_type & small_lval ()
    {
        static erased_type retval{hi_printable()};
        return retval;
    }

    const erased_type & small_const_lval ()
    {
        static const erased_type retval{hi_printable()};
        return retval;
    }

    erased_type large_rval ()
    { return erased_type(large_printable()); }

    const erased_type large_const_rval ()
    { return erased_type(large_printable()); }

    erased_type & large_lval ()
    {
        static erased_type retval{large_printable()};
        return retval;
    }

    const erased_type & large_const_lval ()
    {
        static const erased_type retval{large_printable()};
        return retval;
    }

}

#define BOOST_TEST_MODULE TypeErasure

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(hand_rolled)
{
    value::small_lval();
    value::small_const_lval();
    value::large_lval();
    value::large_const_lval();

    erased_value::small_lval();
    erased_value::small_const_lval();
    erased_value::large_lval();
    erased_value::large_const_lval();

    reset_allocations();

    std::cout << "sizeof(erased_type) = " << sizeof(erased_type) << "\n";

    // CONSTRUCTION

    {
        hi_printable value_small_rval = value::small_rval();
        reset_allocations();
        erased_type value_small_rval_move(std::move(value_small_rval));
        const std::size_t value_small_rval_move_count = allocations();
        reset_allocations();

        const hi_printable value_small_const_rval = value::small_const_rval();
        reset_allocations();
        erased_type value_small_const_rval_copy(std::move(value_small_const_rval));
        const std::size_t value_small_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type value_small_lval_copy(value::small_lval());
        const std::size_t value_small_lval_copy_count = allocations();
        reset_allocations();

        erased_type value_small_const_lval_copy(value::small_const_lval());
        const std::size_t value_small_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(value_small_lval_copy_count, value_small_const_rval_copy_count);
        BOOST_CHECK_EQUAL(value_small_lval_copy_count, value_small_const_lval_copy_count);
        BOOST_CHECK(value_small_rval_move_count == value_small_lval_copy_count);
    }

    {
        large_printable value_large_rval = value::large_rval();
        reset_allocations();
        erased_type value_large_rval_move(std::move(value_large_rval));
        const std::size_t value_large_rval_move_count = allocations();
        reset_allocations();

        const large_printable value_large_const_rval = value::large_const_rval();
        reset_allocations();
        erased_type value_large_const_rval_copy(std::move(value_large_const_rval));
        const std::size_t value_large_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type value_large_lval_copy(value::large_lval());
        const std::size_t value_large_lval_copy_count = allocations();
        reset_allocations();

        erased_type value_large_const_lval_copy(value::large_const_lval());
        const std::size_t value_large_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(value_large_lval_copy_count, value_large_const_rval_copy_count);
        BOOST_CHECK_EQUAL(value_large_lval_copy_count, value_large_const_lval_copy_count);
        BOOST_CHECK(value_large_rval_move_count < value_large_lval_copy_count);
    }

    {
        erased_type erased_small_rval = erased_value::small_rval();
        reset_allocations();
        erased_type erased_small_rval_move(std::move(erased_small_rval));
        const std::size_t erased_small_rval_move_count = allocations();
        reset_allocations();

        const erased_type erased_small_const_rval = value::small_const_rval();
        reset_allocations();
        erased_type erased_small_const_rval_copy(std::move(erased_small_const_rval));
        const std::size_t erased_small_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type erased_small_lval_copy(erased_value::small_lval());
        const std::size_t erased_small_lval_copy_count = allocations();
        reset_allocations();

        erased_type erased_small_const_lval_copy(erased_value::small_const_lval());
        const std::size_t erased_small_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(erased_small_lval_copy_count, erased_small_const_rval_copy_count);
        BOOST_CHECK_EQUAL(erased_small_lval_copy_count, erased_small_const_lval_copy_count);
        BOOST_CHECK(erased_small_rval_move_count < erased_small_lval_copy_count);
    }

    {
        erased_type erased_large_rval = erased_value::large_rval();
        reset_allocations();
        erased_type erased_large_rval_move(std::move(erased_large_rval));
        const std::size_t erased_large_rval_move_count = allocations();
        reset_allocations();

        const erased_type erased_large_const_rval = erased_value::large_const_rval();
        reset_allocations();
        erased_type erased_large_const_rval_copy(std::move(erased_large_const_rval));
        const std::size_t erased_large_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type erased_large_lval_copy(erased_value::large_lval());
        const std::size_t erased_large_lval_copy_count = allocations();
        reset_allocations();

        erased_type erased_large_const_lval_copy(erased_value::large_const_lval());
        const std::size_t erased_large_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(erased_large_lval_copy_count, erased_large_const_rval_copy_count);
        BOOST_CHECK_EQUAL(erased_large_lval_copy_count, erased_large_const_lval_copy_count);
        BOOST_CHECK(erased_large_rval_move_count < erased_large_lval_copy_count);
    }


    // ASSIGNMENT

    {
        hi_printable value_small_rval = value::small_rval();
        reset_allocations();
        erased_type value_small_rval_move = std::move(value_small_rval);
        const std::size_t value_small_rval_move_count = allocations();
        reset_allocations();

        const hi_printable value_small_const_rval = value::small_const_rval();
        reset_allocations();
        erased_type value_small_const_rval_copy = std::move(value_small_const_rval);
        const std::size_t value_small_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type value_small_lval_copy = value::small_lval();
        const std::size_t value_small_lval_copy_count = allocations();
        reset_allocations();

        erased_type value_small_const_lval_copy = value::small_const_lval();
        const std::size_t value_small_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(value_small_lval_copy_count, value_small_const_rval_copy_count);
        BOOST_CHECK_EQUAL(value_small_lval_copy_count, value_small_const_lval_copy_count);
        BOOST_CHECK(value_small_rval_move_count == value_small_lval_copy_count);
    }

    {
        large_printable value_large_rval = value::large_rval();
        reset_allocations();
        erased_type value_large_rval_move = std::move(value_large_rval);
        const std::size_t value_large_rval_move_count = allocations();
        reset_allocations();

        const large_printable value_large_const_rval = value::large_const_rval();
        reset_allocations();
        erased_type value_large_const_rval_copy = std::move(value_large_const_rval);
        const std::size_t value_large_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type value_large_lval_copy = value::large_lval();
        const std::size_t value_large_lval_copy_count = allocations();
        reset_allocations();

        erased_type value_large_const_lval_copy = value::large_const_lval();
        const std::size_t value_large_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(value_large_lval_copy_count, value_large_const_rval_copy_count);
        BOOST_CHECK_EQUAL(value_large_lval_copy_count, value_large_const_lval_copy_count);
        BOOST_CHECK(value_large_rval_move_count < value_large_lval_copy_count);
    }

    {
        erased_type erased_small_rval = erased_value::small_rval();
        reset_allocations();
        erased_type erased_small_rval_move = std::move(erased_small_rval);
        const std::size_t erased_small_rval_move_count = allocations();
        reset_allocations();

        const erased_type erased_small_const_rval = value::small_const_rval();
        reset_allocations();
        erased_type erased_small_const_rval_copy = std::move(erased_small_const_rval);
        const std::size_t erased_small_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type erased_small_lval_copy = erased_value::small_lval();
        const std::size_t erased_small_lval_copy_count = allocations();
        reset_allocations();

        erased_type erased_small_const_lval_copy = erased_value::small_const_lval();
        const std::size_t erased_small_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(erased_small_lval_copy_count, erased_small_const_rval_copy_count);
        BOOST_CHECK_EQUAL(erased_small_lval_copy_count, erased_small_const_lval_copy_count);
        BOOST_CHECK(erased_small_rval_move_count < erased_small_lval_copy_count);
    }

    {
        erased_type erased_large_rval = erased_value::large_rval();
        reset_allocations();
        erased_type erased_large_rval_move = std::move(erased_large_rval);
        const std::size_t erased_large_rval_move_count = allocations();
        reset_allocations();

        const erased_type erased_large_const_rval = erased_value::large_const_rval();
        reset_allocations();
        erased_type erased_large_const_rval_copy = std::move(erased_large_const_rval);
        const std::size_t erased_large_const_rval_copy_count = allocations();
        reset_allocations();

        erased_type erased_large_lval_copy = erased_value::large_lval();
        const std::size_t erased_large_lval_copy_count = allocations();
        reset_allocations();

        erased_type erased_large_const_lval_copy = erased_value::large_const_lval();
        const std::size_t erased_large_const_lval_copy_count = allocations();
        reset_allocations();

        BOOST_CHECK_EQUAL(erased_large_lval_copy_count, erased_large_const_rval_copy_count);
        BOOST_CHECK_EQUAL(erased_large_lval_copy_count, erased_large_const_lval_copy_count);
        BOOST_CHECK(erased_large_rval_move_count < erased_large_lval_copy_count);
    }

    // SMALL VS. LARGE PERMUTATIONS

    {
        erased_type erased_large = erased_value::large_rval();
        erased_type erased_small = erased_value::small_rval();
        erased_small = std::move(erased_large);
    }

    {
        erased_type erased_large = erased_value::large_rval();
        erased_type erased_small = erased_value::small_rval();
        erased_large = std::move(erased_small);
    }

    {
        erased_type erased_small_1 = erased_value::small_rval();
        erased_type erased_small_2 = erased_value::small_rval();
        erased_small_1 = std::move(erased_small_2);
    }

    {
        erased_type erased_large_1 = erased_value::large_rval();
        erased_type erased_large_2 = erased_value::large_rval();
        erased_large_1 = std::move(erased_large_2);
    }
}

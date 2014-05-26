#include "small_buffer.hpp"

#define BOOST_TEST_MODULE SmallBuffer

#include <boost/test/included/unit_test.hpp>

struct large
{
    double a;
    double b;

    friend inline bool operator== (const large & lhs, const large & rhs)
    { return lhs.a == rhs.a && lhs.b == rhs.b; }

    friend inline std::ostream & operator<< (std::ostream & os, large l)
    { return os << '{' << l.a << ", " << l.b << '}'; }
};

BOOST_AUTO_TEST_CASE(small_buffer_small_int)
{
    small_buffer<int> sb_int;
    BOOST_CHECK_EQUAL(sb_int.value(), 0);

    small_buffer<int> sb_int_3(3);
    BOOST_CHECK_EQUAL(sb_int_3.value(), 3);

    small_buffer<int> sb_int_3_copy(sb_int_3);
    BOOST_CHECK_EQUAL(sb_int_3_copy.value(), 3);

    small_buffer<int> sb_int_5_move(small_buffer<int>(5));
    BOOST_CHECK_EQUAL(sb_int_5_move.value(), 5);

    sb_int = sb_int_3;
    BOOST_CHECK_EQUAL(sb_int.value(), 3);

    sb_int = small_buffer<int>(7);
    BOOST_CHECK_EQUAL(sb_int.value(), 7);
}

BOOST_AUTO_TEST_CASE(small_buffer_small_int_pointer)
{
    using buffer_type = small_buffer<std::shared_ptr<int>, 16>;

    buffer_type sb_int_ptr;
    BOOST_CHECK_EQUAL(sb_int_ptr.value(), std::shared_ptr<int>());

    buffer_type sb_int_ptr_3(std::make_shared<int>(3));
    BOOST_CHECK_EQUAL(*sb_int_ptr_3.value(), 3);

    buffer_type sb_int_ptr_3_copy(sb_int_ptr_3);
    BOOST_CHECK_EQUAL(*sb_int_ptr_3_copy.value(), 3);

    buffer_type sb_int_ptr_5_move(buffer_type(std::make_shared<int>(5)));
    BOOST_CHECK_EQUAL(*sb_int_ptr_5_move.value(), 5);

    sb_int_ptr = sb_int_ptr_3;
    BOOST_CHECK_EQUAL(*sb_int_ptr.value(), 3);

    sb_int_ptr = buffer_type(std::make_shared<int>(7));
    BOOST_CHECK_EQUAL(*sb_int_ptr.value(), 7);
}

BOOST_AUTO_TEST_CASE(small_buffer_large_array)
{
    small_buffer<large> sb_large;
    BOOST_CHECK_EQUAL(sb_large.value(), large{});

    small_buffer<large> sb_large_1_3({1.0, 3.0});
    BOOST_CHECK_EQUAL(sb_large_1_3.value(), (large{1.0, 3.0}));

    small_buffer<large> sb_large_1_3_copy(sb_large_1_3);
    BOOST_CHECK_EQUAL(sb_large_1_3_copy.value(), (large{1.0, 3.0}));

    small_buffer<large> sb_large_1_5_move(small_buffer<large>({1.0, 5.0}));
    BOOST_CHECK_EQUAL(sb_large_1_5_move.value(), (large{1.0, 5.0}));

    sb_large = sb_large_1_3;
    BOOST_CHECK_EQUAL(sb_large.value(), (large{1.0, 3.0}));

    sb_large = small_buffer<large>({1.0, 7.0});
    BOOST_CHECK_EQUAL(sb_large.value(), (large{1.0, 7.0}));
}

BOOST_AUTO_TEST_CASE(small_buffer_large_int_pointer)
{
    using buffer_type = small_buffer<std::shared_ptr<int>>;

    buffer_type sb_int_ptr;
    BOOST_CHECK_EQUAL(sb_int_ptr.value(), std::shared_ptr<int>());

    buffer_type sb_int_ptr_3(std::make_shared<int>(3));
    BOOST_CHECK_EQUAL(*sb_int_ptr_3.value(), 3);

    buffer_type sb_int_ptr_3_copy(sb_int_ptr_3);
    BOOST_CHECK_EQUAL(*sb_int_ptr_3_copy.value(), 3);

    buffer_type sb_int_ptr_5_move(buffer_type(std::make_shared<int>(5)));
    BOOST_CHECK_EQUAL(*sb_int_ptr_5_move.value(), 5);

    sb_int_ptr = sb_int_ptr_3;
    BOOST_CHECK_EQUAL(*sb_int_ptr.value(), 3);

    sb_int_ptr = buffer_type(std::make_shared<int>(7));
    BOOST_CHECK_EQUAL(*sb_int_ptr.value(), 7);
}

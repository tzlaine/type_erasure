#include <type_traits>


// sample(compile_time_return_type)
template <typename TrueType, typename FalseType, bool Selection>
typename std::conditional<Selection, TrueType, FalseType>::type
factory_function () {
    return
        typename std::conditional<Selection, TrueType, FalseType>::type();
}

int int_1 = factory_function<int, float, true>();
float float_1 = factory_function<int, float, false>();
// end-sample

int main ()
{
    static_assert(std::is_same<decltype(int_1), int>::value, "");
    static_assert(std::is_same<decltype(float_1), float>::value, "");

    return 0;
}

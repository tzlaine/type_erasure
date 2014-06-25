#include <type_traits>


// sample(compile_time_return_type)
template <typename TrueType, typename FalseType, bool Selection>
typename std::conditional<Selection, TrueType, FalseType>::type
factory_function () {
    return
        typename std::conditional<Selection, TrueType, FalseType>::type();
}

int an_int = factory_function<int, float, true>();
float a_float = factory_function<int, float, false>();
// end-sample

int main ()
{
    static_assert(std::is_same<decltype(an_int), int>::value, "");
    static_assert(std::is_same<decltype(a_float), float>::value, "");

    return 0;
}

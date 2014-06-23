struct some_t {};
struct base_t {};

// sample(nonpolymorphic_predicate)
bool my_predicate (some_t obj);
// end-sample

// sample(runtime_polymorphic_predicate)
bool my_predicate (const base_t & obj);
// end-sample

// sample(compile_time_polymorphic_predicate)
template <typename T>
bool my_predicate (T obj);
// end-sample

int main ()
{
    return 0;
}

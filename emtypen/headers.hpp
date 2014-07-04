#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#if defined(_MSC_VER) && _MSC_VER == 1800
#define noexcept
#define alignof __alignof
#endif

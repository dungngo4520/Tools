#pragma once
#include <memory>
namespace utils {
    namespace memory {
        // clang-format off
        #ifndef defer
        struct defer_dummy {};
        template <class F> struct deferrer { F f; ~deferrer() { f(); } };
        template <class F> deferrer<F> operator*(utils::memory::defer_dummy, F f) { return {f}; }
        #define DEFER_(LINE) zz_defer##LINE
        #define DEFER(LINE) DEFER_(LINE)
        #define defer auto DEFER(__LINE__) = utils::memory::defer_dummy{} *[&]()
        #endif // defer
        // clang-format on

    };  // namespace memory
};      // namespace utils

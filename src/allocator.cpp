#include <ccl/allocator.hpp>

namespace ccl {
    static allocator *s_allocator = nullptr;

    void CCLAPI set_default_allocator(allocator &allocator) {
        s_allocator = &allocator;
    }

    allocator * CCLAPI get_default_allocator() {
        return s_allocator;
    }
}

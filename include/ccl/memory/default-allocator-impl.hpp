namespace ccl {
    static allocator *s_allocator = nullptr;

    void CCL_ALLOCAPI set_default_allocator(allocator &allocator) {
        s_allocator = &allocator;
    }

    allocator * CCL_ALLOCAPI get_default_allocator() {
        return s_allocator;
    }
}

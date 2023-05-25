#ifndef CCL_HANDLE_MANAGER_HPP
#define CCL_HANDLE_MANAGER_HPP

#include <ccl/api.hpp>
#include <ccl/hash.hpp>
#include <ccl/handle.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    template<typename ObjectType, typed_allocator<ObjectType> Allocator>
    class handle_manager {
        public:
            using object_type = ObjectType;
            using handle_type = versioned_handle<object_type>;
            using handle_ptr = handle_type*;
            using allocator_type = Allocator;
            using vector_type = paged_vector<handle_type, handle_ptr, allocator_type>;

        private:
            vector_type handles;

            handle_type acquire();
            void release(const handle_t handle);
    };
}

#endif // CCL_HANDLE_MANAGER_HPP

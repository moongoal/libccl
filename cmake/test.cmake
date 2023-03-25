include_guard()

function(add_ccl_test test_name test_file_path)
    add_executable(${test_name} ${test_file_path})
    target_link_libraries(${test_name} ccl)
    target_compile_definitions(${test_name} PRIVATE CCL_ALLOCATOR_IMPL)

    set_target_properties(
        ${test_name}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY test
    )

    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
    )

    if($CACHE{CCL_COVERAGE})
        set_tests_properties(
            ${test_name}
            PROPERTIES
                ENVIRONMENT
                    LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/test/${test_name}.profraw
        )
    endif()

    target_compile_options(
        ${test_name}
        PRIVATE
            -Wall -Wextra -pedantic -Werror
            $<$<BOOL:$CACHE{CCL_COVERAGE}>:-fprofile-instr-generate -fcoverage-mapping>
    )
endfunction(add_ccl_test)

add_ccl_test(test_self_test test/test/test.cpp)
add_ccl_test(test_util test/util.cpp)
add_ccl_test(test_vector test/vector.cpp)
add_ccl_test(test_compressed_pair test/compressed-pair.cpp)
add_ccl_test(test_bitset test/bitset.cpp)
add_ccl_test(test_maybe test/maybe.cpp)
add_ccl_test(test_hashtable test/hashtable.cpp)
add_ccl_test(test_tables_table test/tables/table.cpp)
add_ccl_test(test_tables_view test/tables/view.cpp)
add_ccl_test(test_set test/set.cpp)
add_ccl_test(test_local_allocator test/memory/local-allocator.cpp)
add_ccl_test(test_composite_allocator test/memory/composite-allocator.cpp)
add_ccl_test(test_sparse_set test/sparse-set.cpp)
add_ccl_test(test_hash test/hash.cpp)
add_ccl_test(test_dense_map test/dense-map.cpp)
add_ccl_test(test_type_traits test/type-traits.cpp)
add_ccl_test(test_paged_vector test/paged-vector.cpp)
add_ccl_test(test_packed_integer test/packed-integer.cpp)
add_ccl_test(test_handle test/handle.cpp)
add_ccl_test(test_ecs_component test/ecs/component.cpp)
add_ccl_test(test_ecs_archetype test/ecs/archetype.cpp)
add_ccl_test(test_ecs_registry test/ecs/registry.cpp)
add_ccl_test(test_ecs_view test/ecs/view.cpp)
add_ccl_test(test_tagged_pointer test/tagged-pointer.cpp)
add_ccl_test(test_pair test/pair.cpp)
add_ccl_test(test_atomic test/atomic.cpp)
add_ccl_test(test_atomic_flag test/atomic-flag.cpp)
add_ccl_test(test_compat test/compat.cpp)
add_ccl_test(test_deque_begin_policy test/deque-begin-policy.cpp)
add_ccl_test(test_deque_center_policy test/deque-center-policy.cpp)

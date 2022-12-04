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

    target_compile_options(
        ${test_name}
        PRIVATE
            -Wall -Wextra -pedantic -Werror
            $<$<BOOL:${CCL_FEATURE_SANITIZE_MEMORY}>:-fsanitize=memory>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fsanitize=address>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fno-omit-frame-pointer>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fsanitize-recover=address>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_UNDEFINED_BEHAVIOR}>:-fsanitize=undefined>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_STACK}>:-fsanitize=safe-stack>
    )

    target_link_options(
        ${test_name}
        PRIVATE
            $<$<BOOL:${CCL_FEATURE_SANITIZE_MEMORY}>:-fsanitize=memory>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fsanitize=address>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_UNDEFINED_BEHAVIOR}>:-fsanitize=undefined>
            $<$<BOOL:${CCL_FEATURE_SANITIZE_STACK}>:-fsanitize=safe-stack>
    )
endfunction(add_ccl_test)

add_ccl_test(test_self_test test/test.cpp)
add_ccl_test(test_util test/util.cpp)
add_ccl_test(test_vector test/vector.cpp)
add_ccl_test(test_compressed_pair test/compressed-pair.cpp)
add_ccl_test(test_bitset test/bitset.cpp)
add_ccl_test(test_maybe test/maybe.cpp)
add_ccl_test(test_hashtable test/hashtable.cpp)
add_ccl_test(test_tables_table test/tables/table.cpp)
add_ccl_test(test_tables_view test/tables/view.cpp)
add_ccl_test(test_set test/set.cpp)
add_ccl_test(test_local_allocator test/local-allocator.cpp)
add_ccl_test(test_composite_allocator test/composite-allocator.cpp)
add_ccl_test(test_sparse_set test/sparse-set.cpp)

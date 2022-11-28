include_guard()

function(add_ccl_test test_name test_file_path)
    add_executable(${test_name} ${test_file_path})
    target_link_libraries(${test_name} ccl)
    set_target_properties(
        ${test_name}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY test
    )
    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
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

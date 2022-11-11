include_guard()

function(add_ccl_test test_name test_file_path)
    add_executable(${test_name} ${test_file_path})
    target_link_libraries(${test_name} charcoal)
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

include_guard()

add_executable(test_vector test/vector.cpp)
target_link_libraries(test_vector libcharcoal)
set_target_properties(
    test_vector
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY test
)
add_test(
    NAME test_vector
    COMMAND test_vector
)

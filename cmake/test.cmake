include_guard()

function(_add_ccl_test test_name test_file_path profraw_file exe_file)
    add_executable(${test_name} ${test_file_path})
    target_link_libraries(${test_name} ccl)
    target_compile_definitions(${test_name} PRIVATE CCL_ALLOCATOR_IMPL)

    target_link_options(
        ${test_name}
        PRIVATE -fprofile-instr-generate
    )

    set_target_properties(
        ${test_name}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY test
    )

    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
    )

    if(CCL_COVERAGE)
        set_tests_properties(
            ${test_name}
            PROPERTIES
                ENVIRONMENT
                    LLVM_PROFILE_FILE=${profraw_file}
        )
    endif()

    target_compile_options(
        ${test_name}
        PRIVATE
            -Wall -Wextra -pedantic -Werror
            $<$<BOOL:$CCL_COVERAGE>:-fprofile-instr-generate -fcoverage-mapping>
    )
endfunction()

function(_add_ccl_coverage_report exe_files profraw_files profdata_file coverage_report_file coverage_sources)
    set(i 0)
    list(LENGTH exe_files exe_files_length)

    while(i LESS exe_files_length)
        list(GET exe_files ${i} exe_file)
        list(GET profraw_files ${i} profraw_file)

        add_custom_command(
            OUTPUT ${profraw_file}
            COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${profraw_file} ${exe_file}
            DEPENDS ${exe_file}
        )

        math(EXPR i "${i} + 1")
    endwhile()

    add_custom_command(
        OUTPUT ${profdata_file}
        COMMAND llvm-profdata merge ${profraw_files} -o ${profdata_file}
        DEPENDS ${profraw_files}
    )

    foreach(src ${coverage_sources})
        set(llvm_cov_sources_args ${llvm_cov_sources_args} -sources ${CMAKE_SOURCE_DIR}/${src})
    endforeach()

    foreach(exe ${exe_files})
        set(llvm_cov_object_args ${llvm_cov_object_args} -object ${exe})
    endforeach()

    add_custom_command(
        OUTPUT ${coverage_report_file}
        DEPENDS ${profdata_file}
        COMMAND llvm-cov show --format=html --project-title=ccl --instr-profile ${profdata_file} ${llvm_cov_object_args} ${llvm_cov_sources_args} > ${coverage_report_file}
    )
endfunction()

#
# SYNOPSIS
#
# add_ccl_test(
#   [REPORT report_name]
#   (TEST test_name test_source)...
#   COVERAGE covered_source_files...
# )
#
# Add the given tests and related coverage report. The coverage results of all the specified
# tests are collated together and named after either `report_name` or the first test name if the
# former is not provided.
#
function(add_ccl_test)
    set(arg_single_value_keywords REPORT)
    set(arg_multi_value_keywords TEST COVERAGE)
    cmake_parse_arguments(PARSE_ARGV 0 ADD_CCL_TEST "" "${arg_single_value_keywords}" "${arg_multi_value_keywords}")

    if(DEFINED ADD_CCL_TEST_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid arguments in function call \"${ADD_CCL_TEST_UNPARSED_ARGUMENTS}\".")
    endif()

    if(NOT DEFINED ADD_CCL_TEST_TEST)
        message(FATAL_ERROR "At least one TEST is required.")
    endif()

    set(all_tests ${ADD_CCL_TEST_TEST})

    while(all_tests)
        list(POP_FRONT all_tests test_name test_file_path)
        set(profraw_file ${CCL_COVERAGE_DIR}/raw/${test_name}.profraw)
        set(exe_file ${CMAKE_CURRENT_BINARY_DIR}/test/${test_name}.exe)

        _add_ccl_test(${test_name} ${test_file_path} ${profraw_file} ${exe_file})

        if(DEFINED CCL_TEST_EXECUTABLES)
            set(CCL_TEST_EXECUTABLES ${CCL_TEST_EXECUTABLES};${exe_file} PARENT_SCOPE)
            set(CCL_TEST_SOURCES ${CCL_TEST_SOURCES};${test_file_path} PARENT_SCOPE)
        else()
            set(CCL_TEST_EXECUTABLES ${exe_file} PARENT_SCOPE)
            set(CCL_TEST_SOURCES ${test_file_path} PARENT_SCOPE)
        endif()

        set(CCL_COVERAGE_RAW_DATA_FILES ${CCL_COVERAGE_RAW_DATA_FILES} ${profraw_file} PARENT_SCOPE)

        list(APPEND test_exe_files ${exe_file})
        list(APPEND test_profraw_files ${profraw_file})
    endwhile()

    if(NOT DEFINED ADD_CCL_TEST_REPORT)
        list(GET ADD_CCL_TEST_TEST 0 report_name)
    else()
        set(report_name ${ADD_CCL_TEST_REPORT})
    endif()

    if(DEFINED ADD_CCL_TEST_COVERAGE)
        set(profdata_file ${CCL_COVERAGE_DIR}/${report_name}.profdata)
        set(coverage_report_file ${CCL_COVERAGE_DIR}/${report_name}.cov.html)
        set(CCL_COVERAGE_REPORT_FILES ${CCL_COVERAGE_REPORT_FILES} ${coverage_report_file} PARENT_SCOPE)

        _add_ccl_coverage_report("${test_exe_files}" "${test_profraw_files}" "${profdata_file}" "${coverage_report_file}" "${ADD_CCL_TEST_COVERAGE}")
    endif()
endfunction(add_ccl_test)

set(CCL_COVERAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/coverage)
set(CCL_COVERAGE_DATA_FILE ${CCL_COVERAGE_DIR}/ccl.profdata)

file(MAKE_DIRECTORY ${CCL_COVERAGE_DIR}/raw)

add_ccl_test(
    TEST test_self_test test/test/test.cpp
    COVERAGE include/ccl/test/test.hpp
)

add_ccl_test(
    TEST test_util test/util.cpp
    COVERAGE include/ccl/util.hpp
)

add_ccl_test(
    TEST test_vector test/vector.cpp
    COVERAGE include/ccl/vector.hpp
)

add_ccl_test(
    TEST test_compressed_pair test/compressed-pair.cpp
    COVERAGE include/ccl/compressed-pair.hpp
)

add_ccl_test(
    TEST test_bitset test/bitset.cpp
    COVERAGE include/ccl/bitset.hpp
)

add_ccl_test(
    TEST test_maybe test/maybe.cpp
    COVERAGE include/ccl/maybe.hpp
)

add_ccl_test(
    REPORT test_hashtable
    TEST test_hashtable test/hashtable.cpp
    TEST test_hashtable_iterator test/hashtable-iterator.cpp
    COVERAGE include/ccl/hashtable.hpp
)

add_ccl_test(
    TEST test_tables_table test/tables/table.cpp
    COVERAGE include/ccl/tables/table.hpp
)

add_ccl_test(
    TEST test_tables_view test/tables/view.cpp
    COVERAGE include/ccl/tables/view.hpp
)

add_ccl_test(
    TEST test_set test/set.cpp
    TEST test_set_iterator test/set-iterator.cpp
    COVERAGE include/ccl/set.hpp
)

add_ccl_test(
    TEST test_local_allocator test/memory/local-allocator.cpp
    COVERAGE include/ccl/memory/local-allocator.hpp
)

add_ccl_test(
    TEST test_composite_allocator test/memory/composite-allocator.cpp
    COVERAGE include/ccl/memory/composite-allocator.hpp
)

add_ccl_test(
    TEST test_sparse_set test/sparse-set.cpp
    COVERAGE include/ccl/sparse-set.hpp
)

add_ccl_test(
    TEST test_hash test/hash.cpp
    COVERAGE include/ccl/hash.hpp
)

add_ccl_test(
    TEST test_dense_map test/dense-map.cpp
    COVERAGE include/ccl/dense-map.hpp
)

add_ccl_test(
    TEST test_type_traits test/type-traits.cpp
    COVERAGE include/ccl/type-traits.hpp
)

add_ccl_test(
    TEST test_paged_vector test/paged-vector.cpp
    COVERAGE include/ccl/paged-vector.hpp
)

add_ccl_test(
    TEST test_packed_integer test/packed-integer.cpp
    COVERAGE include/ccl/packed-integer.hpp
)

add_ccl_test(
    TEST test_versioned_handle test/handle/versioned-handle.cpp
    COVERAGE include/ccl/handle/versioned-handle.hpp
)

add_ccl_test(
    TEST test_typed_handle test/handle/typed-handle.cpp
    COVERAGE include/ccl/handle/typed-handle.hpp
)

add_ccl_test(
    TEST test_handle_manager test/handle-manager.cpp
    COVERAGE include/ccl/handle-manager.hpp
)

add_ccl_test(
    TEST test_pool test/pool.cpp
    COVERAGE include/ccl/pool.hpp
)

add_ccl_test(
    TEST test_dependent_pool test/dependent-pool.cpp
    COVERAGE include/ccl/dependent-pool.hpp
)

add_ccl_test(
    TEST test_ecs_component test/ecs/component.cpp
    COVERAGE include/ccl/ecs/component.hpp
)

add_ccl_test(
    TEST test_ecs_archetype test/ecs/archetype.cpp
    COVERAGE include/ccl/ecs/archetype.hpp
)

add_ccl_test(
    TEST test_ecs_registry test/ecs/registry.cpp
    COVERAGE include/ccl/ecs/registry.hpp
)

add_ccl_test(
    TEST test_ecs_view test/ecs/view.cpp
    COVERAGE include/ccl/ecs/view.hpp
)

add_ccl_test(
    TEST test_tagged_pointer test/tagged-pointer.cpp
    COVERAGE include/ccl/tagged-pointer.hpp
)

add_ccl_test(
    TEST test_pair test/pair.cpp
    COVERAGE include/ccl/pair.hpp
)

add_ccl_test(
    TEST test_compat test/compat.cpp
    COVERAGE include/ccl/compat.hpp
)

add_ccl_test(
    TEST test_contiguous_iterator test/contiguous-iterator.cpp
    COVERAGE include/ccl/contiguous-iterator.hpp
)

add_ccl_test(
    TEST test_atomic test/atomic.cpp
    TEST test_atomic_flag test/atomic-flag.cpp
    COVERAGE include/ccl/atomic.hpp
)

add_ccl_test(
    REPORT test_deque
    TEST test_deque_begin_policy test/deque-begin-policy.cpp
    TEST test_deque_center_policy test/deque-center-policy.cpp
    COVERAGE include/ccl/deque.hpp
)

add_ccl_test(
    TEST test_memory_pool test/memory-pool.cpp
    COVERAGE include/ccl/memory-pool.hpp
)

add_ccl_test(
    TEST test_internal_optional_allocator test/internal/optional-allocator.cpp
    COVERAGE include/ccl/internal/optional-allocator.hpp
)

add_ccl_test(
    TEST test_pointer_shared test/pointer/shared.cpp
    COVERAGE include/ccl/pointer/shared.hpp
)

add_ccl_test(
    REPORT test_allocator
    TEST test_default_allocator test/memory/default-allocator.cpp
    TEST test_null_allocator test/memory/null-allocator.cpp
    COVERAGE include/ccl/memory/allocator.hpp include/ccl/memory/default-allocator-impl.hpp
)

add_ccl_test(
    TEST test_i18n_language test/i18n/language.cpp
)

add_ccl_test(
    TEST test_string_basic_string test/string/basic-string.cpp
    COVERAGE include/ccl/string/basic-string.hpp
)

add_ccl_test(
    TEST test_string_builder test/string/builder.cpp
    COVERAGE include/ccl/string/builder.hpp
)

add_ccl_test(
    TEST test_string_nul_terminated test/string/nul-terminated.cpp
    COVERAGE include/ccl/string/nul-terminated.hpp
)

add_ccl_test(
    TEST test_concurrent_channel test/concurrent/channel.cpp
    COVERAGE include/ccl/concurrent/channel.hpp
)

add_custom_command(
    OUTPUT ${CCL_COVERAGE_DATA_FILE}
    COMMAND llvm-profdata merge ${CCL_COVERAGE_RAW_DATA_FILES} -o ${CCL_COVERAGE_DATA_FILE}
    DEPENDS ${CCL_COVERAGE_RAW_DATA_FILES} ${CCL_TEST_SOURCES}
)

foreach(f ${CCL_TEST_EXECUTABLES})
    set(CCL_COVERAGE_REPORT_SOURCE_ARGS ${CCL_COVERAGE_REPORT_SOURCE_ARGS} -object ${f})
endforeach()

add_custom_target(
    coverage-summary
    DEPENDS ${CCL_COVERAGE_DATA_FILE} ${CCL_TEST_SOURCES}
    COMMAND llvm-cov report -ignore-filename-regex="\\.cpp$$" --instr-profile ${CCL_COVERAGE_DATA_FILE} ${CCL_COVERAGE_REPORT_SOURCE_ARGS}
)

add_custom_target(
    coverage-report
    DEPENDS ${CCL_COVERAGE_REPORT_FILES} ${CCL_TEST_SOURCES}
)

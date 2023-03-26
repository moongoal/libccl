include_guard()

function(add_ccl_test test_name test_file_path main_tested_source_file)
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

    set(profraw_file ${CCL_COVERAGE_DIR}/raw/${test_name}.profraw)
    set(exe_file ${CMAKE_BINARY_DIR}/test/${test_name}.exe)
    set(profdata_file ${CCL_COVERAGE_DIR}/${test_name}.profdata)
    set(coverage_report_file ${CCL_COVERAGE_DIR}/${test_name}.cov.html)

    if(DEFINED CCL_TEST_EXECUTABLES)
        set(CCL_TEST_EXECUTABLES ${CCL_TEST_EXECUTABLES};${exe_file} PARENT_SCOPE)
        set(CCL_TEST_SOURCES ${CCL_TEST_SOURCES};${test_file_path} PARENT_SCOPE)
    else()
        set(CCL_TEST_EXECUTABLES ${exe_file} PARENT_SCOPE)
        set(CCL_TEST_SOURCES ${test_file_path} PARENT_SCOPE)
    endif()

    if($CACHE{CCL_COVERAGE})
        set_tests_properties(
            ${test_name}
            PROPERTIES
                ENVIRONMENT
                    LLVM_PROFILE_FILE=${profraw_file}
        )
    endif()

    set(CCL_COVERAGE_RAW_DATA_FILES ${CCL_COVERAGE_RAW_DATA_FILES} ${profraw_file} PARENT_SCOPE)
    set(CCL_COVERAGE_REPORT_FILES ${CCL_COVERAGE_REPORT_FILES} ${coverage_report_file} PARENT_SCOPE)

    target_compile_options(
        ${test_name}
        PRIVATE
            -Wall -Wextra -pedantic -Werror
            $<$<BOOL:$CACHE{CCL_COVERAGE}>:-fprofile-instr-generate -fcoverage-mapping>
    )

    add_custom_command(
        OUTPUT ${profdata_file}
        COMMAND llvm-profdata merge ${profraw_file} -o ${profdata_file}
        DEPENDS ${test_file_path}
    )

    if(DEFINED main_tested_source_file)
        set(llvm_cov_sources_args -sources ${CMAKE_SOURCE_DIR}/${main_tested_source_file})
    endif()

    add_custom_command(
        OUTPUT ${coverage_report_file}
        DEPENDS ${profdata_file}
        COMMAND llvm-cov show --show-line-counts --format=html -Xdemangler=llvm-cxxfilt -Xdemangler=-nt --project-title=ccl --instr-profile ${profdata_file} -object ${exe_file} ${llvm_cov_sources_args} > ${coverage_report_file}
    )
endfunction(add_ccl_test)

set(CCL_COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)
set(CCL_COVERAGE_DATA_FILE ${CCL_COVERAGE_DIR}/ccl.profdata)

file(MAKE_DIRECTORY ${CCL_COVERAGE_DIR}/raw)

add_ccl_test(test_self_test test/test/test.cpp include/ccl/test/test.hpp)
add_ccl_test(test_util test/util.cpp include/ccl/util.hpp)
add_ccl_test(test_vector test/vector.cpp include/ccl/vector.hpp)
add_ccl_test(test_compressed_pair test/compressed-pair.cpp include/ccl/compressed-pair.hpp)
add_ccl_test(test_bitset test/bitset.cpp include/ccl/bitset.hpp)
add_ccl_test(test_maybe test/maybe.cpp include/ccl/maybe.hpp)
add_ccl_test(test_hashtable test/hashtable.cpp include/ccl/hashtable.hpp)
add_ccl_test(test_tables_table test/tables/table.cpp include/ccl/tables/table.hpp)
add_ccl_test(test_tables_view test/tables/view.cpp include/ccl/tables/view.hpp)
add_ccl_test(test_set test/set.cpp include/ccl/set.hpp)
add_ccl_test(test_local_allocator test/memory/local-allocator.cpp include/ccl/memory/local-allocator.hpp)
add_ccl_test(test_composite_allocator test/memory/composite-allocator.cpp include/ccl/memory/composite-allocator.hpp)
add_ccl_test(test_sparse_set test/sparse-set.cpp include/ccl/sparse-set.hpp)
add_ccl_test(test_hash test/hash.cpp include/ccl/hash.hpp)
add_ccl_test(test_dense_map test/dense-map.cpp include/ccl/dense-map.hpp)
add_ccl_test(test_type_traits test/type-traits.cpp include/ccl/type-traits.hpp)
add_ccl_test(test_paged_vector test/paged-vector.cpp include/ccl/paged-vector.hpp)
add_ccl_test(test_packed_integer test/packed-integer.cpp include/ccl/packed-integer.hpp)
add_ccl_test(test_handle test/handle.cpp include/ccl/handle.hpp)
add_ccl_test(test_ecs_component test/ecs/component.cpp include/ccl/ecs/component.hpp)
add_ccl_test(test_ecs_archetype test/ecs/archetype.cpp include/ccl/ecs/archetype.hpp)
add_ccl_test(test_ecs_registry test/ecs/registry.cpp include/ccl/ecs/registry.hpp)
add_ccl_test(test_ecs_view test/ecs/view.cpp include/ccl/ecs/view.hpp)
add_ccl_test(test_tagged_pointer test/tagged-pointer.cpp include/ccl/tagged-pointer.hpp)
add_ccl_test(test_pair test/pair.cpp include/ccl/pair.hpp)
add_ccl_test(test_atomic test/atomic.cpp include/ccl/atomic.hpp)
add_ccl_test(test_atomic_flag test/atomic-flag.cpp include/ccl/atomic.hpp)
add_ccl_test(test_compat test/compat.cpp include/ccl/compat.hpp)
add_ccl_test(test_deque_begin_policy test/deque-begin-policy.cpp include/ccl/deque.hpp)
add_ccl_test(test_deque_center_policy test/deque-center-policy.cpp include/ccl/deque.hpp)
add_ccl_test(test_contiguous_iterator test/contiguous-iterator.cpp include/ccl/contiguous-iterator.hpp)

# Coverage
add_custom_command(
    OUTPUT ${CCL_COVERAGE_DATA_FILE}
    COMMAND llvm-profdata merge ${CCL_COVERAGE_RAW_DATA_FILES} -o ${CCL_COVERAGE_DATA_FILE}
    DEPENDS ${CCL_COVERAGE_RAW_DATA_FILES}
)

foreach(f ${CCL_TEST_EXECUTABLES})
    set(CCL_COVERAGE_REPORT_SOURCE_ARGS ${CCL_COVERAGE_REPORT_SOURCE_ARGS} -object ${f})
endforeach()

add_custom_target(
    coverage-summary
    DEPENDS ${CCL_COVERAGE_DATA_FILE}
    COMMAND llvm-cov report -ignore-filename-regex="\\.cpp$$" --instr-profile ${CCL_COVERAGE_DATA_FILE} ${CCL_COVERAGE_REPORT_SOURCE_ARGS}
)

add_custom_target(
    coverage-report
    DEPENDS ${CCL_COVERAGE_REPORT_FILES}
)

set(VECTOR_TEST_FILES
    instruction_correctness_single_cycle/single_cycle_vector_correctness_test.cpp
    instruction_correctness_pipelined/pipelined_vector_correctness_test.cpp
    scalar_vector_comparison/scalar_vector_comparison_test.cpp
)

add_executable(kites_vector_test
    ${VECTOR_TEST_FILES}
)

target_link_libraries(kites_vector_test PRIVATE
    kites_core_cli
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

gtest_discover_tests(kites_vector_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "Vector"
)
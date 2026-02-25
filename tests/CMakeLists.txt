# ======================================================================
#               dmhaman Integration Tests
# ======================================================================
cmake_minimum_required(VERSION 3.18)

project(dmhaman_tests VERSION 1.0 DESCRIPTION "dmhaman Integration Tests" LANGUAGES C)

# Ensure DMOD is available
if(NOT DMOD_DIR)
    message(FATAL_ERROR "DMOD_DIR not set. Tests must be built as part of the main project.")
endif()

# ======================================================================
#               Build verification tests
# ======================================================================
# These tests verify the module was built correctly.

# Test 1: Verify the DMF module binary was created
add_test(NAME dmhaman_module_built
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/dmf/dmhaman.dmf
        ${CMAKE_BINARY_DIR}/dmf/dmhaman.dmf.test
)

# Cleanup test file
add_test(NAME dmhaman_module_built_cleanup
    COMMAND ${CMAKE_COMMAND} -E remove -f
        ${CMAKE_BINARY_DIR}/dmf/dmhaman.dmf.test
)
set_tests_properties(dmhaman_module_built_cleanup PROPERTIES
    DEPENDS dmhaman_module_built
)

# Test 2: Verify the generated API header was created
add_test(NAME dmhaman_defs_header_generated
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/dmhaman_defs.h
        ${CMAKE_BINARY_DIR}/dmhaman_defs.h.test
)

add_test(NAME dmhaman_defs_header_cleanup
    COMMAND ${CMAKE_COMMAND} -E remove -f
        ${CMAKE_BINARY_DIR}/dmhaman_defs.h.test
)
set_tests_properties(dmhaman_defs_header_cleanup PROPERTIES
    DEPENDS dmhaman_defs_header_generated
)

message(STATUS "dmhaman integration tests configured")
message(STATUS "  Module output directory: ${CMAKE_BINARY_DIR}/dmf")
message(STATUS "  Run 'ctest' to verify the build")

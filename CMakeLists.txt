# =====================================================================
#               DMOD Handlers Manager Module
# =====================================================================
cmake_minimum_required(VERSION 3.18)

# ======================================================================
#               Unit Tests Option (early to control build behavior)
# ======================================================================
option(DMHAMAN_BUILD_TESTS "Build unit tests" OFF)

# ======================================================================
#               dmhaman Version
# ======================================================================
if(NOT DEFINED DMOD_MODULE_VERSION)
    set(DMOD_MODULE_VERSION "0.1" CACHE STRING "DMOD module version")
endif()

# ======================================================================
#               Fetch DMOD repository
# ======================================================================
include(FetchContent)
FetchContent_Declare(
    dmod
    GIT_REPOSITORY https://github.com/choco-technologies/dmod.git
    GIT_TAG        develop
)

# ======================================================================
#               DMOD Configuration
# ======================================================================
set(DMOD_MODE "DMOD_MODULE" CACHE STRING "DMOD build mode")
set(DMOD_BUILD_TESTS OFF CACHE BOOL "Build tests")
set(DMOD_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(DMOD_BUILD_TOOLS OFF CACHE BOOL "Build tools")
set(DMOD_BUILD_TEMPLATES OFF CACHE BOOL "Build templates")

FetchContent_MakeAvailable(dmod)

set(DMOD_DIR ${dmod_SOURCE_DIR} CACHE PATH "DMOD source directory")

# ======================================================================
#               Fetch dmlist
# ======================================================================
FetchContent_Declare(
    dmlist
    GIT_REPOSITORY https://github.com/choco-technologies/dmlist.git
    GIT_TAG        v0.2
)
set(DMLIST_BUILD_TESTS OFF CACHE BOOL "Build dmlist tests")
FetchContent_MakeAvailable(dmlist)

# ======================================================================
#               For VS Code
# ======================================================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(dmhaman
    VERSION ${DMOD_MODULE_VERSION}
    DESCRIPTION "DMOD Handlers Manager"
    LANGUAGES C CXX)

# ======================================================================
#               Import dmod functions and macros
# ======================================================================
include(${DMOD_DIR}/paths.cmake)
dmod_setup_external_module()

# ======================================================================
#               Build module only when not in test mode
# ======================================================================
if(NOT DMHAMAN_BUILD_TESTS)
    # Name of the module
    set(DMOD_MODULE_NAME    dmhaman)

    # Author (should be string)
    set(DMOD_AUTHOR_NAME    "Choco Technologies")

    # Stack size for the module (should be integer)
    set(DMOD_STACK_SIZE     1024)

    dmod_add_library(${DMOD_MODULE_NAME} ${DMOD_MODULE_VERSION}
        src/dmhaman.c
    )

    target_include_directories(${DMOD_MODULE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    dmod_link_modules(${DMOD_MODULE_NAME}
        dmlist
    )

    target_link_libraries(${DMOD_MODULE_NAME} dmlist_if)
endif()

# ======================================================================
#               Unit Tests
# ======================================================================
if(DMHAMAN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()


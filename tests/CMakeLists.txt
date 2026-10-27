cmake_minimum_required(VERSION 3.28)

# Host tests of this chip's clock limits (src/chip/ClockLimits.hpp) through chip_atsam_common's ClockSolver.hpp, from
# the src/chip/atsam_common this repository records - the solver itself is tested in chip_atsam_common - and of its
# reset cause (src/chip/PM.hpp). Needs Kvasir_SDK's src/ (Util/Prescaler.hpp, Register/) (KVASIR_ROOT: the checkout next
# to this one, else fetched):
#
# cmake -S tests -B tests/build -G Ninja && cmake --build tests/build && ctest --test-dir tests/build
#
# Most checks are static_asserts: building is the test.
project(chip_atsamc21_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Kvasir_SDK: KVASIR_ROOT, else the checkout next to this repository, else fetched; its submodules and the other
# packages: kvasir_locate (Kvasir_SDK cmake/kvasir_locate.cmake)
if(NOT KVASIR_ROOT AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../Kvasir_SDK/cmake/kvasir_locate.cmake")
    get_filename_component(KVASIR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../Kvasir_SDK" ABSOLUTE)
elseif(NOT KVASIR_ROOT)
    include(FetchContent)
    FetchContent_Declare(
        kvasir_sdk
        GIT_REPOSITORY https://github.com/kvasir-io/Kvasir_SDK.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_SUBMODULES "" SOURCE_SUBDIR fetch-only)
    FetchContent_MakeAvailable(kvasir_sdk)
    set(KVASIR_ROOT "${kvasir_sdk_SOURCE_DIR}")
endif()
include("${KVASIR_ROOT}/cmake/kvasir_locate.cmake")
kvasir_locate(SDK_SUBMODULES svd_converter)
get_filename_component(_chip_src "${CMAKE_CURRENT_LIST_DIR}/../src/chip" ABSOLUTE)
# kvasir::base: the SDK's src/ (Util/Prescaler.hpp, Register/, ...)
include(${KVASIR_ROOT}/cmake/kvasir_sdk_targets.cmake)

add_executable(clock_limits clock_limits_test.cpp)
target_include_directories(clock_limits PRIVATE ${_chip_src})
target_link_libraries(clock_limits PRIVATE kvasir::base)
target_compile_options(clock_limits PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME clock_limits COMMAND clock_limits)

# PM.hpp's reset cause (no BACKUP bit); the clock gates (Blocks.hpp) are checked by tests/conformance.
add_executable(reset_cause reset_cause_test.cpp)
target_include_directories(reset_cause PRIVATE ${_chip_src})
target_link_libraries(reset_cause PRIVATE kvasir::base)
target_compile_options(reset_cause PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME reset_cause COMMAND reset_cause)

# The write-only guard: every write-only field of this chip's SVD(s) is classified (oneToSet, a key, or <!-- Kvasir:
# write-only accepted -->), so svd_converter --write-only-guard=error must exit 0. Needs the SDK's svd_converter (the
# sibling or fetched SDK's submodule); left out without it, except in CI. chip.svd (the E17A) and the G17A's SVD, which
# this tree generates from it the way a firmware tree does (the SDK's cmake/chip_package.cmake).
include(${KVASIR_ROOT}/cmake/chip_package.cmake)
kvasir_chip_generate_svd(
    _g17a_svd
    PART
    ATSAMC21G17A
    SCRIPT
    ${CMAKE_CURRENT_LIST_DIR}/../scripts/make_g17a_svd.py
    INPUTS
    ${CMAKE_CURRENT_LIST_DIR}/../chip.svd)
if(EXISTS ${KVASIR_ROOT}/svd_converter/CMakeLists.txt)
    add_subdirectory(${KVASIR_ROOT}/svd_converter svd_converter)
    foreach(svd ${CMAKE_CURRENT_LIST_DIR}/../chip.svd ${_g17a_svd})
        get_filename_component(_svd_name ${svd} NAME)
        string(MAKE_C_IDENTIFIER ${_svd_name} _svd_name)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/write_only_guard/${_svd_name})
        add_test(NAME svd_write_only_guard_${_svd_name}
                 COMMAND svd_converter --write-only-guard=error --write-only-registers=derived ${svd}
                         ${CMAKE_CURRENT_BINARY_DIR}/write_only_guard/${_svd_name} json)
    endforeach()
else()
    # in CI the guard must run: a missing svd_converter there is a broken fetch, not a reason to pass
    if(DEFINED ENV{CI})
        message(FATAL_ERROR "svd_write_only_guard: no ${KVASIR_ROOT}/svd_converter in CI")
    endif()
    message(STATUS "svd_write_only_guard left out: no ${KVASIR_ROOT}/svd_converter")
endif()

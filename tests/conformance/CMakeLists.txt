cmake_minimum_required(VERSION 3.28)

# The chip's conformance tree (Kvasir_SDK tests/conformance/conformance.cmake): this package's headers compiled for the
# target with a firmware's flags, against the generated register headers. Building is the test - the contract rows
# (conformance.cpp), the dependent register writes (instantiate.cpp) and every public header alone. The chip and core
# SVDs convert with the write-only guard on the way.
#
# cmake -S tests/conformance -B tests/conformance/build -G Ninja [-DCOMPILE_TARGET=arm_gcc]
# [-DCONFORMANCE_VARIANT=<part>] && cmake --build tests/conformance/build
#
# Kvasir_SDK and kvasir_devices: the checkouts next to this one, else fetched (FETCHCONTENT_SOURCE_DIR_KVASIR_SDK /
# _KVASIR_DEVICES point a lone checkout at local copies).

set(CONFORMANCE_VARIANT
    ATSAMC21E17A
    CACHE STRING "the part the tree is configured for (KVASIR_ATSAMC21_MPU): ATSAMC21E17A or ATSAMC21G17A")
set(KVASIR_ATSAMC21_MPU ${CONFORMANCE_VARIANT})

get_filename_component(CHIP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
# Kvasir_SDK: KVASIR_ROOT, else the checkout next to this repository, else fetched; its submodules and the other
# packages: kvasir_locate (Kvasir_SDK cmake/kvasir_locate.cmake)
if(NOT KVASIR_ROOT AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../../Kvasir_SDK/cmake/kvasir_locate.cmake")
    get_filename_component(KVASIR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../Kvasir_SDK" ABSOLUTE)
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
kvasir_locate(SDK_SUBMODULES ALL DEVICES)
if(NOT KVASIR_DEVICES_ROOT)
    FetchContent_Declare(
        kvasir_devices
        GIT_REPOSITORY https://github.com/kvasir-io/kvasir_devices.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR none)
    FetchContent_MakeAvailable(kvasir_devices)
    set(KVASIR_DEVICES_ROOT ${kvasir_devices_SOURCE_DIR})
endif()

include(${KVASIR_ROOT}/tests/conformance/conformance.cmake)
kvasir_conformance_begin()

project(chip_atsamc21_conformance CXX C ASM)

include(${KVASIR_ROOT}/cmake/util.cmake)

kvasir_conformance_project(
    NAME
    chip_atsamc21_conformance
    SOURCES
    conformance.cpp
    INSTANTIATE
    instantiate.cpp
    CONFIG_HEADER
    kvasir_config.hpp
    HEADERS
    ${CHIP_ROOT}/src/chip/*.hpp
    ${CHIP_ROOT}/src/chip/atsam_common/*.hpp
    NOT_ALONE
    not_alone.txt)

# What the C21 refuses at compile time (tree_must_fail.cpp): TREE_FAIL=0 compiles in `all` (the control), each case must
# fail with its own message (ctest in this tree, both compilers). 6 and 7 are the E17A's (no SERCOM4).
set(_tree_cases
    "1|f_CPU above the part's maximum" "2|this SAM part has no such clock source"
    "3|SAM C21 XOSC: a crystal of 0[.]4[.][.]32 MHz" "4|no setting makes 47000001 Hz from 8000000 Hz"
    "5|or an XIN clock up to 48 MHz")
if(CONFORMANCE_VARIANT STREQUAL "ATSAMC21E17A")
    list(APPEND _tree_cases "6|no clock gate row in chip/Blocks[.]hpp for 0x42001400"
         "7|no clock gate row in chip/Blocks[.]hpp for 0x42001400")
endif()
kvasir_conformance_must_fail(
    NAME
    tree_must_fail
    PROJECT
    chip_atsamc21_conformance
    SOURCE
    tree_must_fail.cpp
    DEFINE
    TREE_FAIL
    CASES
    ${_tree_cases})

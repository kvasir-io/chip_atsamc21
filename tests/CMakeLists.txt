cmake_minimum_required(VERSION 3.28)

# Host tests of this chip's clock limits (src/chip/ClockLimits.hpp) through chip_atsam_common's ClockSolver.hpp, from
# the src/chip/atsam_common this repository records - the solver itself is tested in chip_atsam_common. Needs
# Kvasir_SDK's Util/Prescaler.hpp (KVASIR_ROOT, default: the checkout next to this one):
#
# cmake -S tests -B tests/build -G Ninja && cmake --build tests/build && ctest --test-dir tests/build
#
# Most checks are static_asserts: building is the test.
project(chip_atsamc21_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

if(NOT KVASIR_ROOT)
    get_filename_component(KVASIR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../Kvasir_SDK" ABSOLUTE)
endif()
get_filename_component(_chip_src "${CMAKE_CURRENT_LIST_DIR}/../src/chip" ABSOLUTE)

add_executable(clock_limits clock_limits_test.cpp)
target_include_directories(clock_limits PRIVATE ${KVASIR_ROOT}/src ${_chip_src})
target_compile_options(clock_limits PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME clock_limits COMMAND clock_limits)

# svd/ATSAMC21G17A.svd is generated from chip.svd (svd/README.md): stale after an edit of chip.svd that was not followed
# by scripts/make_g17a_svd.py.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_test(NAME g17a_svd_up_to_date COMMAND ${Python3_EXECUTABLE} -B
                                              ${CMAKE_CURRENT_LIST_DIR}/../scripts/make_g17a_svd.py --check)
endif()

# The write-only guard: every write-only field of this chip's SVD(s) is classified (oneToSet, a key, or <!-- Kvasir:
# write-only accepted -->), so svd_converter --write-only-guard=error must exit 0. Needs the SDK's svd_converter (the
# sibling checkout's submodule); left out without it.
if(EXISTS ${KVASIR_ROOT}/svd_converter/CMakeLists.txt)
    add_subdirectory(${KVASIR_ROOT}/svd_converter svd_converter)
    foreach(svd chip.svd)
        string(MAKE_C_IDENTIFIER ${svd} _svd_name)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/write_only_guard/${_svd_name})
        add_test(
            NAME svd_write_only_guard_${_svd_name}
            COMMAND svd_converter --write-only-guard=error --write-only-registers=derived
                    ${CMAKE_CURRENT_LIST_DIR}/../${svd} ${CMAKE_CURRENT_BINARY_DIR}/write_only_guard/${_svd_name} json)
    endforeach()
else()
    message(STATUS "svd_write_only_guard left out: no ${KVASIR_ROOT}/svd_converter")
endif()

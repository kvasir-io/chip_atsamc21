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

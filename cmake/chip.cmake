include(${CMAKE_CURRENT_LIST_DIR}/../core/cmake/core.cmake)

# Which SAM C21 this firmware is for. A project sets it before include(kvasir.cmake), or passes it as
# -DKVASIR_ATSAMC21_MPU=...; without it the part is the ATSAMC21E17A this package started with.
if(NOT KVASIR_ATSAMC21_MPU)
    set(KVASIR_ATSAMC21_MPU ATSAMC21E17A)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/variants/${KVASIR_ATSAMC21_MPU}.cmake)
    file(
        GLOB _known
        RELATIVE ${CMAKE_CURRENT_LIST_DIR}/variants
        ${CMAKE_CURRENT_LIST_DIR}/variants/*.cmake)
    list(TRANSFORM _known REPLACE "\\.cmake$" "")
    message(FATAL_ERROR "KVASIR_ATSAMC21_MPU is '${KVASIR_ATSAMC21_MPU}'; chip_atsamc21 knows: ${_known}")
endif()
include(${CMAKE_CURRENT_LIST_DIR}/variants/${KVASIR_ATSAMC21_MPU}.cmake)

# The SDK writes a UF2 next to every image and passes this through as the family id, so it has to be set even though
# this part has no UF2 bootloader - it is flashed over SWD. There is no registered UF2 family for the SAM C21 (the list
# has SAMD21, SAML21, SAMD51 and no C21), and borrowing the SAMD21's would mislabel the image, so the family is left as
# none.
set(TARGET_UF2_CODE 0x00000000)

# The write-only guard: every write-only field of the SVD is classified (oneToSet, a key, or <!-- Kvasir: write-only
# accepted -->), so a new one stops the build; registers with no readable field are never read
svd_convert(
    peripherals
    SVD_FILE
    ${CHIP_SVD_FILE}
    OUTPUT_DIRECTORY
    peripherals
    WRITE_ONLY_GUARD
    error
    WRITE_ONLY_REGISTERS
    derived
    WRITE_ONLY_MASK
    ON)

# kvasir_devices: chip.hpp includes its drivers unconditionally (SamPushButton/SamRotaryEncoder ->
# kvasir/Devices/PushButton.hpp, RotaryEncoder.hpp; Sercom_I2CQueued.hpp -> kvasir/Devices/I2C/LineRecovery.hpp), so
# every image needs it. Found like CHIP_ROOT (KVASIR_DEVICES_ROOT: variable, environment, else next to the SDK); the SDK
# adds it after project() unless the firmware has it already.
kvasir_resolve_root(KVASIR_DEVICES_ROOT kvasir_devices)
kvasir_add_package(${KVASIR_DEVICES_ROOT} kvasir_devices kvasir_devices)
target_link_libraries(peripherals INTERFACE kvasir::devices)

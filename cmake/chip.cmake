include(${CMAKE_CURRENT_LIST_DIR}/../core/cmake/core.cmake)

set(TARGET_MPU ATSAMC21E17A)
set(TARGET_FLASH_SIZE 131072)
set(TARGET_RAM_SIZE 16384)
set(TARGET_EEPROM_SIZE 4096)

# The SDK writes a UF2 next to every image and passes this through as the family id, so it has to be set even though
# this part has no UF2 bootloader - it is flashed over SWD. There is no registered UF2 family for the SAM C21 (the list
# has SAMD21, SAML21, SAMD51 and no C21), and borrowing the SAMD21's would mislabel the image, so the family is left as
# none.
set(TARGET_UF2_CODE 0x00000000)

set(LINKER_FILE ${CMAKE_CURRENT_LIST_DIR}/../linker/chip.ld)

svd_convert(peripherals SVD_FILE ${CMAKE_CURRENT_LIST_DIR}/../chip.svd OUTPUT_DIRECTORY peripherals)

# kvasir_devices: chip.hpp includes its drivers unconditionally (SamPushButton/SamRotaryEncoder ->
# kvasir/Devices/PushButton.hpp, RotaryEncoder.hpp; Sercom_I2CQueued.hpp -> kvasir/Devices/I2C/LineRecovery.hpp), so
# every image needs it. Found like CHIP_ROOT (KVASIR_DEVICES_ROOT: variable, environment, else next to the SDK); the SDK
# adds it after project() unless the firmware has it already.
kvasir_resolve_root(KVASIR_DEVICES_ROOT kvasir_devices)
kvasir_add_package(${KVASIR_DEVICES_ROOT} kvasir_devices kvasir_devices)
target_link_libraries(peripherals INTERFACE kvasir::devices)

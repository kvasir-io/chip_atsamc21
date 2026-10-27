include(${CMAKE_CURRENT_LIST_DIR}/../core/cmake/core.cmake)

# Which SAM C21 this firmware is for. A project sets it before include(kvasir.cmake), or passes it as
# -DKVASIR_ATSAMC21_MPU=...; without it the part is the ATSAMC21E17A this package started with. cmake/variants/ holds
# what differs per part; the ATSAMC21G17A's SVD is made from chip.svd at configure time.
kvasir_chip_variant(
    _variant
    VARIABLE
    KVASIR_ATSAMC21_MPU
    DEFAULT
    ATSAMC21E17A
    DIR
    ${CMAKE_CURRENT_LIST_DIR}/variants
    PACKAGE
    chip_atsamc21
    GENERATED_SVD
    ATSAMC21G17A
    ${CMAKE_CURRENT_LIST_DIR}/../scripts/make_g17a_svd.py
    ${CMAKE_CURRENT_LIST_DIR}/../chip.svd
    SVD_VARIABLE
    chip_svd_file)
include(${_variant})

# Kvasir's write semantics (svd_convert's defaults): a new unclassified write-only field stops the build
kvasir_chip_svd(peripherals ${chip_svd_file})

# the chip's part of the contract (Kvasir_SDK cmake/kvasir_chip.cmake), with the part's values from its variant file
# DEVICES: chip/drivers.hpp builds on kvasir_devices, which the SDK then adds after project()
kvasir_chip(
    DEVICES
    MPU
    ${chip_mpu}
    # The SDK writes a UF2 next to every image with this family id, though this part has no UF2 bootloader - it is
    # flashed over SWD. There is no registered UF2 family for the SAM C21 (the list has SAMD21, SAML21, SAMD51 and no
    # C21), and borrowing the SAMD21's would mislabel the image, so the family is left as none.
    UF2_FAMILY
    0x00000000
    LINKER_FILE
    ${chip_linker_file}
    INCLUDE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/../src
    REGISTERS
    peripherals
    DEFINITIONS
    ${chip_definitions})

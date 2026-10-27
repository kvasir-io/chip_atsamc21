# ATSAMC21G17A: 48 pins, PORTA and part of PORTB, 128 KB flash, 16 KB RAM, a 4 KB RWW section used as EEPROM, SERCOM0-5,
# CAN0 and CAN1 (SAM C20/C21 data sheet DS60001479M, Table 1-2 and Table 1-3). The memories are the E17A's, so is the
# linker script.
set(chip_mpu ATSAMC21G17A)

set(chip_linker_file ${CMAKE_CURRENT_LIST_DIR}/../../linker/chip.ld)
# its SVD is chip.svd with SERCOM4/5 and CAN1 put in, made at configure (never checked in): generated, see chip.cmake

# What src/chip/Variant.hpp reads (the part, the series, the family). A define and not __has_include(<peripherals/...>):
# a build tree that changed its part keeps the other part's generated headers.
set(chip_definitions KVASIR_CHIP_ATSAMC21G17A=1 KVASIR_CHIP_ATSAMC21=1 KVASIR_CHIP_FAMILY_SAM=1)

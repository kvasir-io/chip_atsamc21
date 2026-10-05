# ATSAMC21G17A: 48 pins, PORTA and part of PORTB, 128 KB flash, 16 KB RAM, a 4 KB RWW section used as EEPROM, SERCOM0-5,
# CAN0 and CAN1 (SAM C20/C21 data sheet DS60001479M, Table 1-2 and Table 1-3). The memories are the E17A's, so is the
# linker script.
set(TARGET_MPU ATSAMC21G17A)
set(TARGET_FLASH_SIZE 131072)
set(TARGET_RAM_SIZE 16384)
set(TARGET_EEPROM_SIZE 4096)

set(LINKER_FILE ${CMAKE_CURRENT_LIST_DIR}/../../linker/chip.ld)
set(CHIP_SVD_FILE ${CMAKE_CURRENT_LIST_DIR}/../../svd/ATSAMC21G17A.svd)

# What src/chip/Variant.hpp reads. A define and not __has_include(<peripherals/...>): a build tree that changed its part
# keeps the other part's generated headers.
list(APPEND CHIP_OPTIONS -DKVASIR_CHIP_ATSAMC21G17A=1)

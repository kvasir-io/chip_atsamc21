# chip_atsamc21

Kvasir chip package for the **Microchip ATSAMC21E17A**: Cortex-M0+, 128 KiB flash, 16 KiB RAM,
4 KiB RWW flash used as an emulated EEPROM, one CAN controller and four SERCOMs.

Nothing builds here. `cmake/chip.cmake` is an `include()` fragment the Kvasir SDK pulls in
through `CHIP_ROOT`, the peripheral headers are generated from `chip.svd` into the consumer's
binary directory, and `src/chip/*.hpp` is hand-written. To exercise the package, build
something that uses it:

    cd ../rgb_rotary && just chip_root=$PWD/../chip_atsamc21 build

## Layout

| Path | What |
| --- | --- |
| `chip.svd` | the part's CMSIS SVD, the source of `peripherals/*.hpp` |
| `cmake/chip.cmake` | MPU, memory sizes, the linker script, `svd_convert()` |
| `linker/chip.ld` | flash at 0, the RWW EEPROM at 0x00400000, RAM at 0x20000000 |
| `core/` | `core_cortex_m0plus`, a submodule |
| `src/chip/atsam_common/` | `chip_atsam_common`, a submodule shared with chip_atsamd21 |
| `src/chip/*.hpp` | what is specific to this part (below) |

`src/chip/` holds the part's own tables: `Interrupt.hpp` (the vector table),
`Io.hpp` (PORTA only, and which of its pins the 32-pin package does not bond),
`GCLK.hpp` (the C21's generator/peripheral-channel scheme, which is not the D21's),
`MCLK.hpp` and `PM.hpp` (clock gating, and the reset cause out of RSTC),
`Sercom_Traits.hpp`, `CAN_Traits.hpp`, `NVMCTRL_Traits.hpp`, `Dmac_Traits.hpp` and
`Serial_Number_Traits.hpp`.

## What this package does not offer yet

`src/chip/chip.hpp` deliberately leaves three drivers out of the shared set, each with the
reason written where the include would be:

- **SPI and USART.** The C21's SVD splits those SERCOM modes into `SERCOM_SPIM`/`SERCOM_SPIS`
  and `SERCOM_USART_INT`/`SERCOM_USART_EXT` where the D21 has one of each, and the register
  sets differ with them. I2C is unaffected: `SERCOM_I2CM` is the same on both parts.
- **The event system.** `atsam_common/EVSYS.hpp` is written against the D21's `CTRL` and
  `CHANNEL_TRIG`; the C21 carries the later event system with indexed `CHANNEL[n]`/`USER[n]`.
- **The fuses.** `atsam_common/Fuses.hpp` uses the D21's NVM user-row layout, where several
  fields this part widens are single bits.

Each is a straightforward port; none had a user when the package was written.

## Status

The CAN driver in `chip_atsam_common` had no consumer at all until this package existed - the
D21 has no CAN peripheral - so `rgb_rotary` is the first firmware to compile it since the SDK
moved to C++26. It builds; it has not been run against a bus.

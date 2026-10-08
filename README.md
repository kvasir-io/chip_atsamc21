# chip_atsamc21

Kvasir chip package for the **Microchip ATSAMC21E17A** and **ATSAMC21G17A**: Cortex-M0+, 128 KiB
flash, 16 KiB RAM, 4 KiB RWW flash used as an emulated EEPROM; the 32-pin E with PORTA, four
SERCOMs and one CAN controller, the 48-pin G with part of PORTB, six SERCOMs and two CANs.

The part is picked with `KVASIR_ATSAMC21_MPU` (set before `include(kvasir.cmake)`, or
`-DKVASIR_ATSAMC21_MPU=...`); without it the part is the ATSAMC21E17A:

    set(KVASIR_ATSAMC21_MPU ATSAMC21G17A)

Nothing builds here. `cmake/chip.cmake` is an `include()` fragment the Kvasir SDK pulls in
through `CHIP_ROOT`, the peripheral headers are generated from `chip.svd` into the consumer's
binary directory, and `src/chip/*.hpp` is hand-written. To exercise the package, build
something that uses it:

    cd <a firmware on this chip> && just chip_root=$PWD/../chip_atsamc21 build

## Layout

| Path | What |
| --- | --- |
| `chip.svd` | the ATSAMC21E17A's CMSIS SVD, the source of `peripherals/*.hpp` |
| `svd/ATSAMC21G17A.svd` | the G part's, made from `chip.svd` by `scripts/make_g17a_svd.py` (`svd/README.md`) |
| `cmake/chip.cmake` | picks the part (`cmake/variants/<part>.cmake`: MPU, memory sizes, linker script, SVD), `svd_convert()` |
| `linker/chip.ld` | flash at 0, the RWW EEPROM at 0x00400000, RAM at 0x20000000 (both parts) |
| `core/` | `core_cortex_m0plus`, a submodule |
| `src/chip/atsam_common/` | `chip_atsam_common`, a submodule shared with chip_atsamd21 |
| `src/chip/*.hpp` | what is specific to this part (below) |

`src/chip/` holds the part's own tables: `Variant.hpp` (which part, from the define the G
variant's cmake file sets; every header below asks it where the parts differ),
`Interrupt.hpp` (the vector table; SERCOM4/5 and CAN1 on the G only),
`Io.hpp` (the ports and which of their pins each package does not bond),
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

The G part: its SVD matches Microchip's own ATSAMC21G17A.svd field for field except
for the names chip.svd spells the shared drivers' way (USART_INT/USART_EXT, CAN for CAN0) and
the write semantics marked in chip.svd; adding it left E17A images byte-identical.
Firmware builds on it; nothing has run on a G board yet, and SERCOM4/5 and CAN1 have no user.

The CAN driver in `chip_atsam_common` had no consumer at all until this package existed - the
D21 has no CAN peripheral - so an E17A firmware was the first to compile it since the SDK
moved to C++26. It builds; it has not been run against a bus.

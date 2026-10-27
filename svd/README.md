# SVD files of the further parts

`../chip.svd` is the ATSAMC21E17A (the default part, `KVASIR_ATSAMC21_MPU` unset). Its register,
field and enum names are the ones `chip_atsam_common`'s drivers are written against, and it carries
the hand-made write semantics (`<!-- Kvasir: ... -->`, see Kvasir_SDK's notes on SVD edits).

| file | part | how it is made |
|---|---|---|
| `<build>/chip_svd/ATSAMC21G17A.svd` | ATSAMC21G17A | `scripts/make_g17a_svd.py`: `../chip.svd` plus what the 48-pin die bonds out on top of the E - SERCOM4, SERCOM5 and CAN1 (derived peripherals with their NVIC lines), their MCLK clock-mask bits, PAC flags and DMAC trigger sources, each with a `<!-- Kvasir: G part only, ... -->` note naming the data sheet line |

**The G file is generated at configure into the build tree, `<build>/chip_svd/`, and never checked in**:
`cmake/chip.cmake` runs `scripts/make_g17a_svd.py --out` for the G17A (the SDK's `kvasir_chip_variant()`, `cmake/chip_package.cmake`), and a
change of `../chip.svd` or the script re-runs it. Edit `../chip.svd`, never the generated file. The tests
(`tests/CMakeLists.txt`) generate it too and run the write-only guard over it.

Checked against Microchip's own `ATSAMC21G17A.svd` (the copy in the 2021 package
`kvasir_atsamc21g17a`): every register offset, field position and width and enumerated value
agrees, except the SERCOM USART register set (Microchip: one `USART`, here `USART_INT` and
`USART_EXT` as in the E's file), `CAN` for Microchip's `CAN0`, and `MPU`/`NVIC`, which only
`chip.svd` lists.

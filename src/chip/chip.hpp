#pragma once

#include "peripherals/AC.hpp"
#include "peripherals/ADC.hpp"
#include "peripherals/CAN.hpp"
#include "peripherals/CCL.hpp"
#include "peripherals/DAC.hpp"
#include "peripherals/DIVAS.hpp"
#include "peripherals/DMAC.hpp"
#include "peripherals/DSU.hpp"
#include "peripherals/EIC.hpp"
#include "peripherals/EVSYS.hpp"
#include "peripherals/FREQM.hpp"
#include "peripherals/GCLK.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/MTB.hpp"
#include "peripherals/NVMCTRL.hpp"
#include "peripherals/OSC32KCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"
#include "peripherals/PAC.hpp"
#include "peripherals/PM.hpp"
#include "peripherals/PORT.hpp"
#include "peripherals/PTC.hpp"
#include "peripherals/RSTC.hpp"
#include "peripherals/RTC_MODE0.hpp"
#include "peripherals/RTC_MODE1.hpp"
#include "peripherals/RTC_MODE2.hpp"
#include "peripherals/SDADC.hpp"
#include "peripherals/SERCOM_I2CM.hpp"
#include "peripherals/SERCOM_I2CS.hpp"
#include "peripherals/SERCOM_SPIM.hpp"
#include "peripherals/SERCOM_SPIS.hpp"
#include "peripherals/SERCOM_USART_EXT.hpp"
#include "peripherals/SERCOM_USART_INT.hpp"
#include "peripherals/SUPC.hpp"
#include "peripherals/TCC.hpp"
#include "peripherals/TC_COUNT16.hpp"
#include "peripherals/TC_COUNT32.hpp"
#include "peripherals/TC_COUNT8.hpp"
#include "peripherals/TSENS.hpp"
#include "peripherals/WDT.hpp"
//
#include "PM.hpp"
//
#include "GCLK.hpp"
#include "Interrupt.hpp"
#include "Io.hpp"
#include "atsam_common/EIC.hpp"
// atsam_common/EVSYS.hpp is not included: it is written against the D21's event system (CTRL,
// CHANNEL_TRIG), and the C21 carries the later one with indexed CHANNEL[n]/USER[n] registers. A
// C21 event-system driver would be a new file; nothing here needs one yet.
// atsam_common/Fuses.hpp is not included either: its NVM user-row field layout is the D21's, and
// the C21 packs that row differently - the bootloader-size field the D21 driver set() and clear()
// as single bits is several bits wide here, which is a hard error rather than a wrong value.
//
#include "CAN_Traits.hpp"
#include "Sercom_Traits.hpp"
//
#include "atsam_common/CAN.hpp"
#include "atsam_common/DMAC.hpp"
#include "atsam_common/NVMCTRL.hpp"
#include "atsam_common/SamPushButton.hpp"
#include "atsam_common/SamRotaryEncoder.hpp"
#include "atsam_common/Sercom_I2C.hpp"
#include "atsam_common/Sercom_I2CQueued.hpp"
// atsam_common/Sercom_SPI.hpp and Sercom_Usart.hpp are not included: the C21's SVD splits those
// SERCOM modes into SERCOM_SPIM/SERCOM_SPIS and SERCOM_USART_INT/SERCOM_USART_EXT, where the D21
// has one SERCOM_SPI and one SERCOM_USART, and the register sets differ with them. Both drivers
// want a C21 pass before this package claims to offer SPI or a UART. I2C is unaffected -
// SERCOM_I2CM is the same header on both parts.
#include "atsam_common/Serial_Number.hpp"
#include "atsam_common/StartUp.hpp"
#include "core/core.hpp"

#pragma once
// Every driver of the SAM C21, opt-in: chip/chip.hpp is the chip without them. A firmware includes the drivers
// it uses by name (chip/atsam_common/<Driver>.hpp) or all of them here. The queued buses, the input drivers and
// the USB backend build on kvasir_devices (LIBRARIES kvasir::bus or kvasir::devices).
#include "chip.hpp"
//
#include "atsam_common/CAN.hpp"
#include "atsam_common/DMAC.hpp"
#include "atsam_common/EIC.hpp"
#include "atsam_common/NVMCTRL.hpp"
#include "atsam_common/SamPushButton.hpp"
#include "atsam_common/SamQuadratureEncoder.hpp"
#include "atsam_common/SamRotaryEncoder.hpp"
#include "atsam_common/Sercom_I2C.hpp"
#include "atsam_common/Sercom_I2CQueued.hpp"
#include "atsam_common/Sercom_SPI.hpp"
#include "atsam_common/Sercom_Usart.hpp"
#include "atsam_common/Serial_Number.hpp"
// The fuses (user row) and the event system are D21 files (chip_atsamd21 Fuses.hpp, Evsys.hpp). The queued SPI
// master (atsam_common/Sercom_SPIQueued.hpp) needs kvasir::devices: included by name, as on the D21.

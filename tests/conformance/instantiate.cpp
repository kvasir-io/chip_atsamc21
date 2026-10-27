// The drivers' register writes, instantiated once each (one representative instance and configuration per driver
// template): an explicit instantiation compiles every member function, so every dependent apply() of the driver is
// checked against its register's write-only rules (Register::Semantics: keys, reads with side effects) as the
// conformance tree converts the SVD (guard error, registers derived, mask on). Nothing here is called; what needs a
// board's configuration to be instantiated at all is in not_instantiated.txt.

// the config header first: the chip below needs its log backend and log clock (uc_log.hpp would read it too, but the
// chip headers before it need the same)
#include "kvasir_config.hpp"

// then the chip
#include "chip/chip.hpp"

// then the drivers (after chip.hpp: they need its PM.hpp, GCLK.hpp and traits)
#include "chip/atsam_common/CAN.hpp"
#include "chip/atsam_common/DMAC.hpp"
#include "chip/atsam_common/EIC.hpp"
#include "chip/atsam_common/ImageCheck.hpp"
#include "chip/atsam_common/NVMCTRL.hpp"
#include "chip/atsam_common/SamPushButton.hpp"
#include "chip/atsam_common/SamQuadratureEncoder.hpp"
#include "chip/atsam_common/SamRotaryEncoder.hpp"
#include "chip/atsam_common/Sercom_I2CQueued.hpp"
#include "chip/atsam_common/Sercom_SPIQueued.hpp"
#include "chip/atsam_common/Sercom_Usart.hpp"

#include <chrono>
#include <cstdint>

namespace Conformance {
inline constexpr auto crystalSpeed = 8'000'000;

template<int Pin>
using PA = decltype(makePinLocation(Kvasir::Io::portA, Kvasir::Io::Pin<Pin>{}));

struct DmacConfig {
    static constexpr auto numberOfChannels = 2;
};

struct I2cConfig {
    static constexpr auto clockSpeed     = clockSpeedValue;
    static constexpr auto instance       = 0;
    static constexpr auto sdaPinLocation = PA<8>{};
    static constexpr auto sclPinLocation = PA<9>{};
    static constexpr auto baudRate       = 100'000;
    static constexpr auto isrPriority    = 3;
};

struct ButtonConfig {
    static constexpr auto useHit          = true;
    static constexpr auto useShortRelease = true;
    static constexpr auto useLong         = true;
    static constexpr auto useLongRelease  = true;
    static constexpr auto longPressTime   = std::chrono::milliseconds{700};
    static constexpr auto invert          = false;
    static constexpr auto eicPriority     = 2;
};

/// The debounced mode: handler() polls the pin; no EXTINT line, so no eicPriority.
struct DebouncedButtonConfig {
    static constexpr auto useHit          = true;
    static constexpr auto useShortRelease = true;
    static constexpr auto useLong         = true;
    static constexpr auto useLongRelease  = true;
    static constexpr auto longPressTime   = std::chrono::milliseconds{700};
    static constexpr auto debouncePress   = std::chrono::milliseconds{5};
    static constexpr auto debounceRelease = std::chrono::milliseconds{20};
};

struct RotaryConfig {
    static constexpr auto useAcceleration = false;
    static constexpr auto edge            = Kvasir::EIC::InterruptType::edgeFall;
    static constexpr auto eicPriority     = 2;
};

struct EncoderConfig {
    static constexpr auto eicPriority = 2;
};

inline void onEdge() {}

// the queued SPI master and the USART on the C21's SERCOM_SPIM / SERCOM_USART_INT (Sam::Package::SercomSpi/Usart)
struct SpiConfig {
    static constexpr auto clockSpeed      = clockSpeedValue;
    static constexpr auto instance        = 1;
    static constexpr auto misoPinLocation = PA<19>{};
    static constexpr auto mosiPinLocation = PA<16>{};
    static constexpr auto sclkPinLocation = PA<17>{};
    static constexpr auto csPinLocation   = Kvasir::Io::NotUsed<>{};
    static constexpr auto baudRate        = 1'000'000;
    static constexpr auto mode            = Kvasir::Hal::ClockMode::mode0;
    static constexpr auto isrPriority     = 1;
};

// the USART: transmit by DMA, receive interrupt into a queue (the selector's ISR half), on the chip package's
// SERCOM_USART register set (Sam::Package::SercomUsart)
struct UartConfig {
    static constexpr auto clockSpeed    = clockSpeedValue;
    static constexpr auto instance      = 3;
    static constexpr auto txPinLocation = PA<22>{};
    static constexpr auto rxPinLocation = PA<23>{};
    static constexpr auto baudRate      = 115'200;
    static constexpr auto dataBits      = Kvasir::Hal::Uart::DataBits::bits8;
    static constexpr auto parity        = Kvasir::Hal::Uart::Parity::none;
    static constexpr auto stopBits      = Kvasir::Hal::Uart::StopBits::stop1;
    static constexpr auto isrPriority   = 3;
};

using Dma = Kvasir::DMAC::DmaBase<DmacConfig>;

struct CanConfig {
    static constexpr auto clockSpeed    = crystalSpeed;
    static constexpr auto instance      = 0;
    static constexpr auto rxPinLocation = PA<25>{};
    static constexpr auto txPinLocation = PA<24>{};
    static constexpr auto baudRate      = 500'000;
    static constexpr auto isrPriority   = 3;
};

// the watchdog, gated (the health supervisor's) and ungated (a Startup entry)
struct WatchdogConfig {
    static constexpr auto overrunTime = std::chrono::seconds{2};
    static constexpr bool gatedFeed   = true;
};

struct UngatedWatchdogConfig {
    static constexpr auto overrunTime = std::chrono::seconds{1};
};
}   // namespace Conformance

template struct Kvasir::DMAC::DmaBase<Conformance::DmacConfig>;
// the DMAC's CRC engine (the image engine set DmaCrc::Dmac), once for each CRC it computes
template class Kvasir::DmaCrc::DmacEngine<Kvasir::Crc::Presets::crc32IsoHdlc>;
template class Kvasir::DmaCrc::DmacEngine<Kvasir::Crc::Presets::crc16Ibm3740>;
template class Kvasir::DmaCrc::DmacEngine<Kvasir::Crc::Presets::crc16Xmodem>;
template struct Kvasir::Sercom::I2C::
  I2CBehaviorQueued<Conformance::I2cConfig, Conformance::SystickClock, 2, 16>;
// I2CBehaviorQueued is its hardware half and the front (kvasir_devices I2C/Controller/QueuedFront.hpp), named here as SPIQueued's
template struct Kvasir::Sercom::I2C::Detail::
  I2CQueuedHalf<Conformance::I2cConfig, Conformance::SystickClock, 2, 16>;
template struct Kvasir::I2C::QueuedFront<
  Kvasir::Sercom::I2C::I2CBehaviorQueued<Conformance::I2cConfig, Conformance::SystickClock, 2, 16>::
    Core>;
template struct Kvasir::EIC::ExtInt<Conformance::PA<6>,
                                    Kvasir::EIC::InterruptType::edgeBoth,
                                    &Conformance::onEdge,
                                    2,
                                    Kvasir::Io::PullConfiguration::pullUp,
                                    false>;
template struct Kvasir::
  SamPushButton<Conformance::SystickClock, Conformance::PA<6>, 4, Conformance::ButtonConfig>;
template struct Kvasir::SamPushButton<Conformance::SystickClock,
                                      Conformance::PA<6>,
                                      4,
                                      Conformance::DebouncedButtonConfig>;
// a variable template: a requires-expression on a non-dependent type would be a hard error
template<typename B>
constexpr bool hasEicLine = requires { typename B::SubIsrs; };
static_assert(hasEicLine<Kvasir::SamPushButton<Conformance::SystickClock,
                                               Conformance::PA<6>,
                                               4,
                                               Conformance::ButtonConfig>>
                && !hasEicLine<Kvasir::SamPushButton<Conformance::SystickClock,
                                                     Conformance::PA<6>,
                                                     4,
                                                     Conformance::DebouncedButtonConfig>>,
              "an edge button has its EXTINT line, a debounced one none");
template struct Kvasir::SamRotaryEncoder<Conformance::SystickClock,
                                         Conformance::PA<6>,
                                         Conformance::PA<7>,
                                         std::uint32_t,
                                         Conformance::RotaryConfig>;
template struct Kvasir::SamQuadratureEncoder<Conformance::PA<6>,
                                             Conformance::PA<7>,
                                             std::int32_t,
                                             Conformance::EncoderConfig>;
template struct Kvasir::Nvm::Region<0x1'FE00, 2>;
template struct Kvasir::ImageCheck::SamDsu<>;
template struct Kvasir::CAN::CANBehavior<Conformance::CanConfig, Conformance::SystickClock>;
template struct Kvasir::Sercom::SPI::SPIQueued<Conformance::SpiConfig,
                                               Conformance::SystickClock,
                                               Conformance::Dma,
                                               Kvasir::DMAC::DMAChannel::ch0,
                                               Kvasir::DMAC::DMAChannel::ch1,
                                               Kvasir::DMAC::DMAPriority::p0,
                                               2>;
// SPIQueued is its hardware half and the front (kvasir_devices SPI/QueuedFront.hpp); an explicit instantiation of
// SPIQueued does not reach inherited members, so the half (Hw: the register writes) and the front are named here
template struct Kvasir::Sercom::SPI::Detail::SPIQueuedHalf<Conformance::SpiConfig,
                                                           Conformance::SystickClock,
                                                           Conformance::Dma,
                                                           Kvasir::DMAC::DMAChannel::ch0,
                                                           Kvasir::DMAC::DMAChannel::ch1,
                                                           Kvasir::DMAC::DMAPriority::p0,
                                                           2,
                                                           16,
                                                           Kvasir::SPI::QueueCoreDefaults>;
template struct Kvasir::SPI::QueuedFront<
  Kvasir::Sercom::SPI::SPIQueued<Conformance::SpiConfig,
                                 Conformance::SystickClock,
                                 Conformance::Dma,
                                 Kvasir::DMAC::DMAChannel::ch0,
                                 Kvasir::DMAC::DMAChannel::ch1,
                                 Kvasir::DMAC::DMAPriority::p0,
                                 2>::Core>;
// UartBehavior derives from its Impl: an explicit instantiation of it reaches no inherited member, so the Impl
// (transmit, timeout, repair) is named as well; UartBehavior itself carries the receive interrupt
template struct Kvasir::Sercom::Usart::UartBehaviorImpl<Conformance::UartConfig,
                                                        Conformance::SystickClock,
                                                        Conformance::Dma,
                                                        Kvasir::DMAC::DMAChannel::ch1,
                                                        Kvasir::DMAC::DMAPriority::p0,
                                                        8>;
template struct Kvasir::Sercom::Usart::UartBehavior<Conformance::UartConfig,
                                                    Conformance::SystickClock,
                                                    Conformance::Dma,
                                                    Kvasir::DMAC::DMAChannel::ch1,
                                                    Kvasir::DMAC::DMAPriority::p0,
                                                    8>;
template struct Kvasir::Watchdog<Conformance::WatchdogConfig>;
template struct Kvasir::Watchdog<Conformance::UngatedWatchdogConfig>;

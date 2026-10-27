// The chip contract of chip_atsamc21 as it is today (Kvasir_SDK tests/conformance/ChipContract.hpp): every row the chip
// has is REQUIRE, every row it lacks is GAP - a GAP that the chip fills fails here and moves to REQUIRE.

// the config header first: the chip below needs its log backend and log clock (uc_log.hpp would read it too, but the
// chip headers before it need the same)
#include "kvasir_config.hpp"

// then the chip and the drivers
#include "chip/WaitBounds.hpp"
#include "chip/atsam_common/ImageCheck.hpp"
#include "chip/atsam_common/NVMCTRL.hpp"
#include "chip/atsam_common/Serial_Number.hpp"
#include "chip/chip.hpp"
// the core's debug query and feature macros
#include "core/Features.hpp"
#include "cortex_common/Debug.hpp"

// last: the contract rows see only what is declared before them
#include "ChipContract.hpp"

namespace Conformance {
// the last two rows of the smallest part's main array (ATSAMC21E17A: 128 KB, 256-byte rows)
using Region = Kvasir::Nvm::Region<0x1'FE00, 2>;

// the watchdog, gated (the health supervisor's) and ungated (a Startup entry)
struct WatchdogConfig {
    static constexpr auto overrunTime = std::chrono::seconds{2};
    static constexpr bool gatedFeed   = true;
};

struct UngatedWatchdogConfig {
    static constexpr auto overrunTime = std::chrono::seconds{1};
};
}   // namespace Conformance

// Startup hooks the SDK's Startup calls
KVASIR_CONFORMANCE_REQUIRE(
  "Startup::FirstInitStep<Tag::User>",
  requires { Kvasir::Startup::FirstInitStep<Kvasir::Tag::User>{}(); });

// the interrupt table and the pins
KVASIR_CONFORMANCE_REQUIRE("Nvic::InterruptOffsetTraits<Tag::User>::end",
                           Kvasir::Nvic::InterruptOffsetTraits<Kvasir::Tag::User>::end > 0);
KVASIR_CONFORMANCE_REQUIRE("Io::PinLocationTraits<void>",
                           Kvasir::Io::PinLocationTraits<void>::pinEnd > 0);

// reset cause, wait bounds
KVASIR_CONFORMANCE_REQUIRE("PM::resetCause()",
                           Kvasir::Conformance::Row::resetCause);
KVASIR_CONFORMANCE_REQUIRE("PM::resetKind()",
                           Kvasir::Conformance::Row::resetKind);
KVASIR_CONFORMANCE_REQUIRE("Chip::part / partName, the identity constants",
                           Kvasir::Conformance::Row::chipPart);
KVASIR_CONFORMANCE_REQUIRE("Register::CpuHzCeiling<Tag::User>",
                           Kvasir::Conformance::Row::cpuHzCeiling);
// every name the shared drivers use (atsam_common/ChipTraits.hpp; StartUp.hpp asserts the same)
KVASIR_CONFORMANCE_REQUIRE("Sam::ChipPackage<Sam::Package>",
                           Kvasir::Sam::ChipPackage<Kvasir::Sam::Package>);
// the CellStore's flash, the image check's backend
KVASIR_CONFORMANCE_REQUIRE("Hal::NorRegion (Nvm::Region)",
                           Kvasir::Hal::NorRegion<Conformance::Region>);
KVASIR_CONFORMANCE_REQUIRE("ImageCheck::Backend (ImageCheck::SamDsu)",
                           Kvasir::ImageCheck::Backend<Kvasir::ImageCheck::SamDsu<>>);
// known gaps (each moves to REQUIRE when the chip gets it)
// a reboot every chip spells the same, "was a debugger seen" (DSU STATUSB.DBGPRES)
KVASIR_CONFORMANCE_REQUIRE("Chip::reboot()",
                           Kvasir::Conformance::Row::chipReboot);
KVASIR_CONFORMANCE_REQUIRE(
  "Port::Reboot<Tag::User>, the reboot hook",
  requires { Kvasir::Port::Reboot<Kvasir::Tag::User>{}(); });
KVASIR_CONFORMANCE_REQUIRE("Chip::debuggerPresent()",
                           Kvasir::Conformance::Row::chipDebuggerPresent);
// the health supervisor's watchdog (atsam_common/Wdt.hpp)
KVASIR_CONFORMANCE_REQUIRE("Kvasir::Watchdog",
                           Kvasir::Conformance::Row::kvasirWatchdog);
// the serial number, as bytes and as hex text (every chip's two functions)
KVASIR_CONFORMANCE_REQUIRE("Kvasir::serialNumber() / serialNumberString()",
                           Kvasir::Conformance::Row::serialNumber);
KVASIR_CONFORMANCE_REQUIRE(
  "health watchdog (gated feed/arm/disarm, Timeout)",
  Kvasir::Conformance::HealthWatchdog<Kvasir::Watchdog<Conformance::WatchdogConfig>>);
KVASIR_CONFORMANCE_REQUIRE(
  "health watchdog, ungated (feed, Timeout)",
  Kvasir::Health::WatchdogShape<Kvasir::Watchdog<Conformance::UngatedWatchdogConfig>>);
// the M0+ cannot read DHCSR itself (its cortex_common/Debug.hpp has haltingDebug() only, `unknown`)
KVASIR_CORE_CONFORMANCE_GAP("Core::Debug::isDebuggerConnected()",
                            Kvasir::Conformance::Row::coreDebugIsDebuggerConnected);
// "is halting debug on": off / on / unknown on every core (cortex_common/Debug.hpp)
KVASIR_CORE_CONFORMANCE_REQUIRE("Core::haltingDebug()",
                                Kvasir::Conformance::Row::coreHaltingDebug);

// The clock gates (chip/Blocks.hpp over atsam_common/ClockGate.hpp), as the drivers and the gate check see them. A
// block on two clock domains has a row per mask bit (DS60001479M 17.6.2.6): DSU AHBMASK bit 3 and APBBMASK bit 1,
// NVMCTRL AHBMASK bit 5 and APBBMASK bit 2 (17.8.6, 17.8.8), and enable<> sets both; a block with one gate keeps the
// bare action. The gate id is the bit counted from MCLK's base: AHBMASK (0x10) bit 3 = 0x10 * 8 + 3.
namespace Conformance { namespace Gates {
    namespace P = Kvasir::Peripheral;
    using M     = P::MCLK::Registers<>;
    using Table = Kvasir::Chip::Blocks::Table;
    static_assert(std::is_same_v<Kvasir::PM::Enable<P::DSU::Registers<>::baseAddr>::action,
                                 brigand::list<decltype(set(M::AHBMASK::dsu_)),
                                               decltype(set(M::APBBMASK::dsu_))>>);
    static_assert(std::is_same_v<Kvasir::PM::Enable<P::NVMCTRL::Registers<>::baseAddr>::action,
                                 brigand::list<decltype(set(M::AHBMASK::nvmctrl_)),
                                               decltype(set(M::APBBMASK::nvmctrl_))>>);
    static_assert(std::is_same_v<Kvasir::PM::Enable<P::SERCOM_I2CM::Registers<0>::baseAddr>::action,
                                 decltype(set(M::APBCMASK::sercom0_))>);
    static_assert(
      std::is_same_v<Kvasir::PM::Disable<P::SERCOM_I2CM::Registers<0>::baseAddr>::action,
                     decltype(clear(M::APBCMASK::sercom0_))>);
    static_assert(std::is_same_v<Kvasir::PM::Enable<P::PORT::Registers<>::baseAddr>::action,
                                 decltype(set(M::APBBMASK::port_))>);
    static_assert(Table::gateAt(P::DSU::Registers<>::baseAddr) == 0x10 * 8 + 3);
    // claimed: APBCMASK resets to 0 (17.8.9); not claimed: PORT and the DSU are clocked out of reset (17.8.8)
    static_assert(Table::claimsAt(P::SERCOM_I2CM::Registers<0>::baseAddr) == 1);
    static_assert(Table::claimsAt(P::PORT::Registers<>::baseAddr) == 0
                  && Table::claimsAt(P::DSU::Registers<>::baseAddr) == 0);
#ifndef KVASIR_CHIP_ATSAMC21G17A
    // the E17A has no SERCOM4 (0x42001400): no row, so PM::enable of it does not compile
    static_assert(Table::rowsOf(0x42001400) == 0);
#endif
}}   // namespace Conformance::Gates

// the indexed GCLK generator (atsam_common/GclkIndexed.hpp): DIV is 16 bits on generator 1, 8 on the others; above
// that a power of two goes through DIVSEL (DIV = log2 - 1, 16.8.3 / 14.8.3)
namespace Conformance::Gclk {
using Kvasir::GCLK::GeneratorSource;
using Kvasir::GCLK::GenericClockGenerator;
constexpr auto src = GeneratorSource::gclkin;
static_assert(Kvasir::Sam::Gclk::directMaximum(Kvasir::ClockLimits::C21::gclkDivBits(0)) == 255
              && Kvasir::Sam::Gclk::directMaximum(Kvasir::ClockLimits::C21::gclkDivBits(1))
                   == 65535);
static_assert(GenericClockGenerator<2,
                                    src,
                                    255>::division.div
                == 255
              && GenericClockGenerator<2,
                                       src,
                                       256>::division.div
                   == 7);
static_assert(GenericClockGenerator<2,
                                    src,
                                    (1ULL << 20)>::division.div
              == 19);
static_assert(GenericClockGenerator<1,
                                    src,
                                    65535>::division.div
                == 65535
              && GenericClockGenerator<1,
                                       src,
                                       (1ULL << 16)>::division.div
                   == 15);
static_assert(GenericClockGenerator<2,
                                    src,
                                    512>::divisionInTable
              && !GenericClockGenerator<2,
                                        src,
                                        1024>::divisionInTable);
}   // namespace Conformance::Gclk

// the clock tree (chip/ClockTree.hpp): XOSC 8 MHz -> DPLL96M at 48 MHz: DIV 3 (1 MHz), LDR 47, GCLK0 / 1, 2 wait
// states at 2.7 V (Table 45-41: 1 only to 38 MHz); a 64 MHz part runs 64 MHz from the same crystal
namespace Conformance::ClockTree {
namespace CS = Kvasir::Clocks::Sam;
using Fdpll  = CS::Tree<{.cpu = 48'000'000, .source = {.hz = 8'000'000}}>;
static_assert(Fdpll::dpll.div == 3 && Fdpll::dpll.ldr == 47 && Fdpll::dpll.gclkDiv == 1
              && Fdpll::waitStates == 2);
using Fast = CS::Tree<{.cpu    = 64'000'000,
                       .source = {.hz = 8'000'000},
                       .speed  = Kvasir::ClockLimits::C21::Speed::mhz64}>;
static_assert(Fast::waitStates >= Fdpll::waitStates);
}   // namespace Conformance::ClockTree

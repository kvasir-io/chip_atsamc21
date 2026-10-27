#pragma once
// The SAM C21's crystal oscillator, DPLL and flash wait states as building blocks: the numbers come from the solver
// (atsam_common/ClockSolver.hpp on ClockLimits.hpp), the register writes are these.
//
//     constexpr auto dpll = Kvasir::DPLL::fromXosc<8'000'000, 48'000'000>();   // DIV 3, LDR 47
//     apply(Kvasir::XOSC::configure<8'000'000>());
//     while(!Kvasir::XOSC::ready()) {}
//     apply(Kvasir::DPLL::configure<dpll>());
//     apply(Kvasir::Nvm::setWaitStates<Kvasir::Nvm::waitStates<48'000'000, Supply::from2V7>()>());
//     while(!Kvasir::DPLL::ready()) {}

#include "ClockLimits.hpp"
#include "atsam_common/Fdpll.hpp"
#include "atsam_common/Xosc.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/NVMCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <cstdint>

namespace Kvasir { namespace XOSC {
    struct Options {
        bool crystal    = true;   // XTALEN: a crystal between XIN and XOUT, else a clock on XIN
        bool ampgc      = true;   // automatic amplitude gain control
        bool runStandby = true;
        std::uint16_t startup
          = 6;   // XOSCCTRL.STARTUP 0x6: 64 OSCULP32K cycles, ~1953 us (Table 20-5, md line 9480)
    };

    // XOSCCTRL.GAIN: the "recommended max frequency" per setting, 2 / 4 / 8 / 16 / 32 MHz (SAM C20/C21
    // DS60001479M 20.8.5 "Bits 10:8 - GAIN", md line 9520)
    consteval std::uint16_t gainFor(std::uint64_t hz) {
        if(hz <= 2'000'000) { return 0; }
        if(hz <= 4'000'000) { return 1; }
        if(hz <= 8'000'000) { return 2; }
        if(hz <= 16'000'000) { return 3; }
        return 4;
    }

    struct GainTable {
        static consteval std::uint16_t gainFor(std::uint64_t hz) { return XOSC::gainFor(hz); }
    };

    using Impl = Sam::Xosc<Kvasir::Peripheral::OSCCTRL::Registers<>::XOSCCTRL,
                           Kvasir::Peripheral::OSCCTRL::Registers<>::STATUS::xoscrdy,
                           GainTable>;

    // XOSC enabled, ONDEMAND off (it runs whether or not a generator asks for it)
    template<std::uint64_t Hz,
             Options       O = {}>
    [[nodiscard]] constexpr auto configure() {
        // crystal 0.4..32 MHz (Table 45-47, md line 51128); XIN clock up to 48 MHz (Table 45-46, line 51088)
        static_assert((O.crystal ? Hz <= 32'000'000 && Hz >= 400'000 : Hz <= 48'000'000),
                      "SAM C21 XOSC: a crystal of 0.4..32 MHz (Table 45-47) or an XIN clock up to "
                      "48 MHz (Table 45-46)");
        return Impl::configure<Hz, O>();
    }

    [[nodiscard]] inline bool ready() { return Impl::ready(); }
}}   // namespace Kvasir::XOSC

namespace Kvasir { namespace DPLL {
    using Impl      = Sam::Fdpll96m<Kvasir::Peripheral::OSCCTRL::Registers<>, /*HasPresc*/ true>;
    using Reference = Impl::Reference;

    struct Options {
        // LBYPASS: below 25 C "spurious DPLL unlocks ... the DPLL output clock is halted and then
        // restarts"; workaround LBYPASS = 1 before ENABLE (errata DS80000740T 1.25.1, md line 3613,
        // PDF p. 56, every E/G/J revision B..H). On by default, as on the D21.
        bool lockBypass = true;
        bool runStandby = false;
    };

    template<std::uint64_t XoscHz,
             std::uint64_t Hz,
             std::uint64_t TolerancePpm = 0,
             bool          Fractional   = false>
    consteval Setting fromXosc() {
        return solveXosc<ClockLimits::C21::fdpll96m,
                         ClockLimits::C21::dpllWhere,
                         XoscHz,
                         Hz,
                         TolerancePpm,
                         Fractional>();
    }

    // DPLLRATIO, DPLLCTRLB, the output prescaler when it divides, then DPLLCTRLA with ENABLE and
    // ONDEMAND off; the caller waits for ready() before a generator takes the clock.
    template<Setting   S,
             Reference R = Reference::xosc,
             Options   O = {}>
    [[nodiscard]] constexpr auto configure() {
        return Impl::configure<S, R, O>();
    }

    [[nodiscard]] inline bool ready() { return Impl::ready(); }
}}   // namespace Kvasir::DPLL

namespace Kvasir { namespace Nvm {
    using ClockLimits::Supply;
    using Rating = ClockLimits::C21::Rating;
    using ClockLimits::C21::waitStates;

    // NVMCTRL.CTRLB.RWS = Ws, the rest of CTRLB at its reset values (the C21 chip.svd has them:
    // MANW = 1). 4 bits (27.8.2, md line 19544); raised before the AHB clock goes up (27.5.2, line
    // 19012).
    template<unsigned Ws>
    [[nodiscard]] constexpr auto setWaitStates() {
        static_assert(Ws <= 15, "NVMCTRL.CTRLB.RWS is 4 bits");
        using KNR = Kvasir::Peripheral::NVMCTRL::Registers<>;
        return KNR::CTRLB::overrideDefaults(write(KNR::CTRLB::rws, Kvasir::Register::value<Ws>()));
    }
}}   // namespace Kvasir::Nvm

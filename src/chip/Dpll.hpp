#pragma once
// The SAM C21's DPLL and flash wait states as building blocks: the numbers come from the solver
// (atsam_common/ClockSolver.hpp on ClockLimits.hpp), the register writes are these.
//
//     constexpr auto dpll = Kvasir::DPLL::fromXosc<8'000'000, 48'000'000>();   // DIV 3, LDR 47
//     apply(Kvasir::DPLL::configure<dpll>());
//     apply(Kvasir::Nvm::setWaitStates<Kvasir::Nvm::waitStates<48'000'000, Supply::from2V7>()>());
//     while(!Kvasir::DPLL::ready()) {}

#include "ClockLimits.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/NVMCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <cstdint>

namespace Kvasir { namespace DPLL {
    // DPLLCTRLB.REFCLK (20.8.14): XOSC32K, XOSC (through DIV), GCLK_DPLL
    enum class Reference : unsigned { xosc32k = 0, xosc = 1, gclk = 2 };

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
        return solveXosc<ClockLimits::C21::Fdpll96m,
                         ClockLimits::C21::DpllWhere,
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
        static_assert(S.found, "DPLL::configure: a setting the solver did not find");
        static_assert(R == Reference::xosc || S.div == 0, "DIV only divides the XOSC reference");
        using OSC = Kvasir::Peripheral::OSCCTRL::Registers<>;
        using Kvasir::Register::value;
        using RV         = typename OSC::DPLLCTRLB::REFCLKVal;
        using PV         = typename OSC::DPLLPRESC::PRESCVal;
        auto const ratio = list(write(OSC::DPLLRATIO::ldr, value<S.ldr>()),
                                write(OSC::DPLLRATIO::ldrfrac, value<S.ldrFrac>()));
        auto const ctrlb = [] {
            if constexpr(O.lockBypass) {
                return OSC::DPLLCTRLB::overrideDefaults(
                  write(OSC::DPLLCTRLB::div, value<S.div>()),
                  set(OSC::DPLLCTRLB::lbypass),
                  write(OSC::DPLLCTRLB::refclk, value<RV, static_cast<RV>(R)>()));
            } else {
                return OSC::DPLLCTRLB::overrideDefaults(
                  write(OSC::DPLLCTRLB::div, value<S.div>()),
                  write(OSC::DPLLCTRLB::refclk, value<RV, static_cast<RV>(R)>()));
            }
        }();
        auto const ctrla = [] {
            if constexpr(O.runStandby) {
                return OSC::DPLLCTRLA::overrideDefaults(set(OSC::DPLLCTRLA::enable),
                                                        set(OSC::DPLLCTRLA::runstdby),
                                                        clear(OSC::DPLLCTRLA::ondemand));
            } else {
                return OSC::DPLLCTRLA::overrideDefaults(set(OSC::DPLLCTRLA::enable),
                                                        clear(OSC::DPLLCTRLA::ondemand));
            }
        }();
        if constexpr(S.presc == 0) {
            return list(ratio, ctrlb, Kvasir::Register::sequencePoint, ctrla);
        } else {
            return list(ratio,
                        ctrlb,
                        write(OSC::DPLLPRESC::presc, value<PV, static_cast<PV>(S.presc)>()),
                        Kvasir::Register::sequencePoint,
                        ctrla);
        }
    }

    [[nodiscard]] inline bool ready() {
        using OSC = Kvasir::Peripheral::OSCCTRL::Registers<>;
        return apply(read(OSC::DPLLSTATUS::clkrdy));
    }
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

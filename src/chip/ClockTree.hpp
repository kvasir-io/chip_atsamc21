#pragma once
// The SAM C21's core clock from a target (atsam_common/ClockTree.hpp's skeleton over this chip's builders): crystal
// (or a clock on XIN) -> XOSC -> DPLL96M -> generator 0, the flash wait states from Table 45-41, f_CPU against the
// part's speed grade, every wait bounded (WaitBounds.hpp).
//
//     struct ClockSettings {
//         using Tree = Kvasir::Clocks::Sam::Tree<{.cpu = 48'000'000, .source = {.hz = 8'000'000}}>;
//         using Provides = brigand::append<Tree::Provides, brigand::list<... the SERCOM/CAN clocks routed ...>>;
//         static void coreClockInit() { Tree::coreClockInit(); }
//     };
//     static_assert(ClockSettings::Tree::dpll.div == 3 && ClockSettings::Tree::dpll.ldr == 47
//                   && ClockSettings::Tree::waitStates == 2);

#include "ClockLimits.hpp"
#include "Dpll.hpp"
#include "GCLK.hpp"
#include "WaitBounds.hpp"
#include "atsam_common/ClockTree.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"

#include <cstdint>

namespace Kvasir { namespace Clocks { namespace Sam {
    struct TreeConfig {
        std::uint64_t            cpu{};
        Source                   kind = Source::xoscFdpll;
        Xosc                     source{};
        ClockLimits::Supply      supply = ClockLimits::Supply::from2V7;    // board fact
        ClockLimits::C21::Speed  speed = ClockLimits::C21::Speed::mhz48;   // the part's speed grade
        ClockLimits::C21::Rating rating         = ClockLimits::C21::Rating::egj85;
        std::uint64_t            tolerancePpm   = 0;
        bool                     dpllRunStandby = false;
        bool                     dpllLockBypass = true;   // errata 1.25.1 (Dpll.hpp)
        XOSC::Options            xosc           = {};
    };

    template<TreeConfig C, Source = C.kind>
    struct C21Kind;

    // XOSC -> DPLL96M (solver: lowest DCO, integer mode) -> RWS for the new clock -> generator 0. Oscillator up,
    // DPLL running (CLKRDY), then RWS and the switch: RWS must be raised before the AHB clock goes up (27.5.2), which
    // the switch does - the hand-written trees raised it before CLKRDY, which is legal too.
    template<TreeConfig C>
    struct C21Kind<C, Source::xoscFdpll> {
        static constexpr DPLL::Setting dpll = DPLL::fromXosc<C.source.hz, C.cpu, C.tolerancePpm>();

        static constexpr XOSC::Options xoscOptions = [] {
            auto o    = C.xosc;
            o.crystal = C.source.crystal;
            return o;
        }();

        using ExtraProvides = brigand::list<>;

        template<unsigned WaitStates>
        static void start() {
            apply(XOSC::configure<C.source.hz, xoscOptions>());
            Kvasir::Register::waitUntil<Kvasir::Chip::Sam::XoscReadyBound<xoscOptions.startup>>(
              [] { return XOSC::ready(); });

            apply(DPLL::configure<dpll,
                                  DPLL::Reference::xosc,
                                  DPLL::Options{.lockBypass = C.dpllLockBypass,
                                                .runStandby = C.dpllRunStandby}>());
            Kvasir::Register::waitUntil<Kvasir::Chip::DpllLockBound>([] { return DPLL::ready(); });

            // RWS first, then the switch: two apply() calls, so the order is the source's (one apply orders its writes
            // by address, Register/Apply.hpp; NVMCTRL 0x41004004 above GCLK 0x40001C20 made it the same)
            apply(Nvm::setWaitStates<WaitStates>());
            apply(GCLK::GenericClockGenerator<0, GCLK::GeneratorSource::dpll96m, dpll.gclkDiv>::
                    enable());
        }
    };

    struct C21Builders {
        using Config = TreeConfig;

        static consteval bool has(Source s) { return s == Source::xoscFdpll; }

        // Table 45-8 (64 MHz parts) / 45-9 (48 MHz parts)
        static consteval std::uint64_t cpuMax(TreeConfig const& c) {
            return ClockLimits::C21::cpuMax(c.speed);
        }

        template<TreeConfig C>
        static consteval unsigned waitStates() {
            return Nvm::waitStates<C.cpu, C.supply, C.rating>();
        }

        template<TreeConfig C>
        using Kind = C21Kind<C>;
    };

    template<TreeConfig C>
    using Tree = TreeOf<C21Builders, C>;
}}}   // namespace Kvasir::Clocks::Sam

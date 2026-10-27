#pragma once
// The SAM C21's start traits for atsam_common/StartUp.hpp's FirstInitStep: whether the chip looks as a reset leaves it
// (restartIfNotFromReset() restarts an image at address 0 that was jumped into), and GCLK0 parked on the clock a reset
// gives it before the clock tree is rewritten.

#include "GCLK.hpp"

namespace Kvasir { namespace Startup { namespace Sam {
    struct StartTraits {
        // Whether the chip looks as a reset leaves it: no APBC bus clock (MCLK.APBCMASK resets to 0, 17.8.9) and GCLK0
        // on OSC48M (7.3.1). Any image that used a SERCOM, TC, TCC, ADC, DAC, AC or CAN... on bridge C, or ran GCLK0 on
        // another source, leaves a difference behind - and a jump resets neither (SysResetReq and WDT reset all but the
        // RTC, OSC32KCTRL, RSTC and WRTLOCKed GCLKs, Table 18-1).
        static bool looksLikeAfterReset() {
            using GCLK::GeneratorSource;
            using GC        = Kvasir::Peripheral::GCLK::Registers<>;
            auto const apbc = Kvasir::Register::readWhole(
              Kvasir::Peripheral::MCLK::Registers<>::APBCMASK::FULLREGISTER);
            GeneratorSource const source = get<0>(apply(read(GC::GENCTRL<0>::src)));
            return apbc == 0U && source == GeneratorSource::osc48m;
        }

        // GCLK0 (the CPU's clock) onto OSC48M before the clock tree is rewritten: an image a bootloader jumps into,
        // or one that jumps into itself, may find GCLK0 on the DPLL or the XOSC the firmware's tree is about to
        // reprogram (reference switching the DPLL means disabling it, 20.6.5.1.2). After a real reset GCLK0 is on
        // OSC48M already (7.3.1) and this only reads. "Before switching the Generic Clock Generator 0 (GCLKGEN0) from
        // a clock source A to another clock source B, enable the ONDEMAND feature of the clock source A" (16.6.2.6):
        // bit 7 of XOSCCTRL, DPLLCTRLA (20.8.5, 20.8.12) and of OSC32KCTRL's XOSC32K, OSC32K. DPLLCTRLA is
        // "Write-Synchronized (ENABLE)" only (DS60001479M 20.8.12, md l.9811) and DPLLSYNCBUSY.ENABLE "is set when the
        // DPLLCTRLA.ENABLE bit is modified" (20.6.5.1.1, md l.8836): the ONDEMAND write needs no wait. OSC48M is enabled at reset (20.6.4); its divider (OSC48MDIV) is left as found - the wait
        // states the image before set for it stay too. GENCTRL0.SRC = 0x06 OSC48M (16.8.3), on the fly: the
        // generator stays on A until B is ready (16.6.2.6); GENCTRLn is write-synchronized, SYNCBUSY.GENCTRL0 (16.6.6).
        // OSC48MCTRL.ONDEMAND resets to 1 (20.8.8, reset 0x82): with nobody requesting OSC48M "the oscillator will be in
        // a disabled state" and STATUS.OSC48MRDY stays 0 - waiting for it with GCLK0 elsewhere waits for ever (seen on
        // the D21's OSC8M). ONDEMAND off for the switch, back on once GCLK0 requests it.
        static void parkMainClock() {
            using GCLK::GeneratorSource;
            using GC                     = Kvasir::Peripheral::GCLK::Registers<>;
            using Osc                    = Kvasir::Peripheral::OSCCTRL::Registers<>;
            using Osc32                  = Kvasir::Peripheral::OSC32KCTRL::Registers<>;
            GeneratorSource const source = get<0>(apply(read(GC::GENCTRL<0>::src)));
            if(source == GeneratorSource::osc48m) { return; }
            switch(source) {
            case GeneratorSource::xosc:    apply(set(Osc::XOSCCTRL::ondemand)); break;
            case GeneratorSource::xosc32k: apply(set(Osc32::XOSC32K::ondemand)); break;
            case GeneratorSource::osc32k:  apply(set(Osc32::OSC32K::ondemand)); break;
            case GeneratorSource::dpll96m: apply(set(Osc::DPLLCTRLA::ondemand)); break;
            default:                       break;   // GCLKIN, GCLKGEN1, OSCULP32K: no ONDEMAND
            }
            apply(set(Osc::OSC48MCTRL::enable), clear(Osc::OSC48MCTRL::ondemand));
            // tSTART max 15 us (Table 45-57)
            Kvasir::Register::waitUntil<Kvasir::Chip::Sam::SyncBound>(
              Kvasir::Register::isSet(Osc::STATUS::osc48mrdy));
            apply(GCLK::GenericClockGenerator<0, GeneratorSource::osc48m, 1>::enable());
            // SYNCBUSY.GENCTRL0 (16.8.2): the SVD gives the one-bit field an enum, so it is compared as a number
            Kvasir::Register::waitUntil<Kvasir::Chip::Sam::SyncBound>([] {
                return static_cast<std::uint32_t>(get<0>(apply(read(GC::SYNCBUSY::genctrl0)))) == 0;
            });
            apply(set(Osc::OSC48MCTRL::ondemand));
        }
    };
}}}   // namespace Kvasir::Startup::Sam

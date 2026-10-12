#pragma once

#include "ClockLimits.hpp"
#include "atsam_common/WaitBounds.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/GCLK.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/OSC32KCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <bit>

namespace Kvasir { namespace GCLK {

    enum class GeneratorSource : unsigned {
        xosc      = 0x00,
        gclkin    = 0x01,
        gclkgen1  = 0x02,
        osculp32k = 0x03,
        osc32k    = 0x04,
        xosc32k   = 0x05,
        osc48m    = 0x06,
        dpll96m   = 0x07
    };

    enum class Peripheral : unsigned {
        dpll              = 0,
        dpll_32k          = 1,
        eic               = 2,
        freqm_msr         = 3,
        freqm_ref         = 4,
        tsens             = 5,
        evsys_ch0         = 6,
        evsys_ch1         = 7,
        evsys_ch2         = 8,
        evsys_ch3         = 9,
        evsys_ch4         = 10,
        evsys_ch5         = 11,
        evsys_ch6         = 12,
        evsys_ch7         = 13,
        evsys_ch8         = 14,
        evsys_ch9         = 15,
        evsys_ch10        = 16,
        evsys_ch11        = 17,
        sercom_01234_slow = 18,
        sercom0_core      = 19,
        sercom1_core      = 20,
        sercom2_core      = 21,
        sercom3_core      = 22,
        sercom4_core      = 23,
        sercom5_slow      = 24,
        sercom5_core      = 25,
        can0              = 26,
        can1              = 27,
        tcc0_tcc1         = 28,
        tcc2              = 29,
        tc0_tc1           = 30,
        tc2_tc3           = 31,
        tc4               = 32,
        adc0              = 33,
        adc1              = 34,
        sdadc             = 35,
        dac               = 36,
        ptc               = 37,
        ccl               = 38,
        ac                = 40,
        tc5               = 43,
        tc6               = 44,
        tc7               = 45
    };

    namespace detail {
        // A warning, not an error: what the silicon does with a division above the maximum is
        // unknown (SAM C21 DS60001479M Table 16-3, md line 6389, gives the bits only; 2^(bits + 1)
        // assumed as on the D21 and E5x). The note names Generator and Div.
        template<unsigned           Generator,
                 unsigned long long Div>
        [[deprecated(
          "GCLK generator division above the datasheet's Maximum Division Factor; "
          "unmeasured what the part does")]]
        constexpr bool divisionAboveTableMaximum() {
            return true;
        }
    }   // namespace detail

    template<unsigned Generator, GeneratorSource Source, unsigned long long Div>
    struct GenericClockGenerator {
        using GC = Kvasir::Peripheral::GCLK::Registers<>::GENCTRL<Generator>;

        static constexpr unsigned maxDiv = (Generator == 1 ? (1 << 16) : (1 << 8)) - 1;

        static_assert((Div <= maxDiv) || (std::popcount(Div) == 1),
                      "must be power of 2 or lower then maxDiv");

        static constexpr auto divsel = []() {
            if constexpr(Div > maxDiv) {
                return GC::DIVSELValC::div2;
            } else {
                return GC::DIVSELValC::div1;
            }
        }();
        static constexpr auto div = []() {
            if constexpr(Div > maxDiv) {
                return std::countr_zero(Div) - 1;
            } else {
                return Div;
            }
        }();

        static constexpr bool divisionInTable = Div <= ClockLimits::C21::gclkMaxDivision(Generator);

        [[nodiscard]] static constexpr auto enable() {
            // Opt-in (-DKVASIR_WARN_GCLK_DIVISION) so firmwares stay warning-free; divisionInTable
            // stays for a check of your own.
#if defined(KVASIR_WARN_GCLK_DIVISION)
            if constexpr(!divisionInTable) {
                [[maybe_unused]] constexpr bool warned
                  = detail::divisionAboveTableMaximum<Generator, Div>();
            }
#endif
            return list(Register::overrideDefaults<typename GC::default_values>::value(
              set(GC::genen),
              write(divsel),
              set(GC::runstdby),
              write(GC::div, Register::value<div>()),
              write(
                GC::src,
                Register::value<typename GC::SRCVal, static_cast<typename GC::SRCVal>(Source)>())));
        }
    };

    template<unsigned Generator, Peripheral peripheral>
    struct PeripheralChannelController {
        using PC
          = Kvasir::Peripheral::GCLK::Registers<>::PCHCTRL<static_cast<unsigned>(peripheral)>;

        [[nodiscard]] static constexpr auto enable() {
            return list(Register::overrideDefaults<typename PC::default_values>::value(
              set(PC::chen),
              write(PC::gen,
                    Register::value<typename PC::GENVal,
                                    static_cast<typename PC::GENVal>(Generator)>())));
        }
    };

    // Whether the chip looks as a reset leaves it: no APBC bus clock (MCLK.APBCMASK resets to 0, 17.8.9) and GCLK0
    // on OSC48M (7.3.1). Any image that used a SERCOM, TC, TCC, ADC, DAC, AC or CAN... on bridge C, or ran GCLK0 on
    // another source, leaves a difference behind - and a jump resets neither (SysResetReq and WDT reset all but the
    // RTC, OSC32KCTRL, RSTC and WRTLOCKed GCLKs, Table 18-1).
    inline bool looksLikeAfterReset() {
        using GC        = Kvasir::Peripheral::GCLK::Registers<>;
        auto const apbc = *reinterpret_cast<std::uint32_t const volatile*>(
          Kvasir::Peripheral::MCLK::Registers<>::APBCMASK::Addr::value);
        auto const source = static_cast<GeneratorSource>(
          static_cast<unsigned>(get<0>(apply(read(GC::GENCTRL<0>::src)))));
        return apbc == 0U && source == GeneratorSource::osc48m;
    }

    // GCLK0 (the CPU's clock) onto OSC48M before the clock tree is rewritten: an image a bootloader jumps into,
    // or one that jumps into itself, may find GCLK0 on the DPLL or the XOSC the firmware's tree is about to
    // reprogram (reference switching the DPLL means disabling it, 20.6.5.1.2). After a real reset GCLK0 is on
    // OSC48M already (7.3.1) and this only reads. "Before switching the Generic Clock Generator 0 (GCLKGEN0) from
    // a clock source A to another clock source B, enable the ONDEMAND feature of the clock source A" (16.6.2.6):
    // bit 7 of XOSCCTRL, DPLLCTRLA (20.8.5, 20.8.12; DPLLCTRLA is write-synchronized, 20.8.16) and of OSC32KCTRL's
    // XOSC32K, OSC32K. OSC48M is enabled at reset (20.6.4); its divider (OSC48MDIV) is left as found - the wait
    // states the image before set for it stay too. GENCTRL0.SRC = 0x06 OSC48M (16.8.3), on the fly: the
    // generator stays on A until B is ready (16.6.2.6); GENCTRLn is write-synchronized, SYNCBUSY.GENCTRL0 (16.6.6).
    // OSC48MCTRL.ONDEMAND resets to 1 (20.8.8, reset 0x82): with nobody requesting OSC48M "the oscillator will be in
    // a disabled state" and STATUS.OSC48MRDY stays 0 - waiting for it with GCLK0 elsewhere waits for ever (seen on
    // the D21's OSC8M). ONDEMAND off for the switch, back on once GCLK0 requests it.
    inline void parkMainClock() {
        using GC          = Kvasir::Peripheral::GCLK::Registers<>;
        using Osc         = Kvasir::Peripheral::OSCCTRL::Registers<>;
        using Osc32       = Kvasir::Peripheral::OSC32KCTRL::Registers<>;
        auto const source = static_cast<GeneratorSource>(
          static_cast<unsigned>(get<0>(apply(read(GC::GENCTRL<0>::src)))));
        if(source == GeneratorSource::osc48m) { return; }
        switch(source) {
        case GeneratorSource::xosc:    apply(set(Osc::XOSCCTRL::ondemand)); break;
        case GeneratorSource::xosc32k: apply(set(Osc32::XOSC32K::ondemand)); break;
        case GeneratorSource::osc32k:  apply(set(Osc32::OSC32K::ondemand)); break;
        case GeneratorSource::dpll96m:
            apply(set(Osc::DPLLCTRLA::ondemand));
            Kvasir::Register::waitUntil<Kvasir::Chip::Sam::SyncBound>(
              Kvasir::Register::isClear(Osc::DPLLSYNCBUSY::enable));
            break;
        default: break;   // GCLKIN, GCLKGEN1, OSCULP32K: no ONDEMAND
        }
        apply(set(Osc::OSC48MCTRL::enable), clear(Osc::OSC48MCTRL::ondemand));
        // tSTART max 15 us (Table 45-57)
        Kvasir::Register::waitUntil<Kvasir::Chip::Sam::SyncBound>(
          Kvasir::Register::isSet(Osc::STATUS::osc48mrdy));
        apply(GenericClockGenerator<0, GeneratorSource::osc48m, 1>::enable());
        // SYNCBUSY.GENCTRL0 is bit 2 (16.8.2); the SVD gives the field an enum, so the bit is read as such
        Kvasir::Register::waitUntil<Kvasir::Chip::Sam::SyncBound>(
          [] { return (get<0>(apply(read(GC::SYNCBUSY::FULLREGISTER))) & (1U << 2U)) == 0; });
        apply(set(Osc::OSC48MCTRL::ondemand));
    }
}}   // namespace Kvasir::GCLK

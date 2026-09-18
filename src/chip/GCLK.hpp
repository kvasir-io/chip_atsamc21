#pragma once

#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/GCLK.hpp"

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

        [[nodiscard]] static constexpr auto enable() {
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

}}   // namespace Kvasir::GCLK

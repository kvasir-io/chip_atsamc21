#pragma once

#include "ClockLimits.hpp"
#include "WaitBounds.hpp"
#include "atsam_common/GclkIndexed.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/GCLK.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/OSC32KCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"

namespace Kvasir { namespace GCLK {

    // GENCTRLn.SRC (DS60001479M 16.8.3): the generated enum of generator 0's register - Generator casts it to its own
    // generator's (the same values; each GENCTRL<n> of the SVD's dim has its enum type).
    using GeneratorSource = Kvasir::Peripheral::GCLK::Registers<>::GENCTRL<0>::SRCVal;

    // The PCHCTRLm index, Table 16-9 "PCHCTRLm Mapping" (md l.6590-6650; the SVD has no enumeration for it). 39 is
    // reserved; SERCOM6/7 (41, 42) are N parts only.
    enum class Peripheral : unsigned {
        dpll            = 0,
        dpll32k         = 1,
        eic             = 2,
        freqmMsr        = 3,
        freqmRef        = 4,
        tsens           = 5,
        evsysCh0        = 6,
        evsysCh1        = 7,
        evsysCh2        = 8,
        evsysCh3        = 9,
        evsysCh4        = 10,
        evsysCh5        = 11,
        evsysCh6        = 12,
        evsysCh7        = 13,
        evsysCh8        = 14,
        evsysCh9        = 15,
        evsysCh10       = 16,
        evsysCh11       = 17,
        sercom01234Slow = 18,
        sercom0Core     = 19,
        sercom1Core     = 20,
        sercom2Core     = 21,
        sercom3Core     = 22,
        sercom4Core     = 23,
        sercom5Slow     = 24,
        sercom5Core     = 25,
        can0            = 26,
        can1            = 27,
        tcc0Tcc1        = 28,
        tcc2            = 29,
        tc0Tc1          = 30,
        tc2Tc3          = 31,
        tc4             = 32,
        adc0            = 33,
        adc1            = 34,
        sdadc           = 35,
        dac             = 36,
        ptc             = 37,
        ccl             = 38,
        ac              = 40
    };

    template<unsigned Generator, GeneratorSource Source, unsigned long long Div>
    using GenericClockGenerator = Sam::Gclk::Generator<ClockLimits::C21::gclkDivBits(Generator),
                                                       ClockLimits::C21::gclkMaxDivision(Generator),
                                                       Generator,
                                                       Source,
                                                       Div>;

    template<unsigned Generator, Peripheral PeripheralId>
    using PeripheralChannelController = Sam::Gclk::Channel<Generator, PeripheralId>;
}}   // namespace Kvasir::GCLK

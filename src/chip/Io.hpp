#pragma once

#include "Variant.hpp"
#include "kvasir/Io/Io.hpp"
#include "kvasir/Mpl/Utility.hpp"
#include "kvasir/Register/Register.hpp"
#include "peripherals/PORT.hpp"

#include <array>

namespace Kvasir { namespace Io {
    template<typename>
    struct PinLocationTraits {
        static constexpr unsigned baseAddress = Kvasir::Peripheral::PORT::Registers<>::baseAddr;
        static constexpr int      portBegin   = 0;
        static constexpr int      pinBegin    = 0;
        static constexpr int      pinEnd      = 32;
        static constexpr int      ListEndIndicator = 255;
        // SAM C20/C21 data sheet DS60001479M Table 6-2 "PORT Function Multiplexing for SAM C21
        // E/G/J", columns SAM C21E and SAM C21G.
#if defined(KVASIR_CHIP_ATSAMC21G17A)
        /// The G variant: PA00-PA25, PA27, PA28, PA30, PA31; PB02, PB03, PB08-PB11, PB22, PB23.
        static constexpr int portEnd = 2;
        static constexpr std::array<std::array<int, pinEnd - pinBegin>, portEnd - portBegin>
          PinsDisabled{
            {{{26, 29, ListEndIndicator}}, {{0,  1,  4,  5,  6,
                                             7,  12, 13, 14, 15,
                                             16, 17, 18, 19, 20,
                                             21, 24, 25, 26, 27,
                                             28, 29, 30, 31, ListEndIndicator}}}
        };
#else
        /// The E variant bonds PORTA only.
        static constexpr int portEnd = 1;
        static constexpr std::array<std::array<int, pinEnd - pinBegin>, portEnd - portBegin>
          PinsDisabled{{{{12, 13, 20, 21, 26, 29, ListEndIndicator}}}};
#endif
    };

}}   // namespace Kvasir::Io

#include "atsam_common/Io.hpp"

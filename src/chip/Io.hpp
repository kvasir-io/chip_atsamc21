#pragma once

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
        /// The E variant bonds PORTA only.
        static constexpr int portEnd          = 1;
        static constexpr int pinBegin         = 0;
        static constexpr int pinEnd           = 32;
        static constexpr int ListEndIndicator = 255;
        static constexpr std::array<std::array<int, pinEnd - pinBegin>, portEnd - portBegin>
          PinsDisabled{{{{12, 13, 20, 21, 26, 29, ListEndIndicator}}}};
    };

}}   // namespace Kvasir::Io

#include "atsam_common/Io.hpp"

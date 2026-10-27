#pragma once
#include "Variant.hpp"
#include "atsam_common/Io.hpp"
#include "chip/atsam_common/CAN_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace CAN { namespace Traits {

    struct CanTraits : CanTraitsBase<CanTraits> {
        static constexpr std::array pinMuxInfos{
          // Instance 0
          PinInfo{0, 0, 24, 6, PinInfo::tx},
          PinInfo{0, 0, 25, 6, PinInfo::rx},
          PinInfo{0, 1, 22, 6, PinInfo::tx},
          PinInfo{0, 1, 23, 6, PinInfo::rx},
#ifdef KVASIR_CHIP_ATSAMC21G17A
          // Instance 1, G part only (DS60001479M Table 6-2, function G; PB14/PB15 are J only)
          PinInfo{1, 1, 10, 6, PinInfo::tx},
          PinInfo{1, 1, 11, 6, PinInfo::rx},
#endif
        };
    };

}}}   // namespace Kvasir::CAN::Traits

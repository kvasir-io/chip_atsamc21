#pragma once
#include "Interrupt.hpp"
#include "Io.hpp"
#include "chip/atsam_common/CAN_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace CAN { namespace Traits {

    struct CanTraits : CanTraitsBase<CanTraits> {
        static constexpr int                canInstanceBegin  = 0;
        static constexpr int                canInstanceEnd    = 1;
        static constexpr std::array<int, 0> disabledInstances = {{}};

        template<unsigned Instance>
        struct IsrIndex;

        template<>
        struct IsrIndex<0> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::can0)>;
        };

        static constexpr std::array pinMuxInfos{
          // Instance 0
          PinInfo{0, 0, 24, 6, PinInfo::TX},
          PinInfo{0, 0, 25, 6, PinInfo::RX},
          PinInfo{0, 1, 22, 6, PinInfo::TX},
          PinInfo{0, 1, 23, 6, PinInfo::RX}
        };
    };

}}}   // namespace Kvasir::CAN::Traits

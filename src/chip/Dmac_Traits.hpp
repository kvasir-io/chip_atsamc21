#pragma once
#include "peripherals/DMAC.hpp"

#include <cstdint>

namespace Kvasir { namespace DMAC { namespace Traits {
    struct DmacTraits {
        static constexpr std::size_t Channels = 12;
        static constexpr bool        OldImpl  = true;

        using TriggerSource = Kvasir::Peripheral::DMAC::Registers<>::CHCTRLB::TRIGSRCVal;
    };
}}}   // namespace Kvasir::DMAC::Traits

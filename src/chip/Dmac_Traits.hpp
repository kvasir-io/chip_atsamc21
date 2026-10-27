#pragma once
// The SAM D2x/C2x DMAC layout (atsam_common/DmacChannelWindow.hpp: 12 channels behind CHID) over this chip's DMAC.
#include "atsam_common/DmacChannelWindow.hpp"
#include "peripherals/DMAC.hpp"

namespace Kvasir { namespace DMAC { namespace Traits {
    struct DmacTraits : Sam::DmacChannelWindow<Kvasir::Peripheral::DMAC::Registers<>> {};
}}}   // namespace Kvasir::DMAC::Traits

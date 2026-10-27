#pragma once
// The EXTINT line of a pad on the SAM C21 (atsam_common/EicLines.hpp has the rule and its table citation); EIC.hpp
// includes this header itself.
#include "atsam_common/EicLines.hpp"

namespace Kvasir { namespace EIC { namespace Traits {
    using Lines::none;
    using Lines::notEncoded;

    consteval unsigned extIntOf(int port,
                                int pin) {
        return Lines::samD2xC2x(port, pin);
    }
}}}   // namespace Kvasir::EIC::Traits

#pragma once
#include "Variant.hpp"
#include "core/CoreInterrupts.hpp"
#include "kvasir/Common/Interrupt.hpp"
#include "kvasir/Port/Interrupts.hpp"
#include "peripherals/Interrupts.hpp"

#include <array>
#include <type_traits>

namespace Kvasir {
// The core's exceptions (core/CoreInterrupts.hpp) and the device lines of this part's SVD (svd_converter:
// peripherals/Interrupts.hpp, the SVD's names lower-cased without IRQ).
// Line 0 (MCLK, OSCCTRL, OSC32KCTRL, SUPC, PAC) is `system`, CAN0 on 15 `can0`: chip.svd names them so
// (DS60001479M Table 10-3, md l.2186, l.2207); the E part has no line 13, 14 or 16 (SERCOM4/5, CAN1).
struct Interrupt
  : CoreInterrupts
  , Peripheral::DeviceInterrupts {
    // The vector that serves EXTINT line n: one for all sixteen ("EIC - External Interrupt
    // Controller | 3", DS60001479M Table 10-3, md line 2193). Kvasir::EIC's lines are
    // sub-interrupts of it (atsam_common/EIC.hpp).
    template<unsigned Line>
    using EicExtIntVector = std::remove_cvref_t<decltype(eic)>;
};

namespace Nvic {
    // the core's reserved slots, then the lines the part's SVD does not name; hardFault (-13) is
    // served, and the core's FaultInterruptIndexs names it
    template<>
    struct InterruptOffsetTraits<Kvasir::Tag::User>
      : Core::ChipInterruptTraits<Core::InterruptRules,
                                  Peripheral::interruptEnd,
                                  Peripheral::interruptGaps> {};
}   // namespace Nvic
}   // namespace Kvasir

#pragma once
#include "Variant.hpp"
#include "chip/atsam_common/Sercom_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace Sercom { namespace Traits {

    struct SercomTraits : SercomTraitsBase<SercomTraits> {
        static constexpr std::array pinMuxInfos{
          // ATSAMC21E17A, 32 pins: PORTA only, four SERCOMs. C in column four, D in column five
          // (the datasheet's "I/O Multiplexing and Considerations" table).
          // Instance 0
          PinInfo{0, 0,  4, 3, 0},
          PinInfo{0, 0,  5, 3, 1},
          PinInfo{0, 0,  6, 3, 2},
          PinInfo{0, 0,  7, 3, 3},
          PinInfo{0, 0,  8, 2, 0},
          PinInfo{0, 0,  9, 2, 1},
          PinInfo{0, 0, 10, 2, 2},
          PinInfo{0, 0, 11, 2, 3},
          // Instance 1
          PinInfo{1, 0,  0, 3, 0},
          PinInfo{1, 0,  1, 3, 1},
          PinInfo{1, 0, 16, 2, 0},
          PinInfo{1, 0, 17, 2, 1},
          PinInfo{1, 0, 18, 2, 2},
          PinInfo{1, 0, 19, 2, 3},
          PinInfo{1, 0, 30, 3, 2},
          PinInfo{1, 0, 31, 3, 3},
          // Instance 2
          PinInfo{2, 0,  8, 3, 0},
          PinInfo{2, 0,  9, 3, 1},
          PinInfo{2, 0, 10, 3, 2},
          PinInfo{2, 0, 11, 3, 3},
          // PA12/PA13 (SERCOM2 pads 0/1): G part only, below (Table 6-2 lists no E pin for them, md l.1295-1296)
          PinInfo{2, 0, 14, 2, 2},
          PinInfo{2, 0, 15, 2, 3},
          // Instance 3
          PinInfo{3, 0, 16, 3, 0},
          PinInfo{3, 0, 17, 3, 1},
          PinInfo{3, 0, 18, 3, 2},
          PinInfo{3, 0, 19, 3, 3},
          PinInfo{3, 0, 20, 3, 2},
          PinInfo{3, 0, 21, 3, 3},
          PinInfo{3, 0, 22, 2, 0},
          PinInfo{3, 0, 23, 2, 1},
          PinInfo{3, 0, 24, 2, 2},
          PinInfo{3, 0, 25, 2, 3},
#ifdef KVASIR_CHIP_ATSAMC21G17A
          // ATSAMC21G17A, 48 pins: what the G column of Table 6-2 adds - PA12/PA13, part of
          // PORTB, and SERCOM4/SERCOM5.
          // Instance 2
          PinInfo{2, 0, 12, 2, 0},
          PinInfo{2, 0, 13, 2, 1},
          // Instance 4
          PinInfo{4, 1,  8, 3, 0},
          PinInfo{4, 1,  9, 3, 1},
          PinInfo{4, 1, 10, 3, 2},
          PinInfo{4, 1, 11, 3, 3},
          PinInfo{4, 0, 12, 3, 0},
          PinInfo{4, 0, 13, 3, 1},
          PinInfo{4, 0, 14, 3, 2},
          PinInfo{4, 0, 15, 3, 3},
          // Instance 5
          PinInfo{5, 0, 20, 2, 2},
          PinInfo{5, 0, 21, 2, 3},
          PinInfo{5, 0, 22, 3, 0},
          PinInfo{5, 0, 23, 3, 1},
          PinInfo{5, 0, 24, 3, 2},
          PinInfo{5, 0, 25, 3, 3},
          PinInfo{5, 1,  2, 3, 0},
          PinInfo{5, 1,  3, 3, 1},
          PinInfo{5, 1, 22, 3, 2},
          PinInfo{5, 1, 23, 3, 3},
#endif
        };
    };

}}}   // namespace Kvasir::Sercom::Traits

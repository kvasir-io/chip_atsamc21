#pragma once

#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"

#include <cstdint>
#include <string_view>

namespace Kvasir {
namespace MCLK {
    template<unsigned baseAddr = 0x40000800>
    struct Registers {
        static constexpr unsigned BaseAddr = baseAddr;
    };

    namespace Detail {
        template<unsigned Offset, int BitPos>
        using BitSet = Register::Action<
          Kvasir::Register::WOBitLocT<Register::Address<Registers<>::BaseAddr + Offset>, BitPos>,
          Kvasir::Register::WriteLiteralAction<(1U << unsigned(BitPos))>>;

        template<unsigned Offset, int BitPos>
        using BitClear = Register::Action<
          Kvasir::Register::WOBitLocT<Register::Address<Registers<>::BaseAddr + Offset>, BitPos>,
          Kvasir::Register::WriteLiteralAction<0>>;

        struct PeripheryEnableInfo {
            unsigned address;
            unsigned offset;
            unsigned bit;
        };

        static constexpr std::array peripheryEnableInfos{
          PeripheryEnableInfo{0x41002000, 0x10,  3}, // DSU
          PeripheryEnableInfo{0x41004000, 0x10,  5}, // NVMCTRL
          PeripheryEnableInfo{0x41006000, 0x10,  7}, // DMAC
          PeripheryEnableInfo{0x42001C00, 0x10,  8}, // CAN0
          PeripheryEnableInfo{0x42002000, 0x10,  9}, // CAN1
          PeripheryEnableInfo{0x48000000, 0x10, 12}, // DIVAS

          PeripheryEnableInfo{0x40000400, 0x14,  1}, // PM
          PeripheryEnableInfo{0x40000800, 0x14,  2}, // MCLK
          PeripheryEnableInfo{0x40000C00, 0x14,  3}, // RSTC
          PeripheryEnableInfo{0x40001000, 0x14,  4}, // OSCCTRL
          PeripheryEnableInfo{0x40001400, 0x14,  5}, // OSC32KCTRL
          PeripheryEnableInfo{0x40001800, 0x14,  6}, // SUBC
          PeripheryEnableInfo{0x40001C00, 0x14,  7}, // GCLK
          PeripheryEnableInfo{0x40002000, 0x14,  8}, // WDT
          PeripheryEnableInfo{0x40002400, 0x14,  9}, // RTC
          PeripheryEnableInfo{0x40002800, 0x14, 10}, // EIC
          PeripheryEnableInfo{0x40002C00, 0x14, 11}, // FREQM
          PeripheryEnableInfo{0x40003000, 0x14, 12}, // TSENS

          PeripheryEnableInfo{0x41000000, 0x18,  0}, // PORT
          PeripheryEnableInfo{0x41002000, 0x18,  1}, // DSU
          PeripheryEnableInfo{0x41004000, 0x18,  2}, // NVMCTRL

          PeripheryEnableInfo{0x42000000, 0x1C,  0}, // EVSYS
          PeripheryEnableInfo{0x42000400, 0x1C,  1}, // SERCOM0
          PeripheryEnableInfo{0x42000800, 0x1C,  2}, // SERCOM1
          PeripheryEnableInfo{0x42000C00, 0x1C,  3}, // SERCOM2
          PeripheryEnableInfo{0x42001000, 0x1C,  4}, // SERCOM3
          PeripheryEnableInfo{0x42001400, 0x1C,  5}, // SERCOM4
          PeripheryEnableInfo{0x42001800, 0x1C,  6}, // SERCOM5
          PeripheryEnableInfo{0x42002400, 0x1C,  9}, // TCCO
          PeripheryEnableInfo{0x42002800, 0x1C, 10}, // TCC1
          PeripheryEnableInfo{0x42002C00, 0x1C, 11}, // TCC2
          PeripheryEnableInfo{0x42003000, 0x1C, 12}, // TC0
          PeripheryEnableInfo{0x42003400, 0x1C, 13}, // TC1
          PeripheryEnableInfo{0x42003800, 0x1C, 14}, // TC2
          PeripheryEnableInfo{0x42003C00, 0x1C, 15}, // TC3
          PeripheryEnableInfo{0x42004000, 0x1C, 16}, // TC4
          PeripheryEnableInfo{0x42004400, 0x1C, 17}, // ADC0
          PeripheryEnableInfo{0x42004800, 0x1C, 18}, // ADC1
          PeripheryEnableInfo{0x42004C00, 0x1C, 19}, // SDADC
          PeripheryEnableInfo{0x42005000, 0x1C, 20}, // AC
          PeripheryEnableInfo{0x42005400, 0x1C, 21}, // DAC
          PeripheryEnableInfo{0x42005800, 0x1C, 22}, // PTC
          PeripheryEnableInfo{0x42005C00, 0x1C, 23}, // CCL

          PeripheryEnableInfo{0x43000800, 0x20,  2}, // TC5
          PeripheryEnableInfo{0x43000C00, 0x20,  3}, // TC6
          PeripheryEnableInfo{0x43001000, 0x20,  4}, // TC7
        };

        static constexpr bool isValidPeripheryAddress(unsigned peripheryAddress) {
            for(auto pei : peripheryEnableInfos) {
                if(pei.address == peripheryAddress) { return true; }
            }
            return false;
        }

        static constexpr unsigned getOffset(unsigned peripheryAddress) {
            for(auto pei : peripheryEnableInfos) {
                if(pei.address == peripheryAddress) { return pei.offset; }
            }
            return 0;
        }

        static constexpr unsigned getBit(unsigned peripheryAddress) {
            for(auto pei : peripheryEnableInfos) {
                if(pei.address == peripheryAddress) { return pei.bit; }
            }
            return 0;
        }

    }   // namespace Detail

    template<unsigned PeripheryAddress>
    struct enable {
        static_assert(Detail::isValidPeripheryAddress(PeripheryAddress),
                      "invalid PeripheryAddress to enable");
        using action
          = Detail::BitSet<Detail::getOffset(PeripheryAddress), Detail::getBit(PeripheryAddress)>;
    };

    template<unsigned PeripheryAddress>
    struct disable {
        static_assert(Detail::isValidPeripheryAddress(PeripheryAddress),
                      "invalid PeripheryAddress to disable");
        using action
          = Detail::BitClear<Detail::getOffset(PeripheryAddress), Detail::getBit(PeripheryAddress)>;
    };
}   // namespace MCLK

namespace PM {
    using MCLK::disable;
    using MCLK::enable;
}   // namespace PM

}   // namespace Kvasir

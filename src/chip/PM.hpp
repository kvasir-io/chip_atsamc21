#pragma once

#include "MCLK.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"

#include <cstdint>

/// What the D21 calls PM is two peripherals here: MCLK holds the clock-gating masks (MCLK.hpp,
/// which this aliases into Kvasir::PM so the shared drivers reach them by the one name), and RSTC
/// holds the reset cause.
namespace Kvasir { namespace RSTC {
    template<unsigned baseAddr = 0x40000C00>
    struct Registers {
        static constexpr unsigned BaseAddr = baseAddr;

        struct RCAUSE {
            using Addr = Register::Address<baseAddr + 0x00, 0xff, 0x00, unsigned char>;
            static constexpr Register::FieldLocation<Addr,
                                                     Register::maskFromRange(2, 0, 7, 4),
                                                     Register::ReadOnlyAccess,
                                                     unsigned char>
              flags{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b0000'0001>
              POR{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b0000'0010>
              BOD12{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b0000'0100>
              BOD33{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b0001'0000>
              EXT{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b0010'0000>
              WDT{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b0100'0000>
              SYST{};
            static constexpr Register::FieldValue<typename decltype(flags)::type, 0b1000'0000>
              BACKUP{};
        };
    };
}}   // namespace Kvasir::RSTC

namespace Kvasir { namespace PM {
    enum class ResetCause : std::uint8_t { por, bod12, bod33, ext, wdt, syst, backup };

    inline ResetCause reset_cause() {
        using RCAUSE = Kvasir::RSTC::Registers<>::RCAUSE;
        auto const c = apply(read(RCAUSE::flags));
        if(c == RCAUSE::POR) { return ResetCause::por; }
        if(c == RCAUSE::BOD12) { return ResetCause::bod12; }
        if(c == RCAUSE::BOD33) { return ResetCause::bod33; }
        if(c == RCAUSE::EXT) { return ResetCause::ext; }
        if(c == RCAUSE::WDT) { return ResetCause::wdt; }
        if(c == RCAUSE::BACKUP) { return ResetCause::backup; }
        return ResetCause::syst;
    }
}}   // namespace Kvasir::PM

#pragma once

#include "kvasir/Port/ResetKind.hpp"

#include <cstdint>

// The reset causes of RSTC.RCAUSE (SAM C20/C21 data sheet DS60001479M 18.8.1, md l.7983-8019: bit 6 SYST, 5 WDT,
// 4 EXT, 2 BODVDD, 1 BODCORE, 0 POR; bits 3 and 7 reserved - the E5x's BACKUP does not exist here) and their mapping
// onto the SDK's ResetKind, without registers (host-tested in tests/reset_cause_test.cpp); PM.hpp reads the register.
namespace Kvasir { namespace PM {
    /// Named as 18.8.1 names the bits (BODCORE md l.8015, BODVDD md l.8011); `unknown`: no bit or more than one bit
    /// set (atsam_common/ResetCauseDecode.hpp). The names are printed by the boot report.
    enum class ResetCause : std::uint8_t { por, bodcore, bodvdd, ext, wdt, syst, unknown };

    /// resetCause() as the SDK's boot guard sees it (kvasir/Port/ResetKind.hpp). A debugger's reset is SYST like a
    /// software request: without a record the guard counts neither.
    [[nodiscard]] constexpr ResetKind toResetKind(ResetCause c) {
        switch(c) {
        case ResetCause::por:     return ResetKind::powerOn;
        case ResetCause::bodcore:
        case ResetCause::bodvdd:  return ResetKind::brownOut;
        case ResetCause::ext:     return ResetKind::external;
        case ResetCause::wdt:     return ResetKind::watchdogTimeout;
        case ResetCause::syst:    return ResetKind::softwareRequest;
        case ResetCause::unknown: return ResetKind::unknown;
        }
        return ResetKind::unknown;
    }
}}   // namespace Kvasir::PM

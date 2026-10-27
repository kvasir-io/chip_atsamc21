// src/chip/ResetCause.hpp (the reset causes), SAM C21 (DS60001479M). PM.hpp reads the generated RSTC registers and
// the clock gates (Blocks.hpp) the generated MCLK fields: both are compiled by tests/conformance, which also checks
// the gate rows.
#include "ResetCause.hpp"

#include <type_traits>

namespace {
using namespace Kvasir;
// RCAUSE (18.8.1): bit 6 SYST, 5 WDT, 4 EXT, 2 BODVDD, 1 BODCORE, 0 POR; 3 and 7 reserved - no BACKUP bit on this
// chip (that is the E5x). The values stay (0..5 in bit order); the names are 18.8.1's, printed by the boot report;
// `unknown` (no bit or several) comes last.
using PM::ResetCause;
static_assert(static_cast<unsigned>(ResetCause::por) == 0
              && static_cast<unsigned>(ResetCause::bodcore) == 1
              && static_cast<unsigned>(ResetCause::bodvdd) == 2
              && static_cast<unsigned>(ResetCause::ext) == 3
              && static_cast<unsigned>(ResetCause::wdt) == 4
              && static_cast<unsigned>(ResetCause::syst) == 5
              && static_cast<unsigned>(ResetCause::unknown) == 6);

// Every enumerator is mapped (toResetKind()'s switch has no default: -Wswitch -Werror fails on a new one).
using Kvasir::ResetKind;
static_assert(PM::toResetKind(ResetCause::por) == ResetKind::powerOn);
static_assert(PM::toResetKind(ResetCause::bodcore) == ResetKind::brownOut);
static_assert(PM::toResetKind(ResetCause::bodvdd) == ResetKind::brownOut);
static_assert(PM::toResetKind(ResetCause::ext) == ResetKind::external);
static_assert(PM::toResetKind(ResetCause::wdt) == ResetKind::watchdogTimeout);
static_assert(PM::toResetKind(ResetCause::syst) == ResetKind::softwareRequest);
static_assert(PM::toResetKind(ResetCause::unknown) == ResetKind::unknown);
}   // namespace

int main() { return 0; }

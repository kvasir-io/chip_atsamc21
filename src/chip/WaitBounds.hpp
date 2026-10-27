#pragma once
// The bounds of the SAM C20/C21 package's register waits (Kvasir_SDK kvasir/Register/Wait.hpp), each with its source
// (DS60001479M). Under the default wait policy (Unbounded) every site is the loop it was.
#include "ClockLimits.hpp"
#include "kvasir/Register/Wait.hpp"

#include <cstdint>

// f_CPU max 64 MHz for the "64" parts (Table 45-8, md l.49950; 48 MHz for the others, Table 45-9): the SDK's
// MicrosecondsBound converts at the faster one, so a 48 MHz part (or a slower clock) only lengthens the real time.
// Specialised before any bound below names it.
template<>
struct Kvasir::Register::CpuHzCeiling<Kvasir::Tag::User> {
    static constexpr std::uint64_t value
      = Kvasir::ClockLimits::C21::cpuMax(Kvasir::ClockLimits::C21::Speed::mhz64);
};

namespace Kvasir::Chip {
// DPLL lock (CLKRDY): tLOCK max 1.5 ms at f_IN 32 kHz / f_OUT 96 MHz, 35 us at 2 MHz (Table 45-52, md
// l.51313-51314)
using DpllLockBound = Kvasir::Register::MicrosecondsBound<2'000>;
}   // namespace Kvasir::Chip

// the shared SAM drivers' bounds (Sam::SyncBound, ...): they need the ceiling above
#include "atsam_common/SamWaitBounds.hpp"

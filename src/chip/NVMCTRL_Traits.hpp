#pragma once
// The SAM C21's NVM controller for the shared driver (atsam_common/NVMCTRL.hpp): the D2x/C2x command set
// (atsam_common/NvmCommands.hpp) with this chip's bounds and geometry.

#include "ClockLimits.hpp"
#include "WaitBounds.hpp"
#include "atsam_common/NvmCommands.hpp"
#include "peripherals/NVMCTRL.hpp"

#include <cstddef>

namespace Kvasir { namespace NVMCTRL {
    // No live NVMCTRL erratum on the C21 (errata DS80000740T 1.14.1 "EEPROM Cache" is "Reserved" since rev K), so
    // neither the D21's dummy write before RWWEEER nor its cache switch.
    struct Errata {
        template<typename T>
        static constexpr void prepareEraseRww(T) {}

        static constexpr void enableCache() {}

        static constexpr void disableCache() {}
    };

    struct Traits : Sam::NvmCommandsDC<Kvasir::Peripheral::NVMCTRL::Registers<>, Errata> {
        // tFRE (row erase) max 6 ms, tFPP (page write) max 2.5 ms (DS60001479M Table 45-42, md l.51019-51020); x2, at
        // the fastest C21 CPU clock (64 MHz parts, Table 45-8)
        using EraseBound = Kvasir::Register::MicrosecondsBound<12'000>;
        using WriteBound = Kvasir::Register::MicrosecondsBound<5'000>;

        static constexpr std::size_t pageSize    = 64;
        static constexpr std::size_t pagesPerRow = 4;
    };
}}   // namespace Kvasir::NVMCTRL

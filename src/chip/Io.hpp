#pragma once
// The chip's pin table (PinLocationTraits). Users include the family's pin front, chip/atsam_common/Io.hpp, which
// includes this.

#include "Variant.hpp"
#include "kvasir/Hal/Pins.hpp"
#include "kvasir/Io/Io.hpp"
#include "kvasir/Register/Register.hpp"
#include "peripherals/PORT.hpp"

#include <array>
#include <type_traits>

namespace Kvasir { namespace Io {
    template<typename>
    struct PinLocationTraits {
        static constexpr unsigned baseAddress = Kvasir::Peripheral::PORT::Registers<>::baseAddr;
        static constexpr int      portBegin   = 0;
        static constexpr int      pinBegin    = 0;
        static constexpr int      pinEnd      = 32;
        // SAM C20/C21 data sheet DS60001479M Table 6-2 "PORT Function Multiplexing for SAM C21
        // E/G/J", columns SAM C21E and SAM C21G.
#ifdef KVASIR_CHIP_ATSAMC21G17A
        /// The G variant: PA00-PA25, PA27, PA28, PA30, PA31; PB02, PB03, PB08-PB11, PB22, PB23.
        static constexpr int                                            portEnd = 2;
        static constexpr std::array<std::uint64_t, portEnd - portBegin> bonded{0xDBFF'FFFFULL,
                                                                               0x00C0'0F0CULL};
#else
        /// The E variant bonds PORTA only: PA00-PA11, PA14-PA19, PA22-PA25, PA27, PA28, PA30, PA31.
        static constexpr int                                            portEnd = 1;
        static constexpr std::array<std::uint64_t, portEnd - portBegin> bonded{0xDBCF'CFFFULL};
#endif
    };

}}   // namespace Kvasir::Io

namespace Kvasir { namespace Sam {
    // The PORT registers of one pin group (port), for chip_atsam_common Io.hpp: the SVD's GROUP<n> cluster (DS60001479M 28.7:
    // PINCFGy 0x40 + y and PMUXn 0x30 + n per group, group stride 0x80).
    template<int Port>
    struct PortGroup {
        using Group = Kvasir::Peripheral::PORT::Registers<>::GROUP<Port>;

        static constexpr auto dirclr = Group::DIRCLR::dirclr;
        static constexpr auto dirset = Group::DIRSET::dirset;
        static constexpr auto outclr = Group::OUTCLR::outclr;
        static constexpr auto outset = Group::OUTSET::outset;
        static constexpr auto outtgl = Group::OUTTGL::outtgl;
        static constexpr auto in
          = Group::IN::FULLREGISTER;   // the SVD gives IN no field: one bit per pin

        template<int Pin>
        using Pincfg = typename Group::template PINCFG<Pin>;
        template<int Pin>
        using Pmux = typename Group::template PMUX<Pin / 2>;
    };
}}   // namespace Kvasir::Sam

#pragma once

// Which SAM C21 this firmware is built for: cmake/chip.cmake's KVASIR_ATSAMC21_MPU picks the SVD
// and, for every part but the default one, a define that is read here. The headers of this
// package differ between the parts only where the silicon does, and ask this.
namespace Kvasir { namespace Chip {
    enum class Variant { atsamc21e17a, atsamc21g17a };

#if defined(KVASIR_CHIP_ATSAMC21G17A)
    inline constexpr Variant variant = Variant::atsamc21g17a;
#else
    inline constexpr Variant variant = Variant::atsamc21e17a;
#endif

    // The G parts bond part of PORTB and add SERCOM4, SERCOM5 and CAN1 (SAM C20/C21 data sheet
    // DS60001479M, Table 1-3 "SAM C21 Family Features"; Table 6-2 note 4).
    inline constexpr bool isG = variant == Variant::atsamc21g17a;
}}   // namespace Kvasir::Chip

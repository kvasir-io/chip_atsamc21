#pragma once

#include <string_view>

// Which SAM C21 this firmware is built for: cmake/chip.cmake's KVASIR_ATSAMC21_MPU picks the SVD
// and the part's define (cmake/variants/<part>.cmake: KVASIR_CHIP_<part>, with KVASIR_CHIP_ATSAMC21
// and KVASIR_CHIP_FAMILY_SAM), which is read here. The headers of this package differ between the
// parts only where the silicon does, and ask this.
namespace Kvasir { namespace Chip {
    enum class Part { atsamc21e17a, atsamc21g17a };

#ifdef KVASIR_CHIP_ATSAMC21G17A
    inline constexpr Part part = Part::atsamc21g17a;
#elifdef KVASIR_CHIP_ATSAMC21E17A
    inline constexpr Part part = Part::atsamc21e17a;
#else
    #error \
      "chip_atsamc21 without a KVASIR_CHIP_ATSAMC21<part> define: not compiled with the package's kvasir_chip DEFINITIONS"
#endif

    // which part this image is for, as text
    inline constexpr std::string_view partName
      = part == Part::atsamc21g17a ? "ATSAMC21G17A" : "ATSAMC21E17A";

    // The G parts bond part of PORTB and add SERCOM4, SERCOM5 and CAN1 (SAM C20/C21 data sheet
    // DS60001479M, Table 1-3 "SAM C21 Family Features"; Table 6-2 note 4).
    inline constexpr bool isG = part == Part::atsamc21g17a;
}}   // namespace Kvasir::Chip

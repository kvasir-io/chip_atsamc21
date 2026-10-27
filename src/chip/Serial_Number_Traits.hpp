#pragma once
#include <array>
#include <cstdint>

namespace Kvasir { namespace SerialNumber { namespace Traits {

    /// SAM C20/C21 data sheet (DS60001479M) 9.6 "Serial Number" (md l.2042), words 0 to 3.
    static constexpr std::array<std::uint32_t, 4> addresses{0x0080A00C,
                                                            0x0080A040,
                                                            0x0080A044,
                                                            0x0080A048};

}}}   // namespace Kvasir::SerialNumber::Traits

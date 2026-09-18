#pragma once
#include <array>

namespace Kvasir { namespace SerialNumber { namespace Traits {

    static constexpr std::array<std::uint32_t, 4> addresses{0x0080A00C,
                                                            0x0080A040,
                                                            0x0080A044,
                                                            0x0080A048};

}}}   // namespace Kvasir::SerialNumber::Traits

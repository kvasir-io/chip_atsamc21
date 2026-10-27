#pragma once

#include "Blocks.hpp"
#include "ResetCause.hpp"
#include "atsam_common/ResetCauseDecode.hpp"
#include "kvasir/Port/Hooks.hpp"
#include "kvasir/Port/ResetKind.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/RSTC.hpp"

#include <array>
#include <cstdint>

/// What the D21 calls PM is two peripherals here: MCLK holds the clock-gating masks (MCLK.hpp,
/// which this aliases into Kvasir::PM so the shared drivers reach them by the one name), and RSTC
/// holds the reset cause (the generated peripherals/RSTC.hpp).
namespace Kvasir { namespace PM {
    /// The latest reset's cause (RSTC.RCAUSE, DS60001479M 18.8.1; the causes in ResetCause.hpp), one load.
    inline ResetCause resetCause() {
        using R         = Kvasir::Peripheral::RSTC::Registers<>::RCAUSE;
        auto const bits = apply(read(R::por, R::bodcore, R::bodvdd, R::ext, R::wdt, R::syst));
        return Sam::oneCauseOf(bits,
                               std::array{ResetCause::por,
                                          ResetCause::bodcore,
                                          ResetCause::bodvdd,
                                          ResetCause::ext,
                                          ResetCause::wdt,
                                          ResetCause::syst},
                               ResetCause::unknown);
    }

    /// resetCause() mapped onto the SDK's ResetKind (ResetCause.hpp toResetKind), for the boot guard.
    inline ResetKind resetKind() { return toResetKind(resetCause()); }
}}   // namespace Kvasir::PM

/// resetCause() as the port's hook (kvasir/Port/Hooks.hpp), for the boot report:
/// Port::LateT<Port::ResetCause<Tag::User>, Dep>{}().
template<typename... Ts>
struct Kvasir::Port::ResetCause<Kvasir::Tag::User, Ts...> {
    Kvasir::PM::ResetCause operator()() const { return Kvasir::PM::resetCause(); }
};

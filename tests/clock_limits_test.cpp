// src/chip/ClockLimits.hpp (SAM C21) through chip_atsam_common's ClockSolver.hpp, as the firmwares
// use it - tables per part and grade.
#include "ClockLimits.hpp"

#include <cstdio>

namespace {
using namespace Kvasir;
namespace L = ClockLimits::C21;
using ClockLimits::Supply;
using L::Rating;

// 8 MHz crystal -> 48 MHz (DIV 3, LDR 47, PRESC /1, GCLK0 /1)
constexpr auto d8 = DPLL::fromXosc<L::Fdpll96m>(8'000'000, 48'000'000);
static_assert(d8.found && d8.div == 3 && d8.ldr == 47 && d8.ldrFrac == 0 && d8.presc == 0
              && d8.gclkDiv == 1);
// below the DCO's 48 MHz: a full tie between the output prescaler and the generator divider keeps
// the first candidate, PRESC /1 (the rule in ClockSolver.hpp)
constexpr auto d24 = DPLL::fromXosc<L::Fdpll96m>(8'000'000, 24'000'000);
static_assert(d24.found && d24.ldr == 47 && d24.presc == 0 && d24.gclkDiv == 2);
constexpr auto d12 = DPLL::fromXosc<L::Fdpll96m>(8'000'000, 12'000'000);
static_assert(d12.found && d12.ldr == 47 && d12.presc == 0 && d12.gclkDiv == 4);
// with the generator divider limited to 1 the prescaler does it
constexpr auto d12p = DPLL::fromXosc<L::Fdpll96m>(8'000'000, 12'000'000, {0, 1}, false, 1);
static_assert(d12p.found && d12p.ldr == 47 && d12p.presc == 2 && d12p.gclkDiv == 1);
// 64 MHz (the "64" parts): 1 MHz x 64
constexpr auto d64 = DPLL::fromXosc<L::Fdpll96m>(8'000'000, 64'000'000);
static_assert(d64.found && d64.div == 3 && d64.ldr == 63 && d64.presc == 0);
static_assert(L::cpuMax(L::Speed::mhz48) == 48'000'000 && L::cpuMax(L::Speed::mhz64) == 64'000'000);

// the old numbers: 31 007.75 Hz reference, below f_IN
static_assert(!DPLL::checkXosc<L::Fdpll96m>(8'000'000,
                                            128,
                                            3095,
                                            0,
                                            0,
                                            2)
                 .refInRange);
static_assert(DPLL::checkXosc<L::Fdpll96m>(8'000'000,
                                           3,
                                           47,
                                           0)
                .ok());
static_assert(!DPLL::checkXosc<L::Fdpll96m>(8'000'000,
                                            3,
                                            47,
                                            0,
                                            3)
                 .fieldsFit,
              "PRESC 3 is reserved");

// Table 45-41: 1 WS only to 38 MHz, so 48 MHz takes 2
static_assert(L::waitStates<48'000'000,
                            Supply::from2V7>()
              == 2);
static_assert(L::waitStates<48'000'000,
                            Supply::from4V5>()
              == 2);
static_assert(L::waitStates<19'000'000,
                            Supply::from2V7>()
              == 0);
static_assert(L::waitStates<19'000'001,
                            Supply::from2V7>()
              == 1);
static_assert(L::waitStates<20'000'000,
                            Supply::from4V5>()
              == 0);
static_assert(L::waitStates<64'000'000,
                            Supply::from2V7>()
              == 2);
static_assert(L::waitStates<48'000'000,
                            Supply::from2V7,
                            Rating::egj105>()
              == 2);
static_assert(L::waitStates<48'000'000,
                            Supply::from2V7,
                            Rating::n105>()
              == 2);
static_assert(L::waitStates<38'000'001,
                            Supply::from2V7,
                            Rating::n105>()
              == 2);
// 125 C: 48 MHz takes 3 at > 2.7 V, 2 at > 4.5 V
static_assert(L::waitStates<48'000'000,
                            Supply::from2V7,
                            Rating::aecQ100>()
              == 3);
static_assert(L::waitStates<48'000'000,
                            Supply::from4V5,
                            Rating::aecQ100>()
              == 2);
static_assert(Nvm::waitStates(L::WaitStates85C,
                              Supply::from2V7,
                              64'000'001)
              == Nvm::NoWaitStateCount);
static_assert(Nvm::waitStates(L::WaitStates85C,
                              Supply::from1V62,
                              1)
              == Nvm::NoWaitStateCount);

static_assert(L::gclkMaxDivision(0) == 512 && L::gclkMaxDivision(1) == 131072
              && L::gclkMaxDivision(2) == 512);
static_assert((1ULL << 20) > L::gclkMaxDivision(2),
              "GCLK2 / 2^20 is past the clamp");
}   // namespace

int main() {
    std::puts("clock solver (SAM C21): every check is a static_assert");
    return 0;
}

// What the SAM C21 refuses at compile time, as must-fail cases (kvasir_conformance_must_fail, CMakeLists.txt): the
// clock tree (Kvasir::Clocks::Sam::Tree, chip/ClockTree.hpp over atsam_common/ClockTree.hpp), the crystal oscillator,
// the DPLL solver, and the clock gates of a SERCOM the E17A does not have. TREE_FAIL=0 must compile: the control
// names the valid tree a board uses (XOSC 8 MHz -> DPLL96M -> 48 MHz) and the gate of a SERCOM every part has. Each
// of 1..n names one setting the chip must refuse, and its ctest passes only on that case's own message (the case list
// in CMakeLists.txt; 6 and 7 only on the E17A).

#include "kvasir_config.hpp"
//
#include <chip/ClockTree.hpp>
#include <chip/chip.hpp>

#ifndef TREE_FAIL
    #error "TREE_FAIL=<0..n> selects the case"
#endif

namespace {
namespace CS = Kvasir::Clocks::Sam;

// the control: an 8 MHz crystal board's tree, and SERCOM0's gate
using Valid = CS::Tree<{.cpu = 48'000'000, .source = {.hz = 8'000'000}}>;
static_assert(Valid::dpll.div == 3 && Valid::dpll.ldr == 47 && Valid::waitStates == 2);

[[maybe_unused]] void valid() {
    Valid::coreClockInit();
    Kvasir::Register::apply(
      Kvasir::PM::Enable<Kvasir::Peripheral::SERCOM_I2CM::Registers<0>::baseAddr>::action{});
}

#if TREE_FAIL == 1   // 64 MHz on the default 48 MHz speed grade (Table 45-9)
using Refused = CS::Tree<{.cpu = 64'000'000, .source = {.hz = 8'000'000}}>;
#elif TREE_FAIL == 2   // a DFLL kind: the C21 has no DFLL48M
using Refused = CS::Tree<{.cpu = 48'000'000, .kind = CS::Source::dfllOpenLoop}>;
#elif TREE_FAIL == 3   // a crystal above the XOSC's range (Table 45-47: 0.4..32 MHz)
using Refused = CS::Tree<{.cpu = 48'000'000, .source = {.hz = 40'000'000}}>;
#elif TREE_FAIL                                                                         \
  == 4   // no DPLL setting makes the CPU clock exactly from this crystal (tolerance 0)
using Refused = CS::Tree<{.cpu = 47'000'001, .source = {.hz = 8'000'000}}>;
#elif TREE_FAIL == 5   // a clock on XIN above the XOSC's external clock limit (Table 45-46: 48 MHz)
using Refused = CS::Tree<{
  .cpu    = 48'000'000,
  .source = {.hz = 50'000'000, .crystal = false}
}>;
#endif
#if TREE_FAIL >= 1 && TREE_FAIL <= 5
[[maybe_unused]] void refused() { Refused::coreClockInit(); }
#endif

#if TREE_FAIL == 6   // the E17A has no SERCOM4 (0x42001400): no clock gate row for it
[[maybe_unused]] void refused() {
    Kvasir::Register::apply(Kvasir::PM::Enable<0x4200'1400U>::action{});
}
#elif TREE_FAIL == 7   // nor a row to read its gate back
[[maybe_unused]] bool refused() { return Kvasir::PM::isEnabled<0x4200'1400U>(); }
#endif
}   // namespace

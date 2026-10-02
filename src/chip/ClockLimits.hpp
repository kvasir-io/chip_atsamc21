#pragma once
// The SAM C20/C21's clock limits, for chip_atsam_common/ClockSolver.hpp. Each number from
// DS60001479M ("line" is the line of SAMC20_C21_Family_Datasheet.md) and its errata DS80000740T
// (SAMC20_C21_Errata.md).
// The ordering code decides two things a part name like "ATSAMC21E17A" (chip.cmake's TARGET_MPU)
// does not say: the CPU maximum (no character or "S2" = 48 MHz, "64" = 64 MHz; Ordering
// Information, line 903, Tables 45-8/45-9) and the temperature grade (U 85 C, N 105 C, Z 125 C
// AEC-Q100). Both are template parameters with the slower / cooler default; a board that knows
// better says so.
#include "atsam_common/ClockSolver.hpp"

namespace Kvasir::ClockLimits::C21 {
// DPLL: f_IN 32..2000 kHz, DCO f_OUT 48..96 MHz (Table 45-52, lines 51288/51302; note 1: "based on
// simulation"); f_GCLK_DPLL max 2 MHz (Table 45-10, line 49994). LDR 12 bits, LDRFRAC 4 bits
// (20.8.13, lines 9871/9867); DIV 11 bits, f_DIV = f_XOSC / (2 (DIV + 1)) (20.8.14, line 9909);
// PRESC 0/1/2 = /1 /2 /4 (20.8.15, line 10013); f_CK = f_CKR (LDR + 1 + LDRFRAC / 16) / 2^PRESC
// (20.6.5, line 8785 ff.).
inline constexpr Dpll                   Fdpll96m{.refMin   = 32'000,
                                                 .refMax   = 2'000'000,
                                                 .outMin   = 48'000'000,
                                                 .outMax   = 96'000'000,
                                                 .ldrBits  = 12,
                                                 .fracBits = 4,
                                                 .divBits  = 11,
                                                 .prescMax = 2};
inline constexpr Prescaler::FixedString DpllWhere = "SAM C21 DPLL96M, Table 45-52";

enum class Speed : std::uint8_t {
    mhz48,   // Table 45-9, line 49977 (no character or "S2" in the ordering code)
    mhz64,   // Table 45-8, line 49950 ("64")
};

consteval std::uint64_t cpuMax(Speed s) { return s == Speed::mhz64 ? 64'000'000 : 48'000'000; }

// Errata 1.25.1 (line 3613, PDF page 56), every E/G/J revision B..H: below 25 C a spurious unlock
// halts the DPLL output; the workaround is DPLLCTRLB.LBYPASS = 1 before ENABLE. Errata 1.3.2 (line
// 436, PDF p. 12, revisions B and C): "high period jitter ... accurate clocking is limited to 32
// MHz and below through XOSC".

// Which wait-state table holds (part letter and temperature grade).
enum class Rating : std::uint8_t {
    egj85,     // Table 45-41
    egj105,    // chapter 46 has none and refers to the 85 C chapter (line 51582): Table 45-41
    n105,      // Table 47-19
    aecQ100,   // Table 48-20 (E/G/J; the N's chapter 49 refers to it first, line 53977)
};

// Table 45-41 "NVM Max Speed Characteristics", line 51006
inline constexpr WaitStateTable WaitStates85C{
  "SAM C21 Table 45-41",
  {{
    {Supply::from2V7, {19'000'000, 38'000'000, 64'000'000}},
    {Supply::from4V5, {20'000'000, 38'000'000, 64'000'000}},
  }}};
// Table 47-19 (SAM C20/C21 N, 105 C), line 52820
inline constexpr WaitStateTable WaitStatesN105C{
  "SAM C21 Table 47-19 (N, 105 C)",
  {{
    {Supply::from2V7, {19'000'000, 38'000'000, 48'000'000}},
    {Supply::from4V5, {19'000'000, 38'000'000, 48'000'000}},
  }}};
// Table 48-20 (AEC-Q100 grade 1, 125 C), line 53604
inline constexpr WaitStateTable WaitStates125C{
  "SAM C21 Table 48-20 (AEC-Q100, 125 C)",
  {{
    {Supply::from2V7, {14'000'000, 35'000'000, 47'000'000, 48'000'000}},
    {Supply::from4V5, {17'000'000, 35'000'000, 48'000'000, 48'000'000}},
  }}};

// NVMCTRL.CTRLB.RWS is 4 bits (27.8.2, line 19544); "when switching to a higher AHB frequency, the
// number of wait states shall be adapted to the future frequency first" (27.5.2, line 19012).
template<std::uint64_t Hz,
         Supply        S,
         Rating        R = Rating::egj85>
consteval unsigned waitStates() {
    if constexpr(R == Rating::egj85 || R == Rating::egj105) {
        return Nvm::waitStatesFrom<Hz, S, WaitStates85C>();
    } else if constexpr(R == Rating::n105) {
        return Nvm::waitStatesFrom<Hz, S, WaitStatesN105C>();
    } else {
        return Nvm::waitStatesFrom<Hz, S, WaitStates125C>();
    }
}

// GCLK generators: DIV bits (Table 16-3, line 6389): generator 0 8 bits, 1 16 bits, 2-8 8 bits;
// DIVSEL = 1 divides by 2^(N + 1) (GENCTRL.DIVSEL, line 6425). This sheet gives no "Maximum
// Division Factor" column; the D21 (15.8.5) and the E5x (Table 14-3) give 2^(bits + 1) for the same
// field, taken here by analogy.
consteval unsigned gclkDivBits(unsigned generator) { return generator == 1 ? 16 : 8; }

consteval unsigned long long gclkMaxDivision(unsigned generator) {
    return 1ULL << (gclkDivBits(generator) + 1);
}
}   // namespace Kvasir::ClockLimits::C21

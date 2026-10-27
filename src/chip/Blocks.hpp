#pragma once

// The SAM C21's clock gates (chip_atsam_common ClockGate.hpp, kvasir/Hal/Block.hpp): every block PM::Enable names,
// with the MCLK mask field that clocks it (DS60001479M 17.8.6 AHBMASK, 17.8.7 APBAMASK, 17.8.8 APBBMASK, 17.8.9
// APBCMASK). Rows marked open at boot have their bit set out of reset - AHBMASK resets to 0x00003CFF, APBAMASK to
// 0x00000FFF, APBBMASK to 0x00000007, APBCMASK to 0 - and claim nothing; a Startup-phase write into any other block
// needs a listed PM::Enable of it. A block on two clock domains has a row per mask bit (17.6.2.6). Only blocks the
// part's SVD has: SERCOM4/5 and CAN1 on the G17A only (on the E17A their enable is a compile error, not a write to
// a reserved bit); TC5-TC7 (APBDMASK, the N parts) and the PTC (no register set) have no row.
//
// The SERCOM and CAN rows also name what the shared drivers read through Sam::Block<Regs>: the interrupt line (Table
// 12-3 "Peripherals Configuration Summary SAM C21 E/G/J", IRQ Line, md l.3547-3554; one line per SERCOM and CAN) and
// a SERCOM's DMAC trigger sources (Table 25-2 "Peripheral Trigger Source", md l.17130-17147: SERCOMn RX = 2 + 2n,
// TX = 3 + 2n), as the generated enumerators. A SERCOM the part does not have has no row, so a driver on it does not
// compile.

#include "Dmac_Traits.hpp"
#include "Interrupt.hpp"
#include "atsam_common/ClockGate.hpp"
#include "peripherals/AC.hpp"
#include "peripherals/ADC.hpp"
#include "peripherals/CAN.hpp"
#include "peripherals/CCL.hpp"
#include "peripherals/DAC.hpp"
#include "peripherals/DIVAS.hpp"
#include "peripherals/DMAC.hpp"
#include "peripherals/DSU.hpp"
#include "peripherals/EIC.hpp"
#include "peripherals/EVSYS.hpp"
#include "peripherals/FREQM.hpp"
#include "peripherals/GCLK.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/NVMCTRL.hpp"
#include "peripherals/OSC32KCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"
#include "peripherals/PM.hpp"
#include "peripherals/PORT.hpp"
#include "peripherals/RSTC.hpp"
#include "peripherals/RTC_MODE0.hpp"
#include "peripherals/SDADC.hpp"
#include "peripherals/SERCOM_I2CM.hpp"
#include "peripherals/SUPC.hpp"
#include "peripherals/TCC.hpp"
#include "peripherals/TC_COUNT16.hpp"
#include "peripherals/TSENS.hpp"
#include "peripherals/WDT.hpp"

namespace Kvasir { namespace Chip { namespace Blocks {
    namespace P = Peripheral;
    using M     = P::MCLK::Registers<>;
    using I     = ::Kvasir::Interrupt;
    using T     = ::Kvasir::DMAC::Traits::DmacTraits::TriggerSource;
    template<typename... Lines>
    using Irqs = brigand::list<Lines...>;
    template<typename Regs, auto const& Field, bool OpenAtBoot = false, typename Lines = Irqs<>>
    using Row = ::Kvasir::Sam::GateRow<M::baseAddr, Regs::baseAddr, Field, OpenAtBoot, Lines>;
    // a SERCOM: its gate, its interrupt line and its DMAC trigger sources
    template<unsigned N, auto const& Field, typename Lines, T Rx, T Tx>
    using SercomRow = ::Kvasir::Sam::
      GateRow<M::baseAddr, P::SERCOM_I2CM::Registers<N>::baseAddr, Field, false, Lines, Rx, Tx>;

    // clang-format off
    using Table = ::Kvasir::Hal::BlockTable<
        // two gates (17.6.2.6): AHBMASK bit 3, APBBMASK bit 1
        Row<P::DSU::Registers<>, M::AHBMASK::dsu_, true>,
        Row<P::DSU::Registers<>, M::APBBMASK::dsu_, true>,
        // two gates: AHBMASK bit 5, APBBMASK bit 2
        Row<P::NVMCTRL::Registers<>, M::AHBMASK::nvmctrl_, true>,
        Row<P::NVMCTRL::Registers<>, M::APBBMASK::nvmctrl_, true>,
        Row<P::DMAC::Registers<>, M::AHBMASK::dmac_, true>,
        Row<P::CAN::Registers<0>, M::AHBMASK::can0_, false, Irqs<decltype(I::can0)>>,
        Row<P::DIVAS::Registers<>, M::AHBMASK::divas_, true>,
        Row<P::PM::Registers<>, M::APBAMASK::pm_, true>,
        Row<P::MCLK::Registers<>, M::APBAMASK::mclk_, true>,
        Row<P::RSTC::Registers<>, M::APBAMASK::rstc_, true>,
        Row<P::OSCCTRL::Registers<>, M::APBAMASK::oscctrl_, true>,
        Row<P::OSC32KCTRL::Registers<>, M::APBAMASK::osc32kctrl_, true>,
        Row<P::SUPC::Registers<>, M::APBAMASK::supc_, true>,
        Row<P::GCLK::Registers<>, M::APBAMASK::gclk_, true>,
        Row<P::WDT::Registers<>, M::APBAMASK::wdt_, true>,
        Row<P::RTC_MODE0::Registers<>, M::APBAMASK::rtc_, true>,
        Row<P::EIC::Registers<>, M::APBAMASK::eic_, true>,
        Row<P::FREQM::Registers<>, M::APBAMASK::freqm_, true>,
        Row<P::TSENS::Registers<>, M::APBAMASK::tsens_>,
        Row<P::PORT::Registers<>, M::APBBMASK::port_, true>,
        Row<P::EVSYS::Registers<>, M::APBCMASK::evsys_>,
        SercomRow<0, M::APBCMASK::sercom0_, Irqs<decltype(I::sercom0)>, T::sercom0_rx, T::sercom0_tx>,
        SercomRow<1, M::APBCMASK::sercom1_, Irqs<decltype(I::sercom1)>, T::sercom1_rx, T::sercom1_tx>,
        SercomRow<2, M::APBCMASK::sercom2_, Irqs<decltype(I::sercom2)>, T::sercom2_rx, T::sercom2_tx>,
        SercomRow<3, M::APBCMASK::sercom3_, Irqs<decltype(I::sercom3)>, T::sercom3_rx, T::sercom3_tx>,
        Row<P::TCC::Registers<0>, M::APBCMASK::tcc0_>,
        Row<P::TCC::Registers<1>, M::APBCMASK::tcc1_>,
        Row<P::TCC::Registers<2>, M::APBCMASK::tcc2_>,
        Row<P::TC_COUNT16::Registers<0>, M::APBCMASK::tc0_>,
        Row<P::TC_COUNT16::Registers<1>, M::APBCMASK::tc1_>,
        Row<P::TC_COUNT16::Registers<2>, M::APBCMASK::tc2_>,
        Row<P::TC_COUNT16::Registers<3>, M::APBCMASK::tc3_>,
        Row<P::TC_COUNT16::Registers<4>, M::APBCMASK::tc4_>,
        Row<P::ADC::Registers<0>, M::APBCMASK::adc0_>,
        Row<P::ADC::Registers<1>, M::APBCMASK::adc1_>,
        Row<P::SDADC::Registers<>, M::APBCMASK::sdadc_>,
        Row<P::AC::Registers<>, M::APBCMASK::ac_>,
        Row<P::DAC::Registers<>, M::APBCMASK::dac_>,
        Row<P::CCL::Registers<>, M::APBCMASK::ccl_>
#ifdef KVASIR_CHIP_ATSAMC21G17A
        ,Row<P::CAN::Registers<1>, M::AHBMASK::can1_, false, Irqs<decltype(I::can1)>>
        ,SercomRow<4, M::APBCMASK::sercom4_, Irqs<decltype(I::sercom4)>, T::sercom4_rx, T::sercom4_tx>
        ,SercomRow<5, M::APBCMASK::sercom5_, Irqs<decltype(I::sercom5)>, T::sercom5_rx, T::sercom5_tx>
#endif
        >;
    // clang-format on
}}}   // namespace Kvasir::Chip::Blocks

template<>
struct Kvasir::Hal::BlockGates<Kvasir::Tag::User> {
    using type = Kvasir::Chip::Blocks::Table;
};

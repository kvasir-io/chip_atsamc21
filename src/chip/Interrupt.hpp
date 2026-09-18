#pragma once
#include "kvasir/Common/Interrupt.hpp"

#include <array>

namespace Kvasir {
namespace Interrupt {
    template<int I>
    using Type = ::Kvasir::Nvic::Index<I>;

    static constexpr Type<-14> nonMaskableInt{};
    static constexpr Type<-13> hardFault{};
    static constexpr Type<-5>  sVCall{};
    static constexpr Type<-2>  pendSV{};
    static constexpr Type<-1>  systick{};
    static constexpr Type<0>   pm{};
    static constexpr Type<1>   wdt{};
    static constexpr Type<2>   rtc{};
    static constexpr Type<3>   eic{};
    static constexpr Type<4>   freqm{};
    static constexpr Type<5>   tsens{};
    static constexpr Type<6>   nvmctrl{};
    static constexpr Type<7>   dmac{};
    static constexpr Type<8>   evsys{};
    static constexpr Type<9>   sercom0{};
    static constexpr Type<10>  sercom1{};
    static constexpr Type<11>  sercom2{};
    static constexpr Type<12>  sercom3{};
    static constexpr Type<15>  can0{};
    static constexpr Type<17>  tcc0{};
    static constexpr Type<18>  tcc1{};
    static constexpr Type<19>  tcc2{};
    static constexpr Type<20>  tc0{};
    static constexpr Type<21>  tc1{};
    static constexpr Type<22>  tc2{};
    static constexpr Type<23>  tc3{};
    static constexpr Type<24>  tc4{};
    static constexpr Type<25>  adc0{};
    static constexpr Type<26>  adc1{};
    static constexpr Type<27>  ac{};
    static constexpr Type<28>  dac{};
    static constexpr Type<29>  sdadc{};
    static constexpr Type<30>  ptc{};
}   // namespace Interrupt

namespace Nvic {
    using namespace Kvasir::Interrupt;

    template<>
    struct InterruptOffsetTraits<void> {
        static constexpr int begin = -14;
        /// Half open: the last vector is PTC at 30.
        static constexpr int end = 31;
        /// The Cortex-M0+ vectors this part does not use, and the three gaps in its own table
        /// (13, 14 and 16 - the SVD names no interrupt at those). hardFault (-13) is *not* here:
        /// it is served, and FaultInterruptIndexs below names it.
        static constexpr std::array disabled = {-12, -11, -10, -9, -8, -7, -6, -4, -3, 13, 14, 16};
        static constexpr std::array noEnable
          = {nonMaskableInt.index(), sVCall.index(), pendSV.index()};
        static constexpr std::array noDisable
          = {nonMaskableInt.index(), sVCall.index(), pendSV.index()};
        static constexpr std::array noSetPending = {sVCall.index(), hardFault.index()};
        static constexpr std::array noClearPending
          = {nonMaskableInt.index(), sVCall.index(), hardFault.index()};
        static constexpr std::array noSetPriority = {nonMaskableInt.index(), hardFault.index()};

        using FaultInterruptIndexs           = brigand::list<decltype(hardFault)>;
        using FaultInterruptIndexsNeedEnable = brigand::list<>;
    };

}   // namespace Nvic
}   // namespace Kvasir

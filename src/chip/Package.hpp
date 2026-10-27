#pragma once
// The SAM C21's names the shared drivers of atsam_common use, in one place: atsam_common/StartUp.hpp asserts
// Kvasir::Sam::ChipPackage (atsam_common/ChipTraits.hpp lists what each member must give) on it.

#include "CAN_Traits.hpp"
#include "Dmac_Traits.hpp"
#include "Interrupt.hpp"
#include "NVMCTRL_Traits.hpp"
#include "Sercom_Traits.hpp"
#include "Serial_Number_Traits.hpp"
#include "Start.hpp"
#include "chip/atsam_common/SercomPads.hpp"
#include "peripherals/SERCOM_SPIM.hpp"
#include "peripherals/SERCOM_USART_INT.hpp"

#include <cstdint>

namespace Kvasir { namespace Sam {
    struct Package {
        using Sercom = Kvasir::Sercom::Traits::SercomTraits;
        using Dmac   = Kvasir::DMAC::Traits::DmacTraits;
        using Nvm    = Kvasir::NVMCTRL::Traits;
        using Start  = Kvasir::Startup::Sam::StartTraits;
        template<unsigned Line>
        using EicVector = Kvasir::Interrupt::EicExtIntVector<Line>;
        template<unsigned I>
        using SercomSpi = Kvasir::Peripheral::SERCOM_SPIM::Registers<I>;
        template<unsigned I>
        using SercomUsart = Kvasir::Peripheral::SERCOM_USART_INT::Registers<I>;
        using SercomPads  = Kvasir::Sam::Pads::SamD2xC2x;
        static constexpr auto const& serialNumberAddresses
          = Kvasir::SerialNumber::Traits::addresses;
        // The chip differences the shared drivers read (atsam_common/ChipTraits.hpp): a SYNCBUSY register for the EIC
        // (DS60001479M 26.8.4, md l.18545) and the WDT (23.8.7, md l.12720); the WDT counts the fixed 1.024 kHz
        // CLK_WDT_OSC (23.5.3); the DMAC's CRC is turned on by CTRL.CRCENABLE (25.8.1 md l.16279)
        static constexpr bool          eicSyncBusyInStatus = false;
        static constexpr bool          wdtSyncBusyInStatus = false;
        static constexpr std::uint32_t wdtFixedClockHz     = 1'024;
        static constexpr bool          dmacHasCrcEnable    = true;
        static constexpr bool          hasCan              = true;
        using Can                                          = Kvasir::CAN::Traits::CanTraits;
    };
}}   // namespace Kvasir::Sam

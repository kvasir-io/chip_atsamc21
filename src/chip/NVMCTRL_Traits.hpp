#pragma once

#include "peripherals/NVMCTRL.hpp"

namespace Kvasir { namespace NVMCTRL { namespace Traits {

    using NVM = Kvasir::Peripheral::NVMCTRL::Registers<>;

    static inline bool ready() { return 0 != apply(read(NVM::INTFLAG::ready)); }

    static inline void command_write_page() {
        apply(write(NVM::CTRLA::CMDValC::wp), write(NVM::CTRLA::CMDEXValC::key));
    }

    static inline void command_erase_row() {
        apply(write(NVM::CTRLA::CMDValC::er), write(NVM::CTRLA::CMDEXValC::key));
    }

    static inline void command_page_buffer_clear() {
        apply(write(NVM::CTRLA::CMDValC::pbc), write(NVM::CTRLA::CMDEXValC::key));
    }

    static inline void command_write_rww() {
        apply(write(NVM::CTRLA::CMDValC::rwweewp), write(NVM::CTRLA::CMDEXValC::key));
    }

    static inline void command_erase_rww() {
        apply(write(NVM::CTRLA::CMDValC::rwweeer), write(NVM::CTRLA::CMDEXValC::key));
    }

    static constexpr void enable_cache() {}

    static constexpr void disable_cache() {}

    template<typename T>
    void set_addr(T addr) {
        apply(write(NVM::ADDR::addr,
                    (reinterpret_cast<std::uint32_t>(addr) / 2)
                      & Kvasir::Register::maskFromRange(21, 0)));
    }

    static constexpr std::size_t PageSize    = 64;
    static constexpr std::size_t PagesPerRow = 4;
}}}   // namespace Kvasir::NVMCTRL::Traits

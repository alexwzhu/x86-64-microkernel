#pragma once

#include <stdint.h>

namespace gdt {

constexpr uint16_t KERNEL_CODE = 0x08;
constexpr uint16_t KERNEL_DATA = 0x10;
constexpr uint16_t USER_CODE   = 0x18 | 3;
constexpr uint16_t USER_DATA   = 0x20 | 3;
constexpr uint16_t TSS_SEL     = 0x28;

void init();

} // namespace gdt

#pragma once

#include <stdint.h>

namespace idt {

void init();
void set_gate(uint8_t vector, void (*handler)(), uint8_t ist = 0, uint8_t dpl = 0);

} // namespace idt

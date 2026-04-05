#pragma once

#include <stdint.h>

namespace pit {

void init(uint32_t frequency_hz);
uint64_t get_ticks();

} // namespace pit

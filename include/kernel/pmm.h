#pragma once

#include <stdint.h>

namespace pmm {

constexpr uint64_t PAGE_SIZE = 4096;

void init();
uint64_t alloc_frame();
void free_frame(uint64_t addr);
uint64_t used_frames();
uint64_t total_frames();

} // namespace pmm

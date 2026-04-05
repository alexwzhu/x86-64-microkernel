#pragma once

#include <stdint.h>

namespace heap {

void init();
void* kmalloc(uint64_t size);
void kfree(void* ptr);

} // namespace heap

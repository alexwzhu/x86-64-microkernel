#pragma once

#include <stdint.h>

namespace vmm {

constexpr uint64_t PAGE_PRESENT  = 1 << 0;
constexpr uint64_t PAGE_WRITABLE = 1 << 1;
constexpr uint64_t PAGE_USER     = 1 << 2;

void init();
void map_page(uint64_t virt, uint64_t phys, uint64_t flags);
void unmap_page(uint64_t virt);

} // namespace vmm

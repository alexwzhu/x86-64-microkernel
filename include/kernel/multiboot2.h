#pragma once

#include <stdint.h>

namespace multiboot2 {

struct MemoryRegion {
    uint64_t base;
    uint64_t length;
    uint32_t type; // 1 = usable, 2 = reserved, 3 = ACPI reclaimable
};

constexpr int MAX_REGIONS = 32;

struct MemoryMap {
    MemoryRegion regions[MAX_REGIONS];
    int count;
};

void parse(uint32_t* info_ptr);
const MemoryMap& get_memory_map();

} // namespace multiboot2

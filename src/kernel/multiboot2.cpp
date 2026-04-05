#include "kernel/multiboot2.h"
#include "kernel/serial.h"

static multiboot2::MemoryMap mmap = {};

namespace multiboot2 {

void parse(uint32_t* info_ptr) {
    // first 4 bytes = total size, next 4 = reserved
    uint8_t* ptr = reinterpret_cast<uint8_t*>(info_ptr) + 8;
    uint8_t* end = reinterpret_cast<uint8_t*>(info_ptr) + *info_ptr;

    while (ptr < end) {
        uint32_t tag_type = *reinterpret_cast<uint32_t*>(ptr);
        uint32_t tag_size = *reinterpret_cast<uint32_t*>(ptr + 4);

        if (tag_type == 0) break; // end tag

        // type 6 = memory map
        if (tag_type == 6) {
            uint32_t entry_size = *reinterpret_cast<uint32_t*>(ptr + 8);
            uint8_t* entry = ptr + 16;
            uint8_t* tag_end = ptr + tag_size;

            while (entry < tag_end && mmap.count < MAX_REGIONS) {
                uint64_t base   = *reinterpret_cast<uint64_t*>(entry);
                uint64_t length = *reinterpret_cast<uint64_t*>(entry + 8);
                uint32_t type   = *reinterpret_cast<uint32_t*>(entry + 16);

                mmap.regions[mmap.count++] = {base, length, type};

                serial::print("  mem: ");
                serial::print_hex(base);
                serial::print(" - ");
                serial::print_hex(base + length);
                serial::print(type == 1 ? " [usable]\n" : " [reserved]\n");

                entry += entry_size;
            }
        }

        // tags are 8-byte aligned
        ptr += (tag_size + 7) & ~7;
    }
}

const MemoryMap& get_memory_map() {
    return mmap;
}

} // namespace multiboot2

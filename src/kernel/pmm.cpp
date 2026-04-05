#include "kernel/pmm.h"
#include "kernel/multiboot2.h"
#include "kernel/serial.h"

extern "C" char _kernel_end;

// bitmap — supports up to 128 MiB (32768 pages)
static constexpr uint64_t MAX_PAGES = 32768;
static uint8_t bitmap[MAX_PAGES / 8];
static uint64_t num_frames = 0;

static void set_bit(uint64_t frame) {
    bitmap[frame / 8] |= (1 << (frame % 8));
}

static void clear_bit(uint64_t frame) {
    bitmap[frame / 8] &= ~(1 << (frame % 8));
}

static bool test_bit(uint64_t frame) {
    return bitmap[frame / 8] & (1 << (frame % 8));
}

namespace pmm {

void init() {
    // mark everything as used
    for (uint64_t i = 0; i < MAX_PAGES / 8; i++)
        bitmap[i] = 0xFF;

    const auto& mmap = multiboot2::get_memory_map();
    uint64_t kernel_end = reinterpret_cast<uint64_t>(&_kernel_end);

    // align kernel_end up to page boundary
    kernel_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // free usable regions
    for (int i = 0; i < mmap.count; i++) {
        if (mmap.regions[i].type != 1) continue;

        uint64_t base = mmap.regions[i].base;
        uint64_t end  = base + mmap.regions[i].length;

        // page-align
        base = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        end  = end & ~(PAGE_SIZE - 1);

        for (uint64_t addr = base; addr < end; addr += PAGE_SIZE) {
            uint64_t frame = addr / PAGE_SIZE;
            if (frame >= MAX_PAGES) break;

            // don't free pages below the kernel
            if (addr < kernel_end) continue;

            clear_bit(frame);
            if (frame >= num_frames) num_frames = frame + 1;
        }
    }

    // also keep first 1 MiB reserved (BIOS, VGA, etc)
    for (uint64_t i = 0; i < (0x100000 / PAGE_SIZE); i++)
        set_bit(i);

    serial::print("  free frames: ");
    serial::print_dec(total_frames() - used_frames());
    serial::print(" / ");
    serial::print_dec(total_frames());
    serial::print("\n");
}

uint64_t alloc_frame() {
    for (uint64_t i = 0; i < num_frames; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            return i * PAGE_SIZE;
        }
    }
    serial::print("pmm: out of memory!\n");
    return 0;
}

void free_frame(uint64_t addr) {
    clear_bit(addr / PAGE_SIZE);
}

uint64_t used_frames() {
    uint64_t count = 0;
    for (uint64_t i = 0; i < num_frames; i++)
        if (test_bit(i)) count++;
    return count;
}

uint64_t total_frames() {
    return num_frames;
}

} // namespace pmm

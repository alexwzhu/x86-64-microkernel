#include "kernel/vmm.h"
#include "kernel/pmm.h"
#include "kernel/serial.h"
#include "kernel/multiboot2.h"

// page table entry helpers
static constexpr uint64_t ENTRY_MASK = 0x000FFFFFFFFFF000ULL;

static uint64_t* get_or_create_table(uint64_t* table, int index, uint64_t flags) {
    if (!(table[index] & vmm::PAGE_PRESENT)) {
        uint64_t frame = pmm::alloc_frame();
        // zero the new table
        uint64_t* new_table = reinterpret_cast<uint64_t*>(frame);
        for (int i = 0; i < 512; i++) new_table[i] = 0;
        table[index] = frame | flags;
    }
    return reinterpret_cast<uint64_t*>(table[index] & ENTRY_MASK);
}

// extract page table indices from virtual address
static int pml4_index(uint64_t virt) { return (virt >> 39) & 0x1FF; }
static int pdpt_index(uint64_t virt) { return (virt >> 30) & 0x1FF; }
static int pd_index(uint64_t virt)   { return (virt >> 21) & 0x1FF; }
static int pt_index(uint64_t virt)   { return (virt >> 12) & 0x1FF; }

static uint64_t* kernel_pml4 = nullptr;

namespace vmm {

void map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t f = PAGE_PRESENT | flags;
    uint64_t* pdpt = get_or_create_table(kernel_pml4, pml4_index(virt), f);
    uint64_t* pd   = get_or_create_table(pdpt, pdpt_index(virt), f);
    uint64_t* pt   = get_or_create_table(pd, pd_index(virt), f);
    pt[pt_index(virt)] = (phys & ENTRY_MASK) | PAGE_PRESENT | flags;

    // flush TLB for this page
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void unmap_page(uint64_t virt) {
    int i4 = pml4_index(virt);
    if (!(kernel_pml4[i4] & PAGE_PRESENT)) return;
    uint64_t* pdpt = reinterpret_cast<uint64_t*>(kernel_pml4[i4] & ENTRY_MASK);

    int i3 = pdpt_index(virt);
    if (!(pdpt[i3] & PAGE_PRESENT)) return;
    uint64_t* pd = reinterpret_cast<uint64_t*>(pdpt[i3] & ENTRY_MASK);

    int i2 = pd_index(virt);
    if (!(pd[i2] & PAGE_PRESENT)) return;
    uint64_t* pt = reinterpret_cast<uint64_t*>(pd[i2] & ENTRY_MASK);

    pt[pt_index(virt)] = 0;
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void init() {
    // allocate a new PML4
    kernel_pml4 = reinterpret_cast<uint64_t*>(pmm::alloc_frame());
    for (int i = 0; i < 512; i++) kernel_pml4[i] = 0;

    // identity-map all usable memory
    const auto& mmap = multiboot2::get_memory_map();
    uint64_t max_addr = 0;

    for (int i = 0; i < mmap.count; i++) {
        uint64_t end = mmap.regions[i].base + mmap.regions[i].length;
        if (end > max_addr) max_addr = end;
    }

    // identity-map from 0 to max_addr
    for (uint64_t addr = 0; addr < max_addr; addr += pmm::PAGE_SIZE) {
        map_page(addr, addr, PAGE_WRITABLE);
    }

    // switch to our new page tables
    asm volatile("mov %0, %%cr3" : : "r"(reinterpret_cast<uint64_t>(kernel_pml4)) : "memory");

    serial::print("  mapped up to ");
    serial::print_hex(max_addr);
    serial::print("\n");
}

} // namespace vmm

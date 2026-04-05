#include "kernel/gdt.h"

namespace {

struct __attribute__((packed)) GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags_limit_high;
    uint8_t  base_high;
};

struct __attribute__((packed)) TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
};

struct __attribute__((packed)) GDTPointer {
    uint16_t limit;
    uint64_t base;
};

GDTEntry gdt_entries[7] = {};
TSS tss = {};
GDTPointer gdt_ptr = {};

void set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt_entries[index].limit_low        = limit & 0xFFFF;
    gdt_entries[index].base_low         = base & 0xFFFF;
    gdt_entries[index].base_mid         = (base >> 16) & 0xFF;
    gdt_entries[index].access           = access;
    gdt_entries[index].flags_limit_high = ((flags & 0x0F) << 4) | ((limit >> 16) & 0x0F);
    gdt_entries[index].base_high        = (base >> 24) & 0xFF;
}

} // anonymous namespace

namespace gdt {

void init() {
    set_entry(0, 0, 0,       0,    0);    // null
    set_entry(1, 0, 0xFFFFF, 0x9A, 0xA);  // kernel code
    set_entry(2, 0, 0xFFFFF, 0x92, 0xC);  // kernel data
    set_entry(3, 0, 0xFFFFF, 0xFA, 0xA);  // user code
    set_entry(4, 0, 0xFFFFF, 0xF2, 0xC);  // user data

    // TSS takes two GDT slots in long mode
    uint64_t tss_addr = reinterpret_cast<uint64_t>(&tss);
    uint32_t tss_size = sizeof(TSS) - 1;

    gdt_entries[5].limit_low        = tss_size & 0xFFFF;
    gdt_entries[5].base_low         = tss_addr & 0xFFFF;
    gdt_entries[5].base_mid         = (tss_addr >> 16) & 0xFF;
    gdt_entries[5].access           = 0x89; // present, 64-bit TSS available
    gdt_entries[5].flags_limit_high = (tss_size >> 16) & 0x0F;
    gdt_entries[5].base_high        = (tss_addr >> 24) & 0xFF;

    // upper 32 bits of TSS base go in the next slot
    uint32_t* tss_high = reinterpret_cast<uint32_t*>(&gdt_entries[6]);
    tss_high[0] = (tss_addr >> 32) & 0xFFFFFFFF;
    tss_high[1] = 0;

    tss.iopb_offset = sizeof(TSS);

    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base  = reinterpret_cast<uint64_t>(&gdt_entries);
    asm volatile("lgdt %0" : : "m"(gdt_ptr));

    // reload CS via far return
    asm volatile(
        "push $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        : : : "rax", "memory"
    );

    // reload data segments
    asm volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        : : : "ax", "memory"
    );

    asm volatile("ltr %0" : : "r"(static_cast<uint16_t>(TSS_SEL)));
}

} // namespace gdt

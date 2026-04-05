#include "kernel/idt.h"
#include "kernel/isr.h"
#include "kernel/gdt.h"

namespace {

struct __attribute__((packed)) IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
};

struct __attribute__((packed)) IDTPointer {
    uint16_t limit;
    uint64_t base;
};

IDTEntry idt_entries[256] = {};
IDTPointer idt_ptr = {};

} // anonymous namespace

namespace idt {

void set_gate(uint8_t vector, void (*handler)(), uint8_t ist, uint8_t dpl) {
    uint64_t addr = reinterpret_cast<uint64_t>(handler);
    idt_entries[vector].offset_low  = addr & 0xFFFF;
    idt_entries[vector].selector    = gdt::KERNEL_CODE;
    idt_entries[vector].ist         = ist & 0x7;
    idt_entries[vector].type_attr   = 0x8E | ((dpl & 0x3) << 5);
    idt_entries[vector].offset_mid  = (addr >> 16) & 0xFFFF;
    idt_entries[vector].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt_entries[vector].reserved    = 0;
}

void init() {
    isr::register_all();
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base  = reinterpret_cast<uint64_t>(&idt_entries);
    asm volatile("lidt %0" : : "m"(idt_ptr));
}

} // namespace idt

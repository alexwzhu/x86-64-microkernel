#include "kernel/serial.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/pit.h"
#include "kernel/multiboot2.h"
#include "kernel/pmm.h"
#include "kernel/vmm.h"
#include "kernel/heap.h"

extern "C" void kernel_main(uint32_t* multiboot_info) {
    serial::init();
    serial::print("Booting kernel...\n");

    gdt::init();
    serial::print("GDT loaded\n");

    idt::init();
    serial::print("IDT loaded\n");

    pic::init();
    serial::print("PIC remapped\n");

    pit::init(100);
    serial::print("PIT started at 100 Hz\n");

    serial::print("Parsing memory map...\n");
    multiboot2::parse(multiboot_info);

    serial::print("PMM init...\n");
    pmm::init();

    serial::print("VMM init...\n");
    vmm::init();

    serial::print("Heap init...\n");
    heap::init();

    // quick test: allocate, use, free
    void* p = heap::kmalloc(128);
    serial::print("kmalloc(128) = ");
    serial::print_hex(reinterpret_cast<uint64_t>(p));
    serial::print("\n");
    heap::kfree(p);

    asm volatile("sti");
    serial::print("Interrupts enabled\n");

    while (true) {
        asm volatile("hlt");
    }
}

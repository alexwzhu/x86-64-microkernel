#include "kernel/isr.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/serial.h"

extern "C" {
    void isr0();  void isr1();  void isr2();  void isr3();
    void isr4();  void isr5();  void isr6();  void isr7();
    void isr8();  void isr9();  void isr10(); void isr11();
    void isr12(); void isr13(); void isr14(); void isr15();
    void isr16(); void isr17(); void isr18(); void isr19();
    void isr20(); void isr21(); void isr22(); void isr23();
    void isr24(); void isr25(); void isr26(); void isr27();
    void isr28(); void isr29(); void isr30(); void isr31();
    void isr32(); void isr33(); void isr34(); void isr35();
    void isr36(); void isr37(); void isr38(); void isr39();
    void isr40(); void isr41(); void isr42(); void isr43();
    void isr44(); void isr45(); void isr46(); void isr47();
}

static const char* exception_names[] = {
    "Division By Zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FP Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD FP Exception",
    "Virtualization",
    "Control Protection",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Exception",
    "Reserved",
};

static isr::IRQHandler irq_handlers[16] = {};

namespace isr {

void register_all() {
    idt::set_gate(0,  isr0);  idt::set_gate(1,  isr1);
    idt::set_gate(2,  isr2);  idt::set_gate(3,  isr3);
    idt::set_gate(4,  isr4);  idt::set_gate(5,  isr5);
    idt::set_gate(6,  isr6);  idt::set_gate(7,  isr7);
    idt::set_gate(8,  isr8);  idt::set_gate(9,  isr9);
    idt::set_gate(10, isr10); idt::set_gate(11, isr11);
    idt::set_gate(12, isr12); idt::set_gate(13, isr13);
    idt::set_gate(14, isr14); idt::set_gate(15, isr15);
    idt::set_gate(16, isr16); idt::set_gate(17, isr17);
    idt::set_gate(18, isr18); idt::set_gate(19, isr19);
    idt::set_gate(20, isr20); idt::set_gate(21, isr21);
    idt::set_gate(22, isr22); idt::set_gate(23, isr23);
    idt::set_gate(24, isr24); idt::set_gate(25, isr25);
    idt::set_gate(26, isr26); idt::set_gate(27, isr27);
    idt::set_gate(28, isr28); idt::set_gate(29, isr29);
    idt::set_gate(30, isr30); idt::set_gate(31, isr31);

    idt::set_gate(32, isr32); idt::set_gate(33, isr33);
    idt::set_gate(34, isr34); idt::set_gate(35, isr35);
    idt::set_gate(36, isr36); idt::set_gate(37, isr37);
    idt::set_gate(38, isr38); idt::set_gate(39, isr39);
    idt::set_gate(40, isr40); idt::set_gate(41, isr41);
    idt::set_gate(42, isr42); idt::set_gate(43, isr43);
    idt::set_gate(44, isr44); idt::set_gate(45, isr45);
    idt::set_gate(46, isr46); idt::set_gate(47, isr47);
}

void irq_register(uint8_t irq, IRQHandler handler) {
    if (irq < 16)
        irq_handlers[irq] = handler;
}

} // namespace isr

extern "C" void isr_common_handler(isr::InterruptFrame* frame) {
    uint64_t vector = frame->vector;

    if (vector < 32) {
        serial::print("\n!!! EXCEPTION: ");
        serial::print(exception_names[vector]);
        serial::print(" (vector ");
        serial::print_dec(vector);
        serial::print(")\n");
        serial::print("  error: "); serial::print_hex(frame->error_code); serial::print("\n");
        serial::print("  rip:   "); serial::print_hex(frame->rip); serial::print("\n");
        serial::print("  cs:    "); serial::print_hex(frame->cs); serial::print("\n");
        serial::print("  rsp:   "); serial::print_hex(frame->rsp); serial::print("\n");

        if (vector == 14) {
            uint64_t cr2;
            asm volatile("mov %%cr2, %0" : "=r"(cr2));
            serial::print("  cr2:   "); serial::print_hex(cr2); serial::print("\n");
        }

        serial::print("  halted.\n");
        while (true) asm volatile("cli; hlt");
    }

    if (vector >= 32 && vector < 48) {
        uint8_t irq = vector - 32;
        if (irq_handlers[irq])
            irq_handlers[irq](frame);
        pic::send_eoi(irq);
    }
}

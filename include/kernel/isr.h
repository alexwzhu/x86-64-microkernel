#pragma once

#include <stdint.h>

namespace isr {

// has to match the push order in interrupts.asm exactly
struct __attribute__((packed)) InterruptFrame {
    uint64_t ds;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

using IRQHandler = void (*)(InterruptFrame*);

void register_all();
void irq_register(uint8_t irq, IRQHandler handler);

} // namespace isr

extern "C" void isr_common_handler(isr::InterruptFrame* frame);

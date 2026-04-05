#include "kernel/pic.h"
#include "kernel/io.h"

static constexpr uint16_t PIC1_CMD  = 0x20;
static constexpr uint16_t PIC1_DATA = 0x21;
static constexpr uint16_t PIC2_CMD  = 0xA0;
static constexpr uint16_t PIC2_DATA = 0xA1;

namespace pic {

void init() {
    // ICW1: init + expect ICW4
    io::outb(PIC1_CMD, 0x11); io::wait();
    io::outb(PIC2_CMD, 0x11); io::wait();

    // ICW2: remap IRQs — PIC1 to vector 32, PIC2 to vector 40
    io::outb(PIC1_DATA, 0x20); io::wait();
    io::outb(PIC2_DATA, 0x28); io::wait();

    // ICW3: cascade wiring
    io::outb(PIC1_DATA, 0x04); io::wait(); // slave on IRQ2
    io::outb(PIC2_DATA, 0x02); io::wait(); // cascade identity

    // ICW4: 8086 mode
    io::outb(PIC1_DATA, 0x01); io::wait();
    io::outb(PIC2_DATA, 0x01); io::wait();

    // mask everything, drivers unmask what they need
    io::outb(PIC1_DATA, 0xFF);
    io::outb(PIC2_DATA, 0xFF);

    // keep cascade line open so PIC2 can talk to PIC1
    clear_mask(2);
}

void send_eoi(uint8_t irq) {
    if (irq >= 8)
        io::outb(PIC2_CMD, 0x20);
    io::outb(PIC1_CMD, 0x20);
}

void set_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    io::outb(port, io::inb(port) | (1 << (irq % 8)));
}

void clear_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    io::outb(port, io::inb(port) & ~(1 << (irq % 8)));
}

} // namespace pic

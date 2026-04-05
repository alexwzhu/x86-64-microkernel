#include "kernel/serial.h"
#include "kernel/io.h"

static constexpr uint16_t COM1 = 0x3F8;

namespace serial {

void init() {
    io::outb(COM1 + 1, 0x00);  // disable interrupts
    io::outb(COM1 + 3, 0x80);  // enable DLAB (baud rate divisor)
    io::outb(COM1 + 0, 0x01);  // 115200 baud
    io::outb(COM1 + 1, 0x00);
    io::outb(COM1 + 3, 0x03);  // 8N1
    io::outb(COM1 + 2, 0xC7);  // enable FIFO
    io::outb(COM1 + 4, 0x0B);  // RTS/DSR set
}

void putchar(char c) {
    while ((io::inb(COM1 + 5) & 0x20) == 0) {}
    io::outb(COM1, static_cast<uint8_t>(c));
}

void print(const char* msg) {
    for (int i = 0; msg[i] != '\0'; i++)
        putchar(msg[i]);
}

void print_hex(uint64_t value) {
    print("0x");
    bool leading = true;
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t nibble = (value >> i) & 0xF;
        if (nibble == 0 && leading && i > 0) continue;
        leading = false;
        putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    if (leading) putchar('0');
}

void print_dec(uint64_t value) {
    if (value == 0) { putchar('0'); return; }
    char buf[20];
    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    for (int j = i - 1; j >= 0; j--)
        putchar(buf[j]);
}

} // namespace serial

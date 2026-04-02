#include "kernel/vga.h"

static inline void serial_write(char c) {
    // Write to COM1 (port 0x3F8)
    asm volatile("outb %0, %1" : : "a"(c), "Nd"(static_cast<uint16_t>(0x3F8)));
}

static void serial_print(const char* msg) {
    for (int i = 0; msg[i] != '\0'; i++) {
        serial_write(msg[i]);
    }
}

extern "C" void kernel_main() {
    const char* msg = "Hello, kernel!";

    // Print to serial (always works)
    serial_print(msg);
    serial_write('\n');

    // Also try VGA text buffer (works in BIOS mode)
    const uint8_t color = vga::make_color(vga::WHITE, vga::BLACK);
    for (int i = 0; i < vga::WIDTH * vga::HEIGHT; i++) {
        vga::BUFFER[i] = vga::entry(' ', color);
    }
    for (int i = 0; msg[i] != '\0'; i++) {
        vga::BUFFER[i] = vga::entry(msg[i], color);
    }

    // Halt
    while (true) {
        asm volatile("hlt");
    }
}

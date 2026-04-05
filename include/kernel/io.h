#pragma once

#include <stdint.h>

namespace io {

inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// tiny delay — writing to port 0x80 wastes ~1 microsecond
inline void wait() {
    outb(0x80, 0);
}

} // namespace io

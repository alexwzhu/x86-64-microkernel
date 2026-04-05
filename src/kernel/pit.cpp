#include "kernel/pit.h"
#include "kernel/io.h"
#include "kernel/isr.h"
#include "kernel/pic.h"
#include "kernel/serial.h"

static constexpr uint16_t PIT_CH0 = 0x40;
static constexpr uint16_t PIT_CMD = 0x43;
static constexpr uint32_t PIT_FREQ = 1193182;

static volatile uint64_t tick_count = 0;

static void pit_handler(isr::InterruptFrame*) {
    tick_count = tick_count + 1;
    if (tick_count % 100 == 0)
        serial::print(".");
}

namespace pit {

void init(uint32_t frequency_hz) {
    uint16_t divisor = static_cast<uint16_t>(PIT_FREQ / frequency_hz);

    io::outb(PIT_CMD, 0x36); // channel 0, lobyte/hibyte, square wave, binary
    io::outb(PIT_CH0, divisor & 0xFF);
    io::outb(PIT_CH0, (divisor >> 8) & 0xFF);

    isr::irq_register(0, pit_handler);
    pic::clear_mask(0);
}

uint64_t get_ticks() {
    return tick_count;
}

} // namespace pit

#pragma once

#include <stdint.h>

namespace serial {

void init();
void putchar(char c);
void print(const char* msg);
void print_hex(uint64_t value);
void print_dec(uint64_t value);

} // namespace serial

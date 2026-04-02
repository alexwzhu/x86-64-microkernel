## x86-64-microkernel

A from-scratch microkernel for x86-64, written in C++ and assembly. Following the OSDev wiki.

### Dependencies

You need four tools to build and run this

#### NASM

```bash
brew install nasm
```
NASM is an assembler. It takes `boot.asm` file (which is human-readable x86 assembly) turns it into an object file the linker can work with. I write the bootloader entry point in assembly because the CPU starts in 32-bit protected mode and we need raw control over registers, page tables, and CPU flags to get into 64-bit long mode. C++ cannot do this.

#### x86_64-elf-gcc

```bash
brew install x86_64-elf-gcc
```

This is a cross-compiler. It gives `x86_64-elf-g++` (the C++ compiler) and `x86_64-elf-ld`, which is the linker.

Because I'm compiling code on Mac that's meant to run on an x86-64 machine with no operating system, I can't use the normal system compiler. This cross-compiler can produce freestanding binaries with zero assumptions about a host environment.

#### x86_64-elf-grub

```bash
brew install x86_64-elf-grub
```

I use `grub-mkrescue` to package compiled kernel into a bootable ISO image. 

GRUB, the bootloader, is what initally runs the booted ISO. When the ISO boots, GRUB is what actually runs first. It handles all the annoying hardware setup  and then loads kernel into memory. It is able to find the kernel through `grub.cfg`. 

#### QEMU

```bash
brew install qemu
```

QEMU is an emulator. `qemu-system-x86_64` simulates an entire x86-64 PC. 

### Building

Configure then run:

```bash
cmake -B build
cmake --build build --target run
```

This compiles the assembly and C++, links them into kernel.bin, packages it into an ISO with GRUB, and launches QEMU. QEMU window should output "Hello, kernel!"


cmake_minimum_required(VERSION 3.20)
project(microkernel ASM_NASM CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Freestanding C++ flags — no standard library, no runtime features
set(CMAKE_CXX_FLAGS "-ffreestanding -nostdlib -fno-exceptions -fno-rtti -mno-red-zone -Wall -Wextra")
set(CMAKE_ASM_NASM_FLAGS "-f elf64")

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker.ld)
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT} -nostdlib")

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Sources
set(ASM_SOURCES
    src/boot/boot.asm
    src/boot/interrupts.asm
)

set(CXX_SOURCES
    src/kernel/kernel.cpp
    src/kernel/serial.cpp
    src/kernel/gdt.cpp
    src/kernel/idt.cpp
    src/kernel/isr.cpp
    src/kernel/pic.cpp
    src/kernel/pit.cpp
    src/kernel/multiboot2.cpp
    src/kernel/pmm.cpp
    src/kernel/vmm.cpp
    src/kernel/heap.cpp
)

# Build kernel binary
add_executable(kernel.bin ${ASM_SOURCES} ${CXX_SOURCES})
set_target_properties(kernel.bin PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# --- ISO and QEMU targets ---

set(ISO_DIR ${CMAKE_BINARY_DIR}/isodir)
set(ISO_FILE ${CMAKE_BINARY_DIR}/myos.iso)

add_custom_command(
    OUTPUT ${ISO_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.bin> ${ISO_DIR}/boot/kernel.bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/iso/boot/grub/grub.cfg ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND x86_64-elf-grub-mkrescue -o ${ISO_FILE} ${ISO_DIR}
    DEPENDS kernel.bin ${CMAKE_SOURCE_DIR}/iso/boot/grub/grub.cfg
    COMMENT "Building bootable ISO"
)

add_custom_target(iso DEPENDS ${ISO_FILE})

set(OVMF_CODE /opt/homebrew/Cellar/qemu/10.2.2/share/qemu/edk2-x86_64-code.fd)
set(OVMF_VARS /opt/homebrew/Cellar/qemu/10.2.2/share/qemu/edk2-i386-vars.fd)

add_custom_target(run
    COMMAND qemu-system-x86_64
        -drive if=pflash,format=raw,readonly=on,file=${OVMF_CODE}
        -drive if=pflash,format=raw,readonly=on,file=${OVMF_VARS}
        -cdrom ${ISO_FILE}
        -serial stdio
    DEPENDS iso
    COMMENT "Launching QEMU"
)

add_custom_target(debug
    COMMAND qemu-system-x86_64
        -drive if=pflash,format=raw,readonly=on,file=${OVMF_CODE}
        -drive if=pflash,format=raw,readonly=on,file=${OVMF_VARS}
        -cdrom ${ISO_FILE} -s -S -d int,cpu_reset -no-reboot
    DEPENDS iso
    COMMENT "Launching QEMU in debug mode (waiting for GDB on :1234)"
)

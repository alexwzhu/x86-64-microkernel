#include "kernel/heap.h"
#include "kernel/pmm.h"
#include "kernel/vmm.h"
#include "kernel/serial.h"

// heap starts at 16 MiB virtual, grows upward
static constexpr uint64_t HEAP_START = 0x1000000;
static constexpr uint64_t HEAP_INITIAL_PAGES = 16; // 64 KiB to start

struct BlockHeader {
    uint64_t size;
    bool free;
    BlockHeader* next;
};

static BlockHeader* head = nullptr;
static uint64_t heap_end = HEAP_START;

static void expand_heap(uint64_t pages) {
    for (uint64_t i = 0; i < pages; i++) {
        uint64_t frame = pmm::alloc_frame();
        vmm::map_page(heap_end, frame, vmm::PAGE_WRITABLE);
        heap_end += pmm::PAGE_SIZE;
    }
}

namespace heap {

void init() {
    expand_heap(HEAP_INITIAL_PAGES);
    head = reinterpret_cast<BlockHeader*>(HEAP_START);
    head->size = (HEAP_INITIAL_PAGES * pmm::PAGE_SIZE) - sizeof(BlockHeader);
    head->free = true;
    head->next = nullptr;

    serial::print("  heap: ");
    serial::print_hex(HEAP_START);
    serial::print(" - ");
    serial::print_hex(heap_end);
    serial::print("\n");
}

void* kmalloc(uint64_t size) {
    // align to 16 bytes
    size = (size + 15) & ~15ULL;

    BlockHeader* block = head;
    while (block) {
        if (block->free && block->size >= size) {
            // split if there's room for another block
            if (block->size > size + sizeof(BlockHeader) + 16) {
                auto* new_block = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<uint8_t*>(block) + sizeof(BlockHeader) + size
                );
                new_block->size = block->size - size - sizeof(BlockHeader);
                new_block->free = true;
                new_block->next = block->next;
                block->next = new_block;
                block->size = size;
            }
            block->free = false;
            return reinterpret_cast<void*>(
                reinterpret_cast<uint8_t*>(block) + sizeof(BlockHeader)
            );
        }
        block = block->next;
    }

    serial::print("heap: out of memory!\n");
    return nullptr;
}

void kfree(void* ptr) {
    if (!ptr) return;
    auto* block = reinterpret_cast<BlockHeader*>(
        reinterpret_cast<uint8_t*>(ptr) - sizeof(BlockHeader)
    );
    block->free = true;

    // merge adjacent free blocks
    BlockHeader* b = head;
    while (b) {
        if (b->free && b->next && b->next->free) {
            b->size += sizeof(BlockHeader) + b->next->size;
            b->next = b->next->next;
            continue; // check again in case of triple merge
        }
        b = b->next;
    }
}

} // namespace heap

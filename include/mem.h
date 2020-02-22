#ifndef MEM
#define MEM
#include <stddef.h>
#include <stdint.h>
#include <flags.h>

extern uint8_t kheap[KERNEL_HEAP_SIZE];
extern uint8_t bumpHeap[KERNEL_NOFREE_SIZE];

void memset(void* ptr, uint8_t val, size_t size);
void memcpy(void* dest, void* src, size_t size);

void* slabAlloc(size_t size);
void slabFree(void* ptr);
void initSlabAllocator();

void initHeapAllocator();
void* heapAlloc(size_t size);
void heapFree(void* ptr);
void validateHeap();

void* kalloc(size_t size);
void kfree(void* ptr);

void* kalloc_permanent(size_t size);
void* kalloc_permanent(size_t, uint32_t alignment);
#endif

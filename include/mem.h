#ifndef MEM
#define MEM
#include <stddef.h>
#include <stdint.h>
#include <flags.h>

extern uint8_t kheap[KERNEL_HEAP_SIZE];

void memset(void* ptr, uint8_t val, size_t size);

void* slabAlloc(size_t size);
void slabFree(void* ptr);
void initSlabAllocator();

void initHeapAllocator();
void* heapAlloc(size_t size);
void heapFree(void* ptr);

void* kalloc(size_t size);
void kfree(void* ptr);
#endif

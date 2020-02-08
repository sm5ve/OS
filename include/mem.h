#ifndef MEM
#define MEM
#include <stddef.h>
#include <stdint.h>
void memset(void* ptr, uint8_t val, size_t size);
void* slabAlloc(size_t size);
void slabFree(void* ptr);
void initSlabAllocator();
#endif

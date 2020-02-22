#ifndef MEM
#define MEM
#include <stddef.h>
#include <stdint.h>
#include <flags.h>

extern uint8_t kheap[KERNEL_HEAP_SIZE];
extern uint8_t bumpHeap[KERNEL_NOFREE_SIZE];

class SlabAlloc{
public:
	SlabAlloc(void* buffer, size_t buffer_size, size_t slab_size, uint8_t* freed_map = NULL);
	~SlabAlloc();
	
	void* alloc();
	void free(void* ptr);

	bool isPtrInRange(void* ptr);
	size_t getSlabSize();
private:
	void* buffer;
	size_t buffer_size;
	size_t slab_size;
	uint8_t* freed_map;
	void* free_ptr;
};

class HeapAlloc{
	
};

void memset(void* ptr, uint8_t val, size_t size);
void memcpy(void* dest, void* src, size_t size);

void initHeapAllocator();
void* heapAlloc(size_t size);
void heapFree(void* ptr);
void validateHeap();

void initKalloc();
void* kalloc(size_t size);
void kfree(void* ptr);

void* kalloc_permanent(size_t size);
void* kalloc_permanent(size_t, uint32_t alignment);

inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };
#endif

#include <mem.h>
#include <flags.h>

uint8_t bumpHeap[KERNEL_NOFREE_SIZE];
void* bumpPtr = &bumpHeap;

void* kalloc_permanent(size_t size){
	void* ret = bumpPtr;
	bumpPtr += size;
	return ret;
}

void* kalloc_permanent(size_t size, uint32_t alignment){
	uint32_t offset = (uint32_t)bumpPtr;
	offset = offset % alignment;
	offset = alignment - offset;
	offset = offset % alignment;
	bumpPtr += offset;
	return kalloc_permanent(size);
}

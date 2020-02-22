#include <mem.h>
#include <assert.h>

uint8_t SLAB_8_BUFF[KERNEL_SLAB_8_SIZE];
uint8_t SLAB_16_BUFF[KERNEL_SLAB_16_SIZE];
uint8_t SLAB_32_BUFF[KERNEL_SLAB_32_SIZE];
uint8_t SLAB_64_BUFF[KERNEL_SLAB_64_SIZE];

uint8_t SLAB_8_FREED_MAP[KERNEL_SLAB_8_SIZE / 8];
uint8_t SLAB_16_FREED_MAP[KERNEL_SLAB_16_SIZE / 8];
uint8_t SLAB_32_FREED_MAP[KERNEL_SLAB_32_SIZE / 8];
uint8_t SLAB_64_FREED_MAP[KERNEL_SLAB_64_SIZE / 8];

uint8_t uninitialized_slab_allocs[4 * sizeof(SlabAlloc)];
SlabAlloc* slab_allocs;

void initKalloc(){
	initHeapAllocator();
	slab_allocs = (SlabAlloc*)uninitialized_slab_allocs;
	new (&slab_allocs[0]) SlabAlloc(SLAB_8_BUFF, KERNEL_SLAB_8_SIZE, 8, SLAB_8_FREED_MAP);
	new (&slab_allocs[1]) SlabAlloc(SLAB_16_BUFF, KERNEL_SLAB_16_SIZE, 16, SLAB_16_FREED_MAP);
	new (&slab_allocs[2]) SlabAlloc(SLAB_32_BUFF, KERNEL_SLAB_32_SIZE, 32, SLAB_32_FREED_MAP);
	new (&slab_allocs[3]) SlabAlloc(SLAB_64_BUFF, KERNEL_SLAB_64_SIZE, 64, SLAB_64_FREED_MAP);
}

void* kalloc(size_t size){
	for(int i = 0; i < 4; i++){
		if(size <= slab_allocs[i].getSlabSize()){
			return slab_allocs[i].alloc();
		}
	}
	return heapAlloc(size);
}

void kfree(void* ptr){
	if(ptr >= (void*) kheap && ptr < (void*)((int)kheap + KERNEL_HEAP_SIZE)){
		heapFree(ptr);
		return;
	}
	else if(ptr >= (void*) bumpHeap && ptr < (void*)((int)bumpHeap + KERNEL_NOFREE_SIZE)){
		assert(false, "Error: tried to free unfreeable pointer\n");
	}
	for(int i = 0; i < 4; i++){
		if(slab_allocs[i].isPtrInRange(ptr)){
			slab_allocs[i].free(ptr);
			return;
		}
	}
	assert(false, "Error: attemped to free unallocated pointer");
}

void *operator new(size_t size)
{
    return kalloc(size);
}
 
void *operator new[](size_t size)
{
    return kalloc(size);
}
 
void operator delete(void *p)
{
    kfree(p);
}
 
void operator delete[](void *p)
{
    kfree(p);
}

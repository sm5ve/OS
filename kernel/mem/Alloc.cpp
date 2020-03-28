#include <mem.h>
#include <assert.h>
#include <klib/SerialDevice.h>
#include <debug.h>
#include <interrupts.h>

//#define DEBUG_PRINTS

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

uint8_t KERNEL_HEAP[KERNEL_HEAP_SIZE];
uint32_t KERNEL_HEAP_PTRS[KERNEL_HEAP_SIZE/KERNEL_HEAP_GRANULARITY];

uint16_t uninitialized_heap_alloc[sizeof(HeapAlloc)];
HeapAlloc* heap_alloc; 

void initKalloc(){
	slab_allocs = (SlabAlloc*)uninitialized_slab_allocs;
	new (&slab_allocs[0]) SlabAlloc(SLAB_8_BUFF, KERNEL_SLAB_8_SIZE, 8, SLAB_8_FREED_MAP);
	new (&slab_allocs[1]) SlabAlloc(SLAB_16_BUFF, KERNEL_SLAB_16_SIZE, 16, SLAB_16_FREED_MAP);
	new (&slab_allocs[2]) SlabAlloc(SLAB_32_BUFF, KERNEL_SLAB_32_SIZE, 32, SLAB_32_FREED_MAP);
	new (&slab_allocs[3]) SlabAlloc(SLAB_64_BUFF, KERNEL_SLAB_64_SIZE, 64, SLAB_64_FREED_MAP);
	
	heap_alloc = (HeapAlloc*)uninitialized_heap_alloc;
	new (heap_alloc) HeapAlloc(KERNEL_HEAP, KERNEL_HEAP_SIZE, KERNEL_HEAP_PTRS, KERNEL_HEAP_GRANULARITY);
	initBumpHeap();
}

void* kalloc(size_t size){
	DisableInterrupts d;
	void* out = NULL;
	for(int i = 0; i < 4; i++){
		if(size <= slab_allocs[i].getSlabSize()){
			out = slab_allocs[i].alloc();
			#ifdef DEBUG_PRINTS
			SD::the() << "Malloc'd using slab allocator of size " << slab_allocs[i].getSlabSize() << " for ptr ";
			#endif
			break;
		}
	}
	if(out == NULL){
		#ifdef DEBUG_PRINTS
		SD::the() << "Malloc'd using heap allocator for ptr ";
		#endif
		out = heap_alloc -> alloc(size);
	}
	#ifdef DEBUG_PRINTS
	SD::the() << out << "\n";
	stackTrace();
	#endif
	return out;
}

void kfree(void* ptr){
	DisableInterrupts d;
	#ifdef DEBUG_PRINTS
	SD::the() << "Freeing " << ptr << "\n";
	stackTrace();
	#endif
	if(heap_alloc -> isInHeap(ptr)){
		heap_alloc -> free(ptr);
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
	SD::the() << ptr << "\n";
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

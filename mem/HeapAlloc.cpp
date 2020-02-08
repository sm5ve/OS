#include <mem.h>
#include <assert.h>
#include <flags.h>

uint8_t kheap[KERNEL_HEAP_SIZE];

void initHeapAllocator(){

}

void* heapAlloc(size_t size){
	assert(false, "Heap unimplemented");
}

void heapFree(void* ptr){
	assert(ptr >= (void*) kheap && ptr < (void*)(kheap + KERNEL_HEAP_SIZE), "Error: tried to free pointer outside of kernel heap range");
	assert(false, "Heap unimplemented");	
}

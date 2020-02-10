#include <mem.h>
#include <assert.h>

void* kalloc(size_t size){
	if(size <= 64){
		return slabAlloc(size);
	}
	return heapAlloc(size);
}

void kfree(void* ptr){
	if(ptr >= (void*) kheap && ptr < (void*)((int)kheap + KERNEL_HEAP_SIZE)){
		heapFree(ptr);
	}
	else{
		slabFree(ptr);
	}
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

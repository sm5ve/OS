#include <flags.h>
#include <mem.h>
#include <assert.h>

#define SLAB_ALLOC_DOUBLE_FREE_GUARD
#define DOUBLE_FREE_INDEX(ptr, size) (((int)ptr - (int)slabs[size])/sizes[size])

//#define SLAB_ALLOC_IMPLEMENTATION_DEBUG

#ifdef SLAB_ALLOC_IMPLEMENTATION_DEBUG
#include <klib/SerialPrinter.h>
#endif

uint8_t slab8[KERNEL_SLAB_8_SIZE];
uint8_t slab16[KERNEL_SLAB_16_SIZE];
uint8_t slab32[KERNEL_SLAB_32_SIZE];
uint8_t slab64[KERNEL_SLAB_64_SIZE];

#ifdef SLAB_ALLOC_DOUBLE_FREE_GUARD
bool slab8freed[KERNEL_SLAB_8_SIZE];
bool slab16freed[KERNEL_SLAB_16_SIZE];
bool slab32freed[KERNEL_SLAB_32_SIZE];
bool slab64freed[KERNEL_SLAB_64_SIZE];

bool* freedmap[4];
#endif

enum SlabSize{
	SIZE8 = 0,
	SIZE16 = 1,
	SIZE32 = 2,
	SIZE64 = 3
};

int sizes[4] = {8, 16, 32, 64};
int slab_alloc_sizes[4] = {KERNEL_SLAB_8_SIZE, KERNEL_SLAB_16_SIZE, KERNEL_SLAB_32_SIZE, KERNEL_SLAB_64_SIZE};

void* slabFreePtr[4];
void* slabMaxInitPtr[4];
void* slabs[4];

#ifdef SLAB_ALLOC_IMPLEMENTATION_DEBUG
SerialPrinter printer;
#endif


void initSlabAllocator(){
	#ifdef SLAB_ALLOC_IMPLEMENTATION_DEBUG
	printer = SerialPrinter(COMPort::COM1);
	#endif

	#ifdef SLAB_ALLOC_DOUBLE_FREE_GUARD
	freedmap[0] = (bool*) slab8freed;
	freedmap[1] = (bool*) slab16freed;
	freedmap[2] = (bool*) slab32freed;
	freedmap[3] = (bool*) slab64freed;
	
	for(int n = 0; n < 4; n++){
		for(int i = 0; i < slab_alloc_sizes[n]; i++){
			freedmap[n][i] = true;
		}
	}
	#endif

	slabFreePtr[0] = (void*)slab8;
	slabFreePtr[1] = (void*)slab16;
	slabFreePtr[2] = (void*)slab32;
	slabFreePtr[3] = (void*)slab64;

	for(int i = 0; i < 4; i++){
		slabs[i] = slabFreePtr[i];
		slabMaxInitPtr[i] = slabs[i];
		*((void**)slabs[i]) = slabs[i] + sizes[i];	
	}
}

void* allocFromSlab(SlabSize size){
	
	#ifdef SLAB_ALLOC_IMPLEMENTATION_DEBUG
	printer << "Allocating slab of size " << size << "\n";
	#endif

	void* returnPtr = slabFreePtr[size];

	#ifdef SLAB_ALLOC_IMPLEMENTATION_DEBUG
	printer << "Found pointer at " << returnPtr << "\n";
	#endif

	void* nextFree = *((void**)returnPtr);
	
	assert(nextFree >= slabs[size], "Somehow set next free pointer below slab heap");
	assert(nextFree < slabs[size] + slab_alloc_sizes[size], "Ran out of space on slab heap");

	if(nextFree > slabMaxInitPtr[size]){
		slabMaxInitPtr[size] = nextFree;
		*((void**)nextFree) = nextFree + sizes[size];
	}

	slabFreePtr[size] = nextFree;

	memset(returnPtr, 0, (size_t)sizes[size]);	
	
	#ifdef SLAB_ALLOC_DOUBLE_FREE_GUARD
	assert(freedmap[size][DOUBLE_FREE_INDEX(returnPtr, size)], "Error: somehow allocated already allocated pointer");
	assert(freedmap[size][DOUBLE_FREE_INDEX(nextFree, size)], "Error: somehow set next freed pointer to already allocated pointer");
	freedmap[size][DOUBLE_FREE_INDEX(returnPtr, size)] = false;
	#endif	

	return returnPtr;	
}

void slabFree(void* ptr){
	SlabSize size;
	for(int i = 0; i < 5; i++){
		if(i == 4){
			assert(false, "Attempted to free slab outside of pools");
		}
		if(ptr >= slabs[i] && ptr < (slabs[i] + slab_alloc_sizes[i])){
			size = (SlabSize)i;
			break;
		}
	}

	#ifdef SLAB_ALLOC_DOUBLE_FREE_GUARD
    assert(!freedmap[size][DOUBLE_FREE_INDEX(ptr, size)], "Error: double freed pointer");
    assert(freedmap[size][DOUBLE_FREE_INDEX(slabFreePtr[size], size)], "Error: somehow next freed pointer isn't free");
	freedmap[size][DOUBLE_FREE_INDEX(ptr, size)] = true;
	#endif  

	
	*((void**)ptr) = slabFreePtr[size];
	slabFreePtr[size] = ptr;

	#ifdef SLAB_ALLOC_IMPLEMENTATION_DEBUG
	printer << "Freeing pointer " << ptr << "\n";
	printer << "Size class " << size << "\n";
	printer << "Setting value of slab to ptr " << *((void**)ptr) << "\n";
	printer << "slabFreePtr[size] = " << slabFreePtr[size] << "\n";
	#endif
}

void* slabAlloc(size_t size){
	assert(size <= 64, "Error: attemped to use slab allocator on size > 64");
	for(int i = 0; i < 4; i++){
		if(size <= sizes[i]){
			return allocFromSlab((SlabSize)i);
		}
	}	
}

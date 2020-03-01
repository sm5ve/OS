#include <flags.h>
#include <mem.h>
#include <assert.h>
#include <debug.h>

#define ZERO_MEMORY

SlabAlloc::SlabAlloc(void* buff, size_t buff_s, size_t slab_s, uint8_t* fmap){
	this -> buffer = buff;
	this -> buffer_size = buff_s;
	this -> slab_size = slab_s;
	this -> freed_map = fmap;
	this -> free_ptr = buff;
	assert(buffer_size >= 4, "Error: attemped to make slab allocator with slab size < 4 bytes"); //We're going to store a pointer to the next freed slab, which needs 4 bytes.
	for(uint32_t i = 0; i < buffer_size / slab_size - 1; i++){
		writeFreeSlab(i, (void*)((uint32_t)buffer + (i + 1) * slab_size));
	}
	writeFreeSlab(buffer_size / slab_size - 1, NULL);
	if(freed_map){
		memset(freed_map, 0xff, buff_s/8); //Make sure everything's set to free
	}
}

SlabAlloc::~SlabAlloc(){
	assert(false, "I'm not quite sure what to do here");
}

void* SlabAlloc::alloc(){
	void* out = this -> free_ptr;
	void* next = (void*)*((uint32_t*) out);
	assert(next != NULL, "Error: out of memory in slab allocator");
	if(freed_map){
		assert(isSlabFree(ptrToIndex(next)), "Error: next freed slab isn't actually free");
		setFreedState(ptrToIndex(out), false);
	}
	 
	#ifdef ZERO_MEMORY
	memset(out, 0, slab_size);
	#endif
	this -> free_ptr = next;
	return out;
}

void SlabAlloc::free(void* ptr){
	assert(this -> isPtrInRange(ptr), "Error: tried to free ptr outside of slab allocator buffer range");
	if(freed_map){
		if(isSlabFree(ptrToIndex(ptr))){
			stackTrace();
		}
		assert(!isSlabFree(ptrToIndex(ptr)), "Error: double free");
	}
	assert(((uint32_t) ptr - (uint32_t)buffer) % slab_size == 0, "Error: attempted to free misaligned pointer");
	writeFreeSlab(ptrToIndex(ptr), free_ptr);
	if(freed_map){
		setFreedState(ptrToIndex(ptr), true);
	}
	free_ptr = ptr;
}

inline bool SlabAlloc::isPtrInRange(void* ptr){
	return ((uint32_t) ptr >= (uint32_t) buffer) && ((uint32_t)ptr < (uint32_t)buffer + buffer_size);
}

inline void SlabAlloc::writeFreeSlab(uint32_t index, void* ptr){
	*((uint32_t*)((uint32_t) (buffer) + (index) * (slab_size)
)) = (uint32_t)(ptr);
}

inline void SlabAlloc::setFreedState(uint32_t index, bool bit){
	int b = bit ? 1 : 0;
	(freed_map)[(index) >> 3] &= (uint8_t)~(1 << ((index) & 7)); 
	(freed_map)[(index) >> 3] |= (uint8_t)((b) << ((index) & 7));
}

inline bool SlabAlloc::isSlabFree(uint32_t index){
	return (((freed_map[(index) >> 3]) >> ((index) & 7)) & 1) == 1;
}

inline uint32_t SlabAlloc::ptrToIndex(void* ptr){
	return((uint32_t)ptr - (uint32_t) buffer) / slab_size;
}

size_t SlabAlloc::getSlabSize(){
	return slab_size;
}

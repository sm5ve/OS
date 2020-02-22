#include <flags.h>
#include <mem.h>
#include <assert.h>

#define WRITE_FREE_SLAB(index,  ptr) (*((uint32_t*)((uint32_t) (buffer) + (index) * (slab_size))) = (uint32_t)(ptr));

#define SET_FREED_MAP_BIT(index, bit) {(freed_map)[(index) >> 3] &= ~(1 << ((index) & 7)); (freed_map)[(index) >> 3] |= ((bit) << ((index) & 7));};

#define GET_FREED_MAP_BIT(index) (((freed_map[(index) >> 3]) >> ((index) & 7)) & 1)

#define PTR_TO_INDEX(ptr) (((uint32_t)ptr - (uint32_t) buffer) / slab_size)

#define ZERO_MEMORY

SlabAlloc::SlabAlloc(void* buff, size_t buff_s, size_t slab_s, uint8_t* fmap){
	this -> buffer = buff;
	this -> buffer_size = buff_s;
	this -> slab_size = slab_s;
	this -> freed_map = fmap;
	this -> free_ptr = buff;
	assert(buffer_size >= 4, "Error: attemped to make slab allocator with slab size < 4 bytes"); //We're going to store a pointer to the next freed slab, which needs 4 bytes.
	for(uint32_t i = 0; i < buffer_size / slab_size - 1; i++){
		WRITE_FREE_SLAB(i, (uint32_t)buffer + (i + 1) * slab_size);
	}
	WRITE_FREE_SLAB(buffer_size / slab_size - 1, NULL);
	if(freed_map){
		memset(freed_map, 0, buff_s/8); //Make sure everything's set to free
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
		assert(GET_FREED_MAP_BIT(PTR_TO_INDEX(next)) == 0, "Error: next freed slab isn't actually free");
		SET_FREED_MAP_BIT(PTR_TO_INDEX(out), 1);
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
		assert(GET_FREED_MAP_BIT(PTR_TO_INDEX(ptr)) == 1, "Error: double free");
	}
	assert(((uint32_t) ptr - (uint32_t)buffer) % slab_size == 0, "Error: attempted to free misaligned pointer");
	WRITE_FREE_SLAB(PTR_TO_INDEX(ptr), free_ptr);
	if(freed_map){
		SET_FREED_MAP_BIT(PTR_TO_INDEX(ptr), 0);
	}
	free_ptr = ptr;
}

bool SlabAlloc::isPtrInRange(void* ptr){
	return ((uint32_t) ptr >= (uint32_t) buffer) && ((uint32_t)ptr < (uint32_t)buffer + buffer_size);
}

size_t SlabAlloc::getSlabSize(){
	return slab_size;
}

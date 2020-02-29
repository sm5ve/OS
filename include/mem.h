#ifndef MEM
#define MEM
#include <stddef.h>
#include <stdint.h>
#include <flags.h>
#include <klib/ds/BinaryHeap.h>

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

	void writeFreeSlab(uint32_t index, void* nextFree);
	void setFreedState(uint32_t index, bool bit);
	bool isSlabFree(uint32_t index);
	uint32_t ptrToIndex(void* ptr);
};

class HeapAlloc{
public:
	HeapAlloc(void* buffer, size_t buffer_size, uint32_t* ptr_buffer, size_t granularity);
	~HeapAlloc();

	void* alloc(size_t size);
	void free(void* ptr);
	
	void validateHeap();
	bool isInHeap(void* ptr);	
private:
	void* buffer;
	uint32_t* node_ptrs;
	size_t buffer_size;
	size_t granularity;
	uint8_t uninitialized_heap[sizeof(BinaryHeap<size_t>)];
	BinaryHeap<size_t>* heap;

	uint32_t ptrToIndex(void* ptr);
	void updatePtrBuffer(void* ptr, size_t size, bool free);
	BinaryTreeNode<size_t>* findNextBiggestNode(size_t size, BinaryTreeNode<size_t>* node = NULL);
	size_t alignSize(size_t);
	void* shrinkNode(BinaryTreeNode<size_t>*, size_t by_amount);
	bool isPtrFree(void*);
	void mergeAdjacentChunks(BinaryTreeNode<size_t>* left, BinaryTreeNode<size_t>* right);
};

void memset(void* ptr, uint8_t val, size_t size);
void memcpy(void* dest, void* src, size_t size);

void initKalloc();
void* kalloc(size_t size);
void kfree(void* ptr);

void initBumpHeap();
void* kalloc_permanent(size_t size);
void* kalloc_permanent(size_t, uint32_t alignment);

inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { }
inline void  operator delete[](void *, void *) throw() { }
#endif

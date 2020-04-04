#include <mem.h>
#include <assert.h>
#include <devices/SerialDevice.h>

HeapAlloc::HeapAlloc(void* b, size_t s, uint32_t* pbuff, size_t gran){
	heap = (BinaryHeap<size_t>*)uninitialized_heap;
	new (heap) BinaryHeap<size_t>(false);
	buffer = b;
	//SD::the() << "heap buffer from " << b << " to " << (void*)((uint32_t)b + s) << "\n";
	buffer_size = s;
	node_ptrs = pbuff;
	granularity = gran;
	heap -> makeNode(s, NULL, buffer); //Set up the initial unallocated heap
	updatePtrBuffer(buffer, buffer_size, true);
}

HeapAlloc::~HeapAlloc(){
	assert(false, "I don't know what to do here");
}

uint32_t HeapAlloc::ptrToIndex(void* ptr){
	return ((uint32_t)ptr - (uint32_t)buffer) / granularity;
}

void HeapAlloc::updatePtrBuffer(void* ptr, size_t size, bool free){
	for(uint32_t i = 0; i < size/granularity; i++){
		uint32_t ind = ptrToIndex(ptr) + i;
		assert((ind < buffer_size / granularity), "Error: index out of bounds");
		if(!free){
			node_ptrs[ind] = (uint32_t)size;
		}
		else{
			node_ptrs[ind] = (uint32_t)ptr;
		}
	}
}

BinaryTreeNode<size_t>* HeapAlloc::findNextBiggestNode(size_t size, BinaryTreeNode<size_t>* node){
	if(node == NULL){
		node = heap -> getRoot();
	}
	assert(node, "Error: heap is empty. We may be out of memory");
	if((node -> left() != NULL) && (node -> left() -> getValue() >= size)){
		if((node -> right() != NULL) && (node -> right() -> getValue() >= size) && (node -> right() -> getValue() < node -> left() -> getValue())){
			return findNextBiggestNode(size, node -> right());
		}
		return findNextBiggestNode(size, node -> left());
	}
	if((node -> right() != NULL) && (node -> right() -> getValue() >= size)){
		return findNextBiggestNode(size, node -> right());
	}
	assert(node -> getValue() >= size, "Error: heap out of memory");
	return node;
}

size_t HeapAlloc::alignSize(size_t size){
	if(size % granularity == 0){
		return size;
	}
	return (size - (size % granularity)) + granularity;
}

void* HeapAlloc::shrinkNode(BinaryTreeNode<size_t>* node, size_t by_amount){
	by_amount = alignSize(by_amount);
	assert(node -> getValue() >= by_amount, "Error: tried to shrink free block for more than its total size");
	if(by_amount == node -> getValue()){
		heap -> removeNode(node);
		updatePtrBuffer((void*)node, by_amount, false);
		return (void*)node;
	}
	size_t new_size = node -> getValue() - by_amount;
	heap -> removeNode(node);
	void* new_loc = (void*)((uint32_t)node + by_amount);
	heap -> makeNode(new_size, NULL, new_loc);
	updatePtrBuffer((void*)node, by_amount, false);
	updatePtrBuffer(new_loc, new_size, true);
	return (void*)node;
}

bool HeapAlloc::isInHeap(void* ptr){
	return ((uint32_t) ptr >= (uint32_t) buffer) && ((uint32_t)ptr < (uint32_t)buffer + buffer_size);
}

void* HeapAlloc::alloc(size_t size){
	size = alignSize(size);
	BinaryTreeNode<size_t>* to_shrink = findNextBiggestNode(size);
	shrinkNode(to_shrink, size);
	memset((void*)to_shrink, 0, size);
	assert(((uint32_t)to_shrink - (uint32_t)buffer) % granularity == 0, "Error: allocated misaligned memory\n");
	return (void*)to_shrink;	
}

bool HeapAlloc::isPtrFree(void* ptr){
	return isInHeap((void*)node_ptrs[ptrToIndex(ptr)]);
}

void HeapAlloc::mergeAdjacentChunks(BinaryTreeNode<size_t>* left, BinaryTreeNode<size_t>* right){
	assert(left < right, "Error: mixed up arguments for mergeAdjacentChunks");
	assert((uint32_t)left + left -> getValue() == (uint32_t) right, "Error: chunks not adjacent");
	
	size_t combined_size = left -> getValue() + right -> getValue();

	heap -> removeNode(left);
	heap -> removeNode(right);

	BinaryTreeNode<size_t>* new_node = heap -> makeNode(combined_size, NULL, (void*)left);
	updatePtrBuffer((void*)new_node, combined_size, true);
}

void HeapAlloc::free(void* ptr){
	assert(isInHeap(ptr), "Error: tried to free heap outside of pointer");
	assert(!isPtrFree(ptr), "Error: double free");
	assert(((uint32_t)ptr - (uint32_t)buffer) % granularity != 4, "Error: I think you deleted where you should've delete[]'d");
	assert(((uint32_t)ptr - (uint32_t)buffer) % granularity == 0, "Error: tried to free misaligned pointer");

	size_t size = node_ptrs[ptrToIndex(ptr)];
	
	BinaryTreeNode<size_t>* node = heap -> makeNode(size, NULL, ptr);
	updatePtrBuffer((void*)node, size, true);
	//Now we need to merge any adjacent free nodes
	
	BinaryTreeNode<size_t>* next = (BinaryTreeNode<size_t>*)((uint32_t)node + size);
	if((uint32_t)next < (uint32_t) buffer + buffer_size){
		if(isPtrFree(next)){
			mergeAdjacentChunks(node, next);
		}
	}
	if((uint32_t)node != (uint32_t)buffer){
		BinaryTreeNode<size_t>* prev = (BinaryTreeNode<size_t>*)node_ptrs[ptrToIndex(node) - 1];
		if(isInHeap(prev)){
			//This is where we're having our issue
			mergeAdjacentChunks(prev, node);
		}
	}
}

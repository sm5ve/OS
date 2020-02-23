#include <mem.h>
#include <klib/ds/BinaryHeap.h>
#include <assert.h>

HeapAlloc::HeapAlloc(void* b, size_t s, uint32_t* pbuff, size_t gran){
	heap = (BinaryHeap<size_t>*)uninitialized_heap;
	new (heap) BinaryHeap<size_t>(false);
	buffer = b;
	buffer_size = s;
	node_ptrs = pbuff;
	granularity = gran;
	BinaryTreeNode<size_t>* base = heap -> makeNode(s, NULL, buffer);
	updatePtrBuffer(buffer, buffer_size, true);
}

HeapAlloc::~HeapAlloc(){
	assert(false, "I don't know what to do here");
}

uint32_t HeapAlloc::ptrToIndex(void* ptr){
	return ((uint32_t)ptr - (uint32_t)buffer) / granularity;
}

void HeapAlloc::updatePtrBuffer(void* ptr, size_t size, bool free){
	for(int i = 0; i < size/granularity; i++){
		uint32_t ind = ptrToIndex(ptr) + i;
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
	BinaryTreeNode<size_t>* to_shrink = findNextBiggestNode(size);
	shrinkNode(to_shrink, size);
	return (void*)to_shrink;	
}

void HeapAlloc::free(void* ptr){
	assert(false, "Unimplemented");
}

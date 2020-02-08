#include <mem.h>
#include <assert.h>
#include <flags.h>

#include <klib/SerialPrinter.h>

#define HEAP_INTEGRITY_CHECKS

uint8_t kheap[KERNEL_HEAP_SIZE];

struct HeapNode{
	HeapNode* parent;
	size_t size;
	size_t heapSize;
	HeapNode* left;
	HeapNode* right;
};

HeapNode* nodePtrs[KERNEL_HEAP_SIZE >> 7];
HeapNode* heapBase;

size_t availableSpace;

size_t alignSize(size_t size){
	if((size & 0x7f) == 0){
		return size;
	}
	return (size & ~0x7f) + 0x80;
}

HeapNode** ptrToNodeTableEntry(void* ptr){
	assert((int)ptr >= (int)kheap && (int)ptr < (int)kheap + KERNEL_HEAP_SIZE, "Error: tried to convert pointer from outside kernel heap to node table entry");
	return &nodePtrs[((int)ptr - (int)kheap) / 0x80];
}

size_t validateHeapFromBase(HeapNode* base){
	SerialPrinter p(COMPort::COM1);
	size_t enumeratedFreeSize = 0;
	if(base == NULL){
		return enumeratedFreeSize;
	}
	enumeratedFreeSize += base -> size;
	
	//p << "heapBase = " << heapBase << "\n";

	assert((uint32_t)base >= (uint32_t)kheap && (uint32_t)base < (uint32_t)kheap + KERNEL_HEAP_SIZE, "Error: HeapNode outsize of kernel heap");
	for(int i = 0; i * 0x80 < base -> size; i++){
		void* checkPtr = (void*)((int)base + i * 0x80);
		//p << "checkPtr = " << checkPtr << "\n";
		//p << "nodeEntry = " << *ptrToNodeTableEntry(checkPtr) << "\n";
		assert(*ptrToNodeTableEntry(checkPtr) == base, "Error: HeapNode table mismatch");
	}
	enumeratedFreeSize += validateHeapFromBase(base -> left);
	enumeratedFreeSize += validateHeapFromBase(base -> right);
	return enumeratedFreeSize;
}

void validateHeap(){
	//Confirm each entry in the heap is well-formed and enumerate the free space
	size_t enumeratedSize = validateHeapFromBase(heapBase);
	//Confirm the enumerated free space matches the available space counter
	assert(enumeratedSize == availableSpace, "Error: recorded free space does not match result from heap traversal");
	//Manually count the blocks in nodePtrs[] to similarly calculate the remaining free space
	enumeratedSize = 0;
	for(int i = 0; i < KERNEL_HEAP_SIZE >> 7; i++){
		if(nodePtrs[i] != NULL){
			enumeratedSize += 0x80;
		}
	}
	//Confirm this is consistent with the above calculations
	//This also confirms no claimed blocks think they are free in nodePtrs
	assert(enumeratedSize == availableSpace, "Error: claimed blocks erroneously declared unclaimed");
}

//TODO support arbitrary swaps
void swapNodes(HeapNode* parent, HeapNode* child){
	if(parent == heapBase){
		assert(false, "TODO: Figure out how to swap with heap base");
	}
	else{
		assert(parent -> parent != NULL, "Error: somehow we found an orphan node in the heap");
		if(parent -> parent -> left == parent){
			parent -> parent -> left = child;
		}
		else if(parent -> parent -> right == parent){
			parent -> parent -> right = child;
		}
		else{
			assert(false, "Error: somehow parent does not know of child in heap");
		}
		if(parent -> left == child){
			HeapNode* parentRight = parent -> right;
			HeapNode* childLeft = child -> left;
			HeapNode* childRight = child -> right;
			
			child -> left = parent;
			child -> right = parentRight;
			parent -> left = childLeft;
			parent -> right = childRight;
		}
		else if(parent -> right == child){
			HeapNode* parentLeft = parent -> left;
			HeapNode* childLeft = child -> left;
			HeapNode* childRight = child -> right;

			child -> right = parent;
			child -> left = parentLeft;
			parent -> left = childLeft;
			parent -> right = childRight;
		}
		else{
			assert(false, "Error: somehow parent does not know of child in heap");
		}
		size_t childSize = child -> heapSize;
		size_t parentSize = parent -> heapSize;
		child -> heapSize = parentSize;
		parent -> heapSize = parentSize;
		parent -> parent = child;
	}
}

void rebalance(HeapNode* ptr){
	//Ensure current node has a smaller size than its parent
	if(ptr -> parent != NULL){
		if(ptr -> parent -> size < ptr -> size){
			swapNodes(ptr -> parent, ptr);
			rebalance(ptr);
			return;
		}
	}
	//Ensure current node is larger than its left child
	if(ptr -> left != NULL){
		if(ptr -> left -> size > ptr -> size){
			if(ptr -> right != NULL){
				//If the right child is larger than the left, we swap the right child and current node
				if(ptr -> right -> size > ptr -> left -> size){
					swapNodes(ptr, ptr -> right);
					rebalance(ptr);
					return;
				}
				//Otherwise we swap the left child and current node
				else{
					swapNodes(ptr, ptr -> left);
					rebalance(ptr);
					return;
				}
			}
		}
	}
	//If we didn't do a swap above, we might still need to swap with the right child
	if(ptr -> right != NULL){
		if(ptr -> right -> size > ptr -> size){
			swapNodes(ptr, ptr -> right);
			rebalance(ptr);
			return;
		}
	}
}

//Finds where to put next node 
HeapNode* findNextParent(HeapNode* base){
	if(base -> left == NULL){
		return base;
	}
	if(base -> right == NULL){
		return base;
	}
	if(base -> left -> heapSize < base -> right -> heapSize){
		return findNextParent(base -> left);
	}
	return findNextParent(base -> right);
}

void addToParentSizes(HeapNode* parent, int diff){
	if(parent != NULL){
		parent -> heapSize += diff;
	}
	addToParentSizes(parent -> parent, diff);
}

HeapNode* makeNode(void* ptr, HeapNode* parent, size_t size){
	//Make sure size is multiple of 0x80
	size = alignSize(size);
	//Instantiate new node structure
	HeapNode node = {
		.parent = parent,
		.size = size,
		.heapSize = 1,
		.left = NULL,
		.right = NULL
	};
	//Actually write the node to the heap
	*((HeapNode*)ptr) = node;
	//Add pointer to node to node map
	HeapNode** tableBase = ptrToNodeTableEntry(ptr);
	for(int i = 0; i * 0x80 < size; i++){
		tableBase[i] = (HeapNode*) ptr;
	}
	//Add node to parent
	if(parent != NULL){
		if(parent -> left == NULL){
			parent -> left = (HeapNode*)ptr;
		}
		else if(parent -> right == NULL){
			parent -> right = (HeapNode*)ptr;
		}
		else{
			assert(false, "Attemped to add child node to full parent");
		}
		addToParentSizes(parent, 1);
	}
	//Finally return the ptr
	return (HeapNode*)ptr;
}

HeapNode* addNode(void* ptr, size_t size){
	HeapNode* parent = findNextParent(heapBase);
	HeapNode* out = makeNode(ptr, parent, size);
	rebalance(out);
	return out;
}

void removeNode(HeapNode* ptr){
	assert(false, "unimplemented");
}

HeapNode* findNode(HeapNode* base, size_t minSize){
	assert(base != NULL, "Tried to find free node from null base");
	assert(base -> size >= minSize, "Not enough space");
	if(base -> left != NULL && base -> left -> size >= minSize){
		return findNode(base -> left, minSize);
	}
	if(base -> right != NULL && base -> right -> size >= minSize){
		return findNode(base -> right, minSize);
	}
	return base;
}

HeapNode* shrinkNode(HeapNode* node, size_t amount){
	size_t size = alignSize(amount);
	assert(node -> size > size, "Error: Attempted to shrink heap node to a size larger than or equal to original size");
	//move HeapNode struct ahead size bytes
	HeapNode* out = (HeapNode*)((int)node + size);
	memcpy(out, node, sizeof(HeapNode));
	//reduce recorded size in structure
	out -> size -= size;
	availableSpace -= size;
	HeapNode** tablePtr = ptrToNodeTableEntry(node);
	//claim blocks in nodePtrs[] table
	for(int i = 0; i * 0x80 < size; i++){
		tablePtr[i] = NULL;
	}
	//update remaining blocks belongning to node
	tablePtr = ptrToNodeTableEntry(out);
	for(int i = 0; i * 0x80 < out -> size; i++){
		tablePtr[i] = out;
	}
	//update parent's child pointer to new HeapNode location
	if(out -> parent == NULL){
		//This means we must be the base heap node
		assert(node == heapBase, "Error: non-base heap node without parent");
		heapBase = out;
	}
	else if(out -> parent -> left == node){
		out -> parent -> left = out;
	}
	else if(out -> parent -> right == node){
		out -> parent -> right = out;
	}
	else{
		assert(false, "Error: somehow heap node's parent doesn't know of it");
	}
	rebalance(out);
	return out;
}

void initHeapAllocator(){
	memset(kheap, 0, KERNEL_HEAP_SIZE);
	memset(nodePtrs, 0, KERNEL_HEAP_SIZE >> 7);
	heapBase = makeNode(kheap, NULL, KERNEL_HEAP_SIZE);
	availableSpace = KERNEL_HEAP_SIZE;
	validateHeap(); //sanity check
}

void* heapAlloc(size_t size){
	size = alignSize(size);
	HeapNode* toShrink = findNode(heapBase, size);
	if(toShrink -> size == size){
		removeNode(toShrink);
		validateHeap();
		return toShrink;
	}
	HeapNode* newBase = shrinkNode(toShrink, size);
	validateHeap();
	return toShrink;
}

void heapFree(void* ptr){
	assert(ptr >= (void*) kheap && ptr < (void*)(kheap + KERNEL_HEAP_SIZE), "Error: tried to free pointer outside of kernel heap range");
	assert(false, "Heap unimplemented");	
}

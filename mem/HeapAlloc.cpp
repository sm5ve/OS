#include <mem.h>
#include <assert.h>
#include <flags.h>

//TODO remember to disable this once malloc's been better tested!
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

bool isHeapPtrValid(void* ptr){
	return (int)ptr >= (int)kheap && (int)ptr < (int)kheap + KERNEL_HEAP_SIZE;
}

HeapNode** ptrToNodeTableEntry(void* ptr){
	assert(isHeapPtrValid(ptr), "Error: tried to convert pointer from outside kernel heap to node table entry");
	return &nodePtrs[((int)ptr - (int)kheap) / 0x80];
}

size_t validateHeapFromBase(HeapNode* base){
	size_t enumeratedFreeSize = 0;
	if(base == NULL){
		return enumeratedFreeSize;
	}
	enumeratedFreeSize += base -> size;
	
	if(base -> parent != NULL){
		assert(base -> parent -> size >= base -> size, "Error: violation of heap property");
		assert(base -> parent -> left == base || base -> parent -> right == base, "Error: parent does not know of child");
	}
	else{
		assert(base == heapBase, "Error: orphan node");
	}
	assert((uint32_t)base >= (uint32_t)kheap && (uint32_t)base < (uint32_t)kheap + KERNEL_HEAP_SIZE, "Error: HeapNode outsize of kernel heap");
	for(int i = 0; i * 0x80 < base -> size; i++){
		void* checkPtr = (void*)((int)base + i * 0x80);
		assert(*ptrToNodeTableEntry(checkPtr) == base, "Error: HeapNode table mismatch");
	}
	enumeratedFreeSize += validateHeapFromBase(base -> left);
	enumeratedFreeSize += validateHeapFromBase(base -> right);
	return enumeratedFreeSize;
}

size_t validateHeapSizes(HeapNode* base){
	if(base == NULL){
		return 0;
	}
	else{
		assert(base -> heapSize >= 1, "Error: negative heap size");
	}
	size_t size = 1 + validateHeapSizes(base -> left) + validateHeapSizes(base -> right);
	assert(size == base -> heapSize, "Error: heap size mismatches");
	return size;
}

void validateHeap(){
	#ifndef HEAP_INTEGRITY_CHECKS
	return;
	#endif
	assert(heapBase -> parent == NULL, "Error: heap base thinks it has a parent");
	//Confirm each entry in the heap is well-formed and enumerate the free space
	size_t enumeratedSize = validateHeapFromBase(heapBase);
	//Confirm the enumerated free space matches the available space counter
	assert(enumeratedSize == availableSpace, "Error: recorded free space does not match result from heap traversal");
	//Manually count the blocks in nodePtrs[] to similarly calculate the remaining free space
	enumeratedSize = 0;
	validateHeapSizes(heapBase);
	for(int i = 0; i < KERNEL_HEAP_SIZE >> 7; i++){
		if(isHeapPtrValid(nodePtrs[i])){
			enumeratedSize += 0x80;
		}
	}
	//Confirm this is consistent with the above calculations
	//This also confirms no claimed blocks think they are free in nodePtrs
	assert(enumeratedSize == availableSpace, "Error: claimed blocks erroneously declared unclaimed");
}

void fixNodeChildParents(HeapNode* node){
	assert(node != NULL, "Error: null node");
	if(node -> left != NULL){
		node -> left -> parent = node;
	}
	if(node -> right != NULL){
		node -> right -> parent = node;
	}
}

void swapParentChildNodes(HeapNode* parent, HeapNode* child){
	assert(parent == child -> parent, "Error: tried to swap child with non-parent");
	if(parent == heapBase){
		parent -> parent = child;
		child -> parent = NULL;
		heapBase = child;

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
			
			child -> left = parentLeft;
			child -> right = parent;
			parent -> left = childLeft;
			parent -> right = childRight;
		}
		else{
			assert(false, "Error: parent does not know about child");
		}
		size_t childSize = child -> heapSize;
        size_t parentSize = parent -> heapSize;
        child -> heapSize = parentSize;
        parent -> heapSize = childSize;

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
		parent -> heapSize = childSize;
		child -> parent = parent -> parent;
		parent -> parent = child;
	}
	fixNodeChildParents(child);
	fixNodeChildParents(parent);
}

void swapNodes(HeapNode* n1, HeapNode* n2){
	assert(n1 != n2, "Error: tried to swap node with itself");
	if(n1 -> parent == n2){
		swapParentChildNodes(n2, n1);
	}
	else if(n2 -> parent == n1){
		swapParentChildNodes(n1, n2);
	}
	else{
		//In this case, there is no parent-child relation between n1 and n2
		HeapNode* n1parent = n1 -> parent;
		HeapNode* n2parent = n2 -> parent;
		HeapNode* n1left = n1 -> left;
		HeapNode* n2left = n2 -> left;
		HeapNode* n1right = n1 -> right;
		HeapNode* n2right = n2 -> right;

		n1 -> left = n2left;
		n2 -> left = n1left;
		
		n1 -> right = n2right;
		n2 -> right = n1right;
		
		assert(n1 -> parent != NULL || n2 -> parent != NULL, "Error: somehow tried to swap two orphan nodes. This should never be able to happen");
	
		//update parent references
		if(n1 -> parent == NULL){
			assert(heapBase == n1, "Error: non-base orphan");
			heapBase = n2;
			if(n2 -> parent -> left == n2){
				n2 -> parent -> left = n1;
			}	
			else if(n2 -> parent -> right == n2){
				n2 -> parent -> right = n1;
			}
			else{
				assert(false, "Error: parent does not know about child");
			}
		}
		else if(n2 -> parent == NULL){
			assert(heapBase == n2, "Error: non-base orphan");
			heapBase = n1;
			if(n1 -> parent -> left == n1){
				n1 -> parent -> left = n2;
			}
			else if(n1 -> parent -> right == n1){
				n1 -> parent -> right = n2;
			}
			else{
				assert(false, "Error: parent does not know about child");
			}
		}
		else{
			if(n1 -> parent -> left == n1){
				n1 -> parent -> left = n2;
			}
			else if(n1 -> parent -> right == n1){
				n1 -> parent -> right = n2;
			}
			else{
				assert(false, "Error: parent does not know about child");
			}
			if(n2 -> parent -> left == n2){
				n2 -> parent -> left = n1;
			}
			else if(n2 -> parent ->right == n2){
				n2 -> parent -> right = n1;
			}
			else{
				assert(false, "Error: parent does not know about child");
			}
		}
		n1 -> parent = n2parent;
		n2 -> parent = n1parent;
		size_t n1Size = n1 -> heapSize;
        size_t n2Size = n2 -> heapSize;
        n1 -> heapSize = n2Size;
        n2 -> heapSize = n1Size;

		fixNodeChildParents(n1);
		fixNodeChildParents(n2);
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

void addToParentSizes(HeapNode* node, int diff){
	if(node != NULL){
		node -> heapSize += diff;
		addToParentSizes(node -> parent, diff);
	}
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
	availableSpace += size;
	//Finally return the ptr
	return (HeapNode*)ptr;
}

HeapNode* addNode(void* ptr, size_t size){
	HeapNode* parent = findNextParent(heapBase);
	HeapNode* out = makeNode(ptr, parent, size);
	rebalance(out);
	return out;
}

HeapNode* findBottomNode(HeapNode* node){
	if(node -> left != NULL){
		return findBottomNode(node -> left);
	}
	if(node -> right != NULL){
		return findBottomNode(node -> right);
	}
	return node;
}

//FIXME there's a bug here
void removeNode(HeapNode* ptr){
	HeapNode* toSwap = findBottomNode(ptr);
	assert(ptr -> parent != NULL || ptr == heapBase, "Error: attempted to remove orphan non-base node");
	//If toSwap == ptr, then ptr is a leaf, so we may just remove the parent's reference and claim the associated blocks
	if(toSwap == ptr){
		assert(ptr != heapBase, "Error: attempted to remove last node in heap");
	}
	else{
		swapNodes(ptr, toSwap);
		assert(ptr -> parent -> left == ptr || ptr -> parent -> right == ptr, "tesT");
	}
	assert(ptr -> parent != NULL, "Error: node we're deleting somehow has no parent now");
	addToParentSizes(ptr -> parent, -1);
	//remove ptr from the heap structure
	if(ptr -> parent -> left == ptr){
		ptr -> parent -> left = NULL;
	}
	else if(ptr -> parent -> right == ptr){
		ptr -> parent -> right = NULL;
	}
	else{
		assert(false, "Error: parent does not know about child");
	}
	//Now claim associated blocks
	HeapNode** tablePtr = ptrToNodeTableEntry(ptr);
	availableSpace -= ptr -> size;
	for(int i = 0; i * 0x80 < ptr -> size; i++){
		tablePtr[i] = (HeapNode*)(ptr -> size);
	}
	if(toSwap != ptr){
		rebalance(toSwap);
	}
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
		tablePtr[i] = (HeapNode*)size;
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
	fixNodeChildParents(out);
	rebalance(out);
	return out;
}

HeapNode* mergeAdjacentNodes(HeapNode* first, HeapNode* second){
	HeapNode** tableEntry = ptrToNodeTableEntry(second);
	removeNode(second); //FIXME state gets corrupted here.
	
	//Changing block mappings in ptrTable
	first -> size += second -> size;
	for(int i = 0; i * 0x80 < second -> size; i++){
		tableEntry[i] = first;
	}
	availableSpace += second -> size;
	rebalance(first);
	return first;
}

void initHeapAllocator(){
	availableSpace = 0;
	memset(kheap, 0, KERNEL_HEAP_SIZE);
	memset(nodePtrs, 0, KERNEL_HEAP_SIZE >> 7);
	heapBase = makeNode(kheap, NULL, KERNEL_HEAP_SIZE);
	validateHeap(); //sanity check
}

void* heapAlloc(size_t size){
	size = alignSize(size);
	HeapNode* toShrink = findNode(heapBase, size);
	assert(toShrink -> size >= size, "Error: findNode returned too small chunk");
	if(toShrink -> size == size){
		removeNode(toShrink);
		validateHeap();
		return toShrink;
	}
	HeapNode* newBase = shrinkNode(toShrink, size);
	rebalance(newBase);
	validateHeap();
	return toShrink;
}

void heapFree(void* ptr){
	//assert(false, "I don't want to use free");
	assert(ptr >= (void*) kheap && ptr < (void*)(kheap + KERNEL_HEAP_SIZE), "Error: tried to free pointer outside of kernel heap range");
	HeapNode* entryBase = (HeapNode*)ptr;
	HeapNode** tableEntry = ptrToNodeTableEntry(ptr);
	assert(!isHeapPtrValid(*tableEntry), "Error: double free");	
	
	//TODO maybe add more verification here?

	size_t freeSize = (size_t) *tableEntry;
	int nextIndex = freeSize / 0x80;
	assert((freeSize & 0x7f) == 0, "Error: malformed table entry");
	
	bool isNextChunkFree = (isHeapPtrValid(tableEntry[nextIndex]) && isHeapPtrValid((void*)((uint32_t)ptr + freeSize)));
	bool isPrevChunkFree = (isHeapPtrValid(tableEntry[-1]) && ptr != (void*) kheap);
	
	//p << "next " << isNextChunkFree << " prev " << isPrevChunkFree << "\n";	
	//The overall strategy is as follows: create a new HeapNode for the chunk of memory we are freeing
	//Then if the next chunk is free, we'll merge it back into this new chunk
	//Finally if the previous chunk is also free, we'll merge back into it

	//Here we create the new HeapNode for this chunk of memory
	HeapNode* cnode = addNode(ptr, freeSize);
	
	//Avoid unncessarily fragmenting our memory by merging adjacent free chunks
	if(isNextChunkFree){
		HeapNode* nextNode = tableEntry[nextIndex];
		mergeAdjacentNodes(cnode, nextNode);
	}
	if(isPrevChunkFree){
		HeapNode* prevNode = tableEntry[-1];
		mergeAdjacentNodes(prevNode, cnode);
	}
	
	validateHeap();
}

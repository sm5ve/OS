#ifndef BINARY_HEAP
#define BINARY_HEAP

#include <klib/ds/BinaryTree.h>

template <class T>
class BinaryHeap : public BinaryTree<T>{
public:
	BinaryHeap(bool smallest_at_root = false);
	~BinaryHeap();

	BinaryTreeNode<T>* makeNode(T value, BinaryTreeNode<T>* parent = NULL, void* where = NULL);
	void removeNode(BinaryTreeNode<T>* node);
	private:
		bool smallest_at_root;	
	
		void rebalance(BinaryTreeNode<T>* node);
};

template <class T>
BinaryHeap<T>::BinaryHeap(bool sm) : BinaryTree<T>(){
	smallest_at_root = sm;
}

template <class T>
BinaryHeap<T>::~BinaryHeap(){
	
}

template <class T>
void BinaryHeap<T>::rebalance(BinaryTreeNode<T>* node){
	if(node -> getParent() == NULL){
		return;
	}
	//First we'll make sure the given node satisfies the desired relation with its parent
	//If it does not, we'll swap them, and keep going up.
	if(smallest_at_root){
		if(node -> getParent() -> getValue() > node -> getValue()){
			node -> swap(node -> getParent());
		}
		rebalance(node);
		return;
	}
	else{
		if(node -> getParent() -> getValue() < node -> getValue()){
			node -> swap(node -> getParent());
		}
		rebalance(node);
		return;
	}
	//Otherwise we'll check that the desired relationships hold with the children
	if(smallest_at_root){
		if(node -> left() -> getValue() < node -> getValue() || node -> right() -> getValue() < node -> getValue()){
			BinaryTreeNode<T>* toSwap;
			if(node -> left() -> getValue() < node -> right() -> getValue()){
				toSwap = node -> left();
			}
			else{
				toSwap = node -> right();
			}
			node -> swap(toSwap);
			rebalance(node);
			return;
		}
	}
	else{
		if(node -> right() -> getValue() > node -> getValue() || node -> left() -> getValue() > node -> getValue()){
			BinaryTreeNode<T>* toSwap;
			if(node -> left() -> getValue() > node -> right() -> getValue()){
				toSwap = node -> left();
			}
			else{
				toSwap = node -> right();
			}
			node -> swap(toSwap);
			rebalance(node);
			return;
		}
	}
}

template <class T>
BinaryTreeNode<T>* BinaryHeap<T>::makeNode(T value, BinaryTreeNode<T>* parent, void* where){
	BinaryTreeNode<T>* out = BinaryTree<T>::makeNode(value, parent, where);
	rebalance(out);
	return out;
}

template <class T>
BinaryTreeNode<T>* findLeaf(BinaryTreeNode<T>* node){
	if(node -> isLeaf()){
		return node;
	}
	if(node -> left() != NULL){
		return findLeaf(node -> left());
	}
	return findLeaf(node -> right());
}

template <class T>
void BinaryHeap<T>::removeNode(BinaryTreeNode<T>* node){
	BinaryTreeNode<T>* leaf = findLeaf(node);
	node -> swap(leaf);
	BinaryTree<T>::removeNode(node);
	rebalance(leaf);
} 

#endif

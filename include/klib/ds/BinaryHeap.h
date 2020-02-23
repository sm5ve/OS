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
	assert(node, "Error: tried to rebalance null node");
	if(node -> getParent() != NULL){
	//First we'll make sure the given node satisfies the desired relation with its parent
	//If it does not, we'll swap them, and keep going up.
		if(smallest_at_root){
			if(node -> getParent() -> getValue() > node -> getValue()){
				node -> swap(node -> getParent());
				rebalance(node);
				return;
			}
		}
		else{
			BinaryTreeNode<T>* old_parent = node -> getParent();
			if(node -> getParent() -> getValue() < node -> getValue()){
				node -> swap(node -> getParent());
				rebalance(node);
				return;
			}
		}
	}
	//Otherwise we'll check that the desired relationships hold with the children
	if(smallest_at_root){
		BinaryTreeNode<T>* to_swap = NULL;
		if(node -> left() != NULL){
			if(node -> right() != NULL){
				BinaryTreeNode<T>* smaller = (node -> right() -> getValue() < node -> left() -> getValue()) ? node -> right() : node -> left();
				if(smaller -> getValue() < node -> getValue()){
					to_swap = smaller;
				}
			}
			else{
				if(node -> left() -> getValue() < node -> getValue()){
					to_swap = node -> left();
				}
			}
		}
		else if(node -> right() != NULL){
			if(node -> right() -> getValue() < node -> getValue()){
				to_swap = node -> right();
			}
		}
		if(to_swap){
			node -> swap(to_swap);
			rebalance(node);
			return;
		}
	}
	else{
		BinaryTreeNode<T>* to_swap = NULL;
        if(node -> left() != NULL){
        	if(node -> right() != NULL){
                BinaryTreeNode<T>* bigger = (node -> right() -> getValue() > node -> left() ->
 getValue()) ? node -> right() : node -> left();
                if(bigger -> getValue() > node -> getValue()){
                    to_swap = bigger;
                }
            }
            else{
                if(node -> left() -> getValue() > node -> getValue()){
                    to_swap = node -> left();
                }
            }
        }
        else if(node -> right() != NULL){
            if(node -> right() -> getValue() > node -> getValue()){
                to_swap = node -> right();
            }
        }
		else{
			return;
		}
        if(to_swap){
            node -> swap(to_swap);
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

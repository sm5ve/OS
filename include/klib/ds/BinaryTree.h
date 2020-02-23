#ifndef BINARY_TREE
#define BINARY_TREE

#include <stdint.h>
#include <mem.h>

template <class T>
class BinaryTreeNode;

template <class T>
class BinaryTree{
	public:
		BinaryTree();
		~BinaryTree();
		
		BinaryTreeNode<T>* makeNode(T value, BinaryTreeNode<T>* parent = NULL, void* where = NULL);
		void removeNode(BinaryTreeNode<T>* node);
		
		BinaryTreeNode<T>* getRoot();		
	private:
		friend class BinaryTreeNode<T>;
		BinaryTreeNode<T>* root;
		
		BinaryTreeNode<T>* findOpenParent(BinaryTreeNode<T>* base = NULL);
};

template <class T>
class BinaryTreeNode{
	public:
		BinaryTreeNode<T>* left();
		BinaryTreeNode<T>* right();

		T getValue();
		uint32_t getNumberOfNodesBelow();
	
		bool isChild(BinaryTreeNode<T>* ptr);

		void swap(BinaryTreeNode<T>* node);
	private:
		BinaryTree<T>* tree;
		
		BinaryTreeNode<T>* parent;
		
		BinaryTreeNode<T>* l;
		BinaryTreeNode<T>* r;
		
		T value;

		uint32_t nodes_below;
	private:
		friend class BinaryTree<T>;

		BinaryTreeNode(T value, BinaryTree<T>* tree);
		~BinaryTreeNode();

		void remove(BinaryTreeNode<T>* ptr);
		void replace(BinaryTreeNode<T>* child, BinaryTreeNode<T>* with);
		void add(BinaryTreeNode<T>* ptr);
		bool hasRoom();
};

template <class T>
BinaryTreeNode<T>::BinaryTreeNode(T v, BinaryTree<T>* t){
	value = v;
	tree = t;
	
	parent = NULL;
	l = NULL;
	r = NULL;
	
	nodes_below = 0;
}

template <class T>
T BinaryTreeNode<T>::getValue(){
	return value;
}

template <class T>
uint32_t BinaryTreeNode<T>::getNumberOfNodesBelow(){
	return nodes_below;
}

template <class T>
BinaryTreeNode<T>::~BinaryTreeNode(){
    assert(l == NULL && r == NULL, "Error: tried to delete non-leaf BinaryTreeNode");
}

template <class T>
BinaryTreeNode<T>* BinaryTreeNode<T>::left(){
    return l;
}

template <class T>
BinaryTreeNode<T>* BinaryTreeNode<T>::right(){
    return r;
}

template <class T>
bool BinaryTreeNode<T>::isChild(BinaryTreeNode<T>* ptr){
    return r == ptr || l == ptr;
}

template <class T>
void BinaryTreeNode<T>::remove(BinaryTreeNode<T>* ptr){
   	assert(ptr -> tree == tree, "Error: tried to remove node from another tree");
	if(l == ptr){
        l = NULL;
    }
    else if(r == ptr){
        r = NULL;
    }
    else{
        assert(false, "Error: tried to remove non-existent child from BinaryTreeNode");
    }
    ptr -> parent = NULL;
	this -> nodes_below -= ptr -> nodes_below;
}

template <class T>
void BinaryTreeNode<T>::replace(BinaryTreeNode<T>* child, BinaryTreeNode<T>* with){
    assert(child != NULL, "Error: tried to swap out null child.");
	assert(with -> tree == tree, "Error: tried to swap out node with BinaryTreeNode from other tree");
	if(l == child){
        l = with;
    }
    else if(r == child){
        r = with;
    }
    else{
        assert(false, "Error: tried to swap out non-existent child from BinaryTreeNode");
    }
    child -> parent = NULL;
    with -> parent = this;
	this -> nodes_below -= child -> nodes_below;
	this -> nodes_below += with -> nodes_below;
}

template <class T>
void BinaryTreeNode<T>::add(BinaryTreeNode<T>* ptr){
	assert(ptr -> tree == tree, "Error: tried to add BinaryTreeNode from other tree. This doesn't make sense!");
	if(l == NULL){
		l = ptr;
	}
	else if(r == NULL){
		r = ptr;
	}
	else{
		assert(false, "Error: tried to insert node into full BinaryTreeNode");
	}
	l -> parent = this;
	nodes_below++;
}

template <class T>
bool BinaryTreeNode<T>::hasRoom(){
	return l == NULL || r == NULL;
}

template <class T>
void BinaryTreeNode<T>::swap(BinaryTreeNode<T>* node){
	assert(node -> tree == this -> tree, "Error: tried to swap nodes between 2 different trees. This makes no sense!");
    //Always make sure that if there's a parent-child relationship with the argument that this argument is the parent
	//This simplifies the swapping code below
	if(node -> parent == this){
		node -> swap(this);
	}
	//Swap the values, nodes_below counter, and children
	BinaryTreeNode<T>* nleft = node -> l;
	BinaryTreeNode<T>* nright = node -> r;
	T nval = node -> value;
	uint32_t ncount = node -> nodes_below;

	BinaryTreeNode<T>* old_left = l;
	BinaryTreeNode<T>* old_right = r;
	T old_val = value;
	uint32_t old_count = nodes_below;

	node -> l = old_left;
	node -> r = old_right;
	node -> value = old_val;
	node -> nodes_below = old_count;

	l = nleft;
	r = nright;
	value = nval;
	nodes_below = ncount;

	//We know that if there's a nontrivial parent-child relationship with the argument, that the argument will always be the parent. Since we've swapped the children and this was a child of node, this will be a child of itself. We need to swap this out for a reference to the former-parent
	if(parent == node){
		if(l == this){
			l = node;
		}
		else if(r == this){
			r = node;
		}
		else{
			assert(false, "Error: swapped node with parent, but parent did not seem to have reference to child");
		}
	}
	
	//Back up the old parent so we can swap the parent pointers
	BinaryTreeNode<T>* old_parent = parent;
	//Finally swap the parents
	//If the argument's parent is null, we verify that it's the root of the tree and then swap the parents and update the tree's root
	if(node -> parent == NULL){
		assert(tree -> root == node, "Error: tried to swap with orphan non-root node");
		tree -> root = this;

		this -> parent = NULL;
		old_parent -> parent = this;
	}
	//We do the same if in fact this is already the root
	else if(this -> parent == NULL){
		assert(tree -> root == this, "Error: tried to swap with orphan non-root node");
		tree -> root = node;
	
		this -> parent = node -> parent;
		node -> parent = NULL;
	}
	//Otherwise, we can just swap their parents like normal
	else{
		this -> parent = node -> parent;
		node -> parent = old_parent;
	}
}

template <class T>
BinaryTree<T>::BinaryTree(){

}

template <class T>
BinaryTree<T>::~BinaryTree(){

}

template <class T>
BinaryTreeNode<T>* BinaryTree<T>::getRoot(){
	return root;
}

//Finds an open parent that's (hopefully) close to the root, and returns null if the tree has no nodes
template <class T>
BinaryTreeNode<T>* BinaryTree<T>::findOpenParent(BinaryTreeNode<T>* base){
	if(root == NULL){
		return NULL;
	}
	if(base == NULL){
		return findOpenParent(root);
	}
	if(base -> hasRoom()){
		return base;
	}
	if(base -> left() -> getNumberOfNodesBelow() < base -> right() -> getNumberOfNodesBelow()){
		return findOpenParent(base -> left());
	} 
	return findOpenParent(base -> right());
}

template <class T>
BinaryTreeNode<T>* BinaryTree<T>::makeNode(T value, BinaryTreeNode<T>* parent, void* where){
	if(parent == NULL){
		parent = findOpenParent();
	}
	BinaryTreeNode<T>* out;
	if(where != NULL){
		out = new (where) BinaryTreeNode(value, this);
	}
	else{
		out = new BinaryTreeNode(value, this);
	}
	if(parent == NULL){
		assert(root == NULL, "Error: somehow adding parentless node to nonempty tree");
		root = out;
	}
	else{
		parent -> add(out);
	}
	return out;
}

template <class T>
void BinaryTree<T>::removeNode(BinaryTreeNode<T>* node){
	assert(node -> tree == this, "Error: tried to remove node from a different tree");
	assert(node -> left() == NULL && node -> right() == NULL, "Error: tried to remove a non-leaf node from the tree");
	if(node -> parent == NULL){
		assert(node == root, "Error: removing orphan non-root node");
		root = NULL;
	}
	else{
		node -> parent -> remove(node);
	}
}

#endif

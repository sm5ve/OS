#ifndef BINARY_TREE
#define BINARY_TREE

#include <stdint.h>
#include <mem.h>
#include <assert.h>

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
		BinaryTreeNode<T>* getParent();
	
		T getValue();
		uint32_t getNumberOfNodesBelow();
	
		bool isChild(BinaryTreeNode<T>* ptr);
		bool isLeaf();
	
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
		void add(BinaryTreeNode<T>* ptr);
		bool hasRoom();
	private:
		BinaryTreeNode();
		
		void switchOut(BinaryTreeNode<T>* node);
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

//Only for internal use for implementing swap()
template <class T>
BinaryTreeNode<T>::BinaryTreeNode(){
	tree = NULL;
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
BinaryTreeNode<T>* BinaryTreeNode<T>::getParent(){
	return parent;
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
bool BinaryTreeNode<T>::isLeaf(){
	return r == NULL && l == NULL;
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
	ptr -> parent = this;
	nodes_below++;
}

template <class T>
bool BinaryTreeNode<T>::hasRoom(){
	return l == NULL || r == NULL;
}

template <class T>
void BinaryTreeNode<T>::switchOut(BinaryTreeNode<T>* node){
	if(node == this)
		return;
	
	assert(this -> parent != node, "Huh");
	assert(node -> parent != this, "huh");
	assert(this -> parent != this, "SAFASDF");
	assert(node -> parent != node, "DAFSDFAS");
	//First we swap the children
	BinaryTreeNode<T>* nleft = node -> l;
	BinaryTreeNode<T>* nright = node -> r;
	BinaryTreeNode<T>* old_left = l;
	BinaryTreeNode<T>* old_right = r;
	
	if(node -> tree == NULL){
		node -> tree = tree;
	}
	
	node -> l = old_left;
	node -> r = old_right;
	l = nleft;
	r = nright;
	
	//Update the children's parents
	if(node -> l != NULL)
		node -> l -> parent = node;
	if(node -> r != NULL)
		node -> r -> parent = node;
	if(l != NULL)
		l -> parent = this;
	if(r != NULL)
		r -> parent = this;

	//Swap the nodes_below counts
	uint32_t ncount = node -> nodes_below;
	uint32_t old_count = nodes_below;
	
	node -> nodes_below = old_count;
	nodes_below = ncount;

	if(this -> parent != NULL){
        assert(this -> parent -> l == this || this -> parent -> r == this, "Oh fuck");
    }
	if(node -> parent != NULL){
        assert(node -> parent -> l == node || node -> parent -> r == node, "Oh fuck");
    }

	//Swap parent references
	BinaryTreeNode<T>* nparent = node -> parent;
	BinaryTreeNode<T>* old_parent = parent;

	node -> parent = old_parent;
	parent = nparent;

	if(node -> parent != NULL){
        assert(node -> parent -> l == this || node -> parent -> r == this, "Oh fuck");
    }
 if(this -> parent != NULL){
        assert(this -> parent -> l == node || this -> parent -> r == node, "Oh fuck");
    }



	//Make sure to update tree root if applicable
	if(node -> tree -> root == node){
		node -> tree -> root = this;
	}
	else if(tree -> root == this){
		tree -> root = node;
	}

	//Finally update parents' children
	if(node -> parent != NULL){
		if(node -> parent -> l == this){
			node -> parent -> l = node;
		}
		else if(node -> parent -> r == this){
			node -> parent ->r = node;
		}
		else{
			assert(false, "Error: parent does not know of child1");
		}
	}
	if(parent != NULL){
		if(parent -> l == node){
			parent -> l = this;
		}
		else if(parent -> r == node){
			parent -> r = this;
		}
		else{	
			assert(false, "Error: parent does not know of child2");
		}
	}
}

#include <klib/SerialPrinter.h>

template <class T>
void BinaryTreeNode<T>::swap(BinaryTreeNode<T>* node){
	assert(node -> tree == this -> tree, "Error: tried to swap nodes between 2 different trees. This makes no sense!");
   	if(node == this) return;
	SerialPrinter p(COMPort::COM1);
	BinaryTreeNode<T> temp1;
	BinaryTreeNode<T> temp2;
	//p << "1\n";
	switchOut(&temp1);
	//p << "2\n";
	node -> switchOut(&temp2);
	//p << "3\n";
	switchOut(&temp2);
	//p << "4\n";
	node -> switchOut(&temp1);
	//p << "5\n";
}

template <class T>
BinaryTree<T>::BinaryTree(){
	root = NULL;
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

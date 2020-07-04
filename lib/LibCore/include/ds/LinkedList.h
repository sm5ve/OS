#ifndef LINKED_LIST
#define LINKED_LIST

#include <assert.h>
#include <stddef.h>

template <class T>
class LinkedList;

template <class T>
class LinkedListNode{
public:
	T value;
		
	LinkedListNode<T>* next();
	LinkedListNode<T>* previous();
private:
	friend LinkedList<T>;

	LinkedListNode();
	~LinkedListNode();

	LinkedListNode<T>* n;
	LinkedListNode<T>* p;
};

template <class T>
class LinkedList{
public:
	LinkedList();
	~LinkedList();

	LinkedListNode<T>* head();
	LinkedListNode<T>* tail();
	
	LinkedListNode<T>* beginning();
	LinkedListNode<T>* ending();

	LinkedListNode<T>* add(T);
	LinkedListNode<T>* addToBeginning(T);
	LinkedListNode<T>* addBefore(T, LinkedListNode<T>*);
	LinkedListNode<T>* addAfter(T, LinkedListNode<T>*);

	LinkedListNode<T>* find(T);

	LinkedListNode<T>* get(uint32_t);

	void remove(LinkedListNode<T>*);

	size_t length();

	class Iterator{
	public:
		Iterator(LinkedListNode<T>* n): node(n){}
		Iterator operator++() {node = node -> next(); return *this;}
		bool operator!=(const Iterator& rhs){return node != rhs.node;}
		T& operator*() const {return node -> value;}
	private:
		LinkedListNode<T>* node;
	};

	Iterator begin(){
		return Iterator(head());
	}

	Iterator end(){
		return Iterator(ending());
	}
private:
	LinkedListNode<T> first;
	LinkedListNode<T> last;
};

template <class T>
LinkedList<T>::LinkedList(){
	first.n = &last;
	last.p = &first;
}

template <class T>
LinkedList<T>::~LinkedList(){
	//SD::the() << "Deleting linked list\n";
	while(head() != ending()){
		remove(head());
	}
}

template <class T>
LinkedListNode<T>* LinkedList<T>::head(){
	return first.next();
}

template <class T>
LinkedListNode<T>* LinkedList<T>::tail(){
	return last.previous();
}

template <class T>
LinkedListNode<T>* LinkedList<T>::beginning(){
	return &first;
}

template <class T>
LinkedListNode<T>* LinkedList<T>::ending(){
	return &last;
}

template <class T>
LinkedListNode<T>* LinkedList<T>::add(T val){
	return addBefore(val, ending());
}

template <class T>
LinkedListNode<T>* LinkedList<T>::addToBeginning(T val){
	return addAfter(val, beginning());
}

template <class T>
LinkedListNode<T>* LinkedList<T>::addBefore(T val, LinkedListNode<T>* node){
	assert(node != beginning(), "Error: tried to add node before beginning");
	LinkedListNode<T>* newNode = new LinkedListNode<T>();
	newNode -> value = val;
	newNode -> n = node;
	newNode -> p = node -> p;
	node -> p -> n = newNode;
	node -> p = newNode;
	return newNode;
}

template <class T>
LinkedListNode<T>* LinkedList<T>::addAfter(T val, LinkedListNode<T>* node){
	assert(node != end(), "Error: tried to add node after end");
	LinkedListNode<T>* newNode = new LinkedListNode<T>();
	newNode -> value = val;
	newNode -> p = node;
	newNode -> n = node -> n;
	node -> n -> p = newNode;
	node -> n = newNode;
	return newNode;
}

template <class T>
void LinkedList<T>::remove(LinkedListNode<T>* node){
	assert(node != ending(), "Error: tried to remove end node");
	assert(node != beginning(), "Error: tried to remove beginning node");
	
	node -> n -> p = node -> p;
	node -> p -> n = node -> n;
	delete node;
}

template <class T>
LinkedListNode<T>* LinkedList<T>::find(T val){
	LinkedListNode<T>* node = head();
	while(node != end()){
		if(node -> value == val){
			break;
		}
		node = node -> next();
	}
	return node;
}

template <class T>
LinkedListNode<T>::LinkedListNode(){
	n = NULL;
	p = NULL;
}

template <class T>
LinkedListNode<T>::~LinkedListNode(){

}

template <class T>
LinkedListNode<T>* LinkedListNode<T>::next(){
	return n;
}

template <class T>
LinkedListNode<T>* LinkedListNode<T>::previous(){
	return p;
}

template <class T>
LinkedListNode<T>* LinkedList<T>::get(uint32_t index){
	LinkedListNode<T>* out = this -> head();
	for(int i = 0; i < index; i++){
		if(out == this -> end()){
			return NULL;
		}
		out = out -> next();
	}
	return out;
}

template <class T>
size_t LinkedList<T>::length(){
	size_t out = 0;
	auto node = head();
	while(node != end()){
		out++;
		node = node -> next();
	}
	return out;
}
#endif

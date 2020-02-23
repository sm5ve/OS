#ifndef LINKED_LIST
#define LINKED_LIST

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
	LinkedListNode<T>* end();

	LinkedListNode<T>* add(T);
	LinkedListNode<T>* addToBeginning(T);
	LinkedListNode<T>* addBefore(T, LinkedListNode<T>*);
	LinkedListNode<T>* addAfter(T, LinkedListNode<T>*);

	LinkedListNode<T>* find(T);

	void remove(LinkedListNode<T>*);
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
	while(head() != end()){
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
LinkedListNode<T>* LinkedList<T>::end(){
	return &last;
}

template <class T>
LinkedListNode<T>* LinkedList<T>::add(T val){
	return addBefore(val, end());
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
	assert(node != end(), "Error: tried to remove end node");
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
#endif

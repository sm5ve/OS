#ifndef HASH_MAP
#define HASH_MAP

#include <klib/ds/Tuple.h>
#include <klib/ds/LinkedList.h>
#include <assert.h>
#include <klib/hash.h>

#include <klib/SerialPrinter.h>

template <class K, class V>
class HashMap{
public:
	HashMap(size_t initial_capacity = 10, float load_factor = 0.75);
	~HashMap();

	void put(K,V);
	V get(K);
	void remove(K);	

	bool contains(K);

	size_t getSize();
private:
	LinkedList<Tuple<K,V>>* data;
	size_t capacity;
	float load_factor;
	size_t size;

	void resizeIfNecessary();
	void resize(size_t);
};

template <class K, class V>
HashMap<K,V>::HashMap(size_t initial_capacity, float load){
	data = new LinkedList<Tuple<K,V>>[initial_capacity];
	capacity = initial_capacity;
	load_factor = load;
	size = 0;	
}

template <class K, class V>
HashMap<K,V>::~HashMap(){
	
}

template <class K, class V>
void HashMap<K, V>::resize(size_t new_size){
	assert(false, "Unimplemented");
}

template <class K, class V>
void HashMap<K,V>::put(K key, V value){
	LinkedList<Tuple<K,V>>* list = &data[hash<K>{}(key) % capacity];
	LinkedListNode<Tuple<K,V>>* node = list -> head();
	while(node != list -> end()){
		if(node -> value.a == key){
			node -> value.b = value;
			return;
		}
	}
	list -> add(Tuple(key, value));
	size++;
	resizeIfNecessary();
}

template <class K, class V>
V HashMap<K,V>::get(K key){
	LinkedList<Tuple<K,V>>* list = &data[hash<K>{}(key) % capacity];
	LinkedListNode<Tuple<K,V>>* node = list -> head();
	while(node != list -> end()){
		if(node -> value.a == key){
			return node -> value.b;
		}
		node = node -> next();
	}
	assert(false, "Tried to get nonexistent entry from HashMap");
}

template <class K, class V>
bool HashMap<K,V>::contains(K key){
	LinkedList<Tuple<K,V>>* list = &data[hash<K>{}(key) % capacity];
	LinkedListNode<Tuple<K,V>>* node = list -> head();
	while(node != list -> end()){
		if(node -> value.a == key){
			return true;
		}
		node = node -> next();
	}
	return false;
}

template <class K, class V>
void HashMap<K, V>::remove(K key){
	LinkedList<Tuple<K, V>>* list = &data[hash<K>{}(key) % capacity];
	LinkedListNode<Tuple<K, V>>* node = list -> head();
	while(node != list -> end()){
		if(node -> value.a == key){
			list -> remove(node);
			size--;
			resizeIfNecessary();
			return;
		}
		node = node -> next();
	}
	assert(false, "Error: tried to remove nonexistant entry from HashMap");
}

template <class K, class V>
size_t HashMap<K, V>::getSize(){
	return size;
}

template <class K, class V>
void HashMap<K,V>::resizeIfNecessary(){
	
}

#endif

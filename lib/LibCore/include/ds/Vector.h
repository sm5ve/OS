#ifndef VECTOR
#define VECTOR
//FIXME move memset/memcpy out of kernel and into core
#include <mem.h>

template <class T>
class Vector{
public:
	Vector();
	Vector(size_t initial_capacity);
	Vector(T*, size_t count);
	Vector(Vector<T>&);
	~Vector();

	T& operator[](uint32_t index);
	void remove(uint32_t index);
	void push(T);
	T pop();
	T top();
	size_t size();
private:
	T* buffer;
	size_t count;
	size_t capacity;

	void resize(size_t);
	void resizeIfNecessary();
};

template <class T>
Vector<T>::Vector() : Vector(16){}

template <class T>
Vector<T>::Vector(size_t initial_capacity){
	count = 0;
	capacity = initial_capacity;
	void* buff = new T[initial_capacity];//kalloc(sizeof(T) * initial_capacity);
	buffer = (T*)buff;
}

template <class T>
Vector<T>::Vector(T* elems, size_t ct){
	count = ct;
	capacity = ct;
	buffer = new T[ct];
	for(int i = 0; i < ct; i++){
		buffer[i] = elems[i];
	}
}

template <class T>
Vector<T>::Vector(Vector<T>& v) : Vector(v.buffer, v.count){
	
}

template <class T>
Vector<T>::~Vector(){
	delete [] buffer;
}

template <class T>
T& Vector<T>::operator[](uint32_t index){
	assert(index < count, "Error: index out of bounds");
	return buffer[index];
}

template <class T>
void Vector<T>::remove(uint32_t index){
	assert(index < count, "Error: index out of bounds");
	count--;
	resizeIfNecessary();
}

template <class T>
void Vector<T>::push(T elem){
	count++;
	resizeIfNecessary();
	buffer[count - 1] = elem;
}

template <class T>
T Vector<T>::pop(){
	assert(count > 0, "Error: popped from empty vector");
	T out = buffer[count - 1];
	count--;
	resizeIfNecessary();
	return out;
}

template <class T>
T Vector<T>::top(){
	assert(count > 0, "Error: tried to get top of empty vector");
	return buffer[count - 1];
}

template <class T>
size_t Vector<T>::size(){
	return count;
}

template <class T>
void Vector<T>::resize(size_t newSize){
	assert(newSize >= count, "Error: resized to be too small");
	T* newBuffer = new T[newSize];
	memset(newBuffer, 0, capacity * sizeof(T));
	for(uint32_t i = 0; i < capacity; i++){
		newBuffer[i] = buffer[i];
	}
	//memset(buffer, 0, capacity * sizeof(T));
	capacity = newSize;
	delete [] buffer;
	buffer = newBuffer;
}

template <class T>
void Vector<T>::resizeIfNecessary(){
	if(capacity <= count){
		size_t newCap = capacity + (capacity >> 1);
		if(newCap < 2){
			newCap = 2;
		}
		resize(newCap);
	}
}

#endif

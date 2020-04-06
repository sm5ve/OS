#ifndef QUEUE
#define QUEUE

#include <stddef.h>
#include <stdint.h>

template <class T>
class Queue{
public:
	Queue();
	~Queue();
	void enqueue(T);
	void pushFront(T);
	T dequeue();
	T popBack();
	T peekHead();
	T peekBack();
	size_t size();
private:
	T* buffer;
	size_t capacity;
	uint32_t start_index;
	uint32_t stop_index;
	void resize(size_t);
	void resizeIfNecessary();
};

template <class T>
Queue<T>::Queue(){
	capacity = 5;	
	buffer = new T[capacity];
	start_index = 0;
	stop_index = 0;
}

template <class T>
Queue<T>::~Queue(){
	delete [] buffer;
}

template <class T>
void Queue<T>::enqueue(T elem){
	buffer[stop_index] = elem;
	stop_index = (stop_index + 1) % capacity;
	resizeIfNecessary();
}

template <class T>
void Queue<T>::pushFront(T elem){
	start_index = (start_index - 1) % capacity;
	buffer[start_index] = elem;
	resizeIfNecessary();
}

template <class T>
T Queue<T>::dequeue(){
	assert(size() > 0, "Error: tried to dequeue from empty queue");
	T out = buffer[start_index];
	start_index = (start_index + 1) % capacity;
	return out;
}

template <class T>
T Queue<T>::popBack(){
	assert(size() > 0, "Error: tried to pop from empty queue");
	stop_index = (stop_index - 1) % capacity;
	return buffer[stop_index];
}

template <class T>
size_t Queue<T>::size(){
	int signed_difference = ((int)stop_index) - ((int)start_index);
	int reduction = signed_difference % capacity;
	if(reduction < 0)
		reduction += capacity;
	return reduction;
}

#include <devices/SerialDevice.h>

template <class T>
void Queue<T>::resize(size_t newSize){
	assert(newSize > size(), "Error: tried to resize queue too small");
	T* newBuffer = new T[newSize];
	uint32_t index = start_index;
	uint32_t writeIndex = 0;
	while(index != stop_index){
		newBuffer[writeIndex] = buffer[index];
		writeIndex++;
		index = (index + 1) % capacity;
	}
	start_index = 0;
	stop_index = index;
	capacity = newSize;
	delete [] buffer;
	buffer = newBuffer;
	SD::the() << "Resized to " << newSize << "\n";
}

template <class T>
void Queue<T>::resizeIfNecessary(){
	if(size() >= capacity - 2)
		resize(capacity + (capacity >> 1));
}

template <class T>
T Queue<T>::peekHead(){
	assert(size() > 0, "Error: tried to peek in empty queue");
	return buffer[start_index];
}

template <class T>
T Queue<T>::peekBack(){
	assert(size() > 0, "Error: tried to peek in empty queue");
	uint32_t index = (stop_index - 1) % capacity;
	return buffer[index];
}

#endif

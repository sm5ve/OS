#ifndef PROMISE
#define PROMISE

#include <ds/Maybe.h>

template <class T>
using PromiseHandler = void(*)(T, void*);

template <class T>
class Promise{
public:
	Promise();
	Promise(Promise<T>&);
	~Promise();
	
	void await();
	void then(PromiseHandler<T>, void* context);
	void fulfill(T);

	Promise<T>& operator=(Promise<T>& rhs);
private:
	volatile bool fulfilled;
	T value; //maybe this needs to be volatile, but I'm struggling against the compiler here
	void* context;
	PromiseHandler<T> handler;
};

template <class T>
Promise<T>::Promise(){
	fulfilled = false;
}

#include <devices/SerialDevice.h>

template <class T>
Promise<T>::Promise(Promise& p){
	SD::the() << "here!\n";
	fulfilled = p.fulfilled;
	value = p.value;
	context = p.context;
	handler = p.handler;
}

template <class T>
Promise<T>& Promise<T>::operator=(Promise<T>& rhs){
	fulfilled = rhs.fulfilled;
	value = rhs.value;
	context = rhs.context;
	handler = rhs.handler;
	return *this;
}

template <class T>
Promise<T>::~Promise(){
	SD::the() << "Bye\n";
}

template <class T>
void Promise<T>::await(){
	while(!fulfilled){
		#if defined(__x86_64__) || defined(__i386__)
		asm("hlt");
		#endif
	}
}

template <class T>
void Promise<T>::then(PromiseHandler<T> h, void* c){
	handler = h;
	context = c;
	if(fulfilled && (handler != NULL)){
		handler(value, context);
	}
}

#include <debug.h>

template <class T>
void Promise<T>::fulfill(T val){
	value = val;
	fulfilled = true;
	if(handler != NULL){
		handler(value, context);
	}
}

#endif

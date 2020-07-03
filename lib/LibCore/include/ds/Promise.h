#ifndef PROMISE
#define PROMISE

#include <ds/Maybe.h>

template <class T>
using PromiseHandler = void(*)(T, void*);

template <class T>
class Promise{
public:
	Promise();
	~Promise();
	
	void await();
	void then(PromiseHandler, void* context);
	void fulfill(T);
private:
	volatile bool fulfilled;
	volatile T value;
	void* context;
	PromiseHandler handler;
};

template <class T>
Promise<T>::Promise(){
	fulfilled = false;
}

template <class T>
Promise<T>::~Promise(){

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
void Promise<T>::then(PromiseHandler h, void* c){
	handler = h;
	context = c;
	if(fulfilled && (handler != NULL)){
		handler(value, context);
	}
}

template <class T>
void Promise<T>::fulfill(T val){
	value = val;
	fulfilled = true;
	if(handler != NULL){
		handler(value, context);
	}
}

#endif

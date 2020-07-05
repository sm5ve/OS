#ifndef PROMISE
#define PROMISE

#include <ds/Maybe.h>

template <class T, class S>
using PromiseHandler = void(*)(T, S);

template <class T, class S>
class Promise{
public:
	Promise();
	Promise(Promise<T, S>&);
	~Promise();
	
	void await();
	void then(PromiseHandler<T, S>, S context);
	void fulfill(T);

	Promise<T, S>& operator=(Promise<T, S>& rhs);
private:
	volatile bool fulfilled;
	T value; //maybe this needs to be volatile, but I'm struggling against the compiler here
	S context;
	PromiseHandler<T, S> handler;
};

template <class T, class S>
Promise<T, S>::Promise(){
	fulfilled = false;
}

#include <devices/SerialDevice.h>

template <class T, class S>
Promise<T, S>::Promise(Promise<T, S>& p){
	SD::the() << "here!\n";
	fulfilled = p.fulfilled;
	value = p.value;
	context = p.context;
	handler = p.handler;
}

template <class T, class S>
Promise<T, S>& Promise<T, S>::operator=(Promise<T, S>& rhs){
	fulfilled = rhs.fulfilled;
	value = rhs.value;
	context = rhs.context;
	handler = rhs.handler;
	return *this;
}

template <class T, class S>
Promise<T, S>::~Promise(){
	SD::the() << "Bye\n";
}

template <class T, class S>
void Promise<T, S>::await(){
	while(!fulfilled){
		#if defined(__x86_64__) || defined(__i386__)
		asm("hlt");
		#endif
	}
}

template <class T, class S>
void Promise<T, S>::then(PromiseHandler<T, S> h, S c){
	handler = h;
	context = c;
	if(fulfilled && (handler != NULL)){
		handler(value, context);
	}
}

template <class T, class S>
void Promise<T, S>::fulfill(T val){
	value = val;
	fulfilled = true;
	if(handler != NULL){
		handler(value, context);
	}
}

#endif

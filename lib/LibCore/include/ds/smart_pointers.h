#ifndef SMART_PTRS
#define SMART_PTRS

#include <PrintStream.h>
#include <assert.h>
#include <template_magic.h>

template <class T>
class unique_ptr{
public:
	unique_ptr() : ptr(NULL){}

	unique_ptr(T* p) : ptr(p){}
	
	unique_ptr(unique_ptr<T>&) = delete;
	
	unique_ptr(unique_ptr<T>&& rhs) : ptr(rhs.ptr){
		rhs.ptr = NULL;
	}

	~unique_ptr(){
		delete ptr;
	}
	
	bool isNull() const{
		return ptr == NULL;
	}

	unique_ptr<T>& operator=(unique_ptr<T>&) = delete;
	unique_ptr<T>& operator=(unique_ptr<T>&& rhs){
		if(&rhs == this)
			return *this;
		if(ptr != NULL)
			delete ptr;
		ptr = rhs.ptr;
		rhs.ptr = NULL;
		return *this;
	}

	T& operator*() const {
		return *ptr;
	}

	T* operator->() const {
		return ptr;
	}

private:
	T* ptr;
};

template <class T>
struct shared_ptr_counter{
	T* ptr;
	uint32_t strong_ref_count;
	uint32_t weak_ref_count;

	//TODO make atomic
	//Perhaps it would better to make an atomic_shared_ptr instead
	shared_ptr_counter(T* p){
		ptr = p;
		strong_ref_count = 0;
		weak_ref_count = 0;
	}

	void incStrong(){
		strong_ref_count++;
		weak_ref_count++;
	}

	void decStrong(){
		strong_ref_count--;
		weak_ref_count--;
	}

	void incWeak(){
		weak_ref_count++;
	}

	void decWeak(){
		weak_ref_count--;
	}
};

template <class T>
class weak_ptr{
public:
	weak_ptr(){

	}

	weak_ptr(shared_ptr_counter<T>& c) : counter(&c){
		counter -> incWeak();
	}

	weak_ptr(weak_ptr<T>& ptr) : weak_ptr(ptr.counter){

	}

	weak_ptr(weak_ptr<T>&& ptr){
		counter = ptr.counter;
		ptr.counter = NULL;
	}

	~weak_ptr(){
		if(counter == NULL)
			return;
		counter -> decWeak();
		if(counter -> weak_ref_count == 0){
			delete counter;
		}
	}

	T& operator*() const{
		assert(counter != NULL, "Error: null dereference");
		return *(counter -> ptr);
	}

	T* operator->() const{
		if(counter == NULL)
			return NULL;
		return counter -> ptr;
	}
	
	bool isNull() const{
		if(counter == NULL)
			return true;
		return counter -> ptr == NULL;
	}
	
	weak_ptr<T>& operator=(weak_ptr<T>& ptr){
		this -> ~weak_ptr();
		counter = ptr.counter;
		counter -> incWeak();
		return *this;
	}

	weak_ptr<T>& operator=(weak_ptr<T>&& ptr){
		this -> ~weak_ptr();
		counter = ptr.counter;
		ptr.counter = NULL;
		return *this;
	}
private:
	shared_ptr_counter<T>* counter;
};

template <class T>
class shared_ptr{
public:
	shared_ptr(){

	}
	
	shared_ptr(shared_ptr_counter<T>& c) : counter(&c){
		counter -> incStrong();
	}
	
	shared_ptr(shared_ptr<T>& ptr) : shared_ptr(ptr.counter){
		
	}

	shared_ptr(shared_ptr<T>&& ptr){
		counter = ptr.counter;
		ptr.counter = NULL;
	}

	~shared_ptr(){
		if(counter == NULL)
			return;
		counter -> decStrong();
		if(counter -> strong_ref_count == 0){
			delete counter -> ptr;
			counter -> ptr = NULL;
		}
		if(counter -> weak_ref_count == 0){
			delete counter;
		}
	}

	T& operator*() const{
		assert(counter != NULL, "Error: null dereference");
		return *(counter -> ptr);
	}

	T* operator->() const{
		if(counter == NULL)
			return NULL;
		return counter -> ptr;
	}

	bool isNull() const{
		if(counter == NULL)
			return true;
		return counter -> ptr == NULL;
	}

	shared_ptr<T>& operator=(shared_ptr<T>& ptr){
		this -> ~shared_ptr();
		counter = ptr.counter;
		counter -> incStrong();
		return *this;
	}

	shared_ptr<T>& operator=(shared_ptr<T>&& ptr){
		this -> ~shared_ptr();
		counter = ptr.counter;
		ptr.counter = NULL;
		return *this;
	}

	weak_ptr<T> getWeak(){
		if(counter == NULL)
			return weak_ptr<T>();
		return weak_ptr<T>(*counter);
	}

private:
	shared_ptr_counter<T>* counter;
};

template <class T, class... Args>
unique_ptr<T> make_unique(Args&&... args){
	return unique_ptr<T>(new T(forward(args) ...));
}

template <class T, class... Args>
shared_ptr<T> make_shared(Args&&... args){
	T* ptr = new T(forward(args) ...);
	auto* counter = new shared_ptr_counter<T>(ptr); 
	return shared_ptr<T>(*counter);
}

template <class T>
PrintStream& operator<<(PrintStream& p, unique_ptr<T>& ptr){
	if(ptr.isNull())
		p << "unique_ptr(NULL)";
	else
		p << "unique_ptr(" << &(*ptr) << ")";
	return p;
};

template <class T>
PrintStream& operator<<(PrintStream& p, shared_ptr<T>& ptr){
	if(ptr.isNull())
		p << "shared_ptr(NULL)";
	else
		p << "shared_ptr(" << &(*ptr) << ")";
	return p;
}

#endif

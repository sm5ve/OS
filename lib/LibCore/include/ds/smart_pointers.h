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

	bool operator==(unique_ptr<T>& rhs) const{
		return (rhs.ptr == ptr);
	}

private:
	T* ptr;
};

struct shared_ptr_counter{
	uint32_t strong_ref_count;
	uint32_t weak_ref_count;

	//TODO make atomic
	//Perhaps it would better to make an atomic_shared_ptr instead
	shared_ptr_counter(){
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

	weak_ptr(shared_ptr_counter* c, T* ptr) : counter(c), t_ptr(ptr){
		counter -> incWeak();
	}

	weak_ptr(weak_ptr<T>& ptr) : weak_ptr(ptr.counter, ptr.t_ptr){

	}

	weak_ptr(weak_ptr<T>&& ptr){
		counter = ptr.counter;
		ptr.counter = NULL;
		t_ptr = ptr.t_ptr;
		ptr.t_ptr = NULL;
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
		return *t_ptr;
	}

	T* operator->() const{
		return t_ptr;
	}
	
	bool isNull() const{
		return t_ptr == NULL;
	}
	
	weak_ptr<T>& operator=(weak_ptr<T>& ptr){
		this -> ~weak_ptr();
		counter = ptr.counter;
		counter -> incWeak();
		t_ptr = ptr.t_ptr;
		return *this;
	}

	weak_ptr<T>& operator=(weak_ptr<T>&& ptr){
		this -> ~weak_ptr();
		counter = ptr.counter;
		ptr.counter = NULL;
		t_ptr = ptr.t_ptr;
		ptr.t_ptr = NULL;
		return *this;
	}

	bool operator==(weak_ptr<T>& rhs){
		return (rhs.counter == counter) && (rhs.t_ptr == t_ptr);
	}
private:
	shared_ptr_counter* counter;
	T* t_ptr;
};

template <class T>
class shared_ptr{
public:
	shared_ptr(){

	}
	
	shared_ptr(shared_ptr_counter* c, T* ptr) : counter(c), t_ptr(ptr){
		counter -> incStrong();
	}
	
	shared_ptr(shared_ptr<T>& ptr) : shared_ptr(ptr.counter, ptr.t_ptr){
		
	}

	shared_ptr(shared_ptr<T>&& ptr){
		counter = ptr.counter;
		t_ptr = ptr.t_ptr;
		ptr.counter = NULL;
		ptr.t_ptr = NULL;
	}

	//TODO we probably have to do something special for arrays
	~shared_ptr(){
		if(counter == NULL)
			return;
		counter -> decStrong();
		if(counter -> strong_ref_count == 0){
			delete t_ptr;
			t_ptr = NULL;
		}
		if(counter -> weak_ref_count == 0){
			delete counter;
		}
	}

	T& operator*() const{
		return *t_ptr;
	}

	T* operator->() const{
		return t_ptr;
	}

	bool isNull() const{
		return t_ptr == NULL;
	}

	shared_ptr<T>& operator=(shared_ptr<T>& ptr){
		this -> ~shared_ptr();
		counter = ptr.counter;
		counter -> incStrong();
		t_ptr = ptr.t_ptr;
		return *this;
	}

	shared_ptr<T>& operator=(shared_ptr<T>&& ptr){
		this -> ~shared_ptr();
		counter = ptr.counter;
		ptr.counter = NULL;
		t_ptr = ptr.t_ptr;
		ptr.t_ptr = NULL;
		return *this;
	}

	weak_ptr<T> getWeak(){
		return weak_ptr<T>(counter, t_ptr);
	}

	bool operator==(shared_ptr<T>& rhs){
		return (rhs.counter == counter) && (rhs.t_ptr == t_ptr);
	}

private:
	shared_ptr_counter* counter;
	T* t_ptr;
	template <class A, class B>
	friend shared_ptr<A> dynamic_ptr_cast(shared_ptr<B>&);
};

template <class T, class... Args>
unique_ptr<T> make_unique(Args&&... args){
	return unique_ptr<T>(new T(args ...));
}

template <class T, class... Args>
shared_ptr<T> make_shared(Args&&... args){
	T* ptr = new T(args ...);
	auto* counter = new shared_ptr_counter(); 
	return shared_ptr<T>(counter, ptr);
}

template <class T>
PrintStream& operator<<(PrintStream& p, unique_ptr<T>& ptr){
	if(ptr.isNull())
		p << "unique_ptr(NULL)";
	else
		p << "unique_ptr(" << &(*ptr) << ")";
	return p;
}

template <class T>
PrintStream& operator<<(PrintStream& p, shared_ptr<T>& ptr){
	if(ptr.isNull())
		p << "shared_ptr(NULL)";
	else
		p << "shared_ptr(" << &(*ptr) << ")";
	return p;
}

template <class A, class B>
shared_ptr<A> dynamic_ptr_cast(shared_ptr<B>& ptr){
	return shared_ptr<A>(ptr.counter, dynamic_cast<A*>(ptr.t_ptr));
}

#endif

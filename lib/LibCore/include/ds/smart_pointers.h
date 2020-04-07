#ifndef SMART_PTRS
#define SMART_PTRS

#include <PrintStream.h>
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

template <class T, class... Args>
unique_ptr<T> make_unique(Args&&... args){
	return unique_ptr<T>(new T(forward(args) ...));
}

template <class T>
PrintStream& operator<<(PrintStream& p, unique_ptr<T>& ptr){
	if(ptr.isNull())
		p << "unique_ptr(NULL)";
	else
		p << "unique_ptr(" << &(*ptr) << ")";
	return p;
};

#endif

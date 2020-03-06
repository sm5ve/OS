#ifndef MAYBE
#define MAYBE

#include <klib/PrintStream.h>

template <class T>
class Maybe{
public:
	Maybe();
	Maybe(T);
	~Maybe();

	bool has_value() const;
	T value() const;
private:
	T val;
	bool has;
};

template <class T>
Maybe<T>::Maybe(){
	has = false;
}

template <class T>
Maybe<T>::Maybe(T t){
	val = t;
	has = true;
}

template <class T>
Maybe<T>::~Maybe(){

}

template <class T>
bool Maybe<T>::has_value() const{
	return has;
}

template <class T>
T Maybe<T>::value() const{
	assert(has, "Error: tried to get value from valueless Maybe");
	return val;
}

template <class T>
PrintStream& operator<<(PrintStream& p, const Maybe<T>& m){
	if(m.has_value()){
		p << "Maybe(" << m.value() << ")";
	}
	else{
		p << "Maybe(None)";
	}
	return p;
}

#endif

#ifndef TUPLE
#define TUPLE

#include <klib/PrintStream.h>

template <class A, class B>
class Tuple{
public:
	Tuple(A, B);
	Tuple(Tuple<A,B>&);
	Tuple();
	~Tuple();	

	A a;
	B b;

	Tuple<A,B>& operator=(const Tuple<A,B>&);
};

template <class A, class B>
Tuple<A,B>::Tuple(A x, B y){
	a = x;
	b = y;
}

template <class A, class B>
Tuple<A,B>::Tuple(Tuple<A,B>& tup){
	a = tup.a;
	b = tup.b;
}

template <class A, class B>
Tuple<A,B>::Tuple(){}

template <class A, class B>
Tuple<A,B>::~Tuple(){}

template <class A, class B>
PrintStream& operator<<(PrintStream& p, Tuple<A,B> t){
	p << "Tuple(" << t.a << ", " << t.b << ")";
	return p;
}

template <class A, class B>
Tuple<A,B>& Tuple<A,B>::operator=(const Tuple<A,B>& t){
	this -> a = t.a;
	this -> b = t.b;
	return *this;
}

#endif

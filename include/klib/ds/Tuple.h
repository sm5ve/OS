#ifndef TUPLE
#define TUPLE

template <class A, class B>
class Tuple{
public:
	Tuple(A, B);
	Tuple();
	~Tuple();	

	A a;
	B b;
};

template<class A, class B>
Tuple<A,B>::Tuple(A x, B y){
	a = x;
	b = y;
}

template<class A, class B>
Tuple<A,B>::Tuple(){}

template<class A, class B>
Tuple<A,B>::~Tuple(){

}

#endif

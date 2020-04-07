#ifndef INTERVALS
#define INTERVALS

#include <ds/LinkedList.h>
#include <ds/Tuple.h>
#include <ds/Maybe.h>
#include <hash.h>
#include <PrintStream.h>
#include <template_magic.h>

template <class T>
class Interval{
public:
	Interval(T start, T end);
	Interval();
	~Interval();
	Tuple<Maybe<Interval<T>>, Maybe<Interval<T>>> subtract(Interval<T>) const;
	Tuple<Interval<T>, Maybe<Interval<T>>> add(Interval<T>) const;
	
	bool in(T) const;

	T getStart() const;
	T getEnd() const;
	T getSize() const;

	bool operator==(Interval<T>& rhs);
private:
	T start;
	T end;
};

//TODO this template stuff should be moved to its own header

template<class T>
struct is_interval : false_type{};

template<class T>
struct is_interval<Interval<T>> : true_type{};

template <class T>
concept IntervalType = is_interval<T>::value;

template <IntervalType T>// requires is_interval<T>
struct hash<T>
{
size_t operator()(T interval){
	return interval.getStart() * interval.getEnd();
}
};

template <class T>
class IntervalSet{
public:
	IntervalSet();
	~IntervalSet();

	void add(Interval<T> in);
	void add(IntervalSet<T>& ints);
	
	void subtract(Interval<T> in);
	void subtract(IntervalSet<T>& ints);
	
	bool in(T elem);
	Maybe<Interval<T>> findSubintervalOfSize(T size);
	Maybe<Interval<T>> intervalFor(T elem);

	LinkedList<Interval<T>>* getIntervals(); //TODO maybe make a read-only view for linked lists
private:
	LinkedList<Interval<T>> intervals;
};

template <class T>
IntervalSet<T>::IntervalSet(){

}

template <class T>
IntervalSet<T>::~IntervalSet(){
	
}

template <class T>
LinkedList<Interval<T>>* IntervalSet<T>::getIntervals(){
	return &intervals;
}

template <class T>
void IntervalSet<T>::add(Interval<T> in){
	auto node = intervals.head();
	while(node != intervals.end()){
		auto current_node = node;
		node = node -> next();
		
		auto result = current_node -> value.add(in);
		if(result.b.has_value()){
			continue;
		}
		in = result.a;
		intervals.remove(current_node);
	}
	intervals.add(in);
}

template <class T>
void IntervalSet<T>::add(IntervalSet<T>& set){
	auto node = set.intervals.head();
	while(node != set.intervals.end()){
		add(node -> value);
		node = node -> next();
	}
}

template <class T>
void IntervalSet<T>::subtract(Interval<T> in){
	auto sentinal = intervals.add(Interval<T>(0,0));
	while(intervals.head() != sentinal){
		auto result = intervals.head() -> value.subtract(in);
		intervals.remove(intervals.head());
		if(result.a.has_value()){
			intervals.add(result.a.value());
		}
		if(result.b.has_value()){
			intervals.add(result.b.value());
		}
	}
	intervals.remove(sentinal);
}

template <class T>
void IntervalSet<T>::subtract(IntervalSet<T>& set){
	auto node = set.intervals.head();
	while(node != set.intervals.end()){
		subtract(node -> value);
		node = node -> next();
	}
}

template <class T>
bool IntervalSet<T>::in(T elem){
	auto node = this -> intervals.head();
	while(node != this -> intervals.end()){
		if(node -> value.in(elem)){
			return true;
		}
		node = node -> next();
	}
	return false;
}

template <class T>
Maybe<Interval<T>> IntervalSet<T>::intervalFor(T elem){
	auto node = this -> intervals.head();
	while(node != this -> intervals.end()){
		if(node -> value.in(elem)){
			return Maybe<Interval<T>>(node -> value);
		}
		node = node -> next();
	}
	return Maybe<Interval<T>>();
}

template <class T>
Maybe<Interval<T>> IntervalSet<T>::findSubintervalOfSize(T size){
	auto node = this -> intervals.head();
	while(node != intervals.end()){
		if(node -> value.getSize() >= size){
			return Maybe<Interval<T>>(Interval(node -> value.getStart(), node -> value.getStart() + size - 1));
		}
		node = node -> next();
	}
	return Maybe<Interval<T>>();
}
template<class T>
Interval<T>::Interval(T s, T e){
	start = s;
	end = e;
	assert(start <= end, "Error: end of interval must be after start");
}

template<class T>
Interval<T>::Interval(){}

template<class T>
Interval<T>::~Interval(){

}

template <class T>
bool Interval<T>::in(T t) const{
	return (t >= start) && (t <= end);
}

template <class T>
Tuple<Interval<T>, Maybe<Interval<T>>> Interval<T>::add(Interval<T> i) const{
	//If the intervals are overlapping, we merge them and output only one interval
	if(in(i.start) || in(i.end) || i.in(start) || i.in(end)){
		T s = (i.start < start) ? i.start : start;
		T e = (i.end > end) ? i.end : end;
		Interval out(s, e);
		return Tuple(out, Maybe<Interval<T>>());
	}
	return Tuple(*this, Maybe(i));
}

template <class T>
Tuple<Maybe<Interval<T>>, Maybe<Interval<T>>> Interval<T>::subtract(Interval<T> i) const{
	if(in(i.start) && in(i.end)){
		Maybe<Interval<T>> first;
		Maybe<Interval<T>> second;
		T inner_end = i.start - 1;
		T inner_start = i.end + 1;
		if(inner_end >= start){
			first = Maybe(Interval(start, inner_end));
		}
		if(inner_start <= end){
			second = Maybe(Interval(inner_start, end));
		}
		return Tuple(first, second);
	}
	T s = start;
	T e = end;
	if(i.in(start)){
		s = i.end + 1;
	}
	bool clearedEnd = false;
	if(i.in(end)){
		clearedEnd = (i.start <= e);
		e = i.start - 1;
	}
	if((s <= e) && !clearedEnd){
		return Tuple(Maybe(Interval(s, e)), Maybe<Interval<T>>());
	}
	return Tuple(Maybe<Interval<T>>(), Maybe<Interval<T>>());
}

template <class T>
T Interval<T>::getStart() const{
	return start;
}

template <class T>
T Interval<T>::getEnd() const{
	return end;
}

template <class T>
bool Interval<T>::operator==(Interval<T>& rhs){
	return (start == rhs.start) && (end == rhs.end);
}

template <class T>
PrintStream& operator<<(PrintStream& p, const Interval<T>& i){
	p << "Interval(" << i.getStart() << ", " << i.getEnd() << ")";
	return p;
}

template <class T>
PrintStream& operator<<(PrintStream& p, IntervalSet<T>& s){
	LinkedList<Interval<T>>* list = s.getIntervals();
	LinkedListNode<Interval<T>>* node = list -> head();
	p << "IntervalSet(";
	while(node != list -> end()){
		p << node -> value;
		node = node -> next();
		if(node != list -> end()){
			p << ", ";
		}
	}
	p << ")";
	return p;
}

template <class T>
T Interval<T>::getSize() const{
	return getEnd() - getStart() + 1; //I seriously wonder if we should exclude the endpoint
}

#endif

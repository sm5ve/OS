#ifndef INTERVALS
#define INTERVALS

template <class T>
class Interval{
public:
	Interval(T start, T end);
	Interval subtract(Interval I);
	Interval add(Interval I);
private:
	T start;
	T end;
};

template <class T>
class IntervalSet{
public:
	Intervals();
	~Intervals();

	void add(Interval in);
	void add(Intervals ints);
	
	void remove(Interval in);
	void remove(Intervals ints);
private:
	
};

#endif

#ifndef HASH
#define HASH

template <class T>
struct hash{};


template<> struct hash<int>
{
size_t operator()(const int i){
	return (size_t)i; //TODO come up with a better hash function
}
};

/*template<> struct hash<void*>
{
size_t operator()(const void* ptr){
	return (size_t)ptr;
}
};*/

template<> struct hash<const char*>
{
size_t operator()(const char*& str){
	//hashing algorithm from djb2 by Dan Bernstein
	size_t out = 5381;
	while(*str){
		out = ((out << 5) + out) + *str;
		str++;
	}
	return out;
}
};

template<> struct hash<char*>
{
size_t operator()(const char* str){
	//hashing algorithm from djb2 by Dan Bernstein
	size_t out = 5381;
	while(*str){
		out = ((out << 5) + out) + *str;
		str++;
	}
	return out;
}
};
#endif

#ifndef STRING
#define STRING

#include <klib/PrintStream.h>
#include <klib/hash.h>

class String{
public:
	String();
	String(const char*);
	String(char);
	String(int);
	String(uint32_t);
	String(long);
	String(uint64_t);
	String(int, int base);
	String(uint32_t, int base);
	String(long, int base);
	String(uint64_t, int base);
	String(float);
	String(double);
	String(bool);
	String(const String&);	

	~String();

	int length();
	int capacity();

	char* c_str();

	char& operator[](int);
	String operator+(const String&) const;
	String& operator+=(const String&);
	bool operator<(const String& rhs) const;
	bool operator>(const String& rhs) const;
	bool operator==(const String&) const;
	bool operator==(const char*&) const;
	String& operator=(const String&); //I'm not quite sure if this is the behavior that I want, but I think it might be.
	friend PrintStream& operator<<(PrintStream&, const String& str);
	friend PrintStream& operator<<(PrintStream&, const String* str);
private:
	int cap;
	int len;
	char* buffer;
};

class Stringable{
public:
	virtual String& to_string();
};

template<> struct hash<String>
{
size_t operator()(String str){
	return hash<char*>{}(str.c_str());
}
};

template<> struct hash<String&>
{
size_t operator()(String& str){
	return hash<char*>{}(str.c_str());
}
};

#endif

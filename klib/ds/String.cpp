#include <klib/ds/String.h>
#include <klib/util/str.h>
#include <mem.h>
#include <assert.h>
#include <stddef.h>

String::String() : String(""){}

String::String(const char* str){
	len = strlen(str);
	if(len != 0){
		cap = len * 1.5;
	}
	else{
		cap = 16;
	}
	buffer = new char[cap];
	memset(buffer, 0, cap);
	strcpy(buffer, str);
}

String::String(char c){
	len = 1;
	cap = 4;
	buffer = new char[cap];
	memset(buffer, 0, cap);
	buffer[0] = c;
}

String::String(int n) : String(n, 10){

}

String::String(uint32_t n) : String(n, 10){

}

String::String(long n) : String(n, 10){

}

String::String(uint64_t n) : String(n, 10){

}

String::String(int n, int base){
	size_t size = 0;
	int m = n;
	while(m > 0){
		size++;
		m /= base;
	}
	len = size;
	cap = len + 2; //Honestly these numbers are kinda arbitrary.
	buffer = new char[cap];
	itoa(n, buffer, base);
}

String::String(uint32_t n, int base){
	size_t size = 0;
	int m = n;
	while(m > 0){
		size++;
		m /= base;
	}
	len = size;
	cap = len + 2;
	buffer = new char[cap];
	itoa(n, buffer, base);
}

String::String(long n, int base){
	size_t size = 0;
	int m = n;
	while(m > 0){
		size++;
		m /= base;
	}
	len = size;
	cap = len + 2;
	buffer = new char[cap];
	itoa(n, buffer, base);
}

String::String(uint64_t n, int base){
	size_t size = 0;
	int m = n;
	while(m > 0){
		size++;
		m /= base;
	}
	len = size;
	cap = len + 2;
	buffer = new char[cap];
	itoa(n, buffer, base);
}

String::String(float x){
	assert(false, "Unimplemented!");
}

String::String(double x){
	assert(false, "Unimplemented!");
}

String::String(bool b){
	cap = 6;
	buffer = new char[cap];
	if(b){
		len = 4;
		strcpy(buffer, "true");
	}
	else{
		len = 5;
		strcpy(buffer, "false");
	}
}

String::String(const String& str){
	len = str.len;
	cap = str.cap;
	buffer = new char[cap];
	strcpy(buffer, str.buffer);
}

String::~String(){
	delete buffer;
}

int String::length(){
	return len;
}

int String::capacity(){
	return cap;
}

char* String::c_str(){
	return buffer;
}

char& String::operator[](int ind){
	return buffer[ind];
}

String String::operator+(const String& str) const{
	String out;
	out.len = str.len + len;
	delete out.buffer;
	out.buffer = new char[(int)(out.len * 1.5)];
	strcpy(out.buffer, buffer);
	strcat(out.buffer, str.buffer);
	return out;
}

String& String::operator+=(const String& str){
	char* oldbuff = buffer;
	bool to_free = false;
	if(str.len + len >= cap){
		to_free = true;
		cap = str.len + len + 10;
		buffer = new char[cap];
	}

	if(to_free){
		strcpy(buffer, oldbuff);
	}	

	strcat(buffer, str.buffer);

	if(to_free){
		delete oldbuff;
	}
	return *this;
}

bool String::operator<(const String& str) const{
	return strcmp(buffer, str.buffer) < 0;
}

bool String::operator>(const String& str) const{
	return str < *this;
}

bool String::operator==(const String& str) const{
	return strcmp(buffer, str.buffer) == 0;
}

PrintStream& operator<<(PrintStream& p, const String& str){
	return p << str.buffer;
}

#include <PrintStream.h>
#include <util/str.h>

PrintStream& PrintStream::operator<<(const char c){
	put_char(c);
	return *this;
}

PrintStream& PrintStream::operator<<(const char* str){
	while(*str){
		*this << *str;
		str++;
	}
	return *this;
}

PrintStream& PrintStream::operator<<(const void* ptr){
	char str[100];
	paddedItoa((uint32_t)ptr, str, 16, 8);
	*this << "0x" << str;
	return *this;
}

PrintStream& PrintStream::operator<<(const bool b){
	if(b){
		*this << "true";
	}
	else{
		*this << "false";
	}
	return *this;
}

PrintStream& PrintStream::operator<<(const int i){
	char str[100];
	itoa(i, str, 10);
	*this << str;
	return *this;
}

PrintStream& PrintStream::operator<<(const uint32_t i){
	char str[100];
	itoa(i, str, 10);
	*this << str;
	return *this;
}

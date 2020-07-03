#ifndef PRINT_STREAM
#define PRINT_STREAM

#include <stdint.h>

class PrintStream{
public:
	virtual void put_char(const char c) = 0;
	PrintStream& operator<<(const char);
	PrintStream& operator<<(const char*);
	PrintStream& operator<<(const void*);
	PrintStream& operator<<(const bool);
	PrintStream& operator<<(const int);
	PrintStream& operator<<(const uint32_t);
	PrintStream& operator<<(const uint64_t);
};
#endif

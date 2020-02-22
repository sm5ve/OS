#ifndef PRINT_SERIAL
#define PRINT_SERIAL

#include <stdint.h>
#include <klib/util/str.h>

enum COMPort : uint16_t{
	COM1 = 0x3f8,
	COM2 = 0x2f8,
	COM3 = 0x3e8,
	COM4 = 0x2e8
};

class SerialPrinter{
	public:
		SerialPrinter(COMPort p);
		SerialPrinter();
		
		SerialPrinter& operator<<(const char c);
		SerialPrinter& operator<<(const char* c);			
		SerialPrinter& operator<<(const void* ptr);
		SerialPrinter& operator<<(const bool b);
		SerialPrinter& operator<<(const int i);
		SerialPrinter& operator<<(const uint32_t i);
	private:
		bool isTransmitEmpty();
		COMPort port;		
};
#endif

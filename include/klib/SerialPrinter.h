#ifndef PRINT_SERIAL
#define PRINT_SERIAL

enum COMPort{
	COM1 = 0x3f8,
	COM2 = 0x2f8,
	COM3 = 0x3e8,
	COM4 = 0x2e8
};

class SerialPrinter{
	public:
		SerialPrinter(COMPort port);
		
		SerialPrinter& operator<<(char c);
		SerialPrinter& operator<<(char* c);	
		SerialPrinter& operator<<(int i);
		SerialPrinter& operator<<(void* ptr);
	private:
		bool isTransmitEmpty();
		COMPort port;		
};

#endif

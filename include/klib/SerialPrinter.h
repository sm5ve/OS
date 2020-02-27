#ifndef PRINT_SERIAL
#define PRINT_SERIAL

#include <stdint.h>
#include <klib/util/str.h>
#include <klib/PrintStream.h>

enum COMPort : uint16_t{
	COM1 = 0x3f8,
	COM2 = 0x2f8,
	COM3 = 0x3e8,
	COM4 = 0x2e8
};

class SerialPrinter;

namespace SP{
	SerialPrinter& the();
	void init();
}

class SerialPrinter : public virtual PrintStream{
public:
	static SerialPrinter& the();
	static SerialPrinter& the(COMPort);
	void put_char(const char c) override;
	SerialPrinter();
private:
	bool isTransmitEmpty();
	friend void SP::init();
	SerialPrinter(COMPort p);
	COMPort port;		
};
#endif

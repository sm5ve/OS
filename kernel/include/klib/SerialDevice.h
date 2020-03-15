#ifndef PRINT_SERIAL
#define PRINT_SERIAL

#include <stdint.h>
#include <PrintStream.h>

enum COMPort : uint16_t{
	COM1 = 0x3f8,
	COM2 = 0x2f8,
	COM3 = 0x3e8,
	COM4 = 0x2e8
};

class SerialDevice;

namespace SD{
	SerialDevice& the();
	void init();
}

class SerialDevice : public virtual PrintStream{
public:
	static SerialDevice& the();
	static SerialDevice& the(COMPort);
	void put_char(const char c) override;
	SerialDevice();
private:
	bool isTransmitEmpty();
	friend void SD::init();
	SerialDevice(COMPort p);
	COMPort port;		
};
#endif

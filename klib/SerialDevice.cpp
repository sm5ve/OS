#include <klib/SerialDevice.h>
#include <arch/i386/proc.h>
#include <klib/util/str.h>

#include <mem.h>

SerialDevice ps[4];

SerialDevice::SerialDevice(COMPort p){
	port = p;
	outb(port + 1, (char) 0x00);    // Disable all interrupts
	outb(port + 3, (char) 0x80);    // Enable DLAB (set baud rate divisor)
	outb(port + 0, (char) 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(port + 1, (char) 0x00);    //                  (hi byte)
	outb(port + 3, (char) 0x03);    // 8 bits, no parity, one stop bit
	outb(port + 2, (char) 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(port + 4, (char) 0x0B);    // IRQs enabled, RTS/DSR set
}

SerialDevice::SerialDevice(){}

bool SerialDevice::isTransmitEmpty(){
	return (inb(port + 5) & 20) == 0;
}

void SerialDevice::put_char(const char c){
	while(!isTransmitEmpty());
	outb(port, c);
}

void SD::init(){
	new (&ps[0]) SerialDevice(COMPort::COM1);
	new (&ps[1]) SerialDevice(COMPort::COM2);
	new (&ps[2]) SerialDevice(COMPort::COM3);
	new (&ps[3]) SerialDevice(COMPort::COM4);
}

SerialDevice& SerialDevice::the(COMPort c){
	switch(c){
	case COMPort::COM1:
		return ps[0];
	case COMPort::COM2:
		return ps[1];
	case COMPort::COM3:
		return ps[2];
	case COMPort::COM4:
		return ps[3];
	}
	assert(false, "Invalid COM number");
	return ps[0]; //FIXME is there a better way to supress the GCC warning here?
}

SerialDevice& SerialDevice::the(){
	return the(COMPort::COM1);
}

SerialDevice& SD::the(){
	return SerialDevice::the();
}

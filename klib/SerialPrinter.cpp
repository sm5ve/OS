#include <klib/SerialPrinter.h>
#include <arch/i386/proc.h>
#include <klib/util/str.h>

#include <mem.h>

SerialPrinter ps[4];

SerialPrinter::SerialPrinter(COMPort p){
	port = p;
	outb(port + 1, (char) 0x00);    // Disable all interrupts
	outb(port + 3, (char) 0x80);    // Enable DLAB (set baud rate divisor)
	outb(port + 0, (char) 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(port + 1, (char) 0x00);    //                  (hi byte)
	outb(port + 3, (char) 0x03);    // 8 bits, no parity, one stop bit
	outb(port + 2, (char) 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(port + 4, (char) 0x0B);    // IRQs enabled, RTS/DSR set
}

SerialPrinter::SerialPrinter(){}

bool SerialPrinter::isTransmitEmpty(){
	return (inb(port + 5) & 20) == 0;
}

void SerialPrinter::put_char(const char c){
	while(!isTransmitEmpty());
	outb(port, c);
}

void SP::init(){
	new (&ps[0]) SerialPrinter(COMPort::COM1);
	new (&ps[1]) SerialPrinter(COMPort::COM2);
	new (&ps[2]) SerialPrinter(COMPort::COM3);
	new (&ps[3]) SerialPrinter(COMPort::COM4);
}

SerialPrinter& SerialPrinter::the(COMPort c){
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
}

SerialPrinter& SerialPrinter::the(){
	return the(COMPort::COM1);
}

SerialPrinter& SP::the(){
	return SerialPrinter::the();
}

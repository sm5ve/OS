#include <klib/SerialPrinter.h>
#include <arch/i386/proc.h>
#include <klib/util/str.h>

SerialPrinter::SerialPrinter(){}

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

bool SerialPrinter::isTransmitEmpty(){
	return (inb(port + 5) & 20) == 0;
}

SerialPrinter& SerialPrinter::operator<<(const char c){
	while(!isTransmitEmpty());
	outb(port, c);
	return *this;
}

SerialPrinter& SerialPrinter::operator<<(const char* c){
	for(;*c != 0; c++){
		*this << *c;
	}
	return *this;
}

SerialPrinter& SerialPrinter::operator<<(const int i){
	char str[100];
	itoa(i, str, 10);
	*this << str;
	return *this;
}

SerialPrinter& SerialPrinter::operator<<(const void* ptr){
	char str[100];
	paddedItoa((int)ptr, str, 16, 8);
	*this << "0x" << str;
	return *this;
}


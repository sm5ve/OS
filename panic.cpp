#include <panic.h>
#include <interrupts.h>
#include <klib/SerialPrinter.h>

bool first_panic_layer = true;

void [[noreturn]] panic(const char* message){
	SerialPrinter p(COMPort::COM1);
	if(!first_panic_layer){
		p << "KERNEL DOUBLE PANIC\n";
		p << message;
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	p << "KERNEL PANIC\n";
	p << message;
	for(;;);
}

void [[noreturn]] panic(const char* message, const char* filename, const char* func, int lineno){
	SerialPrinter p(COMPort::COM1);
	if(!first_panic_layer){
		p << "KERNEL DOUBLE PANIC\n";
		p << "FILE:   " << filename << "\n";
		p << "FUNC:   " << func << "\n";
		p << "LINENO: " << lineno << "\n";
		p << message;
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	p << "KERNEL PANIC\n";
	p << "FILE:   " << filename << "\n";
	p << "FUNC:   " << func << "\n";
	p << "LINENO: " << lineno << "\n";
	p << message;
	for(;;);
}

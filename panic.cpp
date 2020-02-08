#include <panic.h>
#include <interrupts.h>
#include <klib/SerialPrinter.h>

#include <arch/i386/proc.h> //temp

bool first_panic_layer = true;

void [[noreturn]] panic(const char* message){
	SerialPrinter p(COMPort::COM1);
	if(!first_panic_layer){
		p << "KERNEL DOUBLE PANIC\n";
		p << message << "\n";
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	p << "KERNEL PANIC\n";
	p << message << "\n";
	outw(0x604, 0x2000); //temporary
	for(;;);
}

void [[noreturn]] panic(const char* message, const char* filename, const char* func, int lineno){
	SerialPrinter p(COMPort::COM1);
	if(!first_panic_layer){
		p << "KERNEL DOUBLE PANIC\n";
		p << "FILE:   " << filename << "\n";
		p << "FUNC:   " << func << "\n";
		p << "LINENO: " << lineno << "\n";
		p << message << "\n";
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	p << "KERNEL PANIC\n";
	p << "FILE:   " << filename << "\n";
	p << "FUNC:   " << func << "\n";
	p << "LINENO: " << lineno << "\n";
	p << message << "\n";
	outw(0x604, 0x2000); //temporary
	for(;;);
}

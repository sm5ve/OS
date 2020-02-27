#include <panic.h>
#include <interrupts.h>
#include <klib/SerialPrinter.h>

#include <arch/i386/proc.h> //temp

bool first_panic_layer = true;

void [[noreturn]] panic(const char* message){
	if(!first_panic_layer){
		SerialPrinter::the() << "KERNEL DOUBLE PANIC\n";
		SerialPrinter::the() << message << "\n";
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	SerialPrinter::the() << "KERNEL PANIC\n";
	SerialPrinter::the() << message << "\n";
	outw(0x604, 0x2000); //temporary
	for(;;);
}

void [[noreturn]] panic(const char* message, const char* filename, const char* func, int lineno){
	if(!first_panic_layer){
		SerialPrinter::the() << "KERNEL DOUBLE PANIC\n";
		SerialPrinter::the() << "FILE:   " << filename << "\n";
		SerialPrinter::the() << "FUNC:   " << func << "\n";
		SerialPrinter::the() << "LINENO: " << lineno << "\n";
		SerialPrinter::the() << message << "\n";
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	SerialPrinter::the() << "KERNEL PANIC\n";
	SerialPrinter::the() << "FILE:   " << filename << "\n";
	SerialPrinter::the() << "FUNC:   " << func << "\n";
	SerialPrinter::the() << "LINENO: " << lineno << "\n";
	SerialPrinter::the() << message << "\n";
	outw(0x604, 0x2000); //temporary
	for(;;);
}

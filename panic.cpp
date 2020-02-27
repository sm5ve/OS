#include <panic.h>
#include <interrupts.h>
#include <klib/SerialDevice.h>

#include <arch/i386/proc.h> //temp

bool first_panic_layer = true;

void [[noreturn]] panic(const char* message){
	if(!first_panic_layer){
		SD::the() << "KERNEL DOUBLE PANIC\n";
		SD::the() << message << "\n";
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	SD::the() << "KERNEL PANIC\n";
	SD::the() << message << "\n";
	outw(0x604, 0x2000); //temporary
	for(;;);
}

void [[noreturn]] panic(const char* message, const char* filename, const char* func, int lineno){
	if(!first_panic_layer){
		SD::the() << "KERNEL DOUBLE PANIC\n";
		SD::the() << "FILE:   " << filename << "\n";
		SD::the() << "FUNC:   " << func << "\n";
		SD::the() << "LINENO: " << lineno << "\n";
		SD::the() << message << "\n";
		for(;;);
	}
	first_panic_layer = false;
	DisableInterrupts d;
	
	SD::the() << "KERNEL PANIC\n";
	SD::the() << "FILE:   " << filename << "\n";
	SD::the() << "FUNC:   " << func << "\n";
	SD::the() << "LINENO: " << lineno << "\n";
	SD::the() << message << "\n";
	outw(0x604, 0x2000); //temporary
	for(;;);
}

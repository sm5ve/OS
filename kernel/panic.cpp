#include <debug.h>
#include <devices/SerialDevice.h>
#include <interrupts.h>
#include <panic.h>

#include <arch/i386/proc.h>

bool first_panic_layer = true;

void panic(const char* message)
{
	if (!first_panic_layer) {
		SD::the() << "KERNEL DOUBLE PANIC\n";
		SD::the() << message << "\n";
		plainStackTrace(2);
		for (;;)
			;
	}
	first_panic_layer = false;
	DisableInterrupts d;

	SD::the() << "KERNEL PANIC\n";
	SD::the() << message << "\n";
	outw(0x604, 0x2000);
	stackTrace(2);
	for (;;)
		;
}

void panic(const char* message, const char* filename, const char* func,
	int lineno)
{
	if (!first_panic_layer) {
		SD::the() << "KERNEL DOUBLE PANIC\n";
		SD::the() << "FILE:   " << filename << "\n";
		SD::the() << "FUNC:   " << func << "\n";
		SD::the() << "LINENO: " << lineno << "\n";
		SD::the() << message << "\n";
		plainStackTrace(2);
		for (;;)
			;
	}
	first_panic_layer = false;
	DisableInterrupts d;

	SD::the() << "KERNEL PANIC\n";
	SD::the() << "FILE:   " << filename << "\n";
	SD::the() << "FUNC:   " << func << "\n";
	SD::the() << "LINENO: " << lineno << "\n";
	SD::the() << message << "\n";
	stackTrace(2);
	outw(0x604, 0x2000);
	for (;;)
		;
}

void panic(const char* message, registers regs)
{
	DisableInterrupts d;

	SD::the() << "KERNEL PANIC\n";
	SD::the() << message << "\n";
	SD::the() << regs << "\n";
	prettyStackTraceFromInterrupt(regs.ebp, regs.fault_eip);
	outw(0x604, 0x2000);
	for (;;)
		;
}

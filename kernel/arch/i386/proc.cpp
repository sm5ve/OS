#include <arch/i386/proc.h>

void outb(uint16_t port, uint8_t out)
{
	__asm__ volatile("outb %0, %1" ::"a"(out), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
	uint8_t out;
	__asm__ volatile("inb %1, %0"
					 : "=a"(out)
					 : "Nd"(port));
	return out;
}

void outw(uint16_t port, uint16_t word)
{
	__asm__ volatile("outw %0, %1" ::"a"(word), "Nd"(port));
}

uint16_t inw(uint16_t port)
{
	uint16_t out;
	__asm__ volatile("inw %1, %0"
					 : "=a"(out)
					 : "Nd"(port));
	return out;
}

void cli() { __asm__ volatile("cli"); }

void sti() { __asm__ volatile("sti"); }

PrintStream& operator<<(PrintStream& p, registers regs)
{
	p << "Register dump:\n";
	p << "EAX=" << (void*)regs.eax << " EBX=" << (void*)regs.ebx
	  << " ECX=" << (void*)regs.ecx << " EDX=" << (void*)regs.edx << "\n";
	p << "ESI=" << (void*)regs.esi << " EDI=" << (void*)regs.edi
	  << " EBP=" << (void*)regs.ebp << " ESP=" << (void*)regs.esp << "\n";
	p << "EIP=" << (void*)regs.fault_eip;
	p << " User ESP=" << (void*)regs.user_esp << "\n";
	p << "ES =" << (void*)regs.es << " DS =" << (void*)regs.ds
	  << " GS =" << (void*)regs.gs << " FS =" << (void*)regs.fs << "\n";
	p << "Interrupt number: " << regs.int_number << "\n";
	p << "Error code: " << (void*)regs.error_code << "\n";
	p << "Error flags: " << (void*)regs.error_flags << "\n";
	return p;
}

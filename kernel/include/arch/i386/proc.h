#ifndef I386_HEADER
#define I386_HEADER

#if !(defined(__x86_64__) || defined(__i386__))
#error "Error: included i386 header in non-intel build"
#endif

#include <stdint.h>
#include <PrintStream.h>

struct __attribute__((packed)) registers{
	uint32_t faulting_addr, gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_number, error_code;
	uint32_t fault_eip, cs, error_flags, user_esp, ss;
};

PrintStream& operator<<(PrintStream& p, registers);

void outb(uint16_t port, uint8_t c);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t w);
uint16_t inw(uint16_t port);


void cli();
void sti();

void installGDT();
void installIDT();
#endif

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

struct __attribute__((packed)) tss_entry{
	uint32_t link;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldtr;
	uint32_t iopb;
};

struct __attribute__((packed)) seg_table_descriptor{
	uint16_t size;
	uint32_t addr;
};

void writeAPBootstrapGDT(uint16_t offset);

typedef void(*interrupt_handler)(registers&);

PrintStream& operator<<(PrintStream& p, registers);

void outb(uint16_t port, uint8_t c);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t w);
uint16_t inw(uint16_t port);


void cli();
void sti();

void installGDT();
void flushGDT();

namespace IDT{
	void install();
	void installIRQHandler(interrupt_handler, uint32_t number);
}
#endif

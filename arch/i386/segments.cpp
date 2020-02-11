#include <arch/i386/proc.h>

//I'm not sure if we'll want to dynamically allocate our GDT in the future, but for now
//we'll just statically allocate room for a predetermined number of entries
#define TOTAL_GDT_SEGMENTS 3

//For now, the assumed structure is as follows:
//[0] Null GDT Entry
//[1] System Code GDT Entry (0x00000000 - 0xffffffff)
//[2] System Data GDT Entry (0x00000000 - 0xffffffff)
//In the future, we'll need a TSS entry
//Perhaps some dedicated ring 3 segments as well
//But that might be best to implement after we move over to a higher-half kernel
//And solidify the memory map

uint8_t gdt[8 * TOTAL_GDT_SEGMENTS];

struct __attribute__((__packed__)) seg_table_descriptor{
	uint16_t size;
	uint32_t addr;
};

seg_table_descriptor gdtDescriptor;

void writeSegment(int index, uint32_t base, uint32_t limit, uint8_t access){
	//For now we'll do page-aligned segments.
	uint8_t* entry = &gdt[index * 8];
	
	limit >>= 12;
	
	entry[0] = limit & 0xff;
	entry[1] = (limit >> 8) & 0xff;
	entry[2] = base & 0xff;
	entry[3] = (base >> 8) & 0xff;
	entry[4] = (base >> 16) & 0xff;
	entry[5] = access;
	entry[6] = ((limit >> 16) & 0x0f) | (0xc0);
	entry[7] = (base >> 24) & 0xff;
}

void flushGDT(){
	gdtDescriptor = {
		.size = 8 * TOTAL_GDT_SEGMENTS,
		.addr = (uint32_t)gdt
	};
	//Actually let the CPU know where the GDT is
	__asm__ volatile ("lgdt %0" :: "m" (gdtDescriptor) : "memory");
	//Reload the segment registers
	//Our kernel code segment entry is at gdt 0x08, so we'll point CS there
	__asm__ volatile("ljmpl $0x8, $continue\n" \
					 "continue:");
	//All other segments can point to our data entry at 0x10
	__asm__	volatile("mov %%ax, %%ds\n" \
					 "mov %%ax, %%es\n" \
					 "mov %%ax, %%fs\n" \
					 "mov %%ax, %%gs\n" \
					 "mov %%ax, %%ss\n" \
					 :: "a"(0x10) : "memory");
	//Do we maybe want to separate the segment register flushes to another function?
	//Maybe it's not necessary for each GDT flush? 
}

constexpr uint8_t segmentFlags(bool kernel, bool system, bool executable, bool rw){
	return
	(1 << 7)                    |//Set 'present' flag (required for all valid segment selectors
	((kernel ? 0 : 3) << 5)     |//Set privilege level of segment
	((system ? 0 : 1) << 4)     |//If bit 4 is unset, we declare this a system segment
	((executable ? 1 : 0) << 3) |
	(0 << 2)                    |//We'll always assume our segment grows up
	((rw ? 1 : 0) << 1)         |//If this is a code segment, rw = false prevents reading from the segment, if this is a data segment, rw = false prevents writing to this segment
	(0 << 0);                    //CPU sets this flag if the segment is ever accessed
}

#include <klib/SerialPrinter.h>

void installGDT(){
	SerialPrinter p(COMPort::COM1);
	writeSegment(0, 0x00000000, 0x00000000, 0); //GDT must start with null segment
	uint8_t codeFlags = segmentFlags(true, false, true, true);
	writeSegment(1, 0x00000000, 0xffffffff, codeFlags);
	uint8_t dataFlags = segmentFlags(true, false, false, true);
	writeSegment(2, 0x00000000, 0xffffffff, dataFlags);
	flushGDT();

	p << "GDT located at " << (void*)gdt << "\n";
}

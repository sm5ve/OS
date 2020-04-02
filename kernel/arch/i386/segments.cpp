#include <arch/i386/proc.h>
#include <klib/SerialDevice.h>
#include <mem.h>

#define TOTAL_GDT_SEGMENTS 6

//For now, the assumed structure is as follows:
//[0] Null GDT Entry
//[1] System Code GDT Entry (0x00000000 - 0xffffffff)
//[2] System Data GDT Entry (0x00000000 - 0xffffffff)
//In the future, we'll need a TSS entry
//Perhaps some dedicated ring 3 segments as well
//But that might be best to implement after we move over to a higher-half kernel
//And solidify the memory map
extern uint8_t stack_top;
uint8_t gdt[8 * TOTAL_GDT_SEGMENTS];

seg_table_descriptor gdtDescriptor;
tss_entry tss;

void writeSegment(uint8_t* table, int index, uint32_t base, uint32_t limit, uint8_t access){
	//For now we'll do page-aligned segments.
	uint8_t* entry = &table[index * 8];
	
	limit >>= 12;
	
	entry[0] = limit & 0xff;
	entry[1] = (limit >> 8) & 0xff;
	entry[2] = base & 0xff;
	entry[3] = (base >> 8) & 0xff;
	entry[4] = (base >> 16) & 0xff;
	entry[5] = access;
	entry[6] = (uint8_t)(((limit >> 16) & 0x0f) | (0xc0));
	entry[7] = (uint8_t)((base >> 24) & 0xff);
}

void flushGDT(){
	gdtDescriptor = {
		.size = 8 * TOTAL_GDT_SEGMENTS - 1,
		.addr = (uint32_t)gdt
	};
	//Actually let the CPU know where the GDT is
	__asm__ volatile ("lgdtl %0" :: "m" (gdtDescriptor) : "memory");
	//Reload the segment registers
	//Our kernel code segment entry is at gdt 0x08, so we'll point CS there
	__asm__ volatile("ljmpl $0x8, $segments_continue\n" \
					 "segments_continue:");
	//All other segments can point to our data entry at 0x10
	__asm__	volatile("mov %%ax, %%ds\n" \
					 "mov %%ax, %%es\n" \
					 "mov %%ax, %%fs\n" \
					 "mov %%ax, %%gs\n" \
					 "mov %%ax, %%ss\n" \
					 :: "a"(0x10) : "memory");

	__asm__ volatile("mov $0x2b, %ax \n" \
					 "ltr %ax");
	//Do we maybe want to separate the segment register flushes to another function?
	//Maybe it's not necessary for each GDT flush? 
}

constexpr uint8_t segmentFlags(bool kernel, bool system, bool executable, bool rw){
	return (uint8_t)(
	(1 << 7)                    |//Set 'present' flag (required for all valid segment selectors
	((kernel ? 0 : 3) << 5)     |//Set privilege level of segment
	((system ? 0 : 1) << 4)     |//If bit 4 is unset, we declare this a system segment
	((executable ? 1 : 0) << 3) |
	(0 << 2)                    |//We'll always assume our segment grows up
	((rw ? 1 : 0) << 1)         |//If this is a code segment, rw = false prevents reading from the segment, if this is a data segment, rw = false prevents writing to this segment
	(0 << 0));                    //CPU sets this flag if the segment is ever accessed
}

void installGDT(){
	writeSegment(gdt, 0, 0x00000000, 0x00000000, 0); //GDT must start with null segment
	uint8_t codeFlags = segmentFlags(true, false, true, true);
	writeSegment(gdt, 1, 0x00000000, 0xffffffff, codeFlags);
	uint8_t dataFlags = segmentFlags(true, false, false, true);
	writeSegment(gdt, 2, 0x00000000, 0xffffffff, dataFlags);
	uint8_t userCodeFlags = segmentFlags(false, false, true, true);
	writeSegment(gdt, 3, 0x00000000, 0xbfffffff, userCodeFlags);
	uint8_t userDataFlags = segmentFlags(false, false, false, true);
	writeSegment(gdt, 4, 0x00000000, 0xbfffffff, userDataFlags);
	writeSegment(gdt, 5, (uint32_t)&tss, (uint32_t)&tss + sizeof(tss), 0xe9);	

	memset(&tss, 0, sizeof(tss));

	tss.iopb = sizeof(tss);
	tss.ss0 = 0x10;
	tss.esp0 = (uint32_t)&stack_top;	
	tss.cs = 0x0b;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
	flushGDT();
}

uint8_t ap_gdt[3 * 8];

void writeAPBootstrapGDT(uint16_t offset){
	writeSegment(ap_gdt, 0, 0x00000000, 0x00000000, 0);
	uint8_t codeFlags = segmentFlags(true, false, true, true);
	writeSegment(ap_gdt, 1, 0x00000000, 0xffffffff, codeFlags);
	uint8_t dataFlags = segmentFlags(true, false, false, true);
	writeSegment(ap_gdt, 2, 0x00000000, 0xffffffff, dataFlags);
	seg_table_descriptor desc{
		.size = 8 * 3,
		.addr = ((uint32_t)offset & 0xffff) + 8
	};
	memcpy((void*)(0xc0000000 + offset), &desc, 8);
	memcpy((void*)(0xc0000000 + offset + 8), &ap_gdt, sizeof(ap_gdt));
}

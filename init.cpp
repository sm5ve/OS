#include <multiboot/multiboot.h>
#include <klib/SerialPrinter.h>

#include <arch/i386/proc.h>
#include <interrupts.h>

#include <assert.h>

#include <mem.h>

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot_magic, unsigned long addr){
	
	#ifndef MULTIBOOT_2
	assert(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC, "Error: Multiboot magic does not match. Aborting boot.");
	#else
	assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Error: Multiboot 2 magic does not match. Aborting boot.");
	#endif

	SerialPrinter p(COMPort::COM1);
	p << "\nMultiboot magic verified\n";
	
	initSlabAllocator();
	
	void* p1 = slabAlloc(3);
	void* p2 = slabAlloc(3);
	void* p3 = slabAlloc(4);
	
	slabFree(p2);
	slabAlloc(64);
	void* p4 = slabAlloc(6);
	slabAlloc(32);
	slabFree(p1);
	void* p5 = slabAlloc(8);
	void* p6 = slabAlloc(6);
	void* p7 = slabAlloc(5);

	DisableInterrupts d;
	
	outw(0x604, 0x2000);
}

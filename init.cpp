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
	initHeapAllocator();

	auto test = new int[4];
	delete test;	
	test = new int[1];
	new int[100];

	DisableInterrupts d;
	
	outw(0x604, 0x2000); //shutdown qemu
}

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

	p << "Memory allocators initialized\n";

	auto test = new int[4];
	//delete test;	
	//test = new int[1];
	auto x = new int[100];
	p << "x: " << x << "\n";
	auto y = new int[100];
	p << "y: " << y << "\n";
	auto z = new int[100];
	p << "z: " << z << "\n";
	//delete x;
	delete y;	
	p << "deleted y\n";
	delete z;
	//auto w = new int[100];
	//p << "w: " << w << "\n";
	p << "deleted z\n";
	auto ptrs = new int*[1000];
	for(int i = 0; i <1000; i++){
		ptrs[i] = new int[20];
	}
	p << "allocated garbage\n";
	for(int i = 0; i < 1000; i++){
		//p << (9 * i) % 1000 << "\n";
		delete ptrs[(9*i) % 1000];
	}

	DisableInterrupts d;

	outw(0x604, 0x2000); //shutdown qemu
}


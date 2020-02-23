#include <multiboot/multiboot.h>
#include <klib/SerialPrinter.h>

#include <arch/i386/proc.h>
#include <interrupts.h>
#include <paging.h>

#include <assert.h>

#include <mem.h>

#include <klib/ds/BinaryHeap.h>

extern uint32_t _kend;

//TODO the kernel's allocating a lot of memory as soon as it's loaded. Are we ever at risk of intersecting an important page of memory? Or does multiboot's memory map suggest we're good to use everything past 1 MB?

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot_magic, mboot_info* mboot){	
	#ifndef MULTIBOOT_2
	assert(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC, "Error: Multiboot magic does not match. Aborting boot.");
	#else
	assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Error: Multiboot 2 magic does not match. Aborting boot.");
	#endif

	SerialPrinter p(COMPort::COM1);
	p << "\nMultiboot magic verified\n";
	initKalloc();
	p << "Memory allocators initialized\n";
	p << "Testing the heap allocator\n";
	auto ptrs = new void*[100];

	for(int i = 0; i < 100; i++){
		ptrs[i] = new int[100];
	}
	
	p << "Malloc's\n";
	
	for(int i = 0; i < 100; i++){
		delete ptrs[(7 * i) % 100];
	}

	p << (void*)kernel_init << "\n";
	p << "Installing the GDT\n";
	installGDT();
	p << "GDT installed!\n";	
	p << "Installing the IDT\n";
	installIDT();
	p << "IDT installed!\n";

	p << (char*)((mboot -> bootloader_name) + 0xC0000000) << "\n";
	
	mboot_mmap_entry* entries = (mboot_mmap_entry*)((uint32_t)(mboot -> mmap_ptr) + 0xC0000000);
	uint32_t len = mboot -> mmap_len;
	//enterMirroredFlatPaging();
	
	//initPalloc(entries, len);
	p << "Done!\n";
	p << "_kend at " << &_kend << "\n";

	//sti();

	//for(;;){
	//	asm("hlt");
	//}
	//DisableInterrupts d;
	
	outw(0x604, 0x2000); //shutdown qemu

}


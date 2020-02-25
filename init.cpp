#include <multiboot/multiboot.h>
#include <klib/SerialPrinter.h>

#include <arch/i386/proc.h>
#include <interrupts.h>
#include <paging.h>

#include <assert.h>

#include <mem.h>
#include <klib/ds/Intervals.h>

extern uint32_t _kend;

//TODO the kernel's allocating a lot of memory as soon as it's loaded. Are we ever at risk of intersecting an important page of memory? Or does multiboot's memory map suggest we're good to use everything past 1 MB?

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot_magic, mboot_info* mboot){	
	#ifndef MULTIBOOT_2
	assert(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC, "Error: Multiboot magic does not match. Aborting boot.");
	#else
	assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Error: Multiboot 2 magic does not match. Aborting boot.");
	#endif

	SerialPrinter p(COMPort::COM1);
	p << "\n";
	p << "Multiboot magic verified\n";
	initKalloc();
	p << "Memory allocators initialized\n";

	IntervalSet<int> set;
	IntervalSet<int> set2;
	
	set.add(Interval<int>(-2,-1));
	set.add(Interval<int>(1,2));

	set2.add(Interval<int>(-1,1));
	set2.add(Interval<int>(2,5));

	p << set << "\n";
	p << set2 << "\n";
	
	set.subtract(set2);
	
	p << set << "\n";
	
	p << "Installing the GDT\n";
	installGDT();
	p << "GDT installed!\n";	
	p << "Installing the IDT\n";
	installIDT();
	p << "IDT installed!\n";

	p << "Installing kernel page directory\n";
	initializeKernelPaging();
	p << "Installed!\n";
	
	//for(;;);
	p << (char*)((mboot -> bootloader_name) + 0xC0000000) << "\n";
	
	mboot_mmap_entry* entries = (mboot_mmap_entry*)((uint32_t)(mboot -> mmap_ptr) + 0xC0000000);
	uint32_t len = mboot -> mmap_len;
	//enterMirroredFlatPaging();
	
	//initPalloc(entries, len);
	p << "Done!\n";
	p << "_kend at " << &_kend << "\n";

	p << "Has ELF symbols " << ((mboot -> flags) & 0x20) << "\n";
	p << (void*)(mboot -> elf_section_header.ptr) << "\n";
		
	/*sti();

	for(;;){
		asm("hlt");
	}*/
	//DisableInterrupts d;
	
	outw(0x604, 0x2000); //shutdown qemu

}


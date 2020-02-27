#include <multiboot/multiboot.h>
#include <klib/SerialPrinter.h>

#include <arch/i386/proc.h>
#include <interrupts.h>
#include <paging.h>

#include <assert.h>

#include <mem.h>
#include <klib/ds/Intervals.h>
#include <klib/ds/String.h>

extern uint32_t _kend;

void load_modules(mboot_module* modules, uint32_t count){
	for(int i = 0; i < count; i++){
		SP::the() << (char*)(modules[i].name_ptr + 0xC0000000) << "\n";
	}
}

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot_magic, mboot_info* mboot){	
	SP::init();

	#ifndef MULTIBOOT_2
	assert(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC, "Error: Multiboot magic does not match. Aborting boot.");
	#else
	assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Error: Multiboot 2 magic does not match. Aborting boot.");
	#endif

	SP::the() << "\n";
	SP::the() << "Multiboot magic verified\n";
	initKalloc();
	SP::the() << "Memory allocators initialized\n";

	IntervalSet<int> set;
	IntervalSet<int> set2;
	
	set.add(Interval<int>(-2,-1));
	set.add(Interval<int>(1,2));

	set2.add(Interval<int>(-1,1));
	set2.add(Interval<int>(2,5));

	SP::the() << set << "\n";
	SP::the() << set2 << "\n";
	
	set.subtract(set2);
	
	SP::the() << set << "\n";
	
	SP::the() << "Installing the GDT\n";
	installGDT();
	SP::the() << "GDT installed!\n";	
	SP::the() << "Installing the IDT\n";
	installIDT();
	SP::the() << "IDT installed!\n";

	SP::the() << "Installing kernel page directory\n";
	initializeKernelPaging();
	SP::the() << "Installed!\n";
	
	//for(;;);
	SP::the() << (char*)((mboot -> bootloader_name) + 0xC0000000) << "\n";
	
	mboot_mmap_entry* entries = (mboot_mmap_entry*)((uint32_t)(mboot -> mmap_ptr) + 0xC0000000);
	uint32_t len = mboot -> mmap_len;
	//enterMirroredFlatPaging();
	
	//initPalloc(entries, len);
	SP::the() << "Done!\n";
	SP::the() << "_kend at " << &_kend << "\n";
	
	SP::the() << "Testing strings\n";
	String hello = String("Hello ");
	String world = String("world! ");
	String val = String(69 + 420);

	load_modules((mboot_module*)(mboot -> mods_ptr + 0xC0000000), mboot -> mods_count);

	/*sti();

	for(;;){
		asm("hlt");
	}*/
	//DisableInterrupts d;
	
	outw(0x604, 0x2000); //shutdown qemu

}


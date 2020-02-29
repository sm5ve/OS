#include <multiboot/multiboot.h>
#include <klib/SerialDevice.h>

#include <arch/i386/proc.h>
#include <interrupts.h>
#include <paging.h>

#include <assert.h>

#include <mem.h>
#include <klib/ds/Intervals.h>
#include <klib/ds/String.h>
#include <elf/elf.h>

extern uint32_t _kend;

void load_modules(mboot_module* modules, uint32_t count){
	for(int i = 0; i < count; i++){
		String modName((char*)(modules[i].name_ptr + 0xC0000000));
		SD::the() << modName << "\n";
		if(modName == "kernel"){
			SD::the() << "Found the kernel!" << "\n";
			void* kstart = (void*)(modules[i].start_addr + 0xC0000000);
			ELF elf(kstart);
			SD::the() << elf.getHeader32() << "\n";
		}
	}
}

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot_magic, mboot_info* mboot){	
	SD::init();

	#ifndef MULTIBOOT_2
	assert(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC, "Error: Multiboot magic does not match. Aborting boot.");
	#else
	assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Error: Multiboot 2 magic does not match. Aborting boot.");
	#endif

	SD::the() << "\n";
	SD::the() << "Multiboot magic verified\n";
	initKalloc();
	SD::the() << "Memory allocators initialized\n";
	
	SD::the() << "Installing the GDT\n";
	installGDT();
	SD::the() << "GDT installed!\n";	
	SD::the() << "Installing the IDT\n";
	installIDT();
	SD::the() << "IDT installed!\n";

	SD::the() << "Installing kernel page directory\n";
	initializeKernelPaging();
	SD::the() << "Installed!\n";
	
	//for(;;);
	SD::the() << (char*)((mboot -> bootloader_name) + 0xC0000000) << "\n";
	
	mboot_mmap_entry* entries = (mboot_mmap_entry*)((uint32_t)(mboot -> mmap_ptr) + 0xC0000000);
	uint32_t len = mboot -> mmap_len;
	//enterMirroredFlatPaging();
	
	//initPalloc(entries, len);
	SD::the() << "Done!\n";
	SD::the() << "_kend at " << &_kend << "\n";
	
	load_modules((mboot_module*)(mboot -> mods_ptr + 0xC0000000), mboot -> mods_count);

	/*sti();

	for(;;){
		asm("hlt");
	}*/
	//DisableInterrupts d;
	
	outw(0x604, 0x2000); //shutdown qemu

}


#include <multiboot/multiboot.h>
#include <klib/SerialDevice.h>

#include <arch/i386/proc.h>
#include <interrupts.h>
#include <paging.h>

#include <assert.h>

#include <mem.h>
#include <ds/Intervals.h>
#include <ds/String.h>
#include <elf/elf.h>
#include <elf/dwarf.h>
#include <debug.h>
#include <Scheduler.h>
#include <acpi/tables.h>
#include <devices/pcie.h>

#include <loader.h>

//Since kernel_init has a lot of testing code, we sometimes get used variables for older tests that are commented out
//Hence, while the kernel's still largely in flux, we'll disable unused variable warnings for now
#pragma GCC diagnostic ignored "-Wunused-variable"

extern uint32_t _kend;

ELF* ksyms_elf = NULL;
ELF* test_program_elf = NULL;

void load_modules(mboot_module* modules, uint32_t count){
	for(uint32_t i = 0; i < count; i++){
		String modName((char*)(modules[i].name_ptr + 0xC0000000));
		if(modName == "kernel.sym"){
			SD::the() << "Found the kernel symbols!" << "\n";
			void* kstart = (void*)(modules[i].start_addr + 0xC0000000);
			ksyms_elf = new ELF(kstart);
			ksyms = new DWARF(ksyms_elf);
		}
		if(modName == "test"){
			SD::the() << "Found the test program\n";
			void* pstart = (void*)(modules[i].start_addr + 0xC0000000);
			test_program_elf = new ELF(pstart);
		}
	}
}

void kernel_thing(){
	SD::the() << "kernel_thing\n";
}

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot_magic, mboot_info* mboot){	
	SD::init();

	assert(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC, "Error: Multiboot magic does not match. Aborting boot.");

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
	
	mboot_mmap_entry* entries = (mboot_mmap_entry*)((uint32_t)(mboot -> mmap_ptr) + 0xC0000000);
	
	load_modules((mboot_module*)(mboot -> mods_ptr + 0xC0000000), mboot -> mods_count);
	SD::the() << "Initializing memory manager\n";
	MemoryManager::init(entries, mboot -> mmap_len);
	SD::the() << "Done!\n";
	Scheduler::init();
	ACPI::init();
	PCIe::init();
	sti();

	
	
	outw(0x604, 0x2000); //shutdown qemu
	for(;;){
		__asm__ ("hlt");
	}
}

#pragma GCC diagnostic pop


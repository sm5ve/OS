#include <paging.h>
#include <mem.h>
#include <multiboot/multiboot.h>
#include <klib/SerialDevice.h>
#include <stddef.h>

bool isAddressSystemReserved(uint32_t paddr){
	if(paddr < 0x01000000){
		//We'll just reserve all of low RAM, the first 14 MB of high RAM, as well as the ISA sinkhole
		return true;
	}
	return false;
}

void* nextFree = NULL;

void initMemoryRegion(uint32_t addr, uint32_t len){
	SD::the() << "initializing memory region at " << (void*) addr << " with length " << (void*)len << "\n";
	for(uint32_t page = addr & ~(0xfff); page < addr + len; page += 4096){
		if(!isAddressSystemReserved(page)){
			*((void**)page) = nextFree;
			nextFree = (void*)page;
		}
	}
}

void initPalloc(mboot_mmap_entry* entries, uint32_t len){
	mboot_mmap_entry* ent = entries;
	while((uint32_t) ent < (uint32_t)entries + len){
		if(ent -> type == MEM_AVAILABLE){
			initMemoryRegion(ent -> addr, ent -> len);
		}
		ent = (mboot_mmap_entry*)((uint32_t)ent + (ent -> size) + sizeof(void*));
	}
}

void* getPage(){
	
}

void installPD(void* pd){
    __asm__ volatile("movl %0, %%cr3\n" \
                    :: "r"(pd));
}



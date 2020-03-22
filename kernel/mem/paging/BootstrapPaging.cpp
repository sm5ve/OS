#include <paging.h>
#include <bootstrap.h>
#include <stddef.h>
#include <mem.h>
#include <flags.h>
#include <klib/SerialDevice.h>

BootstrapPaging::BootstrapPaging(){
	directory = (uint32_t*)kalloc_permanent(4096, 4096);
	memset(directory, 0, 4096);
}

BootstrapPaging::~BootstrapPaging(){}

bool BootstrapPaging::isPresent(uint32_t entry){
	if((entry & PAGE_PRESENT) == 0){
		assert(entry == 0, "Error: page entry exists, but is not present. We don't have swap implemented yet, so what gives?");
		return false;
	}
	return true;
}

bool BootstrapPaging::isMapped(virt_addr addr){
	uint32_t directoryIndex = ((uint32_t)addr >> 22) & 0x3ff;
	uint32_t tableIndex = ((uint32_t)addr >> 12) & 0x3ff;
	if(!isPresent(directory[directoryIndex]))
		return false;
	uint32_t* table = physicalToPageTableAddr((phys_addr)(directory[directoryIndex] & (~0xfff)));
	return isPresent(table[tableIndex]);
}

uint32_t* BootstrapPaging::physicalToPageTableAddr(phys_addr addr){
	return (uint32_t*)((uint32_t)addr + 0xc0000000);
}

phys_addr BootstrapPaging::virtualToPhysical(virt_addr addr){
	return (phys_addr)((uint32_t)addr - 0xc0000000);
}

virt_addr BootstrapPaging::mapRangeAfter(Interval<phys_addr> range, virt_addr after){
	uint32_t checking = (uint32_t)after;
	size_t required = (size_t) range.getEnd() - (size_t)range.getStart() + 1;
	size_t found = 0;
	virt_addr base = after;
	while(checking >= (uint32_t)after){
		if(isMapped((virt_addr)checking)){
			found = 0;
		}	
		else{
			if(found == 0)
				base = (virt_addr)checking;
			found += 4096;
			if(found > required){
				mapRange(range, base);
				return base;
			}
		}
		checking += PAGE_SIZE;
	}
	assert(false, "Page directory full!");
	return NULL;
}

void BootstrapPaging::mapRange(Interval<phys_addr> physicalRange, virt_addr base){
	uint32_t start = (uint32_t)physicalRange.getStart();
	assert(start % PAGE_SIZE == 0, "Error: misaligned physical memory region");
	uint32_t end = (uint32_t)physicalRange.getEnd();
	for(uint32_t i = start; i < end; i += PAGE_SIZE){
		addMapping((phys_addr)i, (virt_addr)(base + (i - (uint32_t)start)), PAGE_ENABLE_WRITE | PAGE_PRESENT);
	}
}

void BootstrapPaging::addMapping(phys_addr p, virt_addr v, uint32_t flags){
	uint32_t directoryIndex = ((uint32_t)v >> 22) & 0x3ff;
	uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;

	assert((((uint32_t)directory) & 0xfff) == 0, "Error: misaligned directory");
	
	if(!isPresent(directory[directoryIndex])){
		void* pageTable = kalloc_permanent(4096, 4096); //FIXME This is a TERRIBLE way of allocating memory for page tables. We should just allocate a page and stash it in there.
		assert(((uint32_t)pageTable & 0xfff) == 0, "Error: misaligned page table");
		uint32_t paddr = (uint32_t)virtualToPhysical((virt_addr)pageTable);
		uint32_t directoryEntry = (paddr & (~0xfff)) | flags; //FIXME is this how we want to be setting the table flags? Is there a better interface?
		directory[directoryIndex] = directoryEntry;
	}
	uint32_t* table = physicalToPageTableAddr((phys_addr)(directory[directoryIndex] & (~0xfff)));
	uint32_t tableEntry = (((uint32_t)p) & (~0xfff)) | flags;
	table[tableIndex] = tableEntry;
}

void BootstrapPaging::install(){
	uint32_t ptr = (uint32_t)directory - 0xC0000000;
	__asm__ volatile("movl %0, %%cr3\n" :: "r"(ptr));
}

#include <paging.h>
#include <stddef.h>
#include <mem.h>

uint32_t* physicalToPageTableAddr(phys_addr v);

PageDirectory* active_page_dir;

//FIXME Right now we are allocating all page directories and tables in the kernel bump heap. Obviously this is a terrible idea. We need to fix this once we write the page allocator. Moreover, we need a way of converting from physical addresses back to virtual ones when looking up page tables. A simple way of doing so is to just make sure all page tables are stored in some contiguous chunk of memory and then subtracting a fixed offset. Hopefully this will be sufficient.

PageDirectory::PageDirectory(){
	directory = (uint32_t*)kalloc_permanent(4096, 4096);
	memset(directory, 0, 4096);
}

PageDirectory::~PageDirectory(){}

bool isPresent(uint32_t entry){
	if((entry & PAGE_PRESENT) == 0){
		assert(entry == 0, "Error: page entry exists, but is not present. We don't have swap implemented yet, so what gives?");
		return false;
	}
	return true;
}

void PageDirectory::addMapping(phys_addr p, virt_addr v, uint32_t flags){
	uint32_t directoryIndex = ((uint32_t)v >> 22) & 0x3ff;
	uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;

	assert((((uint32_t)directory) & 0xfff) == 0, "Error: misaligned directory");
	
	if(!isPresent(directory[directoryIndex])){
		void* pageTable = kalloc_permanent(4096, 4096); //FIXME This is a TERRIBLE way of allocating memory for page tables. We should just allocate a page and stash it in there.
		assert(((uint32_t)pageTable & 0xfff) == 0, "Error: misaligned page table");
		uint32_t paddr = (uint32_t)getPhysicalAddr((virt_addr)pageTable);
		uint32_t directoryEntry = (paddr & (~0xfff)) | flags; //FIXME is this how we want to be setting the table flags? Is there a better interface?
		directory[directoryIndex] = directoryEntry;
	}
	uint32_t* table = physicalToPageTableAddr((phys_addr)(directory[directoryIndex] & (~0xfff)));
	uint32_t tableEntry = (((uint32_t)p) & (~0xfff)) | flags;
	table[tableIndex] = tableEntry;
}

void PageDirectory::removeMapping(virt_addr v){
	uint32_t directoryIndex = ((uint32_t)v >> 22);
	uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;
	
	assert(isPresent(directory[directoryIndex]), "Error: tried to unmap already unmapped address");
	uint32_t* table = physicalToPageTableAddr((phys_addr)(directory[directoryIndex] & (~0xfff)));
	assert(isPresent(table[tableIndex]), "Error: tried to unmap already unmapped address");
	table[tableIndex] = 0;
}

//Will return null if the virtual address is unmapped
phys_addr PageDirectory::findPhysicalAddr(virt_addr v){
    uint32_t directoryIndex = ((uint32_t)v >> 22);
    uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;

    if(!isPresent(directory[directoryIndex])){
        return NULL;
    }
    uint32_t* table = physicalToPageTableAddr((phys_addr)(directory[directoryIndex] & (~0xfff)));
    uint32_t entry = table[tableIndex];
	if(!isPresent(entry)){
		return NULL;
	}
	return (phys_addr)(entry & (~0xfff));
}

void PageDirectory::install(){
	active_page_dir = this;
	uint32_t ptr = (uint32_t)findPhysicalAddr((virt_addr)directory);
	__asm__ volatile("movl %0, %%cr3\n" :: "r"(ptr));
}

phys_addr getPhysicalAddr(virt_addr v){
	if(active_page_dir == NULL){
		return (phys_addr)((uint32_t)v - 0xC0000000); //Hacky way of translating kernel virtual addresses to physical ones before we have a page directory built
	}
	return active_page_dir -> findPhysicalAddr(v);
}

//FIXME this is a little tricky. I think we need to store all the page tables in a predefined place in physical memory
uint32_t* physicalToPageTableAddr(phys_addr p){
	return (uint32_t*)((uint32_t)p + 0xC0000000);
}

bool PageDirectory::isActive(){
	return this == active_page_dir;
}

void initializeKernelPaging(){
	active_page_dir = NULL;
	PageDirectory* pd = new PageDirectory();	

	for(int i = 0x00001000; i < 0x01000000; i += 4096){
		pd -> addMapping((phys_addr)i, (virt_addr)(i + 0xC0000000), PAGE_ENABLE_WRITE | PAGE_PRESENT);
	}

	pd -> install();
}

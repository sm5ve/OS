#include <paging.h>
#include <stddef.h>
#include <mem.h>
#include <klib/SerialDevice.h>

using namespace MemoryManager;
namespace MemoryManager{
	uint32_t* physicalToPageTableAddr(phys_addr v);
	PageDirectory* active_page_dir;
}

PageDirectory::PageDirectory(){
	directory = (uint32_t*)allocatePageTable();
	assert(((uint32_t) directory & 0xfff) == 0, "Error: misaligned page directory");
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
		void* pageTable = allocatePageTable();
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
	SD::the() << "installing page directory at paddr " << (void*)ptr << "\n";
	SD::the() << "vaddr " << (virt_addr)directory << "\n";
	__asm__ volatile("movl %0, %%cr3\n" :: "r"(ptr));
}

bool PageDirectory::isActive(){
	return this == active_page_dir;
}

void PageDirectory::installRegion(MemoryRegion& region, virt_addr starting_addr){
	regions.add({region, starting_addr});
	region.install(*this, starting_addr);
}

void PageDirectory::removeRegion(MemoryRegion& region){
	auto node = regions.head();
	while(node != regions.end()){
		auto oldNode = node;
		node = node -> next();
		if(&*(oldNode -> value.region) == &region){
			region.remove(*this, oldNode -> value.base);
			regions.remove(oldNode);
			return;
		}
	}
	assert(false, "Error: tried to remove region not present in page directory");
}

virt_addr PageDirectory::getRegionBase(MemoryRegion& region){
	auto node = regions.head();
	while(node != regions.end()){
		if(&*(node -> value.region) == &region)
			return node -> value.base;
		node = node -> next();
	}
	assert(false, "Error: tried to find base of region not present in page directory");
	return NULL;
}

void PageDirectory::addPageTable(page_table* ptr, virt_addr base, uint32_t flags){
	uint32_t directory_index = ((uint32_t)base >> 22);
	assert(((uint32_t)base & 0x3fffff) == 0, "Error: misaligned base address");
	uint32_t paddr = (uint32_t)getPhysicalAddr((virt_addr)ptr);
	assert(paddr % PAGE_SIZE == 0, "Error: misaligned page table");
	uint32_t entry = (paddr & (~0xfff)) | flags;
	directory[directory_index] = entry;
}

virt_addr PageDirectory::findSpaceBelow(size_t size, virt_addr addr){
	assert(false, "Unimplemented");
	return NULL;
}

virt_addr PageDirectory::findSpaceAbove(size_t size, virt_addr addr){
	assert(false, "Unimplemented");
	return NULL;
}

#include <paging.h>
#include <klib/SerialDevice.h>

using namespace MemoryManager;

MemoryRegion::MemoryRegion(){}

MemoryRegion::~MemoryRegion(){
	SD::the() << "killing region\n";
}

PhysicalMemoryRegion::PhysicalMemoryRegion(Vector<page_table*> pt, size_t s, bool perm, uint32_t flgs) : ptables(pt), size(s), permanent(perm), flags(flgs){
	assert(size % PAGE_SIZE == 0, "Error: misaligned memory region size");
}

PhysicalMemoryRegion::~PhysicalMemoryRegion(){
	
}

void PhysicalMemoryRegion::install(PageDirectory& dir, virt_addr base){
	size_t allocd = 0;
	assert(((uint32_t)base & 0x3fffff) == 0, "Error: misaligned starting address");
	virt_addr at = base;
	for(uint32_t i = 0; i < ptables.size(); i++){
		dir.addPageTable(ptables[i], at, flags); 
		at = (virt_addr)((uint32_t)at + PAGE_SIZE * 1024);
	}
}

void PhysicalMemoryRegion::remove(PageDirectory& dir, virt_addr base){
	assert(false, "Unimplemented");
}

void PhysicalMemoryRegion::handlePageFault(uint32_t offset){
	assert(false, "Unimplemented");
}

PrintStream& operator<<(PrintStream& p, PhysicalMemoryRegion& mr){
	p << "PhysicalMemoryRegion (size " << (void*)mr.size << ")\n";
	for(int i = 0; i < mr.ptables.size(); i++){
		p << mr.ptables[i] << "\n";
	}
	return p;
}

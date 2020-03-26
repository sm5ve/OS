#include <paging.h>
#include <klib/SerialDevice.h>

using namespace MemoryManager;

MemoryRegion::MemoryRegion(){}

MemoryRegion::~MemoryRegion(){
	SD::the() << "killing region\n";
}

PhysicalMemoryRegion::PhysicalMemoryRegion(Vector<page_table*> pt, size_t s, uint32_t fe, bool perm, uint32_t flgs) : ptables(pt), size(s), offset(fe), permanent(perm), flags(flgs){
	assert(size % PAGE_SIZE == 0, "Error: misaligned memory region size");
	assert(offset % PAGE_SIZE == 0, "Error: misaligned offset");
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
	SD::the() << "Installed region at base " << base << "\n";
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

CompositeMemoryRegion::CompositeMemoryRegion(uint32_t f) : flags(f) {

}

CompositeMemoryRegion::~CompositeMemoryRegion(){

}

void CompositeMemoryRegion::install(PageDirectory& dir, virt_addr){
	for(uint32_t i = 0; i < ptables.size(); i++){
		auto table = ptables[i];
		dir.addPageTable(table.a, table.b, flags);
	}
	SD::the() << "Installing\n";
}

void CompositeMemoryRegion::remove(PageDirectory&, virt_addr){
	assert(false, "Unimplemented");
}

void CompositeMemoryRegion::handlePageFault(uint32_t){
	assert(false, "Unimplemented");
}

void CompositeMemoryRegion::addRegion(PhysicalMemoryRegion& region, virt_addr base){
	assert((uint32_t)base % (1024 * PAGE_SIZE) == 0, "Error: misaligned base");
	for(uint32_t i = 0; i < region.ptables.size(); i++){
		uint32_t addr = (uint32_t)base + i * PAGE_SIZE * 1024;
		if(!tbl_map.contains(addr)){
			page_table* ptbl = allocatePageTable();
			page_table* old_tbl = region.ptables[i];
			memcpy(ptbl, old_tbl, sizeof(page_table));
			ptables.push(Tuple<page_table*, virt_addr>(ptbl, (virt_addr)addr));
			tbl_map.put(addr, (uint32_t)ptbl);
		}
		else{
			page_table& ptbl = *(page_table*)tbl_map.get(addr);
			page_table& old_tbl = *region.ptables[i];
			for(uint32_t j = 0; j < 1024; j++){
				if((old_tbl[j] & PAGE_PRESENT) != 0){
					ptbl[j] = old_tbl[j];
				}
			}
		}
	}
}

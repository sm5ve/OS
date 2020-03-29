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

//TODO when we add a region, we should somehow free its old page tables freeing the
//pages themselves
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

void CompositeMemoryRegion::setFlags(virt_addr addr, uint32_t flags){
	assert((uint32_t)addr % PAGE_SIZE == 0, "Error: misaligned address");
	uint32_t base = (uint32_t)addr - ((uint32_t)addr % (1024 * PAGE_SIZE));
	assert(tbl_map.contains(base), "Error: tried to set flags of non-existent entry");
	page_table& ptbl = *(page_table*)tbl_map.get(base);
	uint32_t i = ((uint32_t)addr / PAGE_SIZE) % 1024;
	ptbl[i] = (ptbl[i] & (~0xfff)) | flags;
}

PhysicalRangeRegion::PhysicalRangeRegion(phys_addr b, size_t length, uint32_t flgs){
	assert((uint32_t)b % PAGE_SIZE == 0, "Error, misaligned physical address");
	size = length;
	base = b;
	flags = flgs;
	uint32_t num_ptables = length / PAGE_SIZE;
	if(num_ptables % 1024 != 0){
		num_ptables += 1024 - (num_ptables % 1024);
	}
	num_ptables /= 1024;
	for(uint32_t i = 0; i < num_ptables; i++){
		ptables.push(allocatePageTable());
	}
	for(uint32_t addr = (uint32_t)b; addr < (uint32_t)base + length; addr += PAGE_SIZE){
		uint32_t page_index = (addr - (uint32_t)b) / PAGE_SIZE;
		uint32_t table_index = page_index / 1024;
		
		(*(ptables[table_index]))[page_index % 1024] = (addr & (~0xfff)) | flags;
	}
}

PhysicalRangeRegion::~PhysicalRangeRegion(){
	//TODO implement
}

void PhysicalRangeRegion::install(PageDirectory& pd, virt_addr base){
	assert((uint32_t)base % (1024 * PAGE_SIZE) == 0, "Error: misaligned base address");
	for(uint32_t i = 0; i < ptables.size(); i++){
		pd.addPageTable(ptables[i], (virt_addr)((uint32_t)base + i * 1024 * PAGE_SIZE), flags);
	}
}

void PhysicalRangeRegion::remove(PageDirectory& pd, virt_addr base){
	assert(false, "Unimplemented");
}

void PhysicalRangeRegion::handlePageFault(uint32_t offset){
	assert(false, "Unimplemented");
}

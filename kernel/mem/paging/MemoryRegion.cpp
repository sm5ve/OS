#include <paging.h>

using namespace MemoryManager;

MemoryRegion::MemoryRegion(){}

MemoryRegion::~MemoryRegion(){}

PhysicalMemoryRegion::PhysicalMemoryRegion(Vector<page_table*> pt, size_t s) : ptables(pt), size(s){

}

PhysicalMemoryRegion::~PhysicalMemoryRegion(){

}

void PhysicalMemoryRegion::install(PageDirectory& dir){
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

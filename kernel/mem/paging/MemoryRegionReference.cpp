#include <paging.h>

using namespace MemoryManager;

MemoryRegionReference::MemoryRegionReference(){
	region = NULL;
}

MemoryRegionReference::MemoryRegionReference(MemoryRegion& reg){
	region = &reg;
	region -> incRef();
}

MemoryRegionReference::~MemoryRegionReference(){
	if(region)
		region -> decRef();
}

MemoryRegion& MemoryRegionReference::operator*(){
	return *region;
}

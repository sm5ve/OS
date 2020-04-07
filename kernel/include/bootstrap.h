#ifndef BOOTSTRAP
#define BOOTSTRAP

#include <paging.h>
#include <ds/Intervals.h>
#include <ds/Vector.h>
#include <ds/smart_pointers.h>

class BootstrapPaging{
public:
	BootstrapPaging();
	~BootstrapPaging();
	
	void mapRange(Interval<phys_addr>, virt_addr);
	void* mapRangeAfter(Interval<phys_addr>, virt_addr);
	void addMapping(phys_addr p, virt_addr v, uint32_t flags);
	void install();
	Vector<shared_ptr<MemoryManager::PhysicalMemoryRegion>> getRegions();	
private:
	static uint32_t* physicalToPageTableAddr(phys_addr addr);
	static phys_addr virtualToPhysical(virt_addr addr);
	static bool isPresent(uint32_t entry);
	bool isMapped(virt_addr);

	uint32_t* directory;
};

#endif

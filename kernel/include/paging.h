#ifndef PAGING
#define PAGING

#include <multiboot/multiboot.h>
#include <ds/Intervals.h>
#include <ds/Hashmap.h>

typedef void* phys_addr;
typedef void* virt_addr;

#define PAGE_NO_CACHE         (1 << 4)
#define PAGE_USER_ACCESSIBLE  (1 << 2)
#define PAGE_ENABLE_WRITE     (1 << 1)
#define PAGE_PRESENT          (1 << 0)

#define TABLE_GLOBAL          (1 << 8)
#define TABLE_NO_CACHE        (1 << 4)
#define TABLE_USER_ACCESSIBLE (1 << 2)
#define TABLE_ENABLE_WRITE    (1 << 1)
#define TABLE_PRESENT         (1 << 0) 

class PageDirectory{
public:
	PageDirectory();
	//TODO we probably want a copy constructor that just copies the upper gigabyte of kernel page directory entries
	~PageDirectory();

	void addMapping(phys_addr p, virt_addr v, uint32_t flags);
	void removeMapping(virt_addr v);
	phys_addr findPhysicalAddr(virt_addr v);

	void install();
	bool isActive();
//private:
	uint32_t* directory;
};

extern PageDirectory* active_page_dir;

void initializeKernelPaging();
phys_addr getPhysicalAddr(virt_addr v);

class PageFrameAllocator{
public:
	PageFrameAllocator();
	PageFrameAllocator(uint32_t start, uint32_t end);
	
	phys_addr alloc();
	void free(phys_addr);

	bool hasFree();
	phys_addr tryAllocateContiguousRegion(size_t size);
};

class Palloc{
public:
	Palloc(mboot_mmap_entry* entries, uint32_t entries_length);
	~Palloc();

	phys_addr alloc();
	void free(phys_addr);

	static Palloc the();

private:
	IntervalSet<uint32_t> memory_regions;
	HashMap<Interval<uint32_t>, PageFrameAllocator> allocators;
};



#endif

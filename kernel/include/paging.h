#ifndef PAGING
#define PAGING

#include <multiboot/multiboot.h>
#include <ds/Intervals.h>
#include <ds/Vector.h>
typedef void* phys_addr;
typedef void* virt_addr;

#define PAGE_TABLE_INDICES 1024
#define PAGE_DIR_INDICES 1024

#define PAGE_NO_CACHE         (1 << 4)
#define PAGE_USER_ACCESSIBLE  (1 << 2)
#define PAGE_ENABLE_WRITE     (1 << 1)
#define PAGE_PRESENT          (1 << 0)

#define TABLE_GLOBAL          (1 << 8)
#define TABLE_NO_CACHE        (1 << 4)
#define TABLE_USER_ACCESSIBLE (1 << 2)
#define TABLE_ENABLE_WRITE    (1 << 1)
#define TABLE_PRESENT         (1 << 0) 

typedef uint32_t page_table[PAGE_TABLE_INDICES];

class PageDirectory;

namespace MemoryManager{
	extern PageDirectory* active_page_dir;

	void init(mboot_mmap_entry*, uint32_t len);
	
	void initializeKernelPaging();
	phys_addr getPhysicalAddr(virt_addr v);

	//page_table_entry 

	class MemoryRegion;
	

	class MemoryRegionReference{
	public:
		MemoryRegionReference();
		MemoryRegionReference(MemoryRegion& reg);
		~MemoryRegionReference();

		MemoryRegion& operator*();
	private:
		MemoryRegion* region;
	};

	class MemoryRegion{
	public:
		virtual void install(PageDirectory&) = 0;
		virtual size_t getSize() = 0;
		virtual void handlePageFault(uint32_t offset) = 0;
	private:
		uint32_t refs;
	private:
		friend class MemoryRegionReference;
		void incRef(){
			refs++;
		};
		void decRef(){
			refs--;
			if(refs == 0){
				delete this;
			}
		};
	};

	class PhysicalMemoryRegion : public MemoryRegion{
	public:
		PhysicalMemoryRegion(Vector<page_table*>* ptables, size_t size);
		virtual void install(PageDirectory&) final override;
		virtual size_t getSize() final override{
			return size;
		};
		virtual void handlePageFault(uint32_t offset) final override;
		void grow(Vector<page_table*>& newTables, size_t newSize);
	private:
		Vector<page_table*>* ptables;
		uint32_t size;
	};

	class PageFrameAllocator{
	public:
		PageFrameAllocator(size_t size, uint32_t* buffer, phys_addr start_addr);
		PageFrameAllocator();
		uint32_t grow(PhysicalMemoryRegion&, size_t targetSize);
		void release(phys_addr addr);
	};
}

struct MemoryRegionPlacement{
	MemoryManager::MemoryRegionReference region;
	virt_addr base;
};

class PageDirectory{
public:
	PageDirectory();
	//TODO we probably want a copy constructor that just copies the upper gigabyte of kernel page directory entries
	~PageDirectory();

	void addMapping(phys_addr p, virt_addr v, uint32_t flags);
	void removeMapping(virt_addr v);
	phys_addr findPhysicalAddr(virt_addr v);

	void installRegion(MemoryManager::MemoryRegion& region, virt_addr starting_addr);
	void removeRegion(MemoryManager::MemoryRegion&);

	void install();
	bool isActive();

	uint32_t getRegionOffset(MemoryManager::MemoryRegion&);
private:
	Vector<MemoryRegionPlacement> regions;
	uint32_t* directory;
};


#endif

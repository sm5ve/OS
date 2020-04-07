#ifndef PAGING
#define PAGING

#include <multiboot/multiboot.h>
#include <ds/Intervals.h>
#include <ds/Vector.h>
#include <ds/LinkedList.h>
#include <ds/HashMap.h>
#include <PrintStream.h>
#include <arch/i386/smp.h>
#include <ds/smart_pointers.h>

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
	class PhysicalMemoryRegion;
}

enum TLBInvalidationType{
	NONE,
	INVLPG,
	FULL_FLUSH
};

PrintStream& operator<<(PrintStream&, MemoryManager::PhysicalMemoryRegion&);

namespace MemoryManager{
	extern PageDirectory* active_page_dir;
	extern PageDirectory* kernel_directory;

	void init(mboot_mmap_entry*, uint32_t len);
	
	void initializeKernelPaging();
	phys_addr getPhysicalAddr(virt_addr v);
	
	page_table* allocatePageTable();
	void freePageTable(page_table*);

	//TODO we need some way to update the page directories when we modify a memory region
	class MemoryRegion{
	public:
		MemoryRegion();
		virtual ~MemoryRegion();
		virtual void install(PageDirectory&, virt_addr) = 0;
		virtual void remove(PageDirectory&, virt_addr) = 0;
		virtual size_t getSize() = 0;
		virtual void handlePageFault(uint32_t offset) = 0;
	private:
		uint32_t refs;
	};

	class PageFrameAllocator;
	class CompositeMemoryRegion;

	class PhysicalMemoryRegion : public MemoryRegion{
	public:
		PhysicalMemoryRegion(Vector<page_table*> ptables, size_t size, uint32_t first_entry = 0, bool perm = false, TLBInvalidationType invalidation_type = TLBInvalidationType::INVLPG, uint32_t flags = PAGE_ENABLE_WRITE | PAGE_PRESENT);
		~PhysicalMemoryRegion();
		virtual void install(PageDirectory&, virt_addr) final override;
		virtual void remove(PageDirectory&, virt_addr) final override;
		virtual size_t getSize() final override{
			return size;
		};
		virtual void handlePageFault(uint32_t offset) final override; //FIXME we probably shouldn't be returning void. Should we pass the relevant directory, or just get that from the global?
		uint32_t mapPage(phys_addr);
		uint32_t mapContiguousRegion(phys_addr, size_t);
	private:
		Vector<page_table*> ptables;
		uint32_t size;
		bool permanent;
		TLBInvalidationType inv_type;
		uint32_t flags;
		uint32_t offset;
		friend class PageFrameAllocator;
		friend class CompositeMemoryRegion;
		friend PrintStream& (::operator<<)(PrintStream&, PhysicalMemoryRegion&); //TODO we need to make this virtual somehow, otherwise once we lose the typing data by wrapping with a referece, we won't be able to use this operator
	};

	class CompositeMemoryRegion : public MemoryRegion{
	public:
		CompositeMemoryRegion(uint32_t flags = PAGE_ENABLE_WRITE | PAGE_PRESENT);
		~CompositeMemoryRegion();
		virtual void install(PageDirectory&, virt_addr) final override;
		virtual void remove(PageDirectory&, virt_addr) final override;
		virtual size_t getSize() final override{
			return 0; //TODO I'm not quite sure what to return here, or if this is even a good method to have at all
		}
		virtual void handlePageFault(uint32_t offset) final override;

		void addRegion(PhysicalMemoryRegion& region, virt_addr base);
		void setFlags(virt_addr addr, uint32_t flags);
	private:
		Vector<Tuple<page_table*, virt_addr>> ptables; //TODO get a way of enumerating key-value pairs in a hashmap
		uint32_t flags;
		HashMap<uint32_t, uint32_t> tbl_map;	
	};

	class PageFrameAllocator{
	public:
		PageFrameAllocator(size_t size, phys_addr* ptr_buffer, uint32_t* free_buffer, phys_addr start_addr);
		PageFrameAllocator();
		size_t grow(PhysicalMemoryRegion&, size_t);
		void release(phys_addr);
		phys_addr allocateContiguousRange(PhysicalMemoryRegion&, size_t);
	private:
		uint32_t free_index;
		phys_addr* ptr_buff;
		uint32_t* free_buff;
		size_t sz;
		phys_addr base;
		phys_addr alloc();
	};

	void growPhysicalMemoryRegion(PhysicalMemoryRegion&, size_t);
	phys_addr allocateContiguousRange(PhysicalMemoryRegion&, size_t);
	void shrinkPhysicalMemoryRegion(PhysicalMemoryRegion&, size_t);
	void release(phys_addr);
}

struct MemoryRegionPlacement{
	shared_ptr<MemoryManager::MemoryRegion> region;
	virt_addr base;
};

class PageDirectory{
public:
	PageDirectory();
	//TODO we probably want a copy constructor that just copies the upper gigabyte of kernel page directory entries
	~PageDirectory();

	void addMapping(phys_addr, virt_addr, uint32_t flags);
	void removeMapping(virt_addr);
	phys_addr findPhysicalAddr(virt_addr);
	virt_addr findVirtAddr(phys_addr);
	void addPageTable(page_table*, virt_addr, uint32_t flags, TLBInvalidationType invtype = TLBInvalidationType::INVLPG);
	void removePageTables(virt_addr, size_t, TLBInvalidationType invtype = TLBInvalidationType::INVLPG);

	void installRegion(shared_ptr<MemoryManager::MemoryRegion> region, virt_addr starting_addr);
	void removeRegion(shared_ptr<MemoryManager::MemoryRegion>);
	void copyRegionsInto(PageDirectory&);
	virt_addr getRegionBase(MemoryManager::MemoryRegion&);

	bool isMapped(virt_addr);
	virt_addr findSpaceBelow(size_t, virt_addr, bool align_at_page_table = true);
	virt_addr findSpaceAbove(size_t, virt_addr, bool align_at_page_table = true); 

	void install();
	bool isActive();

	uint32_t getRegionOffset(shared_ptr<MemoryManager::MemoryRegion>);
	void invalidateMappingIfNecessary(virt_addr, TLBInvalidationType);
private:
	LinkedList<MemoryRegionPlacement> regions;
	uint32_t* directory;
	friend void SMP::setupAPTables(uint16_t);
};

PrintStream& operator<<(PrintStream&, MemoryManager::PhysicalMemoryRegion&);

#endif

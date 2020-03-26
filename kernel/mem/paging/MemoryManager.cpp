#include <paging.h>
#include <mem.h>
#include <multiboot/multiboot.h>
#include <klib/SerialDevice.h>
#include <ds/HashMap.h>
#include <stddef.h>
#include <bootstrap.h>

#define PAGE_SIZE 4096

namespace MemoryManager{
	IntervalSet<uint32_t>* memory_regions;
	SlabAlloc* pageTableAllocator;
	HashMap<Interval<uint32_t>, PageFrameAllocator*>* allocators;
	phys_addr page_tables_buffer_addr;	
	virt_addr page_tables_buffer_base;

	Tuple<virt_addr, phys_addr> reserveRangeOfSize(size_t size, IntervalSet<uint32_t>* memory_regions, BootstrapPaging* bspd){
		auto region = memory_regions -> findSubintervalOfSize(size + PAGE_SIZE);
		assert(region.has_value(), "Error: memory too fragmented");
		memory_regions -> subtract(region.value());\
		uint32_t start = region.value().getStart();
		if(start % PAGE_SIZE != 0)
			start += PAGE_SIZE - (start % PAGE_SIZE);
		uint32_t end = start + size;
		return Tuple<virt_addr, phys_addr>(bspd -> mapRangeAfter(Interval<phys_addr>((phys_addr)start, (phys_addr)end), (virt_addr) 0xc0000000), (phys_addr)start);
	}

	PageFrameAllocator* makeAllocatorForRange(Interval<uint32_t>& range, BootstrapPaging& pd){
		uint32_t aligned_start = range.getStart();
		if(aligned_start % PAGE_SIZE != 0){
			aligned_start += PAGE_SIZE - (aligned_start % PAGE_SIZE);
		}
		uint32_t aligned_end = range.getEnd();
		aligned_end -= (aligned_end % PAGE_SIZE);

		range = Interval<uint32_t>(aligned_start, aligned_end - 1);

		size_t size = range.getSize();
		size_t buffer_size = (size / PAGE_SIZE) * sizeof(phys_addr);
		//These size calculations are inaccurate and over-allocate. I am fine with this.
		if(buffer_size % PAGE_SIZE != 0){
			buffer_size += PAGE_SIZE - (buffer_size % PAGE_SIZE);
		}
		size_t free_buff_size = ((size - buffer_size) / PAGE_SIZE) * sizeof(uint32_t);
		if(free_buff_size % PAGE_SIZE != 0){
			free_buff_size += PAGE_SIZE - (free_buff_size % PAGE_SIZE);
		}
		range = Interval<uint32_t>(aligned_start + buffer_size + free_buff_size, aligned_end - 1);
		
		auto ptr_buff_interval = Interval<phys_addr>((phys_addr)aligned_start, (phys_addr)(aligned_start + buffer_size - 1));
		auto free_buff_interval = Interval<phys_addr>((phys_addr)(aligned_start + buffer_size), (phys_addr)(aligned_start + buffer_size + free_buff_size - 1));

		void* ptr_buff = pd.mapRangeAfter(ptr_buff_interval, (virt_addr)0xc0000000);
		void* free_buff = pd.mapRangeAfter(free_buff_interval, (virt_addr)0xc0000000);

		return new PageFrameAllocator(size - (buffer_size + free_buff_size), (phys_addr*)ptr_buff, (uint32_t*)free_buff, (phys_addr)(aligned_start + buffer_size + free_buff_size));
	}

	void init(mboot_mmap_entry* entries, uint32_t len){
		//Initialize bootstrap page tables
		BootstrapPaging* bspd = new BootstrapPaging();
		bspd -> mapRange(Interval<phys_addr>((phys_addr)0, (phys_addr)0x01000000 - 1), (virt_addr)0xc0000000);
		bspd -> install();
	
		memory_regions = new IntervalSet<uint32_t>();
		allocators = new HashMap<Interval<uint32_t>, PageFrameAllocator*>();
		auto ent = entries;
		while((uint32_t)ent < (uint32_t)entries + len){
			if(ent -> type == MEM_AVAILABLE){
				memory_regions -> add(Interval<uint32_t>(ent -> addr, ent -> addr + ent -> len - 1));
			}
			ent = (mboot_mmap_entry*)((uint32_t)ent + (ent -> size) + sizeof(void*));
		}

		memory_regions -> subtract(Interval<uint32_t>(0x00000000, 0x01000000 - 1));
		//Perhaps we want to be a bit smarter about this in the future? 8MB of page tables
		//Is certainly a safe size to allocate for 32 bit systems, but we might not always
		//Need it
		auto table_store_mem = reserveRangeOfSize(8*MB, memory_regions, bspd);
		auto free_ptbls_map_mem = reserveRangeOfSize(8 * MB / (sizeof(page_table) * 8), memory_regions, bspd);

		auto table_store = table_store_mem.a;
		page_tables_buffer_addr = table_store_mem.b;
		page_tables_buffer_base = table_store_mem.a;
		auto free_ptbls_map = (uint8_t*)free_ptbls_map_mem.a;
		
		pageTableAllocator = new SlabAlloc(table_store, 8 * MB, sizeof(page_table), free_ptbls_map);

		auto range = memory_regions -> getIntervals() -> head();
		while(range != memory_regions -> getIntervals() -> end()){
			allocators -> put(range -> value, makeAllocatorForRange(range -> value, *bspd));
			range = range -> next();
		}

		PageDirectory* pd = new PageDirectory();
		auto bootstrap_regions = bspd -> getRegions();
		
		for(uint32_t i = 0; i < bootstrap_regions.size(); i++){
			pd -> installRegion(*bootstrap_regions[i], (virt_addr)0xc0000000);
		}

		pd -> install();
		/*
		PhysicalMemoryRegion* test = new PhysicalMemoryRegion(Vector<page_table*>(), 0, false);
		growPhysicalMemoryRegion(*test, 3 * MB);
		pd -> installRegion(*test, (virt_addr)0x20000000);
		*/
	}

	page_table* allocatePageTable(){
		return (page_table*)(pageTableAllocator -> alloc());
	}

	void freePageTable(page_table* ptbl){
		pageTableAllocator -> free(ptbl);
	}
	
	PageFrameAllocator::PageFrameAllocator(){}

	phys_addr getPhysicalAddr(virt_addr v){
		if(active_page_dir == NULL){
			return (phys_addr)((uint32_t)v - 0xc0000000);
		}
		return active_page_dir -> findPhysicalAddr(v);
	}

	uint32_t* physicalToPageTableAddr(phys_addr p){
		return (uint32_t*)((uint32_t)p - (uint32_t)page_tables_buffer_addr + (uint32_t)page_tables_buffer_base);
	}

	PageFrameAllocator::PageFrameAllocator(size_t size, phys_addr* ptr_buffer, uint32_t* free_buffer, phys_addr start){
		assert(size % PAGE_SIZE == 0, "Error: size of range not multiple of page size");
		assert((uint32_t)start % PAGE_SIZE == 0, "Error: misaligned start of allocation range");
		uint32_t page_num = size / PAGE_SIZE;
		for(int i = 0; i < page_num; i++){
			ptr_buffer[i] = (phys_addr)((uint32_t)start + i * PAGE_SIZE);
			free_buffer[i] = i;
		}
		free_index = 0;
		ptr_buff = ptr_buffer;
		free_buff = free_buffer;
		sz = size;
		base = start;
	}
	
	phys_addr PageFrameAllocator::alloc(){
		assert(free_index * PAGE_SIZE < sz, "error: out of memory");
		return ptr_buff[free_index++];
	}

	void PageFrameAllocator::release(phys_addr addr){
		assert(((uint32_t)addr >= (uint32_t)base) && ((uint32_t)addr < (uint32_t)base + sz), "Error: tried to free page out of range from allocator");
		assert((uint32_t)addr % PAGE_SIZE == 0, "Error: tried to free misaligned page");
		free_index--;
		uint32_t abs_index = ((uint32_t)addr - (uint32_t)base) / PAGE_SIZE;
		uint32_t to_free_index = free_buff[abs_index];
		auto addr_swap = ptr_buff[to_free_index];
		ptr_buff[to_free_index] = ptr_buff[free_index];
		ptr_buff[free_index] = addr_swap;
		
		free_buff[abs_index] = free_buff[free_index];
		free_buff[free_index] = to_free_index;
	}
	
	size_t PageFrameAllocator::grow(PhysicalMemoryRegion& region, size_t target_size){
		size_t to_alloc = target_size;
		if(to_alloc > (sz - free_index * PAGE_SIZE)){
			to_alloc = sz - free_index * PAGE_SIZE;
		}
		size_t num_pages = to_alloc / PAGE_SIZE;
		for(uint32_t i = 0; i < num_pages; i++){
			uint32_t ptbl_index = (i + region.size / PAGE_SIZE + region.offset / PAGE_SIZE) % 1024;
			if((ptbl_index == 0) || (region.ptables.size() == 0)){
				region.ptables.push(allocatePageTable());
			}
			page_table& ptbl = *region.ptables.top();
			phys_addr next_page = alloc();
			ptbl[ptbl_index] = ((uint32_t)next_page & (~0xfff)) | region.flags;
		}
		free_index += num_pages;
		region.size += to_alloc;
		return target_size - to_alloc;
	}

	void growPhysicalMemoryRegion(PhysicalMemoryRegion& reg, size_t target_size){
		auto range = memory_regions -> getIntervals() -> head();
		while(range != memory_regions -> getIntervals() -> end()){
			target_size = allocators -> get(range -> value) -> grow(reg, target_size);
			if(target_size == 0){
				return;
			}
			range = range -> next();
		}
		assert(false, "Error: out of memory");
	}
}

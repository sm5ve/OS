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
	HashMap<Interval<uint32_t>, PageFrameAllocator>* allocators;
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

	void init(mboot_mmap_entry* entries, uint32_t len){
		//Initialize bootstrap page tables
		BootstrapPaging* bspd = new BootstrapPaging();
		bspd -> mapRange(Interval<phys_addr>((phys_addr)0, (phys_addr)0x01000000), (virt_addr)0xc0000000);
		bspd -> install();
	
		memory_regions = new IntervalSet<uint32_t>();
		allocators = new HashMap<Interval<uint32_t>, PageFrameAllocator>();
		auto ent = entries;
		while((uint32_t)ent < (uint32_t)entries + len){
			if(ent -> type == MEM_AVAILABLE){
				memory_regions -> add(Interval<uint32_t>(ent -> addr, ent -> addr + ent -> len - 1));
			}
			ent = (mboot_mmap_entry*)((uint32_t)ent + (ent -> size) + sizeof(void*));
		}

		memory_regions -> subtract(Interval<uint32_t>(0x00000000, 0x01000000 - 1));
		auto table_store_mem = reserveRangeOfSize(8*MB, memory_regions, bspd);
		auto free_ptbls_map_mem = reserveRangeOfSize(8 * MB / (sizeof(page_table) * 8), memory_regions, bspd);

		auto table_store = table_store_mem.a;
		page_tables_buffer_addr = table_store_mem.b;
		page_tables_buffer_base = table_store_mem.a;
		auto free_ptbls_map = (uint8_t*)free_ptbls_map_mem.a;
		
		pageTableAllocator = new SlabAlloc(table_store, 8 * MB, sizeof(page_table), free_ptbls_map);

		PageDirectory* pd = new PageDirectory();
		auto bootstrap_regions = bspd -> getRegions();
		SD::the() << bootstrap_regions.size() << "\n";

		for(uint32_t i = 0; i < bootstrap_regions.size(); i++){
			pd -> installRegion(*bootstrap_regions[i], (virt_addr)0xc0000000);
		}

		pd -> install();

		auto range = memory_regions -> getIntervals() -> head();
		while(range != memory_regions -> getIntervals() -> end()){
			allocators -> put(range -> value, PageFrameAllocator());
			range = range -> next();
		}
		SD::the() << "Physical memory map " << *memory_regions << "\n";
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
}

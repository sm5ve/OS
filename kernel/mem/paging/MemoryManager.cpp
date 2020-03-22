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
	
	void* reserveRangeOfSize(size_t size, IntervalSet<uint32_t>* memory_regions, BootstrapPaging* bspd){
		auto region = memory_regions -> findSubintervalOfSize(size + PAGE_SIZE);
		assert(region.has_value(), "Error: memory too fragmented");
		memory_regions -> subtract(region.value());\
		uint32_t start = region.value().getStart();
		start += PAGE_SIZE - (start % PAGE_SIZE);
		uint32_t end = start + size;
		return bspd -> mapRangeAfter(Interval<phys_addr>((phys_addr)start, (phys_addr)end), (virt_addr) 0xc0000000);
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
		auto table_store = reserveRangeOfSize(8*MB, memory_regions, bspd);
		auto free_ptbls_map = (uint8_t*)reserveRangeOfSize(8 * MB / (sizeof(page_table) * 8), memory_regions, bspd);
		
		pageTableAllocator = new SlabAlloc(table_store, 8 * MB, sizeof(page_table), free_ptbls_map);

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
}

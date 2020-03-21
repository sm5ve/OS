#include <paging.h>
#include <mem.h>
#include <multiboot/multiboot.h>
#include <klib/SerialDevice.h>
#include <ds/HashMap.h>
#include <stddef.h>

#define PAGE_SIZE 4096

namespace MemoryManager{
	IntervalSet<uint32_t>* memory_regions;
	SlabAlloc* pageTableAllocator;
	HashMap<Interval<uint32_t>, PageFrameAllocator>* allocators;
	
	phys_addr table_store_phys;

	phys_addr reserveRangeOfSize(size_t size, IntervalSet<uint32_t>* memory_regions, uint32_t align = 1){
		auto region = memory_regions -> findSubintervalOfSize(size + align - 1);
		assert(region.has_value(), "Error: memory too fragmented");
		memory_regions -> subtract(region.value());
		uint32_t start = (uint32_t)region.value().getStart();
		if(start % align == 0){
			return (phys_addr)start;
		}
		return (phys_addr)(start + (align - (start % align)));
	}

	void init(mboot_mmap_entry* entries, uint32_t len){
		memory_regions = new IntervalSet<uint32_t>();
		allocators = new HashMap<Interval<uint32_t>, PageFrameAllocator>();
		auto ent = entries;
		while((uint32_t)ent < (uint32_t)entries + len){
			if(ent -> type == MEM_AVAILABLE){
				memory_regions -> add(Interval<uint32_t>(ent -> addr, ent -> addr + ent -> len - 1));
			}
			ent = (mboot_mmap_entry*)((uint32_t)ent + (ent -> size) + sizeof(void*));
		}
		//TODO actually be smart and reserve important parts of memory for the kernel in some intelligent manner
		memory_regions -> subtract(Interval<uint32_t>(0x00000000, 0x01000000 - 1));
		table_store_phys = reserveRangeOfSize(8*MB, memory_regions, PAGE_SIZE);
		auto free_ptbls_map_phys = reserveRangeOfSize(8 * MB / sizeof(page_table) / 8, memory_regions);
		//FIXME this is a bit hacky, but assuming that tble_store_phys is in the lower gigabyte, should've already mapped this into virtual memory
		void* table_store = (void*)((uint32_t)table_store_phys + 0xc0000000);
		SD::the() << table_store << "\n";
		uint8_t* free_ptbls_map = (uint8_t*)((uint32_t)free_ptbls_map_phys + 0xc0000000);
//		pageTableAllocator = new SlabAlloc(table_store, 8 * MB, sizeof(page_table), free_ptbls_map);


		auto range = memory_regions -> getIntervals() -> head();
		while(range != memory_regions -> getIntervals() -> end()){
			allocators -> put(range -> value, PageFrameAllocator());
			range = range -> next();
		}
		SD::the() << "Physical memory map " << *memory_regions << "\n";
	}

	
	
	PageFrameAllocator::PageFrameAllocator(){}
}

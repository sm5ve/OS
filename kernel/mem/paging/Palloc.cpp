#include <paging.h>
#include <mem.h>
#include <multiboot/multiboot.h>
#include <klib/SerialDevice.h>
#include <stddef.h>

Palloc::Palloc(mboot_mmap_entry* entries, uint32_t len){
	auto ent = entries;
	while((uint32_t)ent < (uint32_t)entries + len){
		if(ent -> type == MEM_AVAILABLE){
			memory_regions.add(Interval<uint32_t>(ent -> addr, ent -> addr + ent -> len));
		}
		ent = (mboot_mmap_entry*)((uint32_t)ent + (ent -> size) + sizeof(void*));
	}
	//TODO actually be smart and reserve important parts of memory for the kernel in some intelligent manner
	memory_regions.subtract(Interval<uint32_t>(0xc0000000, 0xc1000000));

	auto range = memory_regions.getIntervals() -> head();
	while(range != memory_regions.getIntervals() -> end()){
		allocators.put(range -> value, PageFrameAllocator());
		range = range -> next();
	}
}

phys_addr Palloc::alloc(){

}

void Palloc::free(phys_addr addr){

}

PageFrameAllocator::PageFrameAllocator(){

}

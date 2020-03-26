#include <loader.h>
#include <assert.h>
#include <klib/SerialDevice.h>
#include <ds/Vector.h>

namespace Loader{
	virt_addr load(ELF& elf){
		assert(elf.getHeader32() -> type == ELF_TYPE_EXEC, "Error: tried to load nonexecutable ELF");
		auto composite = new MemoryManager::CompositeMemoryRegion();
		for(uint32_t i = 0; i < elf.getHeader32() -> prog_header_entry_count; i++){
			auto header = elf.getProgramHeader(i);
			if(header -> type == ELF_PT_NULL){
				continue;
			}
			else if(header -> type == ELF_PT_LOAD){
				size_t vsize = header -> segment_size;
				//uint32_t align = 
				if(vsize % PAGE_SIZE != 0){
					vsize += PAGE_SIZE - (vsize % PAGE_SIZE);
				}
				uint32_t seg_start = header -> vaddr;
				seg_start -= seg_start % PAGE_SIZE;

				//TODO implement flags
				//TODO handle alignment
				uint32_t region_offset = (seg_start % (1024 * PAGE_SIZE));
				auto region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, region_offset);
				MemoryManager::growPhysicalMemoryRegion(*region, vsize);
				virt_addr pd_start = (virt_addr)(seg_start - region_offset);
				composite -> addRegion(*region, pd_start);	
			}
			else{
				assert(false, "Unimplemented segment type");
			}
		}
		MemoryManager::active_page_dir -> installRegion(*composite, NULL);	
		for(uint32_t i = 0; i < elf.getHeader32() -> prog_header_entry_count; i++){
			auto header = elf.getProgramHeader(i);
			if(header -> type == ELF_PT_NULL){
				continue;
			}
			else if(header -> type == ELF_PT_LOAD){
				memcpy((void*)(header -> vaddr), elf.dataAtOffset(header -> offset), header -> file_size);
			}
			else{
				assert(false, "Unimplemented segment type");
			}
		}
		uint32_t entry = elf.getHeader32() -> entry_vaddr;
		__asm__("jmp *%0" :: "r"(entry));
	}
}

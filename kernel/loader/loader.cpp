#include <loader.h>
#include <assert.h>
#include <devices/SerialDevice.h>
#include <ds/Vector.h>

namespace Loader{
	Task* load(ELF& elf){
		Task* p = new Task(false);
		assert(elf.getHeader32() -> type == ELF_TYPE_EXEC, "Error: tried to load nonexecutable ELF");
		auto composite = new MemoryManager::CompositeMemoryRegion(PAGE_ENABLE_WRITE | PAGE_USER_ACCESSIBLE | PAGE_PRESENT);
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

				//TODO handle alignment
				uint32_t region_offset = (seg_start % (1024 * PAGE_SIZE));
				auto region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, region_offset, false, PAGE_ENABLE_WRITE | PAGE_PRESENT | PAGE_USER_ACCESSIBLE);
				MemoryManager::growPhysicalMemoryRegion(*region, vsize);
				virt_addr pd_start = (virt_addr)(seg_start - region_offset);
				composite -> addRegion(*region, pd_start);	
			}
			else{
				assert(false, "Unimplemented segment type");
			}
		}
		p -> getPageDirectory().installRegion(*composite, NULL);
		auto oldDir = MemoryManager::active_page_dir;
		p -> getPageDirectory().install();
		for(uint32_t i = 0; i < elf.getHeader32() -> prog_header_entry_count; i++){
			auto header = elf.getProgramHeader(i);
			if(header -> type == ELF_PT_NULL){
				continue;
			}
			else if(header -> type == ELF_PT_LOAD){
				memcpy((void*)(header -> vaddr), elf.dataAtOffset(header -> offset), header -> file_size);
				size_t vsize = header -> segment_size;
				if(vsize % PAGE_SIZE != 0){
					vsize += PAGE_SIZE - (vsize % PAGE_SIZE);
				}
				uint32_t seg_start = header -> vaddr;
				seg_start -= seg_start % PAGE_SIZE;

				uint32_t flags = PAGE_PRESENT | PAGE_USER_ACCESSIBLE;
				if((header -> flags) & ELF_PF_W){
					flags |= PAGE_ENABLE_WRITE;
				}
				for(uint32_t addr = seg_start; addr < seg_start + vsize; addr += PAGE_SIZE){
					composite -> setFlags((virt_addr)addr, flags);
				}
			}
			else{
				assert(false, "Unimplemented segment type");
			}
		}
		oldDir -> install();
		uint32_t entry = elf.getHeader32() -> entry_vaddr;
		p -> setEntrypoint((virt_addr)entry);
		return p;
	}
}

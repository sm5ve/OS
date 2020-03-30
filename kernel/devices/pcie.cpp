#include <devices/pcie.h>
#include <acpi/tables.h>
#include <paging.h>
#include <klib/SerialDevice.h>
//#include <ds/Vector.h>

namespace PCIe{
	bool has_pcie;
	
	void init(){
		auto* header = ACPI::getTable("MCFG");
		if(header == NULL){
			has_pcie = false;
			SD::the() << "No MCFG header found. Will not use PCIe\n";
			return;
		}
		has_pcie = true;
		SD::the() << "Setting up PCIe\n";
		ACPI::MCFGEntry* entries = (ACPI::MCFGEntry*)((uint32_t)header + sizeof(ACPI::SDTHeader) + 8); //the MCFG table has 8 reserved bytes
		uint32_t entry_count = (header -> length - (sizeof(ACPI::SDTHeader) + 8)) / sizeof(ACPI::MCFGEntry);
		Vector<phys_addr> present_devices;
		for(uint32_t i = 0; i < entry_count; i++){
			SD::the() << "entry " << (void*)(entries[i].base_address) << "\n";
			SD::the() << "starting bus " << entries[i].starting_bus << "\n";
			SD::the() << "ending bus " << entries[i].ending_bus << "\n";
			size_t region_size = (entries[i].ending_bus - entries[i].starting_bus + 1) << 20;
			//Map the entire address range for the bus into memory. This could be up to 256 megabytes, but we will unmap it later
			auto* region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0);
			region -> mapContiguousRegion((phys_addr)entries[i].base_address, region_size);
			virt_addr ptr = MemoryManager::kernel_directory -> findSpaceAbove(region_size, (virt_addr)0xc0000000);
			MemoryManager::kernel_directory -> installRegion(*region, ptr);
			for(uint32_t bus = entries[i].starting_bus; bus < entries[i].ending_bus; bus++){
				for(uint32_t device = 0; device < 32; device++){
					void* addr = (void*)((uint32_t)ptr + ((bus - entries[i].starting_bus) << 20) + (device << 15));
					uint16_t* vendor = (uint16_t*)addr;
					if(*vendor != 0xffff){
						SD::the() << "vendor " << (void*)*vendor << "\n";
					}
				}
			}
			//Yes there is a memory leak here where we never free region. I will fix this once we have smart pointers. I am leaving this comment here in part to push prod myself to implement them.
			MemoryManager::kernel_directory -> removeRegion(*region);
		}
	}
}

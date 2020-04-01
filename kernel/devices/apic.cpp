#include <devices/apic.h>
#include <acpi/tables.h>
#include <klib/SerialDevice.h>
#include <paging.h>

virt_addr lapic_base;

void APIC::init(){
	auto* mdat_header = ACPI::getTable("APIC");
	if(mdat_header == NULL){
		SD::the() << "No MADT found. Cannot spin up other cores\n";
		return;
	}

	auto* local_descriptor = (APIC_local_descriptor*)((uint32_t)mdat_header + sizeof(ACPI::SDTHeader));

	//SD::the() << "LAPIC addr " << (void*)(local_descriptor -> local_APIC_addr) << "\n";
	
	auto* region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0, false, PAGE_ENABLE_WRITE | PAGE_PRESENT | PAGE_NO_CACHE);
	region -> mapContiguousRegion((phys_addr)(local_descriptor -> local_APIC_addr), 0x400);
	lapic_base = MemoryManager::kernel_directory -> findSpaceAbove(0x400, (virt_addr)0xc0000000);
	MemoryManager::kernel_directory -> installRegion(*region, lapic_base);
	
	uint32_t addr = (uint32_t)mdat_header + sizeof(ACPI::SDTHeader) + sizeof(APIC_local_descriptor);
	uint32_t limit = (uint32_t)mdat_header + (mdat_header -> length);
	while(addr < limit){
		auto* descriptor = (MDAT_entry_header*)addr;
		addr += sizeof(MDAT_entry_header);
		//SD::the() << descriptor -> entry_type << "\n";
		switch(descriptor -> entry_type){
			case 0:
				auto* local_apic = (MDAT_local_APIC_entry*)addr;
				//SD::the() << "APIC id " << (local_apic -> APIC_id) << " processor ID " << (local_apic -> APIC_processor_id) << "\n";
				break;
		}
		addr += descriptor -> length - sizeof(MDAT_entry_header);
	}
	
	//SD::the() << (apic_header -> oem_id) << "\n";
}

void* APIC::getLAPICAddr(){
	return lapic_base;
}

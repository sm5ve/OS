#include <acpi/tables.h>
#include <devices/SerialDevice.h>
#include <devices/apic.h>
#include <paging.h>

namespace APIC {
virt_addr lapic_base;
HashMap<uint8_t, uint8_t>* coreToAPICID;

void init()
{
	auto* mdat_header = ACPI::getTable("APIC");
	if (mdat_header == NULL) {
		SD::the() << "No MADT found. Cannot spin up other cores\n";
		return;
	}

	coreToAPICID = new HashMap<uint8_t, uint8_t>();

	auto* local_descriptor = (APIC_local_descriptor*)((uint32_t)mdat_header + sizeof(ACPI::SDTHeader));

	// SD::the() << "LAPIC addr " << (void*)(local_descriptor -> local_APIC_addr)
	// << "\n";

	auto region = make_shared<MemoryManager::PhysicalMemoryRegion>(
		Vector<page_table*>(), 0, 0, false, TLBInvalidationType::INVLPG,
		PAGE_ENABLE_WRITE | PAGE_PRESENT | PAGE_NO_CACHE);
	region->mapContiguousRegion((phys_addr)(local_descriptor->local_APIC_addr),
		0x400);
	lapic_base = MemoryManager::kernel_directory->findSpaceAbove(
		0x400, (virt_addr)0xc0000000);
	MemoryManager::kernel_directory->installRegion(dynamic_ptr_cast<MemoryManager::MemoryRegion>(region), lapic_base);

	uint32_t addr = (uint32_t)mdat_header + sizeof(ACPI::SDTHeader) + sizeof(APIC_local_descriptor);
	uint32_t limit = (uint32_t)mdat_header + (mdat_header->length);
	while (addr < limit) {
		auto* descriptor = (MDAT_entry_header*)addr;
		addr += sizeof(MDAT_entry_header);
		switch (descriptor->entry_type) {
		case 0:
			auto* local_apic = (MDAT_local_APIC_entry*)addr;
			coreToAPICID->put(local_apic->APIC_processor_id, local_apic->APIC_id);
			break;
		}
		addr += descriptor->length - sizeof(MDAT_entry_header);
	}
}

void* getLAPICAddr() { return lapic_base; }

uint8_t getLAPICID()
{
	return (uint8_t)(((uint32_t*)lapic_base)[0x20 / 4] >> 24);
}

Vector<Tuple<uint8_t, uint8_t>>* getCoresWithAPICIDs()
{
	return coreToAPICID->getKeyValuePairs();
}
} // namespace APIC

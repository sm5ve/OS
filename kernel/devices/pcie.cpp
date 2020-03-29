#include <devices/pcie.h>
#include <acpi/tables.h>
#include <klib/SerialDevice.h>

namespace PCIe{
	void init(){
		auto* header = ACPI::getTable("MCFG");
		if(header == NULL){
			SD::the() << "Warning: no MCFG header found. Will not configure PCIe\n";
			return;
		}
		ACPI::MCFGEntry* entries = (ACPI::MCFGEntry*)((uint32_t)header + sizeof(ACPI::SDTHeader) + 8); //the MCFG table has 8 reserved bytes
		uint32_t entry_count = (header -> length - (sizeof(ACPI::SDTHeader) + 8)) / sizeof(ACPI::MCFGEntry);
		for(uint32_t i = 0; i < entry_count; i++){
			SD::the() << "entry " << (void*)(entries[i].base_address) << "\n";
		}
	}
}

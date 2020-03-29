#include <util/str.h>
#include <acpi/tables.h>
#include <klib/SerialDevice.h>
#include <paging.h>
#include <mem.h>

namespace ACPI{
	HashMap<String, SDTHeader*>* acpi_tables;

	RSDPDescriptor* findRSDP(){
		for(uint32_t addr = 0xC0000000; addr < 0xc0100000; addr++){
			char* potential = (char*)addr;
			if(streq(potential, "RSD PTR ", 8) && ((addr & 0xf) == 0)){
				auto* out = (RSDPDescriptor*)addr;
				uint8_t checksum = 0;
				uint8_t* bytes = (uint8_t*)out;
				for(uint32_t i = 0; i < sizeof(RSDPDescriptor); i++){
					checksum += bytes[i];
				}
				if(checksum != 0){
					continue;
				}
				return out;
			}
		}
		return NULL;
	}

	SDTHeader* mapTable(phys_addr paddr){
		uint32_t base = (uint32_t)paddr;
		base -= (base % PAGE_SIZE);
		size_t sz = 1024 * PAGE_SIZE; //FIXME this is a very unintelligent way of doing things
		virt_addr out = MemoryManager::kernel_directory -> findSpaceAbove(sz, (virt_addr)0xc0000000);
		auto* region = new MemoryManager::PhysicalRangeRegion((virt_addr)base, sz);
		MemoryManager::kernel_directory -> installRegion(*region, out);
		MemoryManager::kernel_directory -> install();
		return (SDTHeader*)((uint32_t)out + ((uint32_t)paddr % PAGE_SIZE));
	}

	void verifyTableChecksum(SDTHeader& hdr){
		uint8_t* bytes = (uint8_t*)&hdr;
		uint8_t checksum = 0;
		for(uint32_t i = 0; i < hdr.length; i++){
			checksum += bytes[i];
		}
		assert(checksum == 0, "Error: ACPI header checksum mismatch");
	}

	void addTable(phys_addr tbl){
		auto* tbl_header = mapTable(tbl);
		verifyTableChecksum(*tbl_header);
		String name = "ABCD";
		//This is admittedly a little hacky
		memcpy(name.c_str(), tbl_header -> signature, 4);
		acpi_tables -> put(name, tbl_header);
		SD::the() << "found table " << name << "\n";
	}

	void enumerateRSDT(){
		auto* rsdt_header = acpi_tables -> get("RSDT");
		uint32_t* tbl_ptrs = (uint32_t*)((uint32_t)rsdt_header + sizeof(SDTHeader));
		uint32_t num_tbls = (rsdt_header -> length - sizeof(SDTHeader)) / sizeof(uint32_t);
		for(uint32_t i = 0; i < num_tbls; i++){
			addTable((phys_addr)tbl_ptrs[i]);
		}
	}

	SDTHeader* getTable(String st){
		if(acpi_tables -> contains(st)){
			return acpi_tables -> get(st);
		}
		return NULL;
	}

	void init(){
		acpi_tables = new HashMap<String, SDTHeader*>();
		auto* rsdp = findRSDP();
		assert(rsdp, "Error: could not find ACPI tables");
		acpi_tables -> put("RSDT", mapTable((phys_addr)(rsdp -> rsdt_addr)));
		assert(streq((char*)(acpi_tables -> get("RSDT")), "RSDT", 4), "Error: RSDP did not point to RSDT");
		verifyTableChecksum(*(acpi_tables -> get("RSDT")));
		enumerateRSDT();
	}
}

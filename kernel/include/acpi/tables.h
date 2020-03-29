#ifndef ACPI_TABLES
#define ACPI_TABLES

#include <stdint.h>

namespace ACPI{
	struct __attribute__((packed)) RSDPDescriptor{
		char signature[8];
		uint8_t checksum;
		char oem_id[6];
		uint8_t revision;
		uint32_t rsdt_addr;
		/*uint32_t length;
		uint64_t xsdt_addr;
		uint8_t extended_checksum;
		uint8_t reserved[3];*/
	};

	struct __attribute__((packed)) SDTHeader{
		char signature[4];
		uint32_t length;
		uint8_t revision;
		uint8_t checksum;
		char oem_id[6];
		char oem_tbl_id[8];
		uint32_t oem_revision;
		uint32_t creator_id;
		uint32_t creator_revision;
	};

	RSDPDescriptor* findRSDP();
	void init();
}

#endif

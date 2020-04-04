#ifndef AHCI_header
#define AHCI_header

#include <stdint.h>

#define HBA_PORT_DETECT_CONNECTED 3
#define HBA_PORT_POWER_ACTIVE 1

#define SATA_SIG_ATA   0x00000101
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB  0xC33C0101
#define SATA_SIG_PM    0x96690101

namespace AHCI{
	struct __attribute__((packed)) HBAPort{
		uint32_t command_list_paddr;
		uint32_t command_list_paddr_upper; //I think this should be unused in 32 bit systems
		uint32_t fis_base_paddr;
		uint32_t fis_base_paddr_upper;
		uint32_t interrupt_status;
		uint32_t interrupt_enable;
		uint32_t command;
		uint32_t reserved;
		uint32_t task_file_data;
		uint32_t signature;
		uint32_t sata_status;
		uint32_t sata_control;
		uint32_t sata_error;
		uint32_t sata_active;
		uint32_t command_issue;
		uint32_t sata_notification;
		uint32_t fis_switch_control;
		uint32_t reserved2[11];
		uint32_t vendor[4];
	};

	struct __attribute__((packed)) ABARMemory{ //Adapted from OSDev Wiki
		uint32_t host_capabilities;
		uint32_t global_host_control;
		uint32_t interrupt_status;
		uint32_t ports_implemented;
		uint32_t version;
		uint32_t ccc_control;
		uint32_t ccc_ports;
		uint32_t enclosure_management_location;
		uint32_t enclosure_management_control;
		uint32_t host_capabilities_extended;
		uint32_t os_handoff_control_status;
		uint8_t reserved[0xa0 - 0x2c];
		uint8_t vendor[0x100 - 0xa0];
		HBAPort ports[32];
	};

	void init();
	class SATA_AHCIDevice;
	SATA_AHCIDevice* getPrimaryDisk();
}

#endif

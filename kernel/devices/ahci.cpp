#include <devices/ahci.h>
#include <devices/AHCIDevice.h>
#include <devices/pci.h>
#include <klib/SerialDevice.h>
#include <ds/Vector.h>
#include <paging.h>

namespace AHCI{
	Vector<AHCIDevice*>* devices;

	void enumerateAHCIPorts(PCIDevice& device){
		ABARMemory& abar = *(ABARMemory*)device.bar(5, sizeof(ABARMemory));
		for(uint32_t portnum = 0; portnum < 32; portnum++){
			if((abar.ports_implemented & (1 << portnum)) == 0)
				continue;
			HBAPort& port = abar.ports[portnum];
			uint8_t ipm = (port.sata_status >> 8) & 0x7;
			uint8_t det = (port.sata_status) & 0x7;
			if(det != HBA_PORT_DETECT_CONNECTED)
				continue;
			if(ipm != HBA_PORT_POWER_ACTIVE)
				continue;
			switch(port.signature){
				case SATA_SIG_ATAPI:
				case SATA_SIG_SEMB:
				case SATA_SIG_PM:
					SD::the() << "Unsupported SATA device type\n";
					break;
				default:
					SD::the() << "Found a SATA device!\n";
					//Here's where we need to register the SATA device somehow
			}
		}
	}
	
	void init(){
		devices = new Vector<AHCIDevice*>();
		for(uint32_t i = 0; i < PCI::devices -> size(); i++){
			auto& device = *(*PCI::devices)[i];
			if((device.getDeviceType() & 0xffffff00) == 0x01060100){
				enumerateAHCIPorts(device);
				//We should probably set up interrupts down here.
			}
		}
	}
}

#include <devices/ahci/AHCIHostBusAdapter.h>
#include <devices/PCIDevice.h>
#include <devices/ahci/ahci.h>
#include <klib/SerialDevice.h>

void enumerateAHCIPorts(PCIDevice& device){
	AHCI::ABARMemory& abar = *(AHCI::ABARMemory*)device.bar(5, sizeof(AHCI::ABARMemory));
	for(uint32_t portnum = 0; portnum < 32; portnum++){
		if((abar.ports_implemented & (1 << portnum)) == 0)
			continue;
		AHCI::HBAPort& port = abar.ports[portnum];
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

AHCIHostBusAdapter::AHCIHostBusAdapter(PCIDevice& dev) : device(dev){
	enumerateAHCIPorts(device);
	device.getHeader().command &= ~(1 << 10); //Clear the interrupt disable bit
	device.getHeader().command |= (1 << 2); //Enable bus mastering for DMA
}

bool AHCIHostBusAdapter::hasInterrupt(){
	
}

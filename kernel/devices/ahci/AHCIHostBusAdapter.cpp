#include <devices/PCIDevice.h>
#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>
#include <devices/ahci/AHCIHostBusAdapter.h>
#include <devices/ahci/ahci.h>

namespace AHCI {
void AHCIHostBusAdapter::enumerateAHCIPorts()
{
	ABARMemory& abar = getABAR();
	for (uint32_t portnum = 0; portnum < 32; portnum++) {
		if ((abar.ports_implemented & (1 << portnum)) == 0)
			continue;
		HBAPort& port = abar.ports[portnum];
		uint8_t ipm = (port.sata_status >> 8) & 0x7;
		uint8_t det = (port.sata_status) & 0x7;
		if (det != HBA_PORT_DETECT_CONNECTED)
			continue;
		if (ipm != HBA_PORT_POWER_ACTIVE)
			continue;
		switch (port.signature) {
		case SATA_SIG_ATAPI:
		case SATA_SIG_SEMB:
		case SATA_SIG_PM:
			devices[portnum] = new GenericAHCIDevice(port);
			SD::the() << "Unsupported SATA device type\n";
			break;
		default:
			devices[portnum] = new SATA_AHCIDevice(port);
			SD::the() << "Found a SATA device!\n";
			// Here's where we need to register the SATA device somehow
		}
	}
}

AHCIHostBusAdapter::AHCIHostBusAdapter(PCIDevice& dev)
	: device(dev)
{
	getABAR().global_host_control |= ((1 << 31) | (1 << 1)); // enable AHCI mode and interrupts
	enumerateAHCIPorts();
	device.getHeader().command &= ~(1 << 10); // Clear the interrupt disable bit
	device.getHeader().command |= (1 << 2);	  // Enable bus mastering for DMA
}

bool AHCIHostBusAdapter::hasInterrupt()
{
	volatile ABARMemory& abar = getABAR();
	return (abar.interrupt_status & abar.ports_implemented) != 0;
}

void AHCIHostBusAdapter::handleInterrupt()
{
	volatile ABARMemory& abar = getABAR();
	uint32_t maskedInterruptFlags = (abar.interrupt_status & abar.ports_implemented);
	for (uint32_t i = 0; i < 32; i++) {
		if (maskedInterruptFlags & (1 << i) != 0) {
			devices[i]->handleInterrupt();
		}
	}
	abar.interrupt_status = abar.interrupt_status;
}

ABARMemory& AHCIHostBusAdapter::getABAR()
{
	return *(ABARMemory*)device.bar(5, sizeof(ABARMemory));
}

SATA_AHCIDevice* AHCIHostBusAdapter::getPrimaryDisk()
{
	for (uint32_t i = 0; i < 32; i++) {
		if (devices[i] && devices[i]->isDisk())
			return (SATA_AHCIDevice*)devices[i];
	}
}
} // namespace AHCI

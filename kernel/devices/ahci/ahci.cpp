#include <arch/i386/proc.h>
#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>
#include <devices/ahci/AHCIHostBusAdapter.h>
#include <devices/ahci/ahci.h>
#include <devices/pci.h>
#include <ds/Vector.h>
#include <paging.h>

namespace AHCI {
Vector<AHCIHostBusAdapter*>* hbas = NULL;

InterruptHandlerDecision handleDiskInterrupt(registers& regs, void* context)
{
	SD::the() << "Got disk interrupt!\n";
	bool handled = false;
	for (uint32_t i = 0; i < hbas->size(); i++) {
		auto& hba = *(*hbas)[i];
		if (hba.hasInterrupt()) {
			handled = true;
			hba.handleInterrupt();
		}
	}
	if (handled)
		return InterruptHandlerDecision::HANDLE_AND_PASS;
	return InterruptHandlerDecision::PASS;
}

void init()
{
	hbas = new Vector<AHCIHostBusAdapter*>();
	for (uint32_t i = 0; i < PCI::devices->size(); i++) {
		auto& device = *(*PCI::devices)[i];
		if ((device.getDeviceType() & 0xffffff00) == 0x01060100) {
			device.installInterruptHandler(
				handleDiskInterrupt);
			hbas -> push(new AHCIHostBusAdapter(device));
		}
	}
}

SATA_AHCIDevice* getPrimaryDisk()
{
	return (*hbas)[0]->getPrimaryDisk();
}
} // namespace AHCI

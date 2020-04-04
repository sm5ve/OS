#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>

namespace AHCI {
SATA_AHCIDevice::SATA_AHCIDevice(HBAPort& p)
	: port(p)
{
	 
}

void SATA_AHCIDevice::handleInterrupt() { 
	SD::the() << "SATA interrupt!\n"; 
	port.interrupt_status = port.interrupt_status;
}
} // namespace AHCI

#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>

namespace AHCI {
GenericAHCIDevice::GenericAHCIDevice(HBAPort& p)
	: port(p)
{
}

void GenericAHCIDevice::handleInterrupt()
{
	SD::the() << "Generic AHCI Device\n";
}
} // namespace AHCI

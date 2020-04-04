#include <devices/ahci/AHCIDevice.h>
#include <devices/SerialDevice.h>

namespace AHCI{
	GenericAHCIDevice::GenericAHCIDevice(HBAPort& p) : port(p){

	}
	
	void GenericAHCIDevice::handleInterrupt(){
		SD::the() << "Generic AHCI Device\n";
	}
}

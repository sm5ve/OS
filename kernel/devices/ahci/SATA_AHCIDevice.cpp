#include <devices/ahci/AHCIDevice.h>
#include <klib/SerialDevice.h>

namespace AHCI{
	SATA_AHCIDevice::SATA_AHCIDevice(HBAPort& p) : port(p){
	
	}
	
	void SATA_AHCIDevice::handleInterrupt(){
		SD::the() << "SATA interrupt!\n";
	}
}

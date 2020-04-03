#ifndef AHCIDEVICE
#define AHCIDEVICE

#include <devices/PCIDevice.h>

class AHCIHostBusAdapter{
public:
	AHCIHostBusAdapter(PCIDevice&);
	bool hasInterrupt();
	void handleInterrupt();
private:
	PCIDevice& device;	
};

#endif

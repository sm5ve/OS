#ifndef AHCIDEVICE
#define AHCIDEVICE

#include <devices/PCIDevice.h>

class AHCIDevice{
public:
	AHCIDevice(PCIDevice&);
private:
	PCIDevice& device;
};

#endif

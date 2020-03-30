#ifndef PCI_header
#define PCI_header

#include <devices/PCIDevice.h>
#include <ds/Vector.h>

namespace PCI{
	extern Vector<PCIDevice*>* devices;

	void init();
}

#endif

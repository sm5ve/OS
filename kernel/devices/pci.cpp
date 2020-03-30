#include <devices/pci.h>
#include <arch/i386/proc.h>
#include <klib/SerialDevice.h>

namespace PCI{
	Vector<PCIDevice*>* devices;
	
	void init(){
		devices = new Vector<PCIDevice*>();
	}
}

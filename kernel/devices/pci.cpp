#include <arch/i386/proc.h>
#include <devices/pci.h>

namespace PCI {
Vector<PCIDevice*>* devices;

void init() { devices = new Vector<PCIDevice*>(); }
} // namespace PCI

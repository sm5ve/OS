#ifndef PCI_DEVICE
#define PCI_DEVICE

class PCIDevice{
public:
	PCIDevice(void* base, bool isPCIe);
private:
	bool pcie;
	void* base;
};

#endif

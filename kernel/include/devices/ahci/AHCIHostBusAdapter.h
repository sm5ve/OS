#ifndef AHCIHBA
#define AHCIHBA

#include <devices/PCIDevice.h>
#include <devices/ahci/AHCIDevice.h>
#include <devices/ahci/ahci.h>

namespace AHCI{
	class AHCIHostBusAdapter{
	public:
		AHCIHostBusAdapter(PCIDevice&);
		bool hasInterrupt();
		void handleInterrupt();
		ABARMemory& getABAR();
		SATA_AHCIDevice* getPrimaryDisk();
	private:
		PCIDevice& device;
		AHCIDevice* devices[32];
	
		void enumerateAHCIPorts();
	};
}

#endif

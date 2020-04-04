#ifndef AHCIDEVICE
#define AHCIDEVICE

#include <devices/ahci/ahci.h>

namespace AHCI{
	class AHCIDevice{
	public:
		virtual void handleInterrupt() = 0;
	};

	class GenericAHCIDevice : public AHCIDevice{
	public:
		GenericAHCIDevice(HBAPort&);
		void handleInterrupt() override;
	private:
		volatile HBAPort& port;
	};

	class SATA_AHCIDevice : public AHCIDevice{
	public:
		SATA_AHCIDevice(HBAPort&);
		void handleInterrupt() override;
	private:
		volatile HBAPort& port;
	};
}

#endif

#ifndef AHCIDEVICE
#define AHCIDEVICE

#include <devices/ahci/ahci.h>
#include <paging.h>

namespace AHCI{
	struct TransferRequest{
		uint64_t lba;
		void* base;
		size_t size;
		bool write;
		PageDirectory& pd;
	};

	class AHCIDevice{
	public:
		virtual void handleInterrupt() = 0;
		virtual bool isDisk() = 0;
	};

	class GenericAHCIDevice : public AHCIDevice{
	public:
		GenericAHCIDevice(HBAPort&);
		void handleInterrupt() override;
		bool isDisk(){
			return false;
		};
	private:
		volatile HBAPort& port;
	};

	class SATA_AHCIDevice : public AHCIDevice{
	public:
		SATA_AHCIDevice(HBAPort&, uint32_t capabilities);
		void handleInterrupt() override;
		bool isDisk(){
			return true;
		};
		void test();
	private:
		volatile HBAPort& port;
		uint32_t capabilities;
		uint32_t command_slots;
		uint32_t blockSize = 512;  //TODO actually get this information from the hard drive
		volatile FIS* recievedFIS; //must be 256 byte aligned (just allocate new page)
		volatile CMD* commandList; //1k byte aligned (similarly allocate new page)
		volatile CommandTable* commandTables;
		phys_addr port_base;
	
		void rebase();
		void startCommandEngine();
		void stopCommandEngine();
		uint32_t findCommandSlot();
	
		phys_addr getFISPAddr();
		phys_addr getCommandListPAddr();
		phys_addr getCommandTablePAddr(uint32_t index);
		
		bool workOnRequest(TransferRequest&);
	};
}

#endif

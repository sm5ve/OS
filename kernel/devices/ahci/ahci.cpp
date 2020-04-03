#include <devices/ahci/ahci.h>
#include <devices/ahci/AHCIHostBusAdapter.h>
#include <devices/pci.h>
#include <klib/SerialDevice.h>
#include <ds/Vector.h>
#include <paging.h>
#include <arch/i386/proc.h>

namespace AHCI{
	Vector<AHCIHostBusAdapter*>* devices;

	InterruptHandlerDecision handleDiskInterrupt(registers& regs){
		SD::the() << "Got disk interrupt!\n";
		return InterruptHandlerDecision::HANDLE_AND_PASS;
	}
	
	void init(){
		devices = new Vector<AHCIHostBusAdapter*>();
		for(uint32_t i = 0; i < PCI::devices -> size(); i++){
			auto& device = *(*PCI::devices)[i];
			if((device.getDeviceType() & 0xffffff00) == 0x01060100){
				devices -> push(new AHCIHostBusAdapter(device));
				device.installInterruptHandler(handleDiskInterrupt, 14); //14 is apparently conventionally the primary SATA device IRQ
			}
		}
	}
}

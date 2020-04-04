#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>
#include <paging.h>

namespace AHCI {
SATA_AHCIDevice::SATA_AHCIDevice(HBAPort& p)
	: port(p)
{
	rebase();
}

void SATA_AHCIDevice::handleInterrupt()
{
	SD::the() << "SATA interrupt!\n";
	port.interrupt_status = port.interrupt_status;
}

void SATA_AHCIDevice::rebase(){
	stopCommandEngine();
	auto* region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0);
	SD::the() << "range " << MemoryManager::allocateContiguousRange(*region, 0xf0000) << "\n";
	startCommandEngine();
}

void SATA_AHCIDevice::startCommandEngine(){
	while(port.command & (1 << 15)); //wait for the command list to stop running
	port.command |= (1 << 4); //enable recieving FIS's
	port.command |= (1 << 0); //start the command list
}

void SATA_AHCIDevice::stopCommandEngine(){
	port.command &= ~(1 << 0); //stop the command list
	port.command &= ~(1 << 4); //stop recieving FIS's
	while(port.command & (3 << 14));
}

uint32_t SATA_AHCIDevice::findCommandSlot(){
	
}

} // namespace AHCI

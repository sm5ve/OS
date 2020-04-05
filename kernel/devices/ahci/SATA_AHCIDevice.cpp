#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>

namespace AHCI {
SATA_AHCIDevice::SATA_AHCIDevice(HBAPort& p, uint32_t c)
	: port(p), capabilities(c)
{
	command_slots = ((c >> 8) & 31) + 1;
	rebase();
}

void SATA_AHCIDevice::handleInterrupt()
{
	SD::the() << "SATA interrupt!\n";
	port.interrupt_status = port.interrupt_status;
}

void SATA_AHCIDevice::rebase(){
	stopCommandEngine();

	size_t regionSize = sizeof(FIS) + command_slots * (sizeof(CMD) + sizeof(CommandTable));

	auto* portRegion = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0, false, TLBInvalidationType::INVLPG, PAGE_PRESENT | PAGE_ENABLE_WRITE | PAGE_NO_CACHE);
	port_base = MemoryManager::allocateContiguousRange(*portRegion, regionSize); 
	auto region_base = MemoryManager::kernel_directory -> findSpaceAbove(regionSize, (virt_addr)0xc0000000);
	MemoryManager::kernel_directory -> installRegion(*portRegion, region_base);

	memset(region_base, 0, regionSize);

	commandList = (volatile CMD*)region_base;
	recievedFIS = (volatile FIS*)((uint32_t)region_base + command_slots * sizeof(CMD));
	commandTables = (volatile CommandTable*)((uint32_t)region_base + command_slots * sizeof(CMD) + sizeof(FIS));

	port.command_list_paddr = (uint32_t)getCommandListPAddr();
	port.command_list_paddr_upper = 0;
	port.fis_base_paddr = (uint32_t)getFISPAddr();
	port.fis_base_paddr_upper = 0;

	for(uint32_t i = 0; i < command_slots; i++){
		commandList[i].prdt_entries_count = SATA_PRDT_MAX_LENGTH;
		commandList[i].command_table_phys_addr = (uint32_t)getCommandTablePAddr(i);
		commandList[i].command_table_phys_addr_upper = 0;
	}

	startCommandEngine();
}

phys_addr SATA_AHCIDevice::getFISPAddr(){
	return (phys_addr)((uint32_t)port_base + command_slots * sizeof(CMD));
}

phys_addr SATA_AHCIDevice::getCommandListPAddr(){
	return port_base;
}

phys_addr SATA_AHCIDevice::getCommandTablePAddr(uint32_t index){
	return (phys_addr)((uint32_t)port_base + command_slots * sizeof(CMD) + index * sizeof(CommandTable));
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
	for(uint32_t i = 0; i < command_slots; i++){
		if((port.sata_active & (1 << i)) == 0){
			return i;
		}
	}	
	return (uint32_t)(-1);
}

} // namespace AHCI

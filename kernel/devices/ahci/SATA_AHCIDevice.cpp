#include <devices/SerialDevice.h>
#include <devices/ahci/AHCIDevice.h>
#include <devices/pit.h>
#include <assert.h>

namespace AHCI {
SATA_AHCIDevice::SATA_AHCIDevice(HBAPort& p, uint32_t c)
	: port(p), capabilities(c)
{
	command_slots = ((c >> 8) & 31) + 1;
	rebase();
}

void SATA_AHCIDevice::test(){
	uint8_t test[8192];
	uint8_t* buffer = test;
	if((uint32_t)buffer % PAGE_SIZE != 0){
		buffer = (uint8_t*)((uint32_t)buffer + (PAGE_SIZE - ((uint32_t)buffer % PAGE_SIZE)));
	}
	TransferRequest req {
		.lba = 0,
		.base = buffer,
		.size = 4096 + 1024,
		.write = false,
		.pd = *MemoryManager::kernel_directory
	};
	workOnRequest(req);
}

void SATA_AHCIDevice::handleInterrupt()
{
	SD::the() << "SATA interrupt!\n";
	SD::the() << port.command_issue << "\n";
	SD::the() << "transferred bytes " << commandList[0].transferred_bytes_count << "\n";

	port.interrupt_status = port.interrupt_status;
}

void SATA_AHCIDevice::rebase(){
	stopCommandEngine();

	size_t regionSize = sizeof(FIS) + command_slots * (sizeof(CMD) + sizeof(CommandTable));

	auto portRegion = make_shared<MemoryManager::PhysicalMemoryRegion>(Vector<page_table*>(), 0, 0, false, TLBInvalidationType::INVLPG, PAGE_PRESENT | PAGE_ENABLE_WRITE | PAGE_NO_CACHE);
	port_base = MemoryManager::allocateContiguousRange(*portRegion, regionSize);
	auto region_base = MemoryManager::kernel_directory -> findSpaceAbove(regionSize, (virt_addr)0xc0000000);	
	MemoryManager::kernel_directory -> installRegion(dynamic_ptr_cast<MemoryManager::MemoryRegion>(portRegion), region_base);	
		
	memset(region_base, 0, regionSize);

	commandList = (volatile CMD*)region_base;
	recievedFIS = (volatile FIS*)((uint32_t)region_base + command_slots * sizeof(CMD));
	commandTables = (volatile CommandTable*)((uint32_t)region_base + command_slots * sizeof(CMD) + sizeof(FIS));

	port.command_list_paddr = (uint32_t)getCommandListPAddr();
	port.command_list_paddr_upper = 0;
	port.fis_base_paddr = (uint32_t)getFISPAddr();
	port.fis_base_paddr_upper = 0;

	port.interrupt_enable = 0xffffffff;

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
	return (phys_addr)((uint32_t)port_base + command_slots * sizeof(CMD) + index * sizeof(CommandTable) + sizeof(FIS));
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

bool SATA_AHCIDevice::workOnRequest(TransferRequest& req){
	port.interrupt_status = port.interrupt_status; //clear interrupts
	assert((uint32_t)req.base % PAGE_SIZE == 0, "Error: buffer not aligned to page");
	assert((uint32_t)req.size % blockSize == 0, "Error: request size not aligned to sector size"); 
	int spin = 0;
	int slot = findCommandSlot();
	if(slot == -1)
		return false;
	volatile CMD& command = commandList[slot];
	volatile CommandTable& cmdtbl = commandTables[slot];
	volatile PRDTEntry* prdt = cmdtbl.prdt;
	uint32_t prdt_index = 0;
	FIS_H2D& fis = *(FIS_H2D*)(cmdtbl.command_fis);
	uint16_t sector_count = req.size / blockSize;
	//SD::the() << (void*)command.command_table_phys_addr << "\n";
	while(prdt_index < SATA_PRDT_MAX_LENGTH){
		if(req.size == 0)
			break;
		prdt[prdt_index].base_paddr = (uint32_t)(req.pd.findPhysicalAddr(req.base));
		prdt[prdt_index].base_paddr_upper = 0;
		prdt[prdt_index].size_and_interrupt_flag = 0;
		while(true){
			if(req.size < PAGE_SIZE){
				prdt[prdt_index].size_and_interrupt_flag += req.size;
				req.size = 0;
				break;
			}
			else{
				prdt[prdt_index].size_and_interrupt_flag += PAGE_SIZE;
				req.size -= PAGE_SIZE;
				req.base = (void*)((uint32_t)req.base + PAGE_SIZE);
			}
			uint32_t nextPaddr = (uint32_t)(req.pd.findPhysicalAddr(req.base));
			uint32_t nextContiguousPaddr = prdt[prdt_index].base_paddr + prdt[prdt_index].size_and_interrupt_flag;
			if(nextPaddr != nextContiguousPaddr)
				break;
			if(prdt[prdt_index].base_paddr >= (4 * MB - PAGE_SIZE))
				break;
		}
		prdt[prdt_index].size_and_interrupt_flag -= 1;
		prdt_index++;
	}
	command.prdt_entries_count = prdt_index;
	command.command_table_phys_addr = (uint32_t)getCommandTablePAddr(slot);
	command.command_table_phys_addr_upper = 0;
	command.flags = sizeof(FIS_H2D) / 4;
	fis.fis_type = FISType::H2D;
	fis.flags |= (1 << 7);
	fis.command = 0x25;
	fis.lba0 = (uint8_t)req.lba;
	fis.lba1 = (uint8_t)(req.lba >> 8);
	fis.lba2 = (uint8_t)(req.lba >> 16);
	fis.device = (1 << 6);
	fis.lba3 = (uint8_t)(req.lba >> 24);
	fis.lba4 = (uint8_t)(req.lba >> 32);
	fis.lba5 = (uint8_t)(req.lba >> 40);
	fis.countl = (uint8_t)sector_count;
	fis.counth = (uint8_t)(sector_count >> 8);
	SD::the() << (void*)(port.task_file_data) << "\n";
	SD::the() << "Buffer paddr " << (void*)prdt[0].base_paddr << "\n";
	SD::the() << "Buffer size " << prdt[0].size_and_interrupt_flag << "\n";
	port.command_issue = (1 << slot);
	PIT::waitMillis(200);
	SD::the() << "IS " << (void*)(port.interrupt_status) << "\n";
	SD::the() << "CI " << (void*)(port.command_issue) << "\n";
	SD::the() << "transferred " << (command.transferred_bytes_count) << "\n";
}

} // namespace AHCI

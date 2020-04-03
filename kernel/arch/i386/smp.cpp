#include <arch/i386/smp.h>
#include <arch/i386/proc.h>
#include <devices/apic.h>
#include <devices/pit.h>
#include <klib/SerialDevice.h>
#include <paging.h>
#include <flags.h>

extern uint32_t bootup_start;
extern uint32_t bootup_end;

#define APIC_IPI_DEST_REG (0x31 * 16 / sizeof(uint32_t))
#define APIC_IPI_CMD_REG  (0x30 * 16 / sizeof(uint32_t))

namespace SMP{
	volatile uint8_t online_cores = 0;	

	void sendIPI(uint8_t apic_id, uint8_t vector, IPIMode mode){
		uint32_t volatile *apic = (uint32_t volatile*)APIC::getLAPICAddr();
		while(apic[0x30 * 16 / sizeof(uint32_t)] & (1 << 12)){
			asm volatile("pause");
		} //Wait for last IPI to be delivered
		uint32_t dest_reg = apic[APIC_IPI_DEST_REG]; //Make sure to preserve reserved bits
		dest_reg &= ~(0xff << 24);
		dest_reg |= (apic_id << 24);
		apic[APIC_IPI_DEST_REG] = dest_reg;
		uint32_t cmd_reg = apic[APIC_IPI_CMD_REG]; //Make sure to preserve reserved bits
		cmd_reg &= ~((3 << 14) | (1 << 11) | (7 << 8) | (0xff) | (3 << 18));
		uint32_t init_flags = (1 << 14);
		if(mode == IPIMode::INIT){
			init_flags = 0;//(1 << 15);
		}
		cmd_reg |= (vector & 0xff) | (mode << 8) | init_flags;
		apic[0x30 * 4] = cmd_reg;
	}

	void setupAPTables(uint16_t offset){
		writeAPBootstrapGDT(offset);
		uint32_t* pd_loc = (uint32_t*)(0xc0000000 + offset + 0x100);
		*pd_loc = (uint32_t)(MemoryManager::getPhysicalAddr(MemoryManager::kernel_directory -> directory));
		uint32_t* stack_loc = (uint32_t*)(0xc0000000 + offset + 0x200);
		auto* core_stack = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0);
		MemoryManager::growPhysicalMemoryRegion(*core_stack, KERNEL_STACK_SIZE);
		virt_addr stack_start = MemoryManager::kernel_directory -> findSpaceAbove(KERNEL_STACK_SIZE, (virt_addr)0xc0000000);
		MemoryManager::kernel_directory -> installRegion(*core_stack, stack_start);
		*stack_loc = (uint32_t)stack_start + KERNEL_STACK_SIZE;
	}
	//FIXME I think we can get stuck here sometimes. It might be a bug in the PIT code
	void startCore(uint8_t apic_id, uint16_t offset){
		setupAPTables(offset + 0x1000);
		sendIPI(apic_id, 0, IPIMode::INIT);
		PIT::waitMillis(10);
		uint8_t prev_online = online_cores;
		sendIPI(apic_id, (offset >> 12), IPIMode::SIPI);
		PIT::waitMicros(200);
		if(online_cores == prev_online){ //If our core count doesn't go up, try sending another SIPI
			sendIPI(apic_id, (offset >> 12), IPIMode::SIPI);
		}
	}
	
	void init(){
		auto* id_map = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0);
		id_map -> mapContiguousRegion((phys_addr)0, 0x7000);
		MemoryManager::kernel_directory -> installRegion(*id_map, (virt_addr)0);
	
		uint32_t volatile* apic = (uint32_t volatile*)APIC::getLAPICAddr();
		uint32_t ap_boot_addr = 0x4000;
		apic[0xf0/4] |= (1 << 8);
		memcpy((void*)(0xc0000000 + ap_boot_addr), &bootup_start, ((uint32_t)&bootup_end - (uint32_t)&bootup_start));
		auto* cores = APIC::getCoresWithAPICIDs();
		for(uint32_t i = 0; i < cores -> size(); i++){
			if((*cores)[i].b == APIC::getLAPICID())
				continue;
			startCore((*cores)[i].b, ap_boot_addr);
		}

		MemoryManager::kernel_directory -> removeRegion(*id_map);
	}
	
	void incCoreCount(){
		online_cores++;
	}
}

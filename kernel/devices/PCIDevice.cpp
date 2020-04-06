#include <devices/PCIDevice.h>
#include <paging.h>

PCIDevice::PCIDevice(void* b, bool isPCIe)
	: pcie(isPCIe)
	, base(b)
{
	memset(bars, 0, sizeof(bars));
}

uint32_t PCIDevice::getDeviceType()
{
	assert(pcie, "Error: don't know normal PCI yet\n");
	return ((uint32_t*)base)[2];
}

void* PCIDevice::bar(uint32_t num, size_t size)
{
	if (bars[num] != NULL)
		return bars[num];
	auto* region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0);
	PCIHeader0* hdr = (PCIHeader0*)base;
	region->mapContiguousRegion((phys_addr)(hdr->bar[num]), size);
	void* out = MemoryManager::kernel_directory->findSpaceAbove(
		size, (virt_addr)0xc0000000);
	MemoryManager::kernel_directory->installRegion(*region, (virt_addr)out);
	bars[num] = out;
	return out;
}

InterruptHandlerDecision handlePCIInterrupt(registers& regs, void* context){
	PCIDevice& device = *(PCIDevice*)context;
	if(!device.hasInterrupt())
		return InterruptHandlerDecision::PASS;
	device.acknowledgeInterrupt();
	return device.int_handler(regs, device.int_context);
}

bool PCIDevice::hasInterrupt(){
	return (getHeader().status & (1 << 3)) != 0;
}

void PCIDevice::acknowledgeInterrupt(){
	getHeader().status &= ~(1 << 3);
}

void PCIDevice::installInterruptHandler(interrupt_handler handler)
{
	PCIHeader0& hdr = getHeader();
	IDT::installIRQHandler(handler, hdr.interrupt_line, this);
}

PCIHeader0& PCIDevice::getHeader() { return *(PCIHeader0*)base; }

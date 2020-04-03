#include <devices/PCIDevice.h>
#include <paging.h>

PCIDevice::PCIDevice(void* b, bool isPCIe) : pcie(isPCIe), base(b){
	memset(bars, 0, sizeof(bars));
}

uint32_t PCIDevice::getDeviceType(){
	assert(pcie, "Error: don't know normal PCI yet\n");
	return ((uint32_t*)base)[2];
}

void* PCIDevice::bar(uint32_t num, size_t size){
	if(bars[num] != NULL)
		return bars[num];
	auto* region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0);
	PCIHeader0* hdr = (PCIHeader0*)base;
	region -> mapContiguousRegion((phys_addr)(hdr -> bar[num]), size);
	void* out = MemoryManager::kernel_directory -> findSpaceAbove(size, (virt_addr)0xc0000000);
	MemoryManager::kernel_directory -> installRegion(*region, (virt_addr)out);
	bars[num] = out;
	return out;
}

void PCIDevice::installInterruptHandler(interrupt_handler handler, uint32_t irq){
	PCIHeader0& hdr = getHeader();
	hdr.interrupt_line = irq;
	IDT::installIRQHandler(handler, irq);
}

void PCIDevice::installInterruptHandler(interrupt_handler handler){
	PCIHeader0& hdr = getHeader();
	hdr.interrupt_line = (uint8_t)IDT::installIRQHandler(handler);
}

PCIHeader0& PCIDevice::getHeader(){
	return *(PCIHeader0*)base;
}

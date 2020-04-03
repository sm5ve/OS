#ifndef PCI_DEVICE
#define PCI_DEVICE

#include <stdint.h>
#include <stddef.h>
#include <arch/i386/proc.h>

struct PCIHeader0{
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t pci_class;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
	uint32_t bar[6];
	uint32_t cardbus_cis_addr;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;
	uint32_t expansion_rom_base;
	uint8_t capabilities_ptr;
	uint8_t reserved[7];
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_grant;
	uint8_t max_latency;
};

class PCIDevice{
public:
	PCIDevice(void* base, bool isPCIe);

	uint32_t getDeviceType();
	void* bar(uint32_t num, size_t size);
	void installInterruptHandler(interrupt_handler, uint32_t);
	void installInterruptHandler(interrupt_handler);
	void enableInterrupts();
	PCIHeader0& getHeader();
private:
	bool pcie;
	void* base;
	void* bars[6];
};

#endif

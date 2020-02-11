#include <arch/i386/proc.h>

#include <klib/SerialPrinter.h>

struct __attribute__((__packed__)) idt_table_descriptor{
	uint16_t size;
	uint32_t addr;
};

enum GateType{
	TASK = 0,
	INTERRUPT = 1,
	TRAP = 2
};

uint8_t idt[8 * 256];
idt_table_descriptor idtDescriptor;

int counter = 0;

extern "C" void ibody(){
	SerialPrinter p(COMPort::COM1);
	p << "Here!\n";
	p << counter++ << "\n";
	outb(0x20, 0x20);
}

extern "C" void test();
asm(".globl test\n" \
	"test:\n"       \
	"cli\n"\
	"call ibody\n"  \
	"sti\n" \
	"iret");
void flushIDT(){
	idtDescriptor = {
		.size = 8 * 128 - 1,
		.addr = (uint32_t)idt
	};
	__asm__ volatile ("lidt %0" :: "m" (idtDescriptor) : "memory");	
}

void writeDescriptor(int index, uint16_t segmentSelector, uint32_t offset, bool present, uint8_t ring, GateType type){
	uint8_t* entry = &idt[index * 8];

	//Set the segment selector
	entry[2] = segmentSelector & 0xff;
	entry[3] = (segmentSelector >> 8) & 0xff;
	//Set the offset (unused when type = TASK)
	entry[0] = offset & 0xff;
	entry[1] = (offset >> 8) & 0xff;
	entry[6] = (offset >> 16) & 0xff;
	entry[7] = (offset >> 24) & 0xff;
	//Set flags
	entry[5] = 0;
	entry[5] |= (present ? (1 << 7) : 0);
	entry[5] |= (ring << 5);
	entry[4] = 0;
	//Finally set the type of the descriptor
	switch(type){
		case GateType::TASK:
			entry[5] |= 0x05;
			break;
		case GateType::INTERRUPT:
			entry[5] |= 0x0e;
			break;
		case GateType::TRAP:
			entry[5] |= 0x0f; 
	}
}

void installIDT(){
	//Initialize IDT to a state where no segments are present
	SerialPrinter p(COMPort::COM1);
	for(int i = 0; i < 256; i++){
		writeDescriptor(i, 0, 0, false, 0, GateType::INTERRUPT);
	}
	//Now initialize with test vectors
	for(int i = 0; i < 256; i++){
		writeDescriptor(i, 0x08, (uint32_t)test, true, 0, GateType::INTERRUPT);
	}
	flushIDT();
}


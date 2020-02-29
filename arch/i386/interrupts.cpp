#include <arch/i386/proc.h>

#include <klib/SerialDevice.h>

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
	SD::the() << "Here!\n";
	SD::the() << counter++ << "\n";
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
	entry[2] = (uint8_t)(segmentSelector & 0xff);
	entry[3] = (uint8_t)((segmentSelector >> 8) & 0xff);
	//Set the offset (unused when type = TASK)
	entry[0] = (uint8_t)(offset & 0xff);
	entry[1] = (uint8_t)((offset >> 8) & 0xff);
	entry[6] = (uint8_t)((offset >> 16) & 0xff);
	entry[7] = (uint8_t)((offset >> 24) & 0xff);
	//Set flags
	entry[5] = 0;
	entry[5] |= (uint8_t)((present ? (1 << 7) : 0));
	entry[5] |= (uint8_t)(ring << 5);
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

#define INC_ISR(x) extern "C" void  isr_ ## x(); 
#define INST_ISR(x) writeDescriptor(x, 0x08, (uint32_t)isr_ ##x, true, 0, GateType::INTERRUPT);

INC_ISR(0)
INC_ISR(1)
INC_ISR(2)
INC_ISR(3)
INC_ISR(4)
INC_ISR(5)
INC_ISR(6)
INC_ISR(7)
INC_ISR(8)
INC_ISR(9)
INC_ISR(10)
INC_ISR(11)
INC_ISR(12)
INC_ISR(13)
INC_ISR(14)
INC_ISR(15)


void installIDT(){
	//Initialize IDT to a state where no segments are present
	for(int i = 0; i < 256; i++){
		writeDescriptor(i, 0, 0, false, 0, GateType::INTERRUPT);
	}
	//Now initialize with test vectors
	for(int i = 0; i < 256; i++){
		writeDescriptor(i, 0x08, (uint32_t)test, true, 0, GateType::INTERRUPT);
	}

	INST_ISR(0)
	INST_ISR(1)
	INST_ISR(2)
	INST_ISR(3)
	INST_ISR(4)
	INST_ISR(5)
	INST_ISR(6)
	INST_ISR(7)
	INST_ISR(8)
	INST_ISR(9)
	INST_ISR(10)
	INST_ISR(11)
	INST_ISR(12)
	INST_ISR(13)
	INST_ISR(14)
	INST_ISR(15)
	flushIDT();
}


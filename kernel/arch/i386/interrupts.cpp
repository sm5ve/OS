#include <arch/i386/proc.h>

#include <klib/SerialDevice.h>
#include <panic.h>
#include <debug.h>
#include <Scheduler.h>

extern "C" void isrHandler(registers regs);
extern "C" void irqHandler(registers regs);

#define ISR_NO_ERR(x) \
extern "C" void isr_ ## x (); \
asm (".global isr_" #x "\n" \
		 "isr_" #x ": \n" \
		 "cli\n" \
		 "push $0\n" \
		 "push $" #x "\n" \
		 "pusha\n" \
		 "pushl %ds\n" \
		 "pushl %es\n" \
		 "pushl %fs\n" \
		 "pushl %gs\n" \
		 "mov $0x10, %ax\n" \
		 "mov %ax, %ds\n" \
		 "mov %ax, %es\n" \
		 "mov %ax, %fs\n" \
		 "mov %ax, %gs\n" \
		 "mov %cr2, %edx\n"\
		 "pushl %edx\n" \
		 "cld\n" \
		 "call isrHandler\n" \
		 "add $4, %esp\n" \
		 "popl %gs\n"\
		 "popl %fs\n"\
		 "popl %es\n"\
		 "popl %ds\n"\
		 "popa\n"\
		 "add $8, %esp\n"\
		 "iret\n")

#define ISR_ERR(x) \
extern "C" void isr_ ## x (); \
asm (".global isr_" #x "\n" \
		 "isr_" #x ": \n" \
		 "cli\n" \
		 "push $" #x "\n" \
		 "pusha\n" \
		 "pushl %ds\n" \
		 "pushl %es\n" \
		 "pushl %fs\n" \
		 "pushl %gs\n" \
		 "mov $0x10, %ax\n" \
		 "mov %ax, %ds\n" \
		 "mov %ax, %es\n" \
		 "mov %ax, %fs\n" \
		 "mov %ax, %gs\n" \
		 "mov %cr2, %edx\n"\
		 "pushl %edx\n" \
		 "cld\n" \
		 "call isrHandler\n" \
		 "add $4, %esp\n" \
		 "popl %gs\n"\
		 "popl %fs\n"\
		 "popl %es\n"\
		 "popl %ds\n"\
		 "popa\n"\
		 "add $8, %esp\n"\
		 "iret\n")

#define IRQ(x) \
extern "C" void irq_ ## x (); \
asm (".global irq_" #x "\n" \
		 "irq_" #x ": \n" \
		 "cli\n" \
		 "push $0\n" \
		 "push $" #x "\n" \
		 "pusha\n" \
		 "pushl %ds\n" \
		 "pushl %es\n" \
		 "pushl %fs\n" \
		 "pushl %gs\n" \
		 "mov $0x10, %ax\n" \
		 "mov %ax, %ds\n" \
		 "mov %ax, %es\n" \
		 "mov %ax, %fs\n" \
		 "mov %ax, %gs\n" \
		 "mov %cr2, %edx\n"\
		 "pushl %edx\n" \
		 "cld\n" \
		 "call irqHandler\n" \
		 "add $4, %esp\n" \
		 "popl %gs\n"\
		 "popl %fs\n"\
		 "popl %es\n"\
		 "popl %ds\n"\
		 "popa\n"\
		 "add $8, %esp\n"\
		 "iret\n")

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

extern "C" void isrHandler(registers regs){
	if((regs.faulting_addr == 0) && (regs.int_number == 14)){
		panic("Null pointer exception!", regs);
	}

	if(regs.int_number == 80){
		SD::the() << (char*)regs.eax;
		return;
	}
	if(regs.int_number == 81){
		SD::the() << regs.eax << "\n";
		if(!Scheduler::empty()){
			Scheduler::storeState(regs);
			Scheduler::pickNext();
			Scheduler::exec();
		}	
		return;
	}

	SD::the() << "At " << (void*)regs.faulting_addr << "\n";
	SD::the() << "ISR!" << regs.int_number << "\n";
	SD::the() << "IP " << (void*)regs.fault_eip << "\n";
	SD::the() << "Error code " << regs.error_code << "\n";
	//stackTrace();
	panic("ERROR!", regs);
	//__asm__ ("hlt");
	//return regs;
}

extern "C" void irqHandler(registers regs){
	//SD::the() << "Got IRQ " << regs.int_number << "\n";
	outb(0x20, 0x20);
	if((regs.int_number == 0) && (!Scheduler::empty())){
		Scheduler::storeState(regs);
		Scheduler::pickNext();
		Scheduler::exec();
	}
}

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

#define INST_ISR(x) writeDescriptor(x, 0x08, (uint32_t)isr_##x, true, 0, GateType::INTERRUPT);
#define INST_IRQ(x) writeDescriptor(x + 0x30, 0x08, (uint32_t)irq_##x, true, 0, GateType::INTERRUPT);
#define INST_USER_ISR(x) writeDescriptor(x, 0x08, (uint32_t)isr_##x, true, 3, GateType::INTERRUPT);

ISR_NO_ERR(0);
ISR_NO_ERR(1);
ISR_NO_ERR(2);
ISR_NO_ERR(3);
ISR_NO_ERR(4);
ISR_NO_ERR(5);
ISR_NO_ERR(6);
ISR_NO_ERR(7);
ISR_ERR(8);
ISR_NO_ERR(9);
ISR_ERR(10);
ISR_ERR(11);
ISR_ERR(12);
ISR_ERR(13);
ISR_ERR(14);
ISR_NO_ERR(15);
ISR_NO_ERR(16);
ISR_NO_ERR(17);
ISR_NO_ERR(18);
ISR_NO_ERR(19);
ISR_NO_ERR(20);
ISR_NO_ERR(80);
ISR_NO_ERR(81);
//Create IRQ handlers

IRQ(0);
IRQ(1);
IRQ(2);
IRQ(3);
IRQ(4);
IRQ(5);
IRQ(6);

void installIDT(){
	outb(0x20, 0x11);
	outb(0xa0, 0x11);
	outb(0x21, 0x30);
	outb(0xa1, 0x38);
	outb(0x21, 0x04);
	outb(0x21, 0x02);
	outb(0xa1, 0x01);
	outb(0xa1, 0x01);

	//Initialize IDT to a state where no segments are present
	for(int i = 0; i < 256; i++){
		writeDescriptor(i, 0, 0, false, 0, GateType::INTERRUPT);
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
	INST_ISR(16)
	INST_ISR(17)
	INST_ISR(18)
	INST_ISR(19)
	INST_ISR(20)
	INST_USER_ISR(80)
	INST_USER_ISR(81)	

	INST_IRQ(0)
	INST_IRQ(1)
	INST_IRQ(2)
	INST_IRQ(3)
	INST_IRQ(4)
	INST_IRQ(5)
	INST_IRQ(6)

	flushIDT();
}

#ifndef PROCESS
#define PROCESS

#include <paging.h>
#include <ds/String.h>
#include <arch/i386/proc.h>

struct __attribute__((packed)) task_regs{
	uint32_t esp;
	uint32_t eip;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t ebx;
};

class Task{
public:
	Task(bool kernel_task);
	~Task();

	PageDirectory& getPageDirectory();

	void contextSwitch();
	void setEntrypoint(virt_addr);
	task_regs regs;
	void storeState(registers&);	
private:
	PageDirectory pd;
	bool is_kernel_task;
};

extern "C" void userspaceContextSwitch();
extern "C" void kernelContextSwitch();

#endif

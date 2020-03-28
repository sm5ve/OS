#ifndef PROCESS
#define PROCESS

#include <paging.h>
#include <ds/String.h>
#include <arch/i386/proc.h>

struct __attribute__((packed)) thread_regs{
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

class Thread{
public:
	Thread(String name, uint32_t pid, bool kernel_process);
	~Thread();

	PageDirectory& getPageDirectory();

	void contextSwitch();
	void setEntrypoint(virt_addr);
	thread_regs regs;
	void storeState(registers&);	
private:
	PageDirectory pd;
	uint32_t pid;
	String name;
	bool is_kernel_process;

	//thread_regs regs;
};

extern "C" void userspaceContextSwitch();

#endif

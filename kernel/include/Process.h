#ifndef PROCESS
#define PROCESS

#include <paging.h>
#include <ds/String.h>

class Process{
public:
	Process(String name, uint32_t pid, bool kernel_process);
	~Process();

	PageDirectory& getPageDirectory();

	void contextSwitch();
	void setEntrypoint(virt_addr);
private:
	PageDirectory pd;
	uint32_t pid;
	String name;
	bool is_kernel_process;
	
	virt_addr entry_point;
};

#endif

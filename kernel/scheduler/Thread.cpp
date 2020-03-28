#include <Thread.h>

Thread::Thread(String n, uint32_t p, bool kp) : name(n), pid(p), is_kernel_process(kp){
	MemoryManager::kernel_directory -> copyRegionsInto(pd);
	if(!kp){
		auto region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0, false, PAGE_PRESENT | PAGE_ENABLE_WRITE | PAGE_USER_ACCESSIBLE);
		MemoryManager::growPhysicalMemoryRegion(*region, 0x10000);
		pd.installRegion(*region, (virt_addr)0x80000000);
		regs.esp = 0x80010000;
	}
	regs.ecx = 0xdeadbeef;
	regs.ebx = 0x0badf00d;
}

Thread::~Thread(){

}

//TODO in the contextSwitch methods, we'll need to load back in the floating point registers
//Similarly with the SSE registers
void Thread::contextSwitch(){
	if((&pd != MemoryManager::active_page_dir) && !is_kernel_process)
		pd.install();
	asm("mov %0, %%ebx" :: "r"(&regs));
	userspaceContextSwitch();
}

void Thread::setEntrypoint(virt_addr ep){
	regs.eip = (uint32_t)ep;
}

PageDirectory& Thread::getPageDirectory(){
	return pd;
}

void Thread::storeState(registers& stored){
	regs.esp = stored.user_esp;
	regs.eip = stored.fault_eip;
	regs.edi = stored.edi;
	regs.esi = stored.esi;
	regs.ebp = stored.ebp;
	regs.edx = stored.edx;
	regs.ecx = stored.ecx;
	regs.eax = stored.eax;
	regs.ebx = stored.ebx;
}

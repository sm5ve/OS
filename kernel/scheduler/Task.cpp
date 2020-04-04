#include <Task.h>

Task::Task(bool kp)
	: is_kernel_task(kp)
{
	MemoryManager::kernel_directory->copyRegionsInto(pd);
	if (!kp) {
		auto region = new MemoryManager::PhysicalMemoryRegion(
			Vector<page_table*>(), 0, 0, false, TLBInvalidationType::INVLPG,
			PAGE_PRESENT | PAGE_ENABLE_WRITE | PAGE_USER_ACCESSIBLE);
		MemoryManager::growPhysicalMemoryRegion(*region, 0x10000);
		pd.installRegion(*region, (virt_addr)0x80000000);
		regs.esp = 0x80010000;
	} else {
		// FIXME quick and dirty hack to make sure kernel context switching works
		// alright
		void* stack = new uint32_t[1000];
		regs.esp = (uint32_t)stack;
	}
}

Task::~Task() {}

// TODO in the contextSwitch methods, we'll need to load back in the floating
// point registers Similarly with the SSE registers
void Task::contextSwitch()
{
	if ((&pd != MemoryManager::active_page_dir) && !is_kernel_task)
		pd.install();
	if (is_kernel_task) {
		asm("mov %0, %%ebx" ::"r"(&regs));
		kernelContextSwitch();
	} else {
		asm("mov %0, %%ebx" ::"r"(&regs));
		userspaceContextSwitch();
	}
}

void Task::setEntrypoint(virt_addr ep) { regs.eip = (uint32_t)ep; }

PageDirectory& Task::getPageDirectory() { return pd; }

void Task::storeState(registers& stored)
{
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

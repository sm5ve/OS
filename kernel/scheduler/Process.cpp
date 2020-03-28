#include <Process.h>

Process::Process(String n, uint32_t p, bool kp) : name(n), pid(p), is_kernel_process(kp){
	MemoryManager::kernel_directory -> copyRegionsInto(pd);
	if(!kp){
		auto region = new MemoryManager::PhysicalMemoryRegion(Vector<page_table*>(), 0, 0, false, PAGE_PRESENT | PAGE_ENABLE_WRITE | PAGE_USER_ACCESSIBLE);
		MemoryManager::growPhysicalMemoryRegion(*region, 0x10000);
		pd.installRegion(*region, (virt_addr)0x80000000);
	}
}

Process::~Process(){

}

void Process::contextSwitch(){
	if(&pd != MemoryManager::active_page_dir)
		pd.install();
	SD::the() << (void*)entry_point << "\n";
	//for(;;);
	asm("mov $0x23, %%ax\n" \
		"mov %%ax, %%ds\n" \
		"mov %%ax, %%es\n" \
		"mov %%ax, %%fs\n" \
		"mov %%ax, %%gs\n" \
		"push $0x23\n" \
		"push $0x80008000\n" \
		"pushf\n" \
		"push $0x1b\n" \
		"push %0\n" \
		"iret" :: "b" ((uint32_t)entry_point));
	//asm("jmp *%0" :: "r"(entry_point));
}

void Process::setEntrypoint(virt_addr ep){
	entry_point = ep;
}

PageDirectory& Process::getPageDirectory(){
	return pd;
}

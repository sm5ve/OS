#ifndef SCHEDULER
#define SCHEDULER
#include <Task.h>
#include <arch/i386/proc.h>

namespace Scheduler{
	void init();
	void addTask(Task&);
	void removeTask(Task&);
	void pickNext();
	void exec();
	void storeState(registers&);
	bool empty();
}

#endif

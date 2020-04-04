#include <Scheduler.h>
#include <ds/Vector.h> //At some point we should consider using a better data structure
#include <interrupts.h>

namespace Scheduler {
Vector<Task*>* tasks;
Task* current = NULL;
uint32_t index = 0;

void init()
{
	current = NULL;
	index = 0;
	tasks = new Vector<Task*>();
}

void addTask(Task& t)
{
	DisableInterrupts d;
	tasks->push(&t);
}

void pickNext()
{
	DisableInterrupts d;
	assert(tasks->size() > 0, "Error: scheduler empty");
	index = (index + 1) % (tasks->size());
	current = (*tasks)[index];
}

void exec()
{
	assert(current, "Error: no thread to switch to");
	current->contextSwitch();
}

void storeState(registers& regs)
{
	if (current)
		current->storeState(regs);
}

bool empty() { return tasks->size() == 0; }
} // namespace Scheduler

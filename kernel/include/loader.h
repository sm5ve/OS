#ifndef LOADER
#define LOADER

#include <paging.h>
#include <elf/elf.h>
#include <Task.h>

namespace Loader{
	Task* load(ELF&);
}

#endif

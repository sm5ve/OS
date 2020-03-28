#ifndef LOADER
#define LOADER

#include <paging.h>
#include <elf/elf.h>
#include <Thread.h>

namespace Loader{
	Thread* load(ELF&);
}

#endif

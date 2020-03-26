#ifndef LOADER
#define LOADER

#include <paging.h>
#include <elf/elf.h>

namespace Loader{
	virt_addr load(ELF&);
}

#endif

#ifndef PAGING
#define PAGING

#include <multiboot/multiboot.h>

class PageAlloc{
public:
	PageAlloc(mboot_mmap_entry* entries, uint32_t entries_length);
	~PageAlloc();

	static PageAlloc the();
private:
	
};

#endif

#ifndef PAGING
#define PAGING

#include <multiboot/multiboot.h>

void initPalloc(mboot_mmap_entry* entries, uint32_t len);

void enterMirroredFlatPaging();
#endif

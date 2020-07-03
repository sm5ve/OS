#include <devices/SerialDevice.h>
#include <mem.h>
#include <paging.h>
#include <stddef.h>

using namespace MemoryManager;
namespace MemoryManager {
uint32_t* physicalToPageTableAddr(phys_addr v);
PageDirectory* active_page_dir;
} // namespace MemoryManager

PageDirectory::PageDirectory()
{
	directory = (uint32_t*)allocatePageTable();
	assert(((uint32_t)directory & 0xfff) == 0,
		"Error: misaligned page directory");
	assert(directory != NULL, "Error: somehow got null page directory");
	memset(directory, 0, 4096);
}

PageDirectory::~PageDirectory() {}

bool isPresent(uint32_t entry)
{
	if ((entry & PAGE_PRESENT) == 0) {
		assert(entry == 0, "Error: page entry exists, but is not present. We don't "
						   "have swap implemented yet, so what gives?");
		return false;
	}
	return true;
}

void PageDirectory::addMapping(phys_addr p, virt_addr v, uint32_t flags)
{
	uint32_t directoryIndex = ((uint32_t)v >> 22) & 0x3ff;
	uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;

	assert((((uint32_t)directory) & 0xfff) == 0, "Error: misaligned directory");

	if (!isPresent(directory[directoryIndex])) {
		void* pageTable = allocatePageTable();
		assert(((uint32_t)pageTable & 0xfff) == 0, "Error: misaligned page table");
		uint32_t paddr = (uint32_t)getPhysicalAddr((virt_addr)pageTable);
		uint32_t directoryEntry = (paddr & (~0xfff)) | flags; // FIXME is this how we want to be setting the table flags? Is
															  // there a better interface?
		directory[directoryIndex] = directoryEntry;
	}
	uint32_t* table = physicalToPageTableAddr(
		(phys_addr)(directory[directoryIndex] & (~0xfff)));
	uint32_t tableEntry = (((uint32_t)p) & (~0xfff)) | flags;
	table[tableIndex] = tableEntry;
	invalidateMappingIfNecessary(v, TLBInvalidationType::INVLPG);
}

void PageDirectory::removeMapping(virt_addr v)
{
	uint32_t directoryIndex = ((uint32_t)v >> 22);
	uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;

	assert(isPresent(directory[directoryIndex]),
		"Error: tried to unmap already unmapped address");
	uint32_t* table = physicalToPageTableAddr(
		(phys_addr)(directory[directoryIndex] & (~0xfff)));
	assert(isPresent(table[tableIndex]),
		"Error: tried to unmap already unmapped address");
	table[tableIndex] = 0;
	invalidateMappingIfNecessary(v, TLBInvalidationType::INVLPG);
}

// Will return null if the virtual address is unmapped
phys_addr PageDirectory::findPhysicalAddr(virt_addr v)
{
	uint32_t directoryIndex = ((uint32_t)v >> 22);
	uint32_t tableIndex = ((uint32_t)v >> 12) & 0x3ff;

	if (!isPresent(directory[directoryIndex])) {
		return NULL;
	}
	uint32_t* table = physicalToPageTableAddr(
		(phys_addr)(directory[directoryIndex] & (~0xfff)));
	uint32_t entry = table[tableIndex];
	if (!isPresent(entry)) {
		return NULL;
	}
	return (phys_addr)(entry & (~0xfff));
}

virt_addr PageDirectory::findVirtAddr(phys_addr p)
{
	for (uint32_t table_index = 0; table_index < 1024; table_index++) {
		if (!isPresent(directory[table_index])) {
			continue;
		}
		uint32_t* table = physicalToPageTableAddr((phys_addr)(directory[table_index] & (~0xfff)));
		for (uint32_t page_index = 0; page_index < 1024; page_index++) {
			if (isPresent(table[page_index])) {
				if ((table[page_index] & (~0xfff)) == ((uint32_t)p & (~0xfff))) {
					return (virt_addr)((1024 * PAGE_SIZE) * table_index + PAGE_SIZE * page_index);
				}
			}
		}
	}
	return (virt_addr)-1;
}

void PageDirectory::install()
{
	active_page_dir = this;
	uint32_t ptr = (uint32_t)findPhysicalAddr((virt_addr)directory);
	__asm__ volatile("movl %0, %%cr3\n" ::"r"(ptr));
}

bool PageDirectory::isActive() { return this == active_page_dir; }

void PageDirectory::installRegion(shared_ptr<MemoryRegion> region,
	virt_addr starting_addr)
{
	regions.add({ .region = region, .base = starting_addr });
	region -> install(*this, starting_addr);
}

void PageDirectory::removeRegion(shared_ptr<MemoryRegion> region)
{
	auto node = regions.head();
	while (node != regions.ending()) {
		auto oldNode = node;
		node = node->next();
		if (oldNode->value.region == region) {
			region -> remove(*this, oldNode->value.base);
			regions.remove(oldNode);
			return;
		}
	}
	assert(false, "Error: tried to remove region not present in page directory");
}

virt_addr PageDirectory::getRegionBase(MemoryRegion& region)
{
	for (auto& node : regions) {
		if (&*(node.region) == &region)
			return node.base;
	}
	assert(false,
		"Error: tried to find base of region not present in page directory");
	return NULL;
}

void PageDirectory::addPageTable(page_table* ptr, virt_addr base,
	uint32_t flags, TLBInvalidationType invtype)
{
	uint32_t directory_index = ((uint32_t)base / (1024 * PAGE_SIZE));
	assert(((uint32_t)base % (PAGE_SIZE * 1024)) == 0,
		"Error: misaligned base address");
	uint32_t paddr = (uint32_t)getPhysicalAddr((virt_addr)ptr);
	assert(paddr % PAGE_SIZE == 0, "Error: misaligned page table");
	uint32_t entry = (paddr & (~0xfff)) | flags;
	directory[directory_index] = entry;
	for (uint32_t i = (uint32_t)base; i < (uint32_t)base + (1024 * PAGE_SIZE); i += PAGE_SIZE) {
		invalidateMappingIfNecessary((virt_addr)i, invtype);
	}
}

void PageDirectory::removePageTables(virt_addr base, size_t region_size, TLBInvalidationType invtype)
{
	assert((uint32_t)base % (PAGE_SIZE * 1024) == 0,
		"Error: misaligned base address");
	for (uint32_t i = (uint32_t)base; i < (uint32_t)base + region_size;
		 i += 1024 * PAGE_SIZE) {
		uint32_t index = i / (1024 * PAGE_SIZE);
		directory[index] = 0;
		if (invtype == TLBInvalidationType::INVLPG) {
			for (uint32_t j = i; j < j + (1024 * PAGE_SIZE); j += PAGE_SIZE) {
				invalidateMappingIfNecessary((virt_addr)j, invtype);
			}
		}
	}
	if (invtype == TLBInvalidationType::FULL_FLUSH) {
		invalidateMappingIfNecessary(base, invtype);
	}
}

virt_addr PageDirectory::findSpaceBelow(size_t size, virt_addr addr,
	bool align_at_pt)
{
	assert(false, "Unimplemented");
	return NULL;
}

bool PageDirectory::isMapped(virt_addr addr)
{
	uint32_t directoryIndex = ((uint32_t)addr >> 22) & 0x3ff;
	uint32_t tableIndex = ((uint32_t)addr >> 12) & 0x3ff;
	if (!isPresent(directory[directoryIndex]))
		return false;
	uint32_t* table = physicalToPageTableAddr(
		(phys_addr)(directory[directoryIndex] & (~0xfff)));
	return isPresent(table[tableIndex]);
}

virt_addr PageDirectory::findSpaceAbove(size_t required, virt_addr after,
	bool align_at_pt)
{
	uint32_t checking = (uint32_t)after;
	size_t found = 0;
	virt_addr base = after;
	if (align_at_pt) {
		if ((uint32_t)after % (1024 * PAGE_SIZE) != 0) {
			after = (virt_addr)((uint32_t)after + ((uint32_t)after % (1024 * PAGE_SIZE)));
		}
		base = after;
		checking = (uint32_t)after;
		while (checking >= (uint32_t)after) {
			uint32_t pt_index = (checking) / (1024 * PAGE_SIZE);
			if (isPresent(directory[pt_index])) {
				found = 0;
			} else {
				if (found == 0) {
					base = (virt_addr)checking;
				}
				found += 1024 * PAGE_SIZE;
				if (found >= required) {
					return base;
				}
			}
			checking += 1024 * PAGE_SIZE;
		}
	}
	while (checking >= (uint32_t)after) {
		if (isMapped((virt_addr)checking)) {
			found = 0;
		} else {
			if (found == 0) {
				base = (virt_addr)checking;
			}
			found += PAGE_SIZE;
			if (found >= required) {
				return base;
			}
		}
		checking += PAGE_SIZE;
	}
	return (virt_addr)-1;
}

void PageDirectory::copyRegionsInto(PageDirectory& pd)
{
	for (auto& placement : regions) {
		pd.installRegion(placement.region, placement.base);
	}
}

void PageDirectory::invalidateMappingIfNecessary(virt_addr addr, TLBInvalidationType type)
{
	if (this == MemoryManager::active_page_dir) {
		switch (type) {
		case TLBInvalidationType::NONE:
			return;
		case TLBInvalidationType::INVLPG:
			__asm__ volatile("invlpg %0" ::"m"(addr));
			return;
		case TLBInvalidationType::FULL_FLUSH:
			this->install();
		}
	}
}

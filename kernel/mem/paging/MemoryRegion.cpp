#include <devices/SerialDevice.h>
#include <interrupts.h>
#include <paging.h>

using namespace MemoryManager;

MemoryRegion::MemoryRegion() {}

MemoryRegion::~MemoryRegion() { SD::the() << "killing region\n"; }

PhysicalMemoryRegion::PhysicalMemoryRegion(Vector<page_table*> pt, size_t s,
	uint32_t first_entry, bool perm, TLBInvalidationType invtype,
	uint32_t flgs)
	: ptables(pt)
	, size(s)
	, offset(first_entry)
	, permanent(perm)
	, inv_type(invtype)
	, flags(flgs)
{
	assert(size % PAGE_SIZE == 0, "Error: misaligned memory region size");
	assert(offset % PAGE_SIZE == 0, "Error: misaligned offset");
}

PhysicalMemoryRegion::~PhysicalMemoryRegion() {}

void PhysicalMemoryRegion::install(PageDirectory& dir, virt_addr base)
{
	assert(((uint32_t)base & 0x3fffff) == 0,
		"Error: misaligned starting address");
	virt_addr at = base;
	auto type = inv_type;
	if (type == TLBInvalidationType::FULL_FLUSH) {
		type = TLBInvalidationType::NONE;
	}
	for (uint32_t i = 0; i < ptables.size(); i++) {
		dir.addPageTable(ptables[i], at, flags, type);
		at = (virt_addr)((uint32_t)at + PAGE_SIZE * 1024);
	}
	if (inv_type == TLBInvalidationType::FULL_FLUSH) {
		dir.invalidateMappingIfNecessary(NULL, TLBInvalidationType::FULL_FLUSH);
	}
}

void PhysicalMemoryRegion::remove(PageDirectory& dir, virt_addr base)
{
	dir.removePageTables(base, size, inv_type);
}

void PhysicalMemoryRegion::handlePageFault(uint32_t offset)
{
	assert(false, "Unimplemented");
}

uint32_t PhysicalMemoryRegion::mapPage(phys_addr base)
{
	assert(size % PAGE_SIZE == 0,
		"Error: somehow size is misaligned - I blame the pageframe allocator");
	assert((uint32_t)base % PAGE_SIZE == 0, "Error: misaligned base");
	uint32_t pages = size / PAGE_SIZE;
	size += PAGE_SIZE;
	if (pages % 1024 == 0) {
		ptables.push(allocatePageTable());
	}
	page_table& table = *(ptables[ptables.size() - 1]);
	uint32_t index = pages % 1024;
	table[index] = ((uint32_t)base & (~0xfff)) | flags & 0x3;
	return size - PAGE_SIZE;
}

uint32_t PhysicalMemoryRegion::mapContiguousRegion(phys_addr base, size_t sz)
{
	DisableInterrupts d;
	for (uint32_t addr = (uint32_t)base; addr < (uint32_t)base + sz;
		 addr += PAGE_SIZE) {
		mapPage((phys_addr)addr);
	}
	return size - sz;
}

PrintStream& operator<<(PrintStream& p, PhysicalMemoryRegion& mr)
{
	p << "PhysicalMemoryRegion (size " << (void*)mr.size << ")\n";
	for (int i = 0; i < mr.ptables.size(); i++) {
		p << mr.ptables[i] << "\n";
	}
	return p;
}

CompositeMemoryRegion::CompositeMemoryRegion(uint32_t f)
	: flags(f)
{
}

CompositeMemoryRegion::~CompositeMemoryRegion() {}

void CompositeMemoryRegion::install(PageDirectory& dir, virt_addr)
{
	for (uint32_t i = 0; i < ptables.size(); i++) {
		auto table = ptables[i];
		dir.addPageTable(table.a, table.b, flags);
	}
}

void CompositeMemoryRegion::remove(PageDirectory&, virt_addr)
{
	assert(false, "Unimplemented");
}

void CompositeMemoryRegion::handlePageFault(uint32_t)
{
	assert(false, "Unimplemented");
}

// TODO when we add a region, we should somehow free its old page tables without
// freeing the pages themselves
void CompositeMemoryRegion::addRegion(PhysicalMemoryRegion& region,
	virt_addr base)
{
	assert((uint32_t)base % (1024 * PAGE_SIZE) == 0, "Error: misaligned base");
	for (uint32_t i = 0; i < region.ptables.size(); i++) {
		uint32_t addr = (uint32_t)base + i * PAGE_SIZE * 1024;
		if (!tbl_map.contains(addr)) {
			page_table* ptbl = allocatePageTable();
			page_table* old_tbl = region.ptables[i];
			memcpy(ptbl, old_tbl, sizeof(page_table));
			ptables.push(Tuple<page_table*, virt_addr>(ptbl, (virt_addr)addr));
			tbl_map.put(addr, (uint32_t)ptbl);
		} else {
			page_table& ptbl = *(page_table*)tbl_map.get(addr);
			page_table& old_tbl = *region.ptables[i];
			for (uint32_t j = 0; j < 1024; j++) {
				if ((old_tbl[j] & PAGE_PRESENT) != 0) {
					ptbl[j] = old_tbl[j];
				}
			}
		}
	}
}

void CompositeMemoryRegion::setFlags(virt_addr addr, uint32_t flags)
{
	assert((uint32_t)addr % PAGE_SIZE == 0, "Error: misaligned address");
	uint32_t base = (uint32_t)addr - ((uint32_t)addr % (1024 * PAGE_SIZE));
	assert(tbl_map.contains(base),
		"Error: tried to set flags of non-existent entry");
	page_table& ptbl = *(page_table*)tbl_map.get(base);
	uint32_t i = ((uint32_t)addr / PAGE_SIZE) % 1024;
	ptbl[i] = (ptbl[i] & (~0xfff)) | flags;
}

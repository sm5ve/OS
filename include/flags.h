#ifndef KFLAGS
#define KFLAGS

//#define FRAMEBUFFER
#define KERNEL_STACK_SIZE   (64   * 1024)
#define KERNEL_SLAB_8_SIZE  (64  * 1024)
#define KERNEL_SLAB_16_SIZE (64  * 1024)
#define KERNEL_SLAB_32_SIZE (64  * 1024)
#define KERNEL_SLAB_64_SIZE (256  * 1024)
#define KERNEL_HEAP_SIZE    (1024 * 1024)
#define KERNEL_NOFREE_SIZE  (4096 * 1024)
#define PAGE_SIZE 4096
#define PALLOC_MULTIPLIER 1
#define KERNEL_HEAP_GRANULARITY 128
//#define USE_MULTIBOOT_2

#endif

#Unnamed OS

The goal of this project is to create a basic kernel capable of loading ELF binaries from an EXT2 filesystem and then running them in userspace within the next two months. So far, the kernel has the following implemented:

* Basic output through COM1
* A slab and heap allocator
* A basic flat GDT
* Basic interrupt support

Important milestones I hope to achieve in the near-ish future are (in roughly the order I wish to accomplish them):

* Writing a basic interrupt handling framework
* Switching to a higher-half kernel
* Basic scheduler (kernel tasks only, for now)



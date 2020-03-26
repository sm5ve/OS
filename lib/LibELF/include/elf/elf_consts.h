#ifndef ELF_CONSTS
#define ELF_CONSTS

#define ELF_TYPE_NONE        0
#define ELF_TYPE_RELOCATABLE 1
#define ELF_TYPE_EXEC        2
#define ELF_TYPE_DYNAMIC     3
#define ELF_TYPE_CORE        4

#define ELF_ARCH_NONE        0
#define ELF_ARCH_M32         1
#define ELF_ARCH_SPARC       2
#define ELF_ARCH_386         3
#define ELF_ARCH_68K         4
#define ELF_ARCH_88K         5
#define ELF_ARCH_860         7
#define ELF_ARCH_MIPS        8
#define ELF_ARCH_S370        9
#define ELF_ARCH_MIPS_RS3_LE 10
#define ELF_ARCH_PARISC      15
#define ELF_ARCH_EM_VPP500   17
#define ELF_ARCH_SPARC32PLUS 18
#define ELF_ARCH_960         19
#define ELF_ARCH_PPC         20
#define ELF_ARCH_PPC64       21
#define ELF_ARCH_S390        22
#define ELF_ARCH_V800        36
#define ELF_ARCH_FR20        37
#define ELF_ARCH_RH32        38
#define ELF_ARCH_RCE         39
#define ELF_ARCH_ARM         40
#define ELF_ARCH_ALPHA       41
#define ELF_ARCH_SH          42
#define ELF_ARCH_SPARCV9     43
#define ELF_ARCH_TRICORE     44
#define ELF_ARCH_ARC         45
#define ELF_ARCH_H8_300      46
#define ELF_ARCH_H8_300H     47
#define ELF_ARCH_H8S         48
#define ELF_ARCH_H8_500      49
#define ELF_ARCH_IA_64       50
#define ELF_ARCH_MIPS_X      51
#define ELF_ARCH_COLDFIRE    52
#define ELF_ARCH_68HC12      53
#define ELF_ARCH_MMA         54
#define ELF_ARCH_PCP         55
#define ELF_ARCH_NCPU        56
#define ELF_ARCH_NDR1        57
#define ELF_ARCH_STARCORE    58
#define ELF_ARCH_ME16        59
#define ELF_ARCH_ST100       60
#define ELF_ARCH_TINYJ       61
#define ELF_ARCH_X86_64      62
#define ELF_ARCH_PDSP        63
#define ELF_ARCH_PDP10       64
#define ELF_ARCH_PDP11       65
#define ELF_ARCH_FX66        66
#define ELF_ARCH_ST9PLUS     67
#define ELF_ARCH_ST7         68
#define ELF_ARCH_68HC16      69
#define ELF_ARCH_68HC11      70
#define ELF_ARCH_68HC08      71
#define ELF_ARCH_68HC05      72
#define ELF_ARCH_SVX         73
#define ELF_ARCH_ST19        74
#define ELF_ARCH_VAX         75
#define ELF_ARCH_CRIS        76
#define ELF_ARCH_JAVELIN     77
#define ELF_ARCH_FIREPATH    78
#define ELF_ARCH_ZSP         79
#define ELF_ARCH_MMIX        80
#define ELF_ARCH_HUANY       81
#define ELF_ARCH_PRISM       82
#define ELF_ARCH_AVR         83
#define ELF_ARCH_FR30        84
#define ELF_ARCH_D10V        85
#define ELF_ARCH_D30V        86
#define ELF_ARCH_V850        87
#define ELF_ARCH_M32R        88
#define ELF_ARCH_MN10300     89
#define ELF_ARCH_MN10200     90
#define ELF_ARCH_PJ          91
#define ELF_ARCH_OPENRISC    92
#define ELF_ARCH_A5          93
#define ELF_ARCH_XTENSA      94
#define ELF_ARCH_VIDEOCORE   95
#define ELF_ARCH_TMM_GPP     96
#define ELF_ARCH_NS32K       97
#define ELF_ARCH_TPC         98
#define ELF_ARCH_SNP1K       99
#define ELF_ARCH_ST200       100 

#define ELF_CLASS_NONE       0
#define ELF_CLASS_32         1
#define ELF_CLASS_64         2

#define ELF_DATA_NONE        0
#define ELF_DATA2_LSB        1
#define ELF_DATA2_MSB        2

#define ELF_OSABI_SYSTEM_V   0
#define ELF_OSABI_HPUX       1
#define ELF_OSABI_NETBSD     2
#define ELF_OSABI_LINUX      3
#define ELF_OSABI_SOLARIS    4
#define ELF_OSABI_AIX        5
#define ELF_OSABI_IRIX       6
#define ELF_OSABI_FREEBSD    9
#define ELF_OSABI_TRU64      10
#define ELF_OSABI_MODESTO    11
#define ELF_OSABI_OPENBSD    12
#define ELF_OSABI_OPENVMS    13
#define ELF_OSABI_NSK        14

#define ELF_PT_NULL          0
#define ELF_PT_LOAD          1
#define ELF_PT_DYNAMIC       2
#define ELF_PT_INTERP        3
#define ELF_PT_NOTE          4
#define ELF_PT_SHLIB         5
#define ELF_PT_PHDR          6
#define ELF_PT_LOPROC        0x70000000
#define ELF_PT_HIPROC        0x7fffffff

#endif

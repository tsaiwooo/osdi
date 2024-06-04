#ifndef __MMU_H_
#define __MMU_H_

// TCR setting, armv8 P.2038
// 21~16bits set T1SZ and 5~0bits T0SZ to 16KB, and control low address and high address seperately
// 15~14bits set TG0 Granule size for the corresponding translation table base address register, 31~30bits TG1, bits [31:30] TTBR1_EL1 Granule size
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)
// MAIR setting
// armv8 P.1990 7~4bits -> device or normal memory and with cache or not (including its cache strategy)
// 3~0bits -> device and normal memory attribute
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
// Page table setting
#define MM_ACCESS_PERMISSION (0x01 << 6)
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10)
#define BOOT_PGD0_ATTR PD_TABLE
#define BOOT_PUD0_ATTR PD_TABLE
#define BOOT_PUD1_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_PMD0_ATTR PD_TABLE
#define BOOT_PTE_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define BOOT_PTE_DEVICE (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

#define PTE_FLAGS (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | MM_ACCESS_PERMISSION | PD_PAGE)

#define PAGE_SIZE 4096

#endif // __MMU_H_
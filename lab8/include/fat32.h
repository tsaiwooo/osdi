#ifndef __FAT32_H_
#define __FAT32_H_
#include "main.h"
#define uint8_t unsigned char
#define uint16_t unsigned short int
#define uint64_t unsigned long long
#define FAT_entry uint32_t
#define FAT_ENTRIES_PER_BLOCK (BLOCK_SIZE / sizeof(int))
#define MAX_ENTRIES 16
#define BLOCK_SIZE 512
#define DIR 0
#define FILE 1
#define FAT_BAD 0x0FFFFFF7
#define FAT_END 0x0FFFFFF8

struct file;
struct filesystem;
struct mount;
struct MBR_partition {
    uint8_t status;
    uint8_t FIRST_SEC_CHS_addr[3];
    uint8_t partition_type;
    uint8_t LAST_SEC_CHS_addr[3];
    uint32_t LBA;
    uint32_t NUMS_SEC;
} __attribute__((packed));

struct MBR {
    uint8_t content[446];
    struct MBR_partition entry[4];
    uint8_t signature[2];
} __attribute__((packed));

// BPB chrome-extension://bocbaocobfecmglnmeaeppambideimao/pdf/viewer.html?file=https%3A%2F%2Fwww.cs.fsu.edu%2F~cop4610t%2Fassignments%2Fproject3%2Fspec%2Ffatspec.pdf  p.9
struct fat32_bootsector {
    uint8_t jump_instruction[3]; // 0x000 - 0x002
    uint8_t oem_name[8]; // 0x003 - 0x00A
    uint16_t bytes_per_sector; // 0x00B - 0x00C
    uint8_t sectors_per_cluster; // 0x00D
    uint16_t num_of_reserved_sector; // 0x00E - 0x00F
    uint8_t num_of_fat; // 0x010 -> file allocation table
    uint16_t num_of_root_dir_entries; // 0x011 - 0x012
    uint16_t num_of_sectors; // 0x013 - 0x014
    uint8_t media_descriptor; // 0x015
    uint16_t sectors_per_fat; // 0x016 - 0x017 -> sectors of file allocation table
    uint16_t sectors_per_track; // 0x018 - 0x019
    uint16_t num_of_heads; // 0x01A - 0x01B
    uint32_t num_of_hidden_sectors; // 0x01C - 0x01F
    uint32_t num_of_sectors_fat32; // 0x020 - 0x023
    uint32_t sectors_per_fat_fat32; // 0x024 - 0x027
    uint16_t mirror_flags; // 0x028 - 0x029
    uint16_t version; // 0x02A - 0x02B
    uint32_t first_cluster_of_root_dir; // 0x02C - 0x02F
    uint16_t info_sector; // 0x030 - 0x031
    uint16_t back_up_boot_sectors; // 0x032 - 0x033
    uint32_t reserved[3]; // 0x034 - 0x03F
    uint8_t physical_drive_number; // 0x040
    uint8_t for_various_purpose; // 0x041
    uint8_t extended_signature; // 0x042
    uint32_t volume_id; // 0x043 - 0x046
    uint8_t volume_label[11]; // 0x047 - 0x051
    uint64_t filesystem_type; // 0x052 - 0x059
} __attribute__((packed));

// RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec;
// FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors;
// FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
struct fat32_metadata {
    int num_of_reserved_sector;
    int num_of_fat;
    int sectors_per_fat;
    uint32_t first_cluster_of_root_dir;
    uint8_t sectors_per_cluster;
    uint32_t fat_tb_begin_lba;
    uint32_t data_begin_lba;
    uint16_t bytes_per_sector;
} __attribute__((packed));

struct fat32_data {
    char name[256];
    int size;
    // char* type;
    unsigned int first_cluster;
    void* data;
    // struct vnode* vnode;
    struct fat32_data* parent;
};

// struct FAT_entries {
//     unsigned int blk_id;
//     short cached;
//     // uint8_t block[BLOCK_SIZE];
// };

struct FAT {
    uint32_t* table;
};

struct FAT* fat;

struct fat32_directory_entry {
    char filename[8]; // 文件名
    char ext[3]; // 擴展名
    uint8_t attributes; // 屬性
    uint8_t reserved; // 保留
    uint8_t creation_time_tenths; // 創建時間（10 毫秒單位）
    uint16_t creation_time; // 創建時間
    uint16_t creation_date; // 創建日期
    uint16_t last_access_date; // 最後訪問日期
    uint16_t first_cluster_high; // 首簇號（高 16 位）
    uint16_t write_time; // 最後寫入時間
    uint16_t write_date; // 最後寫入日期
    uint16_t first_cluster_low; // 首簇號（低 16 位）
    uint32_t file_size; // 文件大小（字節）
} __attribute__((packed));

struct fat32_file_internal {
    char* data;
    size_t size;
};

struct fat32_dir_internal {
    struct fat32_directory_entry* entries[MAX_ENTRIES];
};

void fat32_init();
int fat32_setup_mount(struct filesystem* fs, struct mount* mount);
struct vnode* fat32_vnode();
struct fat32_data* fat32_new_data(int type, const char* name, unsigned int size, unsigned int cluster_id, struct fat32_data* parent);
int fat32_read(struct file* file, void* buf, size_t len);
int fat32_write(struct file* file, const void* buf, size_t len);
int fat32_open(struct vnode* file_node, struct file** target);
int fat32_close(struct file* file);
int fat32_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name);
int fat32_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);
int fat32_create(struct vnode* dir_node, struct vnode** target, const char* component_name);

int read_directory_entry(uint32_t dir_cluster, const char* name, struct fat32_directory_entry* entry);
int write_directory_entry(uint32_t dir_cluster, struct fat32_directory_entry* entry);

uint32_t get_block_idx(uint32_t cluster_number);
uint32_t get_fat_blk_idx(uint32_t cluster_idx);
uint32_t allocate_new_cluster();
#endif // __FAT32_H_
#include "fat32.h"

struct fat32_metadata fat32_metadata;

void fat32_init()
{
    // parse first partition
    char buf[BLOCK_SIZE];
    int start_lba = 2048;
    readblock(start_lba, buf);

    struct fat32_bootsector* fat32_bs = (struct fat32_bootsector*)buf;

    // fat32 metadata store
    fat32_metadata.num_of_reserved_sector = fat32_bs->num_of_reserved_sector;
    fat32_metadata.num_of_fat = fat32_bs->num_of_fat;
    fat32_metadata.first_cluster_of_root_dir = fat32_bs->first_cluster_of_root_dir;
    fat32_metadata.sectors_per_cluster = fat32_bs->sectors_per_cluster;
    fat32_metadata.sectors_per_fat = fat32_bs->sectors_per_fat_fat32;
    fat32_metadata.fat_tb_begin_lba = fat32_metadata.num_of_reserved_sector + start_lba;
    fat32_metadata.data_begin_lba = fat32_metadata.fat_tb_begin_lba + (fat32_metadata.num_of_fat * fat32_metadata.sectors_per_fat);
    fat32_metadata.bytes_per_sector = fat32_bs->bytes_per_sector;

    // fat create
    fat = (struct FAT*)kmalloc(sizeof(struct FAT));
    fat->table = (uint32_t*)kmalloc(sizeof(uint32_t) * fat32_metadata.sectors_per_fat * fat32_bs->bytes_per_sector);
    for (int i = 0; i < fat32_metadata.sectors_per_fat; ++i) {
        readblock(fat32_metadata.fat_tb_begin_lba + i, (void*)&fat->table[i * (BLOCK_SIZE / sizeof(uint32_t))]);
    }

    // fat32 create
    struct filesystem* fs = (struct filesystem*)kmalloc(sizeof(struct filesystem));
    fs->name = kmalloc(sizeof(char) * 16);
    strcpy(fs->name, "fat32");
    fs->setup_mount = &fat32_setup_mount;
    register_filesystem(fs);
    vfs_mkdir("/boot");
    vfs_mount("/boot", "fat32");
}

int fat32_setup_mount(struct filesystem* fs, struct mount* mount)
{
    mount->fs = fs;
    struct vnode* root = fat32_vnode();
    struct fat32_data* fat32_data = fat32_new_data(DIR, "/", 0, fat32_metadata.first_cluster_of_root_dir, NULL);

    root->internal = fat32_data;
    root->type = VNODE_TYPE_DIRECTORY;
    mount->root = root;

    return 0;
}

struct fat32_data* fat32_new_data(int type, const char* name, unsigned int size, unsigned int cluster_id, struct fat32_data* parent)
{
    struct fat32_data* tmp = (struct fat32_data*)kmalloc(sizeof(struct fat32_data));
    strcpy(tmp->name, name);
    tmp->size = size;
    tmp->first_cluster = cluster_id;
    tmp->parent = parent;
    if (type == DIR) {
        struct fat32_dir_internal* tmp_dir;
        tmp_dir = (struct fat32_dir_internal*)kmalloc(sizeof(struct fat32_dir_internal));
        for (int i = 0; i < MAX_ENTRIES; ++i)
            tmp_dir->entries[i] = NULL;
        tmp->data = (void*)tmp_dir;
    } else if (type == FILE) {
        struct fat32_file_internal* tmp_file;
        tmp_file = (struct fat32_file_internal*)kmalloc(sizeof(struct fat32_file_internal));
        tmp_file->size = 0;
        tmp_file->data = kmalloc(PAGE_SIZE);
        tmp->data = (void*)tmp_file;
    }
    return tmp;
}

struct vnode* fat32_vnode()
{
    struct vnode* vtmp = (struct vnode*)kmalloc(sizeof(struct vnode));
    struct vnode_operations* fat32_v_op = (struct vnode_operations*)kmalloc(sizeof(struct vnode_operations));
    struct file_operations* fat32_f_op = (struct file_operations*)kmalloc(sizeof(struct file_operations));
    fat32_v_op->create = &fat32_create;
    fat32_v_op->lookup = &fat32_lookup;
    fat32_v_op->mkdir = &fat32_mkdir;
    fat32_f_op->read = &fat32_read;
    fat32_f_op->write = &fat32_write;
    fat32_f_op->open = &fat32_open;
    fat32_f_op->close = &fat32_close;
    vtmp->f_ops = fat32_f_op;
    vtmp->v_ops = fat32_v_op;
    vtmp->mount = NULL;
    return vtmp;
}

int fat32_read(struct file* file, void* buf, size_t len)
{
    struct fat32_data* file_data = (struct fat32_data*)file->vnode->internal;
    uint32_t cur_cluster = file_data->first_cluster;
    uint32_t file_offset = file->f_pos;
    uint32_t bytes_2_read = len;
    uint8_t fat[BLOCK_SIZE];
    uint8_t tmp_buf[BLOCK_SIZE];

    uint32_t bytes_read = 0;
    while (bytes_read < bytes_2_read && cur_cluster >= fat32_metadata.first_cluster_of_root_dir && cur_cluster != FAT_END) {
        readblock(get_block_idx(cur_cluster), tmp_buf);

        int chunk_size = (bytes_2_read - bytes_read) < BLOCK_SIZE ? (bytes_2_read - bytes_read) : BLOCK_SIZE;
        memcpy(buf + bytes_read, tmp_buf, chunk_size);

        bytes_read += chunk_size;
        file_offset += chunk_size;

        if (chunk_size < BLOCK_SIZE) {
            bytes_read -= chunk_size;
            bytes_read += strlen(buf);
            break;
        }

        readblock(get_fat_blk_idx(cur_cluster), fat);
        cur_cluster = ((uint32_t*)fat)[cur_cluster % FAT_ENTRIES_PER_BLOCK];
    }

    file->f_pos = file_offset;
    return bytes_read;
}

// int fat32_write(struct file* file, const void* buf, size_t len)
// {
//     struct fat32_data* file_data = (struct fat32_data*)file->vnode->internal;
//     uint32_t cur_cluster = file_data->first_cluster;
//     uint32_t file_offset = file->f_pos;
//     uint32_t bytes_2_write = len;
//     uint8_t fat[BLOCK_SIZE];
//     uint8_t tmp_buf[BLOCK_SIZE];

//     uint32_t bytes_written = 0;
//     uint32_t prev_cluster = 0;

//     if (cur_cluster == 0) { // 如果檔案還沒有分配簇
//         cur_cluster = allocate_new_cluster();
//         if (cur_cluster == FAT_BAD) {
//             return -1; // 無剩餘空間
//         }
//         file_data->first_cluster = cur_cluster; // 為檔案分配第一個簇
//     }

//     while (bytes_written < bytes_2_write) {
//         int chunk_size = (bytes_2_write - bytes_written) < BLOCK_SIZE ? (bytes_2_write - bytes_written) : BLOCK_SIZE;
//         memcpy(tmp_buf, buf + bytes_written, chunk_size);

//         if (chunk_size < BLOCK_SIZE) { // 如果寫入的不是整個塊，先讀取當前塊
//             readblock(get_block_idx(cur_cluster), tmp_buf);
//             memcpy(tmp_buf + file_offset % BLOCK_SIZE, buf + bytes_written, chunk_size);
//         }

//         writeblock(get_block_idx(cur_cluster), tmp_buf);
//         bytes_written += chunk_size;
//         file_offset += chunk_size;

//         if (file_offset % BLOCK_SIZE == 0 || bytes_written == bytes_2_write) { // 如果到達簇的末端或所有字節都已寫入
//             readblock(get_fat_blk_idx(cur_cluster), fat);
//             uint32_t next_cluster = ((uint32_t*)fat)[cur_cluster % FAT_ENTRIES_PER_BLOCK];
//             if (next_cluster == FAT_END) {
//                 next_cluster = allocate_new_cluster();
//                 if (next_cluster == FAT_BAD) {
//                     return -1; // 無剩餘空間
//                 }
//                 ((uint32_t*)fat)[cur_cluster % FAT_ENTRIES_PER_BLOCK] = next_cluster;
//                 writeblock(get_fat_blk_idx(cur_cluster), fat); // 更新磁盤上的FAT
//             }
//             cur_cluster = next_cluster;
//         }
//     }

//     file->f_pos = file_offset; // 更新檔案位置
//     if (file_data->size < file_offset) {
//         file_data->size = file_offset; // 如果檔案增大，更新檔案大小
//     }
//     return bytes_written;
// }

int fat32_write(struct file* file, const void* buf, size_t len)
{
    struct fat32_data* file_data = (struct fat32_data*)file->vnode->internal;
    uint32_t cur_cluster = file_data->first_cluster;
    uint32_t file_offset = file->f_pos;
    uint32_t bytes_2_write = len;
    uint8_t tmp_buf[BLOCK_SIZE];
    uint32_t bytes_written = 0;
    uint32_t prev_cluster = 0;

    // If the file is empty and doesn't have a cluster allocated, allocate one
    if (cur_cluster == 0) {
        cur_cluster = allocate_new_cluster();
        if (cur_cluster == FAT_BAD) {
            return -1; // No space left
        }
        file_data->first_cluster = cur_cluster;
    }

    while (bytes_written < bytes_2_write) {
        if (cur_cluster == FAT_END) {
            // Allocate new cluster
            cur_cluster = allocate_new_cluster();
            if (cur_cluster == FAT_BAD) {
                return -1; // No space left
            }

            if (prev_cluster != 0) {
                // Update FAT table to link the previous cluster to the new one
                uint8_t fat[BLOCK_SIZE];
                readblock(get_fat_blk_idx(prev_cluster), fat);
                ((uint32_t*)fat)[prev_cluster % FAT_ENTRIES_PER_BLOCK] = cur_cluster;
                writeblock(get_fat_blk_idx(prev_cluster), fat);
            }
        }

        readblock(get_block_idx(cur_cluster), tmp_buf);
        int chunk_size = (bytes_2_write - bytes_written) < BLOCK_SIZE ? (bytes_2_write - bytes_written) : BLOCK_SIZE;
        memcpy(tmp_buf, buf + bytes_written, chunk_size);
        // for (int i = chunk_size; i < BLOCK_SIZE; ++i)
        tmp_buf[chunk_size] = '\0';
        writeblock(get_block_idx(cur_cluster), tmp_buf);

        bytes_written += chunk_size;
        file_offset += chunk_size;

        if (chunk_size < BLOCK_SIZE) {
            break;
        }

        prev_cluster = cur_cluster;
        uint8_t fat[BLOCK_SIZE];
        readblock(get_fat_blk_idx(cur_cluster), fat);
        cur_cluster = ((uint32_t*)fat)[cur_cluster % FAT_ENTRIES_PER_BLOCK];
    }

    // If this was the last cluster, mark it as FAT_END
    if (prev_cluster != 0) {
        uint8_t fat[BLOCK_SIZE];
        readblock(get_fat_blk_idx(prev_cluster), fat);
        ((uint32_t*)fat)[prev_cluster % FAT_ENTRIES_PER_BLOCK] = FAT_END;
        writeblock(get_fat_blk_idx(prev_cluster), fat);
    }

    file->f_pos = file_offset;
    file_data->size = file_offset; // Update the file size to the new offset
    // update directoey file size
    // Write updated file size to directory entry
    struct fat32_directory_entry dir_entry;
    if (read_directory_entry(file_data->parent->first_cluster, file_data->name, &dir_entry) == 0) {
        // uart_printf("clus = %d, size = %d\n", file_data->parent->first_cluster, file_data->size);
        dir_entry.file_size = file_data->size;
        write_directory_entry(file_data->parent->first_cluster, &dir_entry);
    }

    return bytes_written;
}

int fat32_open(struct vnode* file_node, struct file** target)
{
    *target = (struct file*)kmalloc(sizeof(struct file));
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    (*target)->flags = RW;
    (*target)->vnode = file_node;
    return 0;
}

int fat32_close(struct file* file)
{
    return 0;
}

int fat32_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct fat32_data* dir_data = (struct fat32_data*)dir_node->internal;
    struct fat32_directory_entry new_entry;

    // memset(&new_entry, 0, sizeof(new_entry));
    // strncpy(new_entry.dir_name, component_name, sizeof(new_entry.dir_name));
    strncpy(new_entry.filename, component_name, 8);
    strncpy(new_entry.ext, component_name + 8, 3);
    new_entry.first_cluster_high = 0; // Modify as needed
    new_entry.first_cluster_low = allocate_new_cluster();
    new_entry.file_size = 0;
    new_entry.attributes = DIR;

    write_directory_entry(dir_data->first_cluster, &new_entry);

    *target = fat32_vnode();
    struct fat32_data* new_dir_data = fat32_new_data(DIR, component_name, 0, new_entry.first_cluster_low, dir_data);
    (*target)->internal = new_dir_data;

    return 0;
}

int fat32_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct fat32_data* dir_data = (struct fat32_data*)dir_node->internal;
    struct fat32_directory_entry dir_entry;

    if (read_directory_entry(dir_data->first_cluster, component_name, &dir_entry) == 0) {
        *target = fat32_vnode();
        struct fat32_data* file_data = fat32_new_data(dir_entry.attributes, component_name, dir_entry.file_size, dir_entry.first_cluster_low, dir_data);
        (*target)->internal = file_data;
        return 0;
    }

    return -1;
}

// int fat32_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
// {
//     struct fat32_data* dir_data = (struct fat32_data*)dir_node->internal;
//     struct fat32_directory_entry new_entry;

//     // memset(&new_entry, 0, sizeof(new_entry));
//     // strncpy(new_entry.dir_name, component_name, sizeof(new_entry.dir_name));
//     strncpy(new_entry.filename, component_name, 8);
//     strncpy(new_entry.ext, component_name + 8, 3);
//     int idx = 0;
//     for (; idx < 8; ++idx) {
//         if (component_name[idx] == '.') {
//             break;
//         } else
//             new_entry.filename[idx] = component_name[idx];
//     }
//     new_entry.filename[idx++] = '\0';
//     for (int ext = 0; ext < 3; ++ext, ++idx) {
//         if (component_name[idx] == '.') {
//             break;
//         } else
//             new_entry.ext[ext] = component_name[idx];
//     }
//     uart_printf("filename = %s\n", new_entry.filename);
//     uart_printf("ext = %s\n", new_entry.ext);
//     new_entry.first_cluster_high = 0; // Modify as needed
//     new_entry.first_cluster_low = allocate_new_cluster();
//     new_entry.file_size = 0;
//     new_entry.attributes = FILE;

//     write_directory_entry(dir_data->first_cluster, &new_entry);

//     *target = fat32_vnode();
//     struct fat32_data* new_file_data = fat32_new_data(FILE, component_name, 0, new_entry.first_cluster_low, dir_data);
//     (*target)->internal = new_file_data;

//     return 0;
// }

int fat32_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct fat32_data* dir_data = (struct fat32_data*)dir_node->internal;
    struct fat32_directory_entry new_entry;

    // memset(&new_entry, 0, sizeof(new_entry));
    int i;
    for (i = 0; i < 8 && component_name[i] != '.' && component_name[i] != '\0'; ++i) {
        new_entry.filename[i] = component_name[i];
    }
    for (int j = i; j < 8; ++j) {
        new_entry.filename[j] = ' ';
    }
    if (component_name[i] == '.') {
        ++i;
        for (int j = 0; j < 3 && component_name[i] != '\0'; ++j, ++i) {
            new_entry.ext[j] = component_name[i];
        }
    }
    for (int j = i; j < 3; ++j) {
        new_entry.ext[j] = ' ';
    }

    new_entry.first_cluster_high = 0; // Modify as needed
    // int x = 10;
    // while (x--)
    new_entry.first_cluster_low = allocate_new_cluster();
    new_entry.file_size = 0;
    new_entry.attributes = 0x04;
    if (write_directory_entry(dir_data->first_cluster, &new_entry) < 0) {
        return -1;
    }

    *target = fat32_vnode();
    struct fat32_data* new_file_data = fat32_new_data(FILE, component_name, 0, new_entry.first_cluster_low, dir_data);
    (*target)->internal = new_file_data;

    return 0;
}

// get absolute clus number
uint32_t get_block_idx(uint32_t cluster_number)
{
    return fat32_metadata.data_begin_lba + (cluster_number - 2) * fat32_metadata.sectors_per_cluster;
}

uint32_t get_fat_blk_idx(uint32_t cluster_idx)
{
    return fat32_metadata.fat_tb_begin_lba + (cluster_idx / FAT_ENTRIES_PER_BLOCK);
}

uint32_t allocate_new_cluster()
{
    for (uint32_t i = 2; i < fat32_metadata.sectors_per_fat * fat32_metadata.bytes_per_sector / sizeof(uint32_t); i++) {
        if (fat->table[i] == 0) {
            fat->table[i] = FAT_END;
            return i;
        }
    }
    return FAT_BAD;
}

int write_directory_entry(uint32_t dir_cluster, struct fat32_directory_entry* entry)
{
    uint8_t tmp_buf[BLOCK_SIZE];
    readblock(get_block_idx(dir_cluster), tmp_buf);
    struct fat32_directory_entry* dir_entries = (struct fat32_directory_entry*)tmp_buf;

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (dir_entries[i].filename[0] == 0 || dir_entries[i].filename[0] == 0xE5) {
            // if (dir_entries[i].filename[0] == 0xE5) {
            memcpy(&dir_entries[i], entry, sizeof(struct fat32_directory_entry));
            writeblock(get_block_idx(dir_cluster), tmp_buf);
            return 0;
        }
    }

    return -1; // No space left in the directory
}

int read_directory_entry(uint32_t dir_cluster, const char* name, struct fat32_directory_entry* entry)
{
    uint8_t tmp_buf[BLOCK_SIZE];
    readblock(get_block_idx(dir_cluster), tmp_buf);
    struct fat32_directory_entry* dir_entries = (struct fat32_directory_entry*)tmp_buf;

    for (int i = 0; i < MAX_ENTRIES; i++) {
        char sfn[12];
        int idx;
        for (idx = 0; idx < 8; ++idx) {
            if (dir_entries[i].filename[idx] != ' ')
                sfn[idx] = dir_entries[i].filename[idx];
            else
                break;
        }
        sfn[idx++] = '.';
        for (int ext_len = 0; ext_len < 3; ++ext_len) {
            if (dir_entries[i].ext[ext_len] != ' ')
                sfn[idx++] = dir_entries[i].ext[ext_len];
            else
                break;
        }
        if (strncmp(sfn, name, strlen(name)) == 0) {
            memcpy(entry, &dir_entries[i], sizeof(struct fat32_directory_entry));
            return 0;
        }
    }

    return -1; // Entry not found
}

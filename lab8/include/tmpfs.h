#ifndef __TMPFS_H_
#define __TMPFS_H_
#include "vfs.h"
#define RO 0
#define RW 1
struct mount;
struct file;
struct filesystem;

struct tmpfs_data {
    char name[256];
    int size;
    char* type;
    void* data;
};

struct tmpfs_file_data {
    char* data; // 文件的實際數據
    size_t size; // 文件大小
};

struct tmpfs_dir_data {
    struct vnode* entries[16]; // 目錄中的文件和子目錄
    char names[16][16]; // 對應的名稱
    int entry_counts;
};

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);
struct vnode* tmpfs_vnode();
int tmpfs_write(struct file* file, const void* buf, size_t len);
int tmpfs_read(struct file* file, void* buf, size_t len);
int tmpfs_open(struct vnode* file_node, struct file** target);
int tmpfs_close(struct file* file);
int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name);

#endif // __TMPFS_H_
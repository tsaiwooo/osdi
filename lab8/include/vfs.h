#ifndef __VFS_H_
#define __VFS_H_
#include "buddy_system.h"
#include "cpio.h"
#include "exception.h"
#include "thread.h"
#include "tmpfs.h"
#include "uart.h"
#define MAX_FILESYSTEMS 10
#define O_CREAT 0100 // Create file if it does not exist
#define O_RDONLY 0x0000 // Open for reading only
#define O_WRONLY 0x0001 // Open for writing only
#define O_RDWR 0x0002 // Open for reading and writing
#define VNODE_TYPE_FILE 1
#define VNODE_TYPE_DIRECTORY 2

struct thread;
struct vnode {
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    void* internal;
    int type;
    struct vnode* parent;
};

// file handle
struct file {
    struct vnode* vnode;
    size_t f_pos; // RW position of this file handle
    struct file_operations* f_ops;
    int flags;
};

struct mount {
    struct vnode* root;
    struct filesystem* fs;
};

struct filesystem {
    char* name;
    int (*setup_mount)(struct filesystem* fs, struct mount* mount);
    struct filesystem* next;
};

struct file_operations {
    int (*write)(struct file* file, const void* buf, size_t len);
    int (*read)(struct file* file, void* buf, size_t len);
    int (*open)(struct vnode* file_node, struct file** target);
    int (*close)(struct file* file);
};

struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target,
        const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target,
        const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target,
        const char* component_name);
};

struct cur_work {
    struct vnode* vnode;
    char dir_name[32];
};
struct cur_work* cur_work;
struct mount* rootfs;
struct filesystem* global_fs_list[MAX_FILESYSTEMS];
unsigned int filesystem_count;
void initramfs();
int get_free_fd(struct thread* cur);
struct filesystem* get_fs(const char* fs);
void rootfs_init();
int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);

int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
int vfs_chdir(const char* pathname);
#endif // __VFS_H_
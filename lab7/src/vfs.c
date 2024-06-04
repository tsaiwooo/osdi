#include "vfs.h"

struct vnode_operations* tmpfs_v_ops;
struct file_operations* tmpfs_f_ops;
void rootfs_init()
{
    // init file system
    filesystem_count = 0;
    rootfs = (struct mount*)kmalloc(sizeof(struct mount));

    // tmpfs create
    struct filesystem* fs = (struct filesystem*)kmalloc(sizeof(struct filesystem));
    fs->name = kmalloc(sizeof(char) * 16);
    strcpy(fs->name, "tmpfs");
    fs->setup_mount = &tmpfs_setup_mount;
    register_filesystem(fs);
    tmpfs_setup_mount(fs, rootfs);
}

int register_filesystem(struct filesystem* fs)
{
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    global_fs_list[filesystem_count] = (struct filesystem*)kmalloc(sizeof(struct filesystem));
    strcpy(global_fs_list[filesystem_count]->name, fs->name);
    global_fs_list[filesystem_count++] = fs;
    return 0;
}

int vfs_open(const char* pathname, int flags, struct file** target)
{
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    struct vnode* vnode;
    int ret = vfs_lookup(pathname, &vnode);
    if (ret == 0) {
        // Found vnode, open file
        vnode->f_ops->open(vnode, target);
    } else if (ret == -1 && (flags & O_CREAT)) {
        // Not found, create new file
        char* dir_path = strdup(pathname);
        char* file_name = strdup(pathname);
        dir_path = dirname(dir_path);
        file_name = basename(file_name);

        struct vnode* target_file;
        ret = vfs_lookup(dir_path, &vnode);
        ret = vnode->v_ops->create(vnode, &target_file, file_name);
        target_file->f_ops->open(target_file, target);
        // }

        // struct vnode* dir_vnode;
        // ret = vfs_lookup(dir_path, &dir_vnode);
        // if (ret == 0) {
        //     dir_vnode->v_ops->create(dir_vnode, &vnode, file_name);
        //     vnode->f_ops->open(vnode, target);
        // } else {
        //     return -1;
        // }
        kfree(dir_path);
        kfree(file_name);
    } else {
        return -1;
    }
    return 0;
}

int vfs_close(struct file* file)
{
    // 1. release the file descriptor
    return file->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, size_t len)
{
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len)
{
    // 1. read min(len, readable file data size) byte to buf from the opened file.
    // 2. return read size or error code if an error occurs.
    return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char* pathname)
{
    char* path_copy = strdup(pathname);
    char* dir_name_path = strdup(pathname);

    char* dir_path = dirname(path_copy);
    char* dir_name = basename(dir_name_path);

    struct vnode* dir_vnode;
    struct vnode* new_vnode;
    int ret = vfs_lookup(dir_path, &dir_vnode);
    // if (ret != 0 && !strtok(NULL, "/"))
    ret = dir_vnode->v_ops->mkdir(dir_vnode, &new_vnode, dir_name);
    kfree(dir_path);
    kfree(dir_name);
    kfree(path_copy);
    return ret;
}

// int vfs_mount(const char* target, const char* filesystem)
// {
//     struct filesystem* fs = get_fs(filesystem);
//     struct vnode* target_vnode;
//     vfs_lookup(target, &target_vnode);
//     struct mount* new_mount = (struct mount*)kmalloc(sizeof(struct mount));
//     // uart_printf("original_fs = %x\n", target_vnode->mount->fs);
//     // uart_printf("original_fs = %x\n", target_vnode->mount->fs);
//     target_vnode->mount = new_mount;
//     // new_mount->root = target_vnode;
//     fs->setup_mount(fs, new_mount);
//     new_mount->root = new_mount->root ? new_mount->root : target_vnode;

//     return 0;
// }

int vfs_mount(const char* target, const char* filesystem)
{
    struct filesystem* fs = get_fs(filesystem);
    if (!fs)
        return -1; // 文件系統未找到

    struct vnode* target_vnode;
    int ret = vfs_lookup(target, &target_vnode);
    if (ret != 0)
        return ret; // 目標節點查找失敗

    struct mount* new_mount = (struct mount*)kmalloc(sizeof(struct mount));
    if (!new_mount)
        return -1; // 內存分配失敗

    // new_mount->fs = fs;
    // new_mount->root = target_vnode; // 將新掛載點的根設置為目標節點
    fs->setup_mount(fs, new_mount);

    target_vnode->mount = new_mount; // 更新節點的掛載點參考
    /* init it internal */
    // ((struct tmpfs_dir_data*)target_vnode->internal)->entry_counts = 0;
    return 0;
}

int vfs_lookup(const char* pathname, struct vnode** target)
{
    struct vnode* current_vnode = rootfs->root;
    // uart_printf("root addr = %x\n", rootfs->root);
    char* path_copy = strdup(pathname);
    char* token = strtok(path_copy, "/");

    while (token != NULL) {
        // 遇到新的掛載點，換成新的vnode 然後往後執行，應該用if,if 還是用if,else if -> 用else if直接檢查接下來的file比較好？
        if (current_vnode->mount != NULL) {
            // change to new fs vnode
            current_vnode = current_vnode->mount->root;
        } else if (current_vnode->v_ops->lookup(current_vnode, target, token) != 0) {
            kfree(path_copy);
            return -1;
        }
        current_vnode = *target;
        token = strtok(NULL, "/");
    }
    kfree(path_copy);
    *target = current_vnode;
    return 0;
}

int get_free_fd(struct thread* cur)
{
    for (int i = 0; i < MAX_FD_SIZE; ++i) {
        if (!cur->fd[i])
            return i;
    }
    return -1;
}

struct filesystem* get_fs(const char* fs)
{
    for (int i = 0; i < filesystem_count; ++i) {
        if (strcmp(global_fs_list[i]->name, fs) == 0) {
            return global_fs_list[i];
        }
    }
    return NULL;
}
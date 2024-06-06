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

    // basic 3
    cur_work = (struct cur_work*)kmalloc(sizeof(struct cur_work));
    strcpy(cur_work->dir_name, "/");
    cur_work->vnode = rootfs->root;
}

int register_filesystem(struct filesystem* fs)
{
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    global_fs_list[filesystem_count] = (struct filesystem*)kmalloc(sizeof(struct filesystem));
    global_fs_list[filesystem_count]->name = kmalloc(32);
    // uart_printf("%d, %s, %x\n", filesystem_count, fs->name, global_fs_list[0]);
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
        // uart_printf("pathname = %s, dir_path = %s\n", pathname, dir_path);
        if (strcmp(dir_path, "..") == 0) {
            vnode = cur_work->vnode->parent;
        } else if (strcmp(dir_path, ".") == 0) {
            vnode = cur_work->vnode;
        } else {
            ret = vfs_lookup(dir_path, &vnode);
        }
        // if (ret == -1) {
        // initramfs bug here
        ret = vnode->v_ops->create(vnode, &target_file, file_name);
        target_file->f_ops->open(target_file, target);
        // }
        // }

        // struct vnode* dir_vnode;
        // ret = vfs_lookup(dir_path, &dir_vnode);
        // if (ret == 0) {
        //     dir_vnode->v_ops->create(dir_vnode, &vnode, file_name);
        //     vnode->f_ops->open(vnode, target);
        // } else {
        //     return -1;
        // }
        // kfree(dir_path);
        // kfree(file_name);
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
    // uart_printf("exit0\n");
    char* dir_name_path = strdup(pathname);
    // uart_printf("exit1\n");

    char* dir_path = dirname(path_copy);
    char* dir_name = basename(dir_name_path);
    // uart_printf("exit2\n");

    struct vnode* dir_vnode;
    struct vnode* new_vnode;
    int ret = vfs_lookup(dir_path, &dir_vnode);
    // uart_printf("exit3\n");
    // if (ret != 0 && !strtok(NULL, "/"))
    // uart_printf("v_ops addr = %x", dir_vnode->v_ops);
    ret = dir_vnode->v_ops->mkdir(dir_vnode, &new_vnode, dir_name);
    // kfree(dir_path);
    // kfree(dir_name);
    // kfree(path_copy);
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
    // basic 3
    new_mount->root->parent = target_vnode->parent;
    return 0;
}

int vfs_lookup(const char* pathname, struct vnode** target)
{
    // struct vnode* current_vnode = rootfs->root;
    struct vnode* current_vnode = cur_work->vnode;

    char* path_copy = strdup(pathname);
    // char* path_copy = kmalloc(64);
    // strncpy(path_copy, pathname, strlen(pathname));
    char* token = strtok(path_copy, "/");

    while (token != NULL) {
        // uart_printf("token = %s\n", token);
        // 遇到新的掛載點，換成新的vnode 然後往後執行，應該用if,if 還是用if,else if -> 用else if直接檢查接下來的file比較好？
        if (strcmp(token, ".") == 0) {
            // do nothing;
            // token = strtok(NULL, "/");
            // continue;
            ;
        } else if (strcmp(token, "..") == 0) {
            if (current_vnode->parent) {
                current_vnode = current_vnode->parent;
            }
            // token = strtok(NULL, "/");
            // continue;
        } else if (current_vnode->mount != NULL) {
            // change to new fs vnode
            current_vnode = current_vnode->mount->root;
            // token = strtok(NULL, "/");
            // continue;
        } else {
            if (current_vnode->v_ops->lookup(current_vnode, target, token) != 0) {
                // kfree(path_copy);
                return -1;
            }
            current_vnode = *target;
        }
        // current_vnode = *target;
        token = strtok(NULL, "/");
    }
    // kfree(path_copy);
    *target = current_vnode;
    return 0;
}

int vfs_chdir(const char* pathname)
{
    if (strcmp(pathname, "/") == 0) {
        cur_work->vnode = rootfs->root;
        strcpy(cur_work->dir_name, "/");
        return 0;
    }
    char* path_copy = strdup(pathname);
    char* dir_name_path = strdup(path_copy);

    char* dir_name = basename(dir_name_path);
    struct vnode* dir_vnode;
    // struct vnode* new_vnode;
    int ret = vfs_lookup(pathname, &dir_vnode);
    cur_work->vnode = dir_vnode;
    strcpy(cur_work->dir_name, dir_name);
    // if (ret != 0 && !strtok(NULL, "/"))
    // ret = dir_vnode->v_ops->mkdir(dir_vnode, &new_vnode, dir_name);
    // kfree(dir_name);
    // kfree(path_copy);
    return ret;
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

void initramfs()
{
    vfs_mkdir("/initramfs");
    // vfs_mount("/initramfs", "tmpfs");
    cpio_newc_header* header = CPIO_DEFAULT_PLACE;
    while (1) {
        // char new_path[32];
        char* new_path = kmalloc(64);
        unsigned long file_size = parse_hex(header->c_filesize, 8);
        unsigned long name_size = parse_hex(header->c_namesize, 8);
        unsigned long mode = parse_hex(header->c_mode, 8);
        char* filename = (char*)((char*)header + sizeof(struct cpio_newc_header));
        if (strncmp(filename, CPIO_FOOTER_MAGIC, 10) == 0) {
            break;
        }
        strcpy(new_path, "/initramfs/");
        new_path[strlen("/initramfs/")] = '\0';
        strcat(new_path, filename);
        new_path[strlen("/initramfs/") + name_size] = '\0';
        uart_printf("initramfs filename = %s\n", filename);
        uart_printf("initramfs absolute file path = %s\n", new_path);
        unsigned long offset = name_size + sizeof(cpio_newc_header);
        offset = (offset + 3) & ~3;
        char* data = (char*)header + offset;
        struct file* file;
        offset = file_size;
        offset = (offset + 3) & ~3;
        header = (cpio_newc_header*)(data + offset);

        if (mode & 0040000) {
            vfs_mkdir(new_path);
        } else if (mode & 0100000) {
            vfs_open(new_path, O_CREAT, &file);
            vfs_write(file, (void*)data, file_size);
            vfs_close(file);
        }
    }
}
#include "tmpfs.h"

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount)
{
    mount->fs = fs;
    // struct vnode* root = (struct vnode*)kmalloc(sizeof(struct vnode));
    struct vnode* root = tmpfs_vnode();
    struct tmpfs_dir_data* root_data = (struct tmpfs_dir_data*)kmalloc(sizeof(struct tmpfs_dir_data));
    root_data->entry_counts = 0;
    // for (int i = 0; i < 16; ++i) {
    //     root_data->entries[i] = NULL;
    // }
    root->internal = root_data;
    // type
    root->type = VNODE_TYPE_DIRECTORY;
    mount->root = root;

    return 0;
}

// int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount)
// {
//     struct vnode* root = (struct vnode*)kmalloc(sizeof(struct vnode));
//     if (!root)
//         return -1; // vnode 創建失敗

//     struct tmpfs_dir_data* root_data = (struct tmpfs_dir_data*)kmalloc(sizeof(struct tmpfs_dir_data));
//     if (!root_data) {
//         // kfree(root); // 清理已分配的 vnode
//         return -1;
//     }

//     // 初始化目錄數據
//     root_data->entry_counts = 0;
//     for (int i = 0; i < 16; ++i) {
//         root_data->entries[i] = NULL;
//     }
//     root->internal = root_data;
//     root->type = VNODE_TYPE_DIRECTORY;
//     mount->root = root;
//     mount->fs = fs;
//     return 0;
// }
// struct vnode *tmpfs_vnode(struct tmpfs_data *td) {
struct vnode* tmpfs_vnode()
{
    struct vnode* vtmp = (struct vnode*)kmalloc(sizeof(struct vnode));
    struct vnode_operations* tmpfs_v_op = (struct vnode_operations*)kmalloc(sizeof(struct vnode_operations));
    struct file_operations* tmpfs_f_op = (struct file_operations*)kmalloc(sizeof(struct file_operations));
    tmpfs_v_op->create = &tmpfs_create;
    tmpfs_v_op->lookup = &tmpfs_lookup;
    tmpfs_v_op->mkdir = &tmpfs_mkdir;
    tmpfs_f_op->read = &tmpfs_read;
    tmpfs_f_op->write = &tmpfs_write;
    tmpfs_f_op->open = &tmpfs_open;
    tmpfs_f_op->close = &tmpfs_close;
    vtmp->f_ops = tmpfs_f_op;
    vtmp->v_ops = tmpfs_v_op;
    vtmp->mount = NULL;
    // if(!td) {
    //     td->vnode = vtmp;
    //     vtmp->internal = td;
    // }
    return vtmp;
}

int tmpfs_write(struct file* file, const void* buf, size_t len)
{
    // struct tmpfs_file_data* file_node = (struct tmpfs_file_data*)kmalloc(sizeof(struct tmpfs_file_data));
    // file_node = (struct tmpfs_file_data*)file->vnode->internal;
    struct tmpfs_file_data* file_node = (struct tmpfs_file_data*)file->vnode->internal;
    if (!file_node)
        return -1;
    if (file_node->size == 0) {
        file_node->data = kmalloc(file->f_pos + len);
        file_node->size = file->f_pos + len;
    } else if (file_node->size < file->f_pos + len) {
        char* data = file_node->data;
        file_node->data = kmalloc(file->f_pos + len);
        memcpy(file_node->data, data, file_node->size);
        kfree(data);
        file_node->size = file->f_pos + len;
    }
    memcpy((void*)(file_node->data + file->f_pos), buf, len);
    // uart_printf("file_node = %x, write buf = %s\n", (char*)file_node, (char*)buf);
    file->f_pos += len;
    // kfree((char*)file_node);
    return len;
}

int tmpfs_read(struct file* file, void* buf, size_t len)
{
    // struct tmpfs_file_data* file_node = (struct tmpfs_file_data*)kmalloc(sizeof(struct tmpfs_file_data));
    // file_node = (struct tmpfs_file_data*)file->vnode->internal;
    struct tmpfs_file_data* file_node = (struct tmpfs_file_data*)file->vnode->internal;
    if (!file_node)
        return -1;
    size_t read_len = (len < file_node->size) ? len : file_node->size;
    // uart_printf("node->data = %s,file pos = %d\n", file_node->data, file->f_pos);
    memcpy((void*)buf, (file_node->data + file->f_pos), read_len);
    // uart_printf("file_node = %x, read buf = %s\n", (char*)file_node, (char*)buf);
    file->f_pos += len;
    // kfree((char*)file_node);
    return read_len;
}

int tmpfs_open(struct vnode* file_node, struct file** target)
{
    *target = (struct file*)kmalloc(sizeof(struct file));
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    (*target)->flags = RW;
    (*target)->vnode = file_node;
    return 0;
}

int tmpfs_close(struct file* file)
{
    kfree((char*)file);
    return 0;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct tmpfs_dir_data* dir_data = (struct tmpfs_dir_data*)dir_node->internal;

    for (int i = 0; i < dir_data->entry_counts; i++) {
        if (strncmp(dir_data->names[i], component_name, strlen(dir_data->names[i])) == 0) {
            *target = dir_data->entries[i];
            return 0;
        }
    }
    return -1;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct tmpfs_dir_data* dir_data = (struct tmpfs_dir_data*)dir_node->internal;
    if (dir_data->entry_counts >= 16) {
        return -1;
    }
    struct vnode* new_node = tmpfs_vnode();
    // if (type == VNODE_TYPE_FILE) {
    // struct tmpfs_file_data* file_data = (struct tmpfs_file_data*)kmalloc(sizeof(struct tmpfs_file_data));
    struct tmpfs_file_data* file_data = (struct tmpfs_file_data*)kmalloc(PAGE_SIZE);
    file_data->data = NULL;
    file_data->size = 0;

    new_node->internal = file_data;

    new_node->type = VNODE_TYPE_FILE;
    // } else if (type == VNODE_TYPE_DIRECTORY) {
    //     struct tmpfs_dir_data* dir_data = (struct tmpfs_dir_data*)kmalloc(sizeof(struct tmpfs_dir_data));
    //     dir_data->entry_counts = 0;
    //     new_node->internal = dir_data;
    //     new_node->type = VNODE_TYPE_DIRECTORY;
    // }

    // add new file
    dir_data->entries[dir_data->entry_counts] = new_node;
    strcpy(dir_data->names[dir_data->entry_counts], component_name);
    dir_data->entry_counts++;

    // basic 3
    new_node->parent = dir_node;
    *target = new_node;
    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct tmpfs_dir_data* dir_data = (struct tmpfs_dir_data*)dir_node->internal;
    if (dir_data->entry_counts >= 16) {
        return -1;
    }
    struct vnode* new_dir = tmpfs_vnode();
    struct tmpfs_dir_data* new_dir_data = (struct tmpfs_dir_data*)kmalloc(sizeof(struct tmpfs_dir_data));
    new_dir_data->entry_counts = 0;
    new_dir->internal = new_dir_data;
    dir_data->entries[dir_data->entry_counts] = new_dir;
    // strcpy(dir_data->names[dir_data->entry_counts], component_name);
    strncpy(dir_data->names[dir_data->entry_counts], component_name, strlen(component_name));
    // strncpy(dir_data->names[dir_data->entry_counts], component_name, sizeof(component_name));
    dir_data->entry_counts++;
    // basic 3
    new_dir->parent = dir_node;
    *target = new_dir;
    return 0;
}
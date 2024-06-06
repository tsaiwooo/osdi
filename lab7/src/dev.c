#include "dev.h"

void init_device()
{
    vfs_mkdir("/dev");
    init_uart_dev();
    // vfs_mkdir("/dev/uart");
    init_framebuf_dev();
}

void init_uart_dev()
{
    struct thread* cur = get_current();
    struct file* uart_file;
    vfs_open("/dev/uart", O_CREAT, &uart_file);
    cur->fd[0] = uart_file;
    cur->fd[1] = uart_file;
    cur->fd[2] = uart_file;
    uart_file->f_ops->read = &uartfs_read;
    uart_file->f_ops->write = &uartfs_write;
}

int uartfs_read(struct file* file, void* buf, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        ((char*)buf)[i] = uart_getc();
    return len;
}

int uartfs_write(struct file* file, const void* buf, size_t len)
{
    uart_printf("%s", buf);
    return len;
}

struct framebuffer_info fb;

int init_framebuf_dev()
{
    struct file* frame_buf_file;
    vfs_open("/dev/framebuffer", O_CREAT, &frame_buf_file);
    frame_buf_file->f_ops->write = &frame_buf_write;
    unsigned int __attribute__((aligned(16))) mbox[36];

    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // FrameBufferInfo.width
    mbox[6] = 768; // FrameBufferInfo.height

    mbox[7] = 0x48004; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox[11] = 768; // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0; // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (sys_mailbox(MBOX_CH_PROP, mbox) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        fb.width = mbox[5]; // get actual physical width
        fb.height = mbox[6]; // get actual physical height
        fb.pitch = mbox[33]; // get number of bytes per line
        fb.isrgb = mbox[24]; // get the actual channel order
        fb.lfb = (void*)((unsigned long)mbox[28]);
    } else {
        uart_printf("Unable to set screen resolution to 1024x768x32\n");
    }
    return 0;
}

int frame_buf_write(struct file* file, const void* buf, size_t len)
{
    memcpy((void*)((char*)fb.lfb + file->f_pos), buf, len);
    file->f_pos += len;
    return 0;
}

long lseek64(struct file* file, long offset, int whence)
{
    long off = -1, pos;
    switch (whence) {
    case SEEK_SET:
        off = 0;
        break;
    default:
        break;
    }
    pos = off + offset;
    if (pos < 0)
        return -1;
    file->f_pos = pos;
    return pos;
}
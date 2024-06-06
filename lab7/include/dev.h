#ifndef __DEV_H_
#define __DEV_H_
#include "main.h"
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
struct file;
void init_device();
void init_uart_dev();
int init_framebuf_dev();
int uartfs_read(struct file* file, void* buf, size_t len);
int uartfs_write(struct file* file, const void* buf, size_t len);
int frame_buf_write(struct file* file, const void* buf, size_t len);
long lseek64(struct file* file, long offset, int whence);
struct framebuffer_info {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int isrgb;
    void* lfb;
};

#endif // __DEV_H_
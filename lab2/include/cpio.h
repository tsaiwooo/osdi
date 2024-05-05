#ifndef __CPIO_H_
#define __CPIO_H_
#include "my_string.h"
#include "uart.h"
/* Magic identifiers for the "cpio" file format. */
#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"
#define CPIO_ALIGNMENT 4

// #define CPIO_DEFAULT_PLACE 0x20000000
void* CPIO_DEFAULT_PLACE;
typedef struct cpio_newc_header cpio_newc_header;
struct cpio_newc_header {
    char c_magic[6]; // header '070701'
    char c_ino[8]; // i-node number
    char c_mode[8]; // permissions
    char c_uid[8]; // UID
    char c_gid[8]; // GID
    char c_nlink[8]; // number of hard links
    char c_mtime[8]; // modification time
    char c_filesize[8]; // file size
    char c_devmajor[8]; // major device number
    char c_devminor[8]; // minor dev number
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8]; // length of filename in bytes
    char c_check[8]; // checksum
};

// sotres info about the underlying implementation.
struct cpio_info {
    /// The number of files in the CPIO archive
    unsigned int file_count;
    /// The maximum size of a file name
    unsigned int max_path_sz;
};
int cpio_parse_header(cpio_newc_header* archive, const char** filename,
    unsigned long* filesize_, char** data, cpio_newc_header** next);
unsigned long parse_hex(char* s, unsigned int max_len);
void ls();
void cat(char*);

#endif // __CPIO_H_
#include "cpio.h"

// pares ASCII hex string into integer
unsigned long parse_hex(char *s, unsigned int max_len) {
  unsigned long i, r = 0;

  for (i = 0; i < max_len; i++) {
    r *= 16;
    if (s[i] >= '0' && s[i] <= '9') {
      r += s[i] - '0';
    } else if (s[i] >= 'a' && s[i] <= 'f') {
      r += s[i] - 'a' + 10;
    } else if (s[i] >= 'A' && s[i] <= 'F') {
      r += s[i] - 'A' + 10;
    } else {
      return r;
    }
  }
  return r;
}

int cpio_parse_header(cpio_newc_header *archive, const char **filename,
                      unsigned long *filesize_, char **data,
                      cpio_newc_header **next) {
  // check magic
  if (strncmp(archive->c_magic, CPIO_HEADER_MAGIC, sizeof(archive->c_magic)) !=
      0) {
    uart_printf("Magic number wrong!\n");
    return -1;
  }

  // filename and filesize
  unsigned long filesize = parse_hex(archive->c_filesize, 8);
  *filesize_ = filesize;

  *filename = ((char *)archive +
               sizeof(cpio_newc_header));  // char * -> unsigned long *

  // check is not the trailer of EOF
  if (strncmp(*filename, CPIO_FOOTER_MAGIC, 10) == 0) {
    return 1;
  }

  // find offset to data
  unsigned long filename_len = parse_hex(archive->c_namesize, 8);

  unsigned long offset = filename_len + sizeof(cpio_newc_header);
  offset = (offset + 3) & ~3;
  *data = (char *)archive + offset;
  // *data = (char *)align_up(((unsigned long *)archive) + sizeof(struct
  // cpio_newc_header) +
  //                     filename_len , CPIO_ALIGNMENT);
  offset = filesize;
  offset = (offset + 3) & ~3;
  *next = (cpio_newc_header *)(*data + offset);
  // *next = (struct cpio_newc_header *)align_up(((unsigned long *)data) +
  // filesize , CPIO_ALIGNMENT);
  return 0;
}

void ls() {
  char *data;
  // struct cpio_newc_header *header = (char *)0x8000000;
  struct cpio_newc_header *header = CPIO_DEFAULT_PLACE;
  char *filename;
  unsigned long filesize = 0;

  while (header != 0) {
    int finish =
        cpio_parse_header(header, &filename, &filesize, &data, &header);
    if (finish) break;
    if (header != 0) uart_printf("%s\n", filename);
  }
}

void cat(char *file) {
  char *data;
  // struct cpio_newc_header *header = (struct cpio_newc_header *)0x8000000;
  struct cpio_newc_header *header = CPIO_DEFAULT_PLACE;
  char *filename;
  unsigned long filesize = 0;

  while (header != 0) {
    int finish =
        cpio_parse_header(header, &filename, &filesize, &data, &header);
    if (finish) break;
    if (!strcmp(file, filename)) {
      while (filesize--) {
        if (*data == '\n') uart_send('\r');
        uart_send(*data++);
      }
      uart_send('\n');
    }
  }
}

void exec(char *file) {
  char *data;
  // struct cpio_newc_header *header = (struct cpio_newc_header *)0x8000000;
  struct cpio_newc_header *header = CPIO_DEFAULT_PLACE;
  char *filename;
  unsigned long filesize = 0;

  while (header != 0) {
    int finish =
        cpio_parse_header(header, &filename, &filesize, &data, &header);
    if (finish) break;
    if (!strcmp(file, filename)) {
      exec_file(data, filesize);
    }
  }
}

void exec_file(char *data, unsigned long size) {
  char *user_ptr = (char *)0x10000;
  for (unsigned long i = 0; i < size; i++) {
    user_ptr[i] = data[i];
  }
  uart_printf("after mov data\n");
  asm volatile(
      "mov x3, #0x3c0\n\t"
      "msr spsr_el1, x3\n\t"
      "msr elr_el1, %[data]\n\t"
      "msr sp_el0, %[stack_ptr]\n\t"
      // "mrs x10,spsr_el1\n\t"
      "eret\n\t"
      :
      : [data] "r"(user_ptr), [stack_ptr] "r"(user_ptr));
}
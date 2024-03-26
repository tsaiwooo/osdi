#include "devicetree.h"

uint32_t uint32_endian_big2lttle(uint32_t data) {
  char *r = (char *)&data;
  return (r[3] << 0) | (r[2] << 8) | (r[1] << 16) | (r[0] << 24);
}

void fdt_traverse(dtb_callback callback) {
  fdt_header *header = (fdt_header *)dtb_place;
  if (uint32_endian_big2lttle(header->magic) != 0xD00DFEED) {
    uart_printf("wrong magic in traverse device tree\n");
    return;
  }

  uint32_t struct_size = uint32_endian_big2lttle(header->size_dt_struct);
  char *dt_struct_ptr =
      (char *)((char *)header + uint32_endian_big2lttle(header->off_dt_struct));
  char *dt_strings_ptr = (char *)((char *)header + uint32_endian_big2lttle(
                                                       header->off_dt_strings));

  char *end = (char *)dt_struct_ptr + struct_size;
  char *ptr = dt_struct_ptr;
  while (ptr < end) {
    // 前4bytes是header
    uint32_t token_type = uint32_endian_big2lttle(*(uint32_t *)ptr);

    // 取完header後+4
    ptr += 4;

    if (token_type == FDT_BEGIN_NODE) {
      callback(token_type, ptr, 0, 0);
      ptr += strlen(ptr);
      ptr += 4 - (unsigned long long)ptr % 4;  // align 4
    } else if (token_type == FDT_END_NODE) {
      callback(token_type, 0, 0, 0);
    } else if (token_type == FDT_PROP) {
      fdt_prod *fdt_prod_ptr = (fdt_prod *)ptr;
      ptr += 8;  // 2 uint32_t data
      uint32_t len = uint32_endian_big2lttle(fdt_prod_ptr->len);
      char *name = (char *)dt_strings_ptr +
                   uint32_endian_big2lttle(fdt_prod_ptr->nameoff);
      // uart_printf("name = %s, addr: 0x%x\n",name,ptr);
      callback(token_type, name, ptr, len);
      ptr += len;
      // ptr = (unsigned long long)(ptr+3) & ~3;
      if ((unsigned long long)ptr % 4 != 0)
        ptr += 4 - (unsigned long long)ptr % 4;
    } else if (token_type == FDT_NOP) {
      callback(token_type, 0, 0, 0);
    } else if (token_type == FDT_END) {
      callback(token_type, 0, 0, 0);
    } else {
      uart_printf("no type : %x\n", token_type);
      return;
    }
  }
}

void dtb_callback_initramfs(uint32_t node_type, char *name, void *value,
                            uint32_t name_size) {
  if (node_type == FDT_PROP && !strcmp(name, "linux,initrd-start")) {
    CPIO_DEFAULT_PLACE =
        (void *)(unsigned long long)uint32_endian_big2lttle(*(uint32_t *)value);
  }
}
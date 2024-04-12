#ifndef _DEVICETREE_H_
#define _DEVICETREE_H_
#define uint32_t unsigned int
#include "my_string.h"
#include "uart.h"

extern void *CPIO_DEFAULT_PLACE;
extern void *CPIO_DEFAULT_PLACE_END;
char *dtb_place;
extern unsigned long dtb_len;

typedef struct {
  uint32_t magic;      // shall contain the value 0xd00dfeed (big-endian)
  uint32_t totalsize;  // contain the total size in bytes of the devicetree data
                       // structure.
  uint32_t off_dt_struct;  // offset in bytes of the structure block (see
                           // section 5.4) from the beginning of the header.
  uint32_t
      off_dt_strings;  // contain the offset in bytes of the strings block (see
                       // section 5.5) from the beginning of the header.
  uint32_t off_mem_rsvmap;  // offset in bytes of the memory reservation block
                            // from the beginning of the header
  uint32_t version;         // the version of the devicetree data structure
  uint32_t last_comp_version;  // the lowest version of the devicetree data
                               // structure with which the version used is
                               // backwards compatible.
  uint32_t boot_cpuid_phys;    // the physical ID of the system’s boot CPU
  uint32_t size_dt_strings;  // the length in bytes of the strings block section
                             // of the devicetree blob.
  uint32_t size_dt_struct;   // the length in bytes of the structure block
                             // section of the devicetree blob.
} fdt_header;

#define FDT_BEGIN_NODE 0x00000001  // the beginning of a node’s representation
#define FDT_END_NODE 0x00000002    // the end of a node’s representation
#define FDT_PROP 0x00000003
// the beginning of the representation of one property in the devicetree.
// It shall be followed by extra data describing the property. This data
// consists first of the property’s length and name represented as the following
// C structure:
typedef struct {
  uint32_t len;
  uint32_t nameoff;
} fdt_prod;

#define FDT_NOP 0x00000004  // ignored by any program parsing the device tree
#define FDT_END 0x00000009  // end of the structure block

uint32_t uint32_endian_big2lttle(uint32_t);
typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value,
                             uint32_t name_size);
void dtb_callback_initramfs(uint32_t node_type, char *name, void *value,
                            uint32_t name_size);
void fdt_traverse(dtb_callback callback);

#endif  // _DEVICETREE_H_
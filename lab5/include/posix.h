#ifndef __POSIX_H_
#define __POSIX_H_
#define uint64_t unsigned long long
#define SIG_NUMS 16
typedef struct posix posix;

struct posix{
    uint64_t signal_handler[16];
    uint64_t posix_stack;
    uint64_t sp_posix;
    unsigned int signal_type;
    uint64_t signal_RA;
    uint64_t signal_sp_el0;
    uint64_t signal_sp_el1;
    uint64_t signal_spsr_el1;
};


#endif // __POSIX_H_
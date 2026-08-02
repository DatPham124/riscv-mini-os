#pragma once

#include "common.h"

#define SATP_SV32 (1u << 31) // enable paging in Sv32 mode
#define PAGE_V (1 << 0)      // valid, existing or not
#define PAGE_R (1 << 1)      // readable
#define PAGE_W (1 << 2)      // writable
#define PAGE_X (1 << 3)      // Excutable
#define PAGE_U (1 << 4)      // User (accessible in user mode)

#define USER_BASE 0x1000000

#define SSTATUS_SPIE (1 << 5)


__attribute__((naked)) void user_entry(void) {
    __asm__ __volatile__(
        "csrw sepc, %[sepc]        \n"
        "csrw sstatus, %[sstatus]  \n"
        "sret                      \n"
        :
        : [sepc] "r" (USER_BASE),
        [sstatus] "r" (SSTATUS_SPIE)
    );
}

extern char __kernel_base[], __free_ram_end[];
extern char __free_ram[];

paddr_t alloc_pages(uint32_t n);
void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags);

struct sbiret
{
    long error;
    long value;
};

#define PANIC(fmt, ...)                                                      \
    do                                                                       \
    {                                                                        \
        printf("PANIC: %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        while (1)                                                            \
        {                                                                    \
        }                                                                    \
    } while (0)

struct trap_frame
{
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));

#define READ_CSR(reg)                                         \
    ({                                                        \
        unsigned long __tmp;                                  \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp)); \
        __tmp;                                                \
    })

#define WRITE_CSR(reg, value)                                   \
    do                                                          \
    {                                                           \
        uint32_t __tmp = (value);                               \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp)); \
    } while (0)

#define PROCS_MAX 8

#define PROC_UNUSED 0
#define PROC_RUNABLE 1

struct process
{
    int pid;
    int state;
    vaddr_t sp;
    uint32_t *page_table;
    uint8_t stack[8192];
};

struct process procs[PROCS_MAX];


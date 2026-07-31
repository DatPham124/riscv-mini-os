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

struct process *create_process(const void *image, size_t image_size)
{
    struct process *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++)
    {
        if (procs[i].state == PROC_UNUSED)
        {
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
        PANIC("no free process slot");

    uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)];
    *--sp = 0;                    // s11
    *--sp = 0;                    // s10
    *--sp = 0;                    // s9
    *--sp = 0;                    // s8
    *--sp = 0;                    // s7
    *--sp = 0;                    // s6
    *--sp = 0;                    // s5
    *--sp = 0;                    // s4
    *--sp = 0;                    // s3
    *--sp = 0;                    // s2
    *--sp = 0;                    // s1
    *--sp = 0;                    // s0
    *--sp = (uint32_t)user_entry; // ra

    uint32_t *page_table = (uint32_t *)alloc_pages(1);

    // Map kernel pages
    for (paddr_t paddr = (paddr_t)__kernel_base;
        paddr < (paddr_t)__free_ram_end; paddr += PAGE_SIZE)
        map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);

    // Map user pages
    for (uint32_t off = 0; off < image_size; off += PAGE_SIZE)
    {
        paddr_t page = alloc_pages(1);

        size_t remaining = image_size - off;
        size_t copy_size = PAGE_SIZE <= remaining ? PAGE_SIZE : remaining;

        memcpy((void *)page, image + off, copy_size);
        map_page(page_table, USER_BASE + off, page, PAGE_U | PAGE_R | PAGE_W | PAGE_X);
    }

    proc->pid = i + 1;
    proc->state = PROC_RUNABLE;
    proc->sp = (uint32_t)sp;
    proc->page_table = page_table;
    return proc;
};

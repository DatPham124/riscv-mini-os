#pragma once

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

#include "common.h"

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
    uint8_t stack[8192];
};

struct process procs[PROCS_MAX];

struct process *create_process(uint32_t pc)
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
    *--sp = 0;            // s11
    *--sp = 0;            // s10
    *--sp = 0;            // s9
    *--sp = 0;            // s8
    *--sp = 0;            // s7
    *--sp = 0;            // s6
    *--sp = 0;            // s5
    *--sp = 0;            // s4
    *--sp = 0;            // s3
    *--sp = 0;            // s2
    *--sp = 0;            // s1
    *--sp = 0;            // s0
    *--sp = (uint32_t)pc; // ra

    proc->pid = i + 1;
    proc->state = PROC_RUNABLE;
    proc->sp = (uint32_t) sp;
    return proc;
};

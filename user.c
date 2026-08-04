#include "user.h"

extern char __stack_top[];

__attribute__((noreturn)) void exit(void)
{
    for (;;)
        ;
}

void putchar(char ch)
{
}

__attribute__((section(".text.start")))
__attribute__((naked))

void
start(void)
{
    __asm__ __volatile__(

        "mv sp, %[stack_top] \n"
        "call main           \n"
        "call exit           \n" ::[stack_top] "r"(__stack_top));
}

int syscall(int sysno, int arg0, int arg1, int arg2)
{
    // Các thanh ghi tham số phụ thêm cho a3, như là muốn in gì, nhập gì.
    // return kernel sẽ được set ở a0
    register int a0 __asm__("a0") = arg0;
    register int a1 __asm__("a1") = arg1;
    register int a2 __asm__("a2") = arg2;
    // Thanh ghi a3 chứa Syscall Number để báo cho Kernel biết cần làm gì.
    // (VD: 1 -> Yêu cầu in chữ, 2 -> Yêu cầu đọc phím)
    register int a3 __asm__("a0") = sysno;

    __asm__ __volatile__("ecall"
                         : "=r"(a0)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3)
                         : "memory");

    return a0;
}

void putchar(char ch){
    syscall(SYS_PUTCHAR, ch, 0, 0);
}
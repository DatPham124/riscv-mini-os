#include "user.h"

void main(void){
    while(1) {
        // Chèn một lệnh Assembly rỗng (No Operation)
        // Lệnh này không làm gì cả, nhưng nó cấm Trình biên dịch xóa vòng lặp
        __asm__ __volatile__("nop"); 
    }
}
#pragma once

#include "common.h"

#define SATP_SV32 (1u << 31) // enable paging in Sv32 mode
#define PAGE_V (1 << 0)      // valid, existing or not
#define PAGE_R (1 << 1)      // readable
#define PAGE_W (1 << 2)      // writable
#define PAGE_X (1 << 3)      // Excutable
#define PAGE_U (1 << 4)      // User (accessible in user mode)

#define SSTATUS_SPIE (1 << 5)

#define USER_BASE 0x1000000

#define SSTATUS_SPIE (1 << 5)

#define SCAUSE_ECALL 8

#define PROC_EXITED 2

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

#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_BLK 2
// Địa chỉ RAM nơi QEMU đặt thiết bị ổ cứng
#define VIRTIO_BLK_PADDR 0x10001000
// Nút đọc chữ ký thiết bị
#define VIRTIO_REG_MAGIC 0x00
// Đọc phiên bản Virtio
#define VIRTIO_REG_VERSION 0x04
// Đọc loại thiết bị
#define VIRTIO_REG_DEVICE_ID 0x08
#define VIRTIO_REG_PAGE_SIZE 0x28
// Chọn hàng đợi muốn cấu hình
#define VIRTIO_REG_QUEUE_SEL 0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
// Báo cho thiết bị kích thước hàng đợi
#define VIRTIO_REG_QUEUE_NUM 0x38
// Ghi địa chỉ RAM của Virtqueue vào đây để QEMU biết đường tìm đến
#define VIRTIO_REG_QUEUE_PFN 0x40
#define VIRTIO_REG_QUEUE_READY 0x44
// Nút báo hiệu cho có việc mới trong Avail Ring
#define VIRTIO_REG_QUEUE_NOTIFY 0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100

// Các cờ báo trạng thái kết nối
#define VIRTIO_STATUS_ACK 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4

#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1

// Virtqueue Descriptor Table entry (như tờ giấy mô tả 1 yêu cầu phần cứng)
struct virtq_desc
{
    uint64_t addr;  // Địa chỉ RAM chứa dữ liệu
    uint32_t len;   // Độ dài vùng nhớ
    uint16_t flags; // VIRTQ_DESC_F_NEXT (báo có desc nối tiếp) / VIRTQ_DESC_F_WRITE (báo cho QEMU ghi vào)
    uint16_t next;  // Chỉ số (index) của Desc tiếp theo trong chuỗi Descriptor Chain
} __attribute__((packed));

// Virtqueue Available Ring.
struct virtq_avail
{
    uint16_t flags;
    uint16_t index;                 // Vị trí đơn hàng mới nhất
    uint16_t ring[VIRTQ_ENTRY_NUM]; // Mảng chứa các ID đại diện cho chuỗi Descriptor
} __attribute__((packed));

// Hôp thư nhận các yêu cầu đã hoàn thành từ QEMU
// Virtqueue Used Ring entry.
struct virtq_used_elem
{
    uint32_t id;  // ID của chuỗi Descriptor đã hoàn thành
    uint32_t len; // Số byte QEMU đã xử lý
} __attribute__((packed));

// Virtqueue Used Ring.
struct virtq_used
{
    uint16_t flags;
    uint16_t index; // Vị trí báo kết quả mới nhất của QEMU
    struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue.
struct virtio_virtq
{
    struct virtq_desc descs[VIRTQ_ENTRY_NUM];   //Bảng chứa 16 Discriptor
    struct virtq_avail avail;                   //Avail Ring
    struct virtq_used used __attribute__((aligned(PAGE_SIZE))); //Used Ring (phải căn chỉnh đúng 4KB RAM)
    
    
    int queue_index;
    volatile uint16_t *used_index;
    uint16_t last_used_index;
} __attribute__((packed));

// Virtio-blk request (đơn xin Đọc/Ghi đĩa).
struct virtio_blk_req
{
    uint32_t type;      // VIRTIO_BLK_T_IN (0: Đọc từ đĩa) hoặc VIRTIO_BLK_T_OUT (1: Ghi vào đĩa)
    uint32_t reserved;  // Ô dự phòng (luôn bằng 0)
    uint64_t sector;    // Vị trí Sector (khối 512B) trên đĩa muốn thao tác
    uint8_t data[512];  // Bộ nhớ đệm chứa 512 byte dữ liệu
    uint8_t status;     // Nơi QEMU trả kết quả (0 = Thành công)
} __attribute__((packed));
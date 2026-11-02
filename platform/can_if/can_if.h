
#ifndef CAN_IF_H
#define CAN_IF_H

#include <stdint.h>
#include "app_types.h"

/* Cấu trúc gói tin CAN thô truyền qua Queue giữa FDCAN ISR và CanRxTask (16 bytes) */
typedef struct {
    uint32_t can_id;
    uint8_t  dlc;
    uint8_t  data[8];
} RawCanFrame_t;

/* Khởi tạo bộ lọc phần cứng, bật ngắt và khởi chạy FDCAN1 (Bước 5 Boot Sequence) */
ServiceResult_t CanIf_InitAndStart(void);

/* Đọc số lần xảy ra lỗi Bus-Off (Phục vụ DID 0xF111 và CLI 'can stats') */
uint32_t        CanIf_GetBusOffCount(void);

#endif /* CAN_IF_H */

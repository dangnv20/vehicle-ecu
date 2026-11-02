/* File: config/can_matrix.h */
#ifndef CAN_MATRIX_H
#define CAN_MATRIX_H

#include <stdint.h>

/* =========================================================================
 * 1. ĐỊNH NGHĨA CAN ID & CHU KỲ (Xuất phát từ Mục 9.2 & 9.3 Customer Spec)
 * ========================================================================= */
/* Bản tin truyền đi (VCU -> TCU) */
#define CAN_ID_VEHICLE_DYNAMIC      (0x100U)
#define CAN_CYCLE_VEHICLE_DYN_MS    (20U)

#define CAN_ID_VEHICLE_STATUS       (0x101U)
#define CAN_CYCLE_VEHICLE_STAT_MS   (100U)

#define CAN_ID_BODY_STATUS          (0x200U)
#define CAN_CYCLE_BODY_STAT_MS      (100U)

#define CAN_ID_POWER_STATUS         (0x300U)
#define CAN_CYCLE_POWER_STAT_MS     (500U)

#define CAN_ID_COMMAND_ACK          (0x500U) /* Bản tin sự kiện (Event) */

#define CAN_ID_ECU_HEARTBEAT        (0x700U)
#define CAN_CYCLE_HEARTBEAT_MS      (1000U)

/* Bản tin nhận vào (TCU -> VCU) */
#define CAN_ID_REMOTE_COMMAND       (0x400U)
#define CAN_ID_VEHICLE_SETPOINT     (0x401U)
#define CAN_ID_UDS_PHYS_REQ         (0x7E0U)
#define CAN_ID_UDS_FUNC_REQ         (0x7DFU)
#define CAN_ID_UDS_PHYS_RESP        (0x7E8U)

/* =========================================================================
 * 2. THÔNG SỐ NGƯỠNG LOGIC NGHIỆP VỤ ( 11 Customer Spec)
 * ========================================================================= */
/* Đơn vị nội bộ của tốc độ là 0.01 km/h (centi-km/h) -> 5.00 km/h = 500 */
#define CFG_LOCK_MAX_SPEED_CENTI_KMH    (500U)

/* Cửa sổ thời gian chống lặp lệnh (Idempotency window: 10 giây theo CMD-002) */
#define CFG_CMD_DUPLICATE_WINDOW_MS     (10000U)

/* Giá trị báo tín hiệu không hợp lệ trên đường truyền CAN (Mục 9.4) */
#define CAN_RAW_U16_INVALID             (0xFFFFU)
#define CAN_RAW_U8_INVALID              (0xFFU)

#endif /* CAN_MATRIX_H */

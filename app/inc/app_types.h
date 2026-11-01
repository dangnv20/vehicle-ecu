#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/* =========================================================================
 * 1. HỢP ĐỒNG TRẢ VỀ & CHẤT LƯỢNG TÍN HIỆU (Mục 4 & 5 Tài liệu Kiến trúc)
 * ========================================================================= */
typedef enum {
    SRV_RESULT_ACCEPTED = 0,    /* Yêu cầu đã được tiếp nhận xử lý */
    SRV_RESULT_BUSY,            /* Đang bận xử lý yêu cầu trước */
    SRV_RESULT_INVALID_PARAM,   /* Tham số đầu vào nằm ngoài miền hợp lệ */
    SRV_RESULT_UNAVAILABLE,     /* Phần cứng/Dịch vụ hiện không sẵn sàng */
    SRV_RESULT_ERROR            /* Lỗi thực thi */
} ServiceResult_t;

typedef enum {
    SIGNAL_VALID = 0,           /* Dữ liệu đáng tin cậy */
    SIGNAL_INVALID,             /* Cảm biến lỗi hoặc bị tiêm lỗi (Fault Injected) */
    SIGNAL_STALE                /* Quá thời gian timeout chưa được cập nhật */
} SignalQuality_t;

/* =========================================================================
 * 2. TRẠNG THÁI HỆ THỐNG & MÔ PHỎNG XE (Mục 6.1 & 6.2 Customer Spec)
 * ========================================================================= */
typedef enum {
    ECU_STATE_RESET = 0,
    ECU_STATE_BOOT,
    ECU_STATE_SELF_TEST,
    ECU_STATE_RUN,
    ECU_STATE_DEGRADED,
    ECU_STATE_FAIL_SAFE
} EcuState_t;

typedef enum {
    IGN_STATE_OFF = 0,
    IGN_STATE_ACC = 1,
    IGN_STATE_ON  = 2,
    IGN_STATE_INVALID = 3
} IgnitionState_t;

/* =========================================================================
 * 3. SNAPSHOT DỮ LIỆU NỘI BỘ (Mục 3 & 4 Tài liệu Kiến trúc)
 * Chỉ chủ sở hữu (State Manager) mới được ghi bản gốc; các module khác đọc Snapshot.
 * ========================================================================= */
typedef struct {
    uint16_t        speed_centi_kmh;    /* 1 unit = 0.01 km/h (0..25000) */
    uint16_t        rpm_quarter;        /* 1 unit = 0.25 rpm (0..64000) */
    uint8_t         gear;               /* 0=P, 1=R, 2=N, 3=D */
    IgnitionState_t ignition;           /* OFF, ACC, ON */
    uint32_t        odometer_deci_km;   /* 1 unit = 0.1 km */
    uint32_t        updated_at_ms;      /* Thời điểm cập nhật mẫu */
    SignalQuality_t speed_quality;      /* Trạng thái hợp lệ của tốc độ */
    SignalQuality_t rpm_quality;        /* Trạng thái hợp lệ của RPM */
} VehicleMotionSnapshot_t;

typedef enum {
    LOCK_STATE_UNLOCKED = 0,
    LOCK_STATE_LOCKED   = 1,
    LOCK_STATE_UNKNOWN  = 2
} LockState_t;

typedef struct {
    uint8_t     door_open_mask;         /* Bit 0:FL, 1:FR, 2:RL, 3:RR (1=Open) */
    LockState_t central_lock;           /* Trạng thái khóa chính thức */
    bool        light_active;           /* Trạng thái đèn hiện tại */
    bool        horn_active;            /* Trạng thái còi hiện tại */
    uint32_t    updated_at_ms;
} BodyStateSnapshot_t;

/* =========================================================================
 * 4. CẤU TRÚC LỆNH ĐIỀU KHIỂN TỪ XA & PHẢN HỒI ACK (Mục 9.2, 9.3 & 11 Spec)
 * ========================================================================= */
typedef enum {
    CMD_ID_LOCK_DOORS           = 0x01,
    CMD_ID_UNLOCK_DOORS         = 0x02,
    CMD_ID_FLASH_LIGHTS         = 0x03,
    CMD_ID_HONK                 = 0x04,
    CMD_ID_START_DRIVE_PROFILE  = 0x05,
    CMD_ID_STOP_DRIVE_PROFILE   = 0x06,
    CMD_ID_SET_IGNITION         = 0x07
} RemoteCmdId_t;

typedef enum {
    ACK_RES_SUCCESS             = 0x00, /* Thực thi thành công */
    ACK_RES_DUPLICATE           = 0x01, /* Lệnh lặp trong 10s (CMD-002) */
    ACK_RES_REJECT_GUARD        = 0x02, /* Vi phạm điều kiện an toàn (CMD-003) */
    ACK_RES_REJECT_INVALID_SIG  = 0x03, /* Tín hiệu xe đang lỗi không thể kiểm tra guard */
    ACK_RES_REJECT_ECU_STATE    = 0x04, /* ECU chưa ở trạng thái RUN */
    ACK_RES_REJECT_FORMAT_CRC   = 0x05, /* Lỗi cú pháp hoặc sai CRC */
    ACK_RES_HW_FAILURE          = 0x06  /* Phần cứng chấp hành (Actuator) báo lỗi */
} CmdAckResult_t;

/* Dữ liệu lệnh đã được tầng Codec giải mã từ CAN frame 0x400 */
typedef struct {
    uint8_t  sequence;                  /* Số thứ tự chống lặp lệnh */
    uint8_t  cmd_id;                    /* Mã lệnh (RemoteCmdId_t) */
    uint16_t parameter;                 /* Tham số đi kèm nếu có */
    bool     crc_valid;                 /* Kết quả kiểm tra CRC từ tầng Codec */
} RemoteCommandReq_t;

/* Dữ liệu kết quả để tầng Codec đóng gói vào CAN frame 0x500 */
typedef struct {
    uint8_t        sequence;
    uint8_t        cmd_id;
    CmdAckResult_t result;
    uint8_t        resulting_state;     /* Trạng thái đạt được sau lệnh */
} CommandAckMsg_t;

/* Mã lỗi phần cứng nội bộ để báo cho FaultManager/DtcManager */
typedef enum {
    FAULT_ACTUATOR_LOCK = 0,
    FAULT_CAN_BUS_OFF,
    FAULT_CAN_RX_TIMEOUT,
    FAULT_BATTERY_LOW,
    FAULT_SIGNAL_INVALID,
    FAULT_NVM_CRC,
    FAULT_TASK_DEADLINE,
    FAULT_UDS_SEC_LOCKOUT
} FaultId_t;

#endif /* APP_TYPES_H */

#ifndef APP_PORTS_H
#define APP_PORTS_H

#include "app_types.h"

/* =========================================================================
 * 1. OS & TIMEBASE PORTS (Che giấu FreeRTOS và Timer phần cứng)
 * Quy định hợp đồng (Mục 5 Kiến trúc & Mục 8 Customer Spec):
 * - OsPort_EnterCritical/ExitCritical chỉ được giữ tối đa < 100 us.
 * ========================================================================= */
uint32_t TimePort_GetTickMs(void);
void     OsPort_EnterCritical(void);
void     OsPort_ExitCritical(void);

/* =========================================================================
 * 2. ACTUATOR PORTS (Che giấu chân GPIO, PWM, Relay - Mục 1 & 5 Kiến trúc)
 * Quy định hợp đồng:
 * - Không block (Non-blocking).
 * - Trả về SRV_RESULT_ACCEPTED chỉ có nghĩa là đã nhận yêu cầu.
 * - Kết quả thực thi vật lý sẽ báo ngược lại cho App qua callback sự kiện.
 * ========================================================================= */
ServiceResult_t DoorPort_RequestLock(bool lock_cmd, uint32_t request_id);
ServiceResult_t LightPort_SetOutput(bool turn_on);
ServiceResult_t HornPort_SetOutput(bool turn_on);

/* =========================================================================
 * 3. COMMUNICATION PORTS (Che giấu FDCAN và CAN Byte Layout)
 * ========================================================================= */
ServiceResult_t ComPort_SendCommandAck(const CommandAckMsg_t *ack_msg);

/* =========================================================================
 * 4. DIAGNOSTIC / FAULT PORTS (Báo cáo lỗi xuống DtcManager)
 * ========================================================================= */
void FaultPort_Report(FaultId_t fault_id, bool is_failed);

#endif /* APP_PORTS_H */

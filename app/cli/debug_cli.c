/*
 * debug_cli.c
 *
 *  Created on: Nov 1, 2026
 *      Author: vandang
 */
#include "debug_cli.h"
#include "vehicle_state_mgr.h"
#include "body_state_mgr.h"
#include "can_if.h"
#include <string.h>
#include <stdio.h>

#define CLI_BUFFER_SIZE  64U
static char     s_line_buf[CLI_BUFFER_SIZE];
static uint8_t  s_buf_idx = 0U;
static volatile bool s_line_ready = false;

/* Hàm port gửi chuỗi ra UART (platform sẽ cài đặt hàm này nối vào HAL_UART_Transmit) */
extern void UartPort_WriteString(const char *str);

void Cli_Init(void)
{
    s_buf_idx = 0U;
    s_line_ready = false;
    memset(s_line_buf, 0, sizeof(s_line_buf));
}

void Cli_ReceiveChar(char rx_char)
{
    if (s_line_ready) {
        return; /* Đang đợi xử lý dòng lệnh cũ, bỏ qua ký tự mới */
    }

    /* 1. Xử lý phím xóa (Backspace) */
    if ((rx_char == 8) || (rx_char == 127)) {
        if (s_buf_idx > 0U) {
            s_buf_idx--;
        }
        return;
    }

    /* 2. Xử lý phím Enter (\r hoặc \n) */
    if ((rx_char == '\r') || (rx_char == '\n')) {
        if (s_buf_idx > 0U) {
            s_line_buf[s_buf_idx] = '\0'; /* Chốt chuỗi lệnh */
            s_line_ready = true;          /* Báo cho CliTask xử lý */
            s_buf_idx = 0U;               /* FIX LỖI: Reset biến đếm về 0 cho lệnh tiếp theo! */
        }
    }
    /* 3. Chỉ nhận các ký tự chữ/số bình thường (mã ASCII từ 32 đến 126), lọc bỏ rác */
    else if ((rx_char >= 32) && (rx_char <= 126)) {
        if (s_buf_idx < (CLI_BUFFER_SIZE - 1U)) {
            s_line_buf[s_buf_idx] = rx_char;
            s_buf_idx++;
        } else {
            s_buf_idx = 0U; /* Nếu gõ quá dài gây tràn buffer, xóa sạch để bảo vệ */
        }
    }
}

void Cli_Process(void)
{
    if (!s_line_ready) {
        return;
    }

    char tx_msg[128];

    /* Lệnh 'status' (Yêu cầu CLI-003: In trạng thái xe, ECU, odometer, bus-off counter)[cite: 23] */
    if (strcmp(s_line_buf, "status") == 0) {
        VehicleMotionSnapshot_t motion;
        BodyStateSnapshot_t     body;
        VehicleStateMgr_ReadMotion(&motion);
        BodyStateMgr_ReadState(&body);

        int len = snprintf(tx_msg, sizeof(tx_msg),
            "[CLI Status] ECU: RUN | Ign: %d | Speed: %u.%02u km/h | Odo: %lu.%lu km | Lock: %d | BusOffCnt: %lu\r\n",
            (int)motion.ignition,
            (unsigned int)(motion.speed_centi_kmh / 100U),
            (unsigned int)(motion.speed_centi_kmh % 100U),
            (unsigned long)(motion.odometer_deci_km / 10U),
            (unsigned long)(motion.odometer_deci_km % 10U),
            (int)body.central_lock,
            (unsigned long)CanIf_GetBusOffCount());

        UartPort_WriteString(tx_msg);
    }
    /* Lệnh 'lock' / 'unlock' giả lập */
    else if (strcmp(s_line_buf, "lock") == 0) {
        (void)BodyStateMgr_RequestDoorLock(true, 0x99U, 0x01U);
        UartPort_WriteString("[CLI] Requested: LOCK DOORS\r\n");
    }
    else if (strcmp(s_line_buf, "unlock") == 0) {
        (void)BodyStateMgr_RequestDoorLock(false, 0x99U, 0x02U);
        UartPort_WriteString("[CLI] Requested: UNLOCK DOORS\r\n");
    }
    else if (strcmp(s_line_buf, "help") == 0) {
        UartPort_WriteString("Available CLI commands:\r\n  status\r\n  lock\r\n  unlock\r\n");
    }
    else {
        UartPort_WriteString("Unknown command. Type 'help'.\r\n");
    }

    s_line_ready = false;
}


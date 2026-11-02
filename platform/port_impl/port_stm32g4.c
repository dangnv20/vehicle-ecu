/*
 * port_stm32g4.c
 *
 *  Created on: Nov 1, 2026
 *      Author: vandang
 */
/* File: platform/port_impl/port_stm32g4.c */
#include "app_ports.h"
#include "body_state_mgr.h"
#include "main.h"
#include "cmsis_os2.h"
#include <string.h>
#include "debug_cli.h"

/* =========================================================================
 * 1. OS & TIMEBASE PORTS (Nối vào CMSIS-Core & CMSIS-RTOS v2)
 * ========================================================================= */
static uint32_t s_primask_state    = 0U;
static uint32_t s_critical_nesting = 0U;
extern UART_HandleTypeDef huart1; /* Handle của USART1 do CubeMX sinh ra */
static uint8_t s_uart_rx_byte;    /* Biến đệm lưu 1 byte nhận được từ ngắt */

uint32_t TimePort_GetTickMs(void)
{
    return osKernelGetTickCount();
}

void OsPort_EnterCritical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (s_critical_nesting == 0U) {
        s_primask_state = primask;
    }
    s_critical_nesting++;
}

void OsPort_ExitCritical(void)
{
    if (s_critical_nesting > 0U) {
        s_critical_nesting--;
        if (s_critical_nesting == 0U) {
            __set_PRIMASK(s_primask_state);
        }
    }
}

/* =========================================================================
 * 2. ACTUATOR PORTS (Nối vào GPIO trên board NUCLEO-G474RE)
 * ========================================================================= */
ServiceResult_t DoorPort_RequestLock(bool lock_cmd, uint32_t request_id)
{
#ifdef LD2_GPIO_Port
    /* Điều khiển LED LD2 (PA5) trên board NUCLEO-G474RE mô phỏng khóa cửa */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, lock_cmd ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
    (void)lock_cmd;
#endif
    /* Báo hoàn thành ngược lên cho BodyStateManager */
    BodyStateMgr_OnLockActuatorDone(request_id, true);
    return SRV_RESULT_ACCEPTED;
}

ServiceResult_t LightPort_SetOutput(bool turn_on)
{
#ifdef LD2_GPIO_Port
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, turn_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
    (void)turn_on;
#endif
    return SRV_RESULT_ACCEPTED;
}

ServiceResult_t HornPort_SetOutput(bool turn_on)
{
    (void)turn_on; /* Nối chân Buzzer/GPIO tại đây nếu có */
    return SRV_RESULT_ACCEPTED;
}

/* =========================================================================
 * 3. DIAGNOSTIC / FAULT PORTS (Sẽ nối vào DtcManager ở Phase 5)
 * ========================================================================= */
void FaultPort_Report(FaultId_t fault_id, bool is_failed)
{
    (void)fault_id;
    (void)is_failed;
}

/* Hàm gửi chuỗi (Application gọi xuống) */
void UartPort_WriteString(const char *str)
{
    if (str != NULL) {
        /* Gửi chuỗi qua UART1 (dùng Polling timeout 10ms vì chuỗi CLI khá ngắn) */
        HAL_UART_Transmit(&huart1, (const uint8_t *)str, (uint16_t)strlen(str), 10U);
    }
}

/* Hàm bắt đầu kích hoạt ngắt nhận UART (Gọi 1 lần lúc khởi động) */
void UartPort_StartReceive(void)
{
    HAL_UART_Receive_IT(&huart1, &s_uart_rx_byte, 1U);
}

/* Callback ngắt phần cứng của STM32 HAL khi nhận xong 1 byte */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 1. Đẩy ký tự vừa nhận lên tầng Application (Module DebugCLI) */
        Cli_ReceiveChar((char)s_uart_rx_byte);

        /* 2. Kích hoạt lại ngắt để nhận byte tiếp theo */
        HAL_UART_Receive_IT(&huart1, &s_uart_rx_byte, 1U);
    }
}


/* File: platform/can_if/can_if_stm32g4.c */
#include "can_if.h"
#include "can_matrix.h"
#include "app_ports.h"
#include "main.h"
#include "cmsis_os2.h"

/* Tham chiếu tới handle FDCAN1 (do CubeMX sinh ra) và Queue nhận CAN (trong app_freertos.c) */
extern FDCAN_HandleTypeDef hfdcan1;
extern osMessageQueueId_t  canRxQueueHandle;

static volatile uint32_t s_bus_off_counter = 0U;

/* Hàm phụ trợ giải mã mã độ dài DLC của STM32G4 FDCAN sang số byte thực tế (0..8) */
static uint8_t ConvertFdcanDlcToBytes(uint32_t fdcan_dlc)
{
    switch (fdcan_dlc) {
        case FDCAN_DLC_BYTES_0: return 0U;
        case FDCAN_DLC_BYTES_1: return 1U;
        case FDCAN_DLC_BYTES_2: return 2U;
        case FDCAN_DLC_BYTES_3: return 3U;
        case FDCAN_DLC_BYTES_4: return 4U;
        case FDCAN_DLC_BYTES_5: return 5U;
        case FDCAN_DLC_BYTES_6: return 6U;
        case FDCAN_DLC_BYTES_7: return 7U;
        default:                return 8U;
    }
}

ServiceResult_t CanIf_InitAndStart(void)
{
    FDCAN_FilterTypeDef sFilterConfig;

    /* 1. Bộ lọc 0: Chỉ nhận cặp ID điều khiển 0x400 (RemoteCommand) & 0x401 (VehicleSetpoint) */
    sFilterConfig.IdType       = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex  = 0U;
    sFilterConfig.FilterType   = FDCAN_FILTER_DUAL;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1    = CAN_ID_REMOTE_COMMAND;   /* 0x400 */
    sFilterConfig.FilterID2    = CAN_ID_VEHICLE_SETPOINT; /* 0x401 */
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) {
        return SRV_RESULT_ERROR;
    }

    /* 2. Bộ lọc 1: Chỉ nhận cặp ID chẩn đoán UDS 0x7E0 (Physical) & 0x7DF (Functional) */
    sFilterConfig.FilterIndex  = 1U;
    sFilterConfig.FilterType   = FDCAN_FILTER_DUAL;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1    = CAN_ID_UDS_PHYS_REQ;     /* 0x7E0 */
    sFilterConfig.FilterID2    = CAN_ID_UDS_FUNC_REQ;     /* 0x7DF */
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) {
        return SRV_RESULT_ERROR;
    }

    /* 3. Cấu hình Global Filter (Đáp ứng yêu cầu CAN-003):
     * Từ chối (FDCAN_REJECT) mọi ID lạ ngay tại phần cứng để chống CAN flood >= 1000 frames/s */
    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
                                     FDCAN_REJECT,
                                     FDCAN_REJECT,
                                     FDCAN_REJECT_REMOTE,
                                     FDCAN_REJECT_REMOTE) != HAL_OK) {
        return SRV_RESULT_ERROR;
    }

    /* 4. Bật ngắt nhận RX FIFO 0 và ngắt cảnh báo trạng thái lỗi Bus-Off (CAN-002) */
    uint32_t active_its = FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_BUS_OFF;
    if (HAL_FDCAN_ActivateNotification(&hfdcan1, active_its, 0U) != HAL_OK) {
        return SRV_RESULT_ERROR;
    }

    /* 5. Khởi chạy bộ điều khiển FDCAN1 */
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        return SRV_RESULT_ERROR;
    }

    return SRV_RESULT_ACCEPTED;
}

uint32_t CanIf_GetBusOffCount(void)
{
    return s_bus_off_counter;
}

/* =========================================================================
 * CÁC HÀM CALLBACK NGẮT PHẦN CỨNG CỦA STM32 HAL (Thực thi < 50 us - Mục 8 Spec)
 * ========================================================================= */

/* Ngắt khi có frame mới lọt qua bộ lọc vào RX FIFO 0 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0U)
    {
        FDCAN_RxHeaderTypeDef rx_header;
        RawCanFrame_t         raw_frame;

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, raw_frame.data) == HAL_OK)
        {
            raw_frame.can_id = rx_header.Identifier;
            raw_frame.dlc    = ConvertFdcanDlcToBytes(rx_header.DataLength);

            /* Đẩy sang CanRxTask qua Queue của CMSIS-RTOS v2 (timeout = 0, không bao giờ block) */
            if (canRxQueueHandle != NULL) {
                (void)osMessageQueuePut(canRxQueueHandle, &raw_frame, 0U, 0U);
            }
        }
    }
}

/* Ngắt khi phát hiện lỗi đường truyền nghiêm trọng (Bus-Off - CAN-002 & DTC 0xC00101) */
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    if ((ErrorStatusITs & FDCAN_IT_BUS_OFF) != 0U)
    {
        FDCAN_ProtocolStatusTypeDef protocol_status;
        if (HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status) == HAL_OK)
        {
            if (protocol_status.BusOff != 0U)
            {
                s_bus_off_counter++;

                /* Báo lỗi Bus-Off lên tầng trên để kích hoạt DTC 0xC00101 */
                FaultPort_Report(FAULT_CAN_BUS_OFF, true);

                /* Thử khởi động lại FDCAN để tự phục hồi (Bus recovery attempt) */
                (void)HAL_FDCAN_Stop(hfdcan);
                (void)HAL_FDCAN_Start(hfdcan);
            }
        }
    }
}


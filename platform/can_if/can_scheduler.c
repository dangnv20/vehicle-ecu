/* File: platform/can_if/can_scheduler.c */
#include "can_codec.h"
#include "can_matrix.h"
#include "vehicle_state_mgr.h"
#include "body_state_mgr.h"
#include "fdcan.h"

static uint32_t s_tick_10ms       = 0U;
static uint8_t  s_dyn_roll_cnt    = 0U;
static uint8_t  s_body_roll_cnt   = 0U;

/* Hàm phụ trợ đẩy 1 frame 8-byte vào FDCAN1 Tx FIFO */
static ServiceResult_t CanIf_TransmitStdFrame(uint32_t std_id, const uint8_t data[8])
{
    FDCAN_TxHeaderTypeDef tx_header = {
        .Identifier          = std_id,
        .IdType              = FDCAN_STANDARD_ID,
        .TxFrameType         = FDCAN_DATA_FRAME,
        .DataLength          = FDCAN_DLC_BYTES_8,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch       = FDCAN_BRS_OFF,
        .FDFormat            = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl  = FDCAN_NO_TX_EVENTS,
        .MessageMarker       = 0U
    };

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, (uint8_t *)data) != HAL_OK) {
        return SRV_RESULT_ERROR;
    }
    return SRV_RESULT_ACCEPTED;
}

/* Cài đặt hàm Port gửi ACK 0x500 ngay khi có kết quả lệnh */
ServiceResult_t ComPort_SendCommandAck(const CommandAckMsg_t *ack_msg)
{
    uint8_t payload[8];
    CanCodec_EncodeCommandAck(ack_msg, payload);
    return CanIf_TransmitStdFrame(CAN_ID_COMMAND_ACK, payload);
}

/* Gọi định kỳ mỗi 10 ms từ StartCanTxTask */
void CanScheduler_Step10ms(void)
{
    /* Yêu cầu VEH-001: Chỉ phát các bản tin định kỳ khi ECU đã vào trạng thái RUN */
    if (VehicleStateMgr_GetEcuState() != ECU_STATE_RUN) {
        return;
    }

    s_tick_10ms++;
    uint8_t payload[8];

    /* 1. Chu kỳ 20 ms (Mỗi 2 ticks): Gửi 0x100 VehicleDynamic */
    if ((s_tick_10ms % 2U) == 0U) {
        VehicleMotionSnapshot_t motion;
        VehicleStateMgr_ReadMotion(&motion);
        CanCodec_EncodeVehicleDynamic(&motion, s_dyn_roll_cnt++, payload);
        (void)CanIf_TransmitStdFrame(CAN_ID_VEHICLE_DYNAMIC, payload);
    }

    /* 2. Chu kỳ 100 ms (Mỗi 10 ticks): Gửi 0x101 VehicleStatus & 0x200 BodyStatus */
    if ((s_tick_10ms % 10U) == 0U) {
        VehicleMotionSnapshot_t motion;
        BodyStateSnapshot_t     body;
        VehicleStateMgr_ReadMotion(&motion);
        BodyStateMgr_ReadState(&body);

        CanCodec_EncodeVehicleStatus(&motion, payload);
        (void)CanIf_TransmitStdFrame(CAN_ID_VEHICLE_STATUS, payload);

        CanCodec_EncodeBodyStatus(&body, s_body_roll_cnt++, payload);
        (void)CanIf_TransmitStdFrame(CAN_ID_BODY_STATUS, payload);
    }
}

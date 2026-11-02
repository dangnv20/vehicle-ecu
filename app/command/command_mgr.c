

#include "command_mgr.h"
#include "vehicle_state_mgr.h"
#include "body_state_mgr.h"
#include "app_ports.h"
#include "can_matrix.h"

/* Bộ nhớ lưu lệnh gần nhất để chống lặp lệnh (Idempotency - CMD-002) */
static struct {
    bool     has_history;
    uint8_t  last_seq;
    uint8_t  last_cmd_id;
    uint16_t last_param;
    uint32_t timestamp_ms;
} s_cmd_history;

static void SendImmediateRejectAck(const RemoteCommandReq_t *req, CmdAckResult_t reason)
{
    BodyStateSnapshot_t body_snap;
    BodyStateMgr_ReadState(&body_snap);

    CommandAckMsg_t ack = {
        .sequence        = req->sequence,
        .cmd_id          = req->cmd_id,
        .result          = reason,
        .resulting_state = (uint8_t)body_snap.central_lock
    };
    (void)ComPort_SendCommandAck(&ack);
}

void CommandMgr_Init(void)
{
    s_cmd_history.has_history = false;
}

void CommandMgr_ProcessRemoteCommand(const RemoteCommandReq_t *req)
{
    if (req == NULL) {
        return;
    }

    /* 1. Kiểm tra CRC/Format của frame nhận vào */
    if (!req->crc_valid) {
        SendImmediateRejectAck(req, ACK_RES_REJECT_FORMAT_CRC);
        return;
    }

    /* 2. Kiểm tra ECU đã vào trạng thái RUN chưa (Mục 6.1 Customer Spec) */
    if (VehicleStateMgr_GetEcuState() != ECU_STATE_RUN) {
        SendImmediateRejectAck(req, ACK_RES_REJECT_ECU_STATE);
        return;
    }

    /* 3. Kiểm tra chống lặp lệnh trong cửa sổ 10 giây (CMD-002) */
    uint32_t now = TimePort_GetTickMs();
    if (s_cmd_history.has_history &&
        (req->sequence == s_cmd_history.last_seq) &&
        (req->cmd_id   == s_cmd_history.last_cmd_id) &&
        (req->parameter == s_cmd_history.last_param) &&
        ((now - s_cmd_history.timestamp_ms) <= CFG_CMD_DUPLICATE_WINDOW_MS))
    {
        SendImmediateRejectAck(req, ACK_RES_DUPLICATE);
        return;
    }

    /* 4. Đọc Snapshot trạng thái xe để kiểm tra điều kiện bảo vệ (Guards - CMD-003) */
    VehicleMotionSnapshot_t motion;
    VehicleStateMgr_ReadMotion(&motion);

    switch ((RemoteCmdId_t)req->cmd_id) {
        case CMD_ID_LOCK_DOORS:
            /* Phải kiểm tra chất lượng tín hiệu trước khi tin vào giá trị tốc độ (Mục 1 Kiến trúc) */
            if (motion.speed_quality != SIGNAL_VALID) {
                SendImmediateRejectAck(req, ACK_RES_REJECT_INVALID_SIG);
                return;
            }
            /* Guard Mục 11: Chỉ cho khóa khi IGN_OFF hoặc IGN_ON và Speed < 5 km/h */
            if (((motion.ignition != IGN_STATE_OFF) && (motion.ignition != IGN_STATE_ON)) ||
                (motion.speed_centi_kmh >= CFG_LOCK_MAX_SPEED_CENTI_KMH))
            {
                SendImmediateRejectAck(req, ACK_RES_REJECT_GUARD);
                return;
            }

            /* Lưu lịch sử lệnh hợp lệ và gửi yêu cầu xuống BodyStateManager */
            s_cmd_history.has_history  = true;
            s_cmd_history.last_seq     = req->sequence;
            s_cmd_history.last_cmd_id  = req->cmd_id;
            s_cmd_history.last_param   = req->parameter;
            s_cmd_history.timestamp_ms = now;

            if (BodyStateMgr_RequestDoorLock(true, req->sequence, req->cmd_id) != SRV_RESULT_ACCEPTED) {
                SendImmediateRejectAck(req, ACK_RES_HW_FAILURE);
            }
            break;

        case CMD_ID_UNLOCK_DOORS:
            s_cmd_history.has_history  = true;
            s_cmd_history.last_seq     = req->sequence;
            s_cmd_history.last_cmd_id  = req->cmd_id;
            s_cmd_history.last_param   = req->parameter;
            s_cmd_history.timestamp_ms = now;

            if (BodyStateMgr_RequestDoorLock(false, req->sequence, req->cmd_id) != SRV_RESULT_ACCEPTED) {
                SendImmediateRejectAck(req, ACK_RES_HW_FAILURE);
            }
            break;

        case CMD_ID_FLASH_LIGHTS:
                    s_cmd_history.has_history  = true;
                    s_cmd_history.last_seq     = req->sequence;
                    s_cmd_history.last_cmd_id  = req->cmd_id;
                    s_cmd_history.last_param   = req->parameter;
                    s_cmd_history.timestamp_ms = now;

                    BodyStateMgr_StartFlashLights();
                    SendImmediateRejectAck(req, ACK_RES_SUCCESS); /* Trả ACK ngay, đèn tự nháy nền */
                    break;

                case CMD_ID_HONK:
                    s_cmd_history.has_history  = true;
                    s_cmd_history.last_seq     = req->sequence;
                    s_cmd_history.last_cmd_id  = req->cmd_id;
                    s_cmd_history.last_param   = req->parameter;
                    s_cmd_history.timestamp_ms = now;

                    BodyStateMgr_StartHonkPattern();
                    SendImmediateRejectAck(req, ACK_RES_SUCCESS); /* Trả ACK ngay, còi tự kêu nền */
                    break;
        default:
            SendImmediateRejectAck(req, ACK_RES_REJECT_FORMAT_CRC);
            break;
    }
}

#include "body_state_mgr.h"
#include "app_ports.h"
#include <string.h>

static BodyStateSnapshot_t s_body_state;

/* Theo dõi yêu cầu đang chờ phần cứng thực thi (Pending Request) */
static struct {
    bool     is_pending;
    uint32_t request_id;
    bool     target_lock;
    uint8_t  cmd_seq;
    uint8_t  cmd_id;
} s_pending_lock_req;

void BodyStateMgr_Init(void)
{
    OsPort_EnterCritical();
    memset(&s_body_state, 0, sizeof(s_body_state));
    s_body_state.central_lock = LOCK_STATE_UNLOCKED; /* Mặc định an toàn (VEH-004) */
    memset(&s_pending_lock_req, 0, sizeof(s_pending_lock_req));
    OsPort_ExitCritical();
}

void BodyStateMgr_ReadState(BodyStateSnapshot_t *out_snapshot)
{
    if (out_snapshot == NULL) {
        return;
    }
    OsPort_EnterCritical();
    *out_snapshot = s_body_state;
    OsPort_ExitCritical();
}

ServiceResult_t BodyStateMgr_RequestDoorLock(bool lock, uint8_t cmd_seq, uint8_t cmd_id)
{
    if (s_pending_lock_req.is_pending) {
        return SRV_RESULT_BUSY;
    }

    uint32_t req_id = (uint32_t)cmd_seq + 1000U;
    s_pending_lock_req.is_pending  = true;
    s_pending_lock_req.request_id  = req_id;
    s_pending_lock_req.target_lock = lock;
    s_pending_lock_req.cmd_seq     = cmd_seq;
    s_pending_lock_req.cmd_id      = cmd_id;

    /* Gọi qua Port xuống phần cứng (Không cần biết dưới đó là LED hay Relay) */
    ServiceResult_t res = DoorPort_RequestLock(lock, req_id);
    if (res != SRV_RESULT_ACCEPTED) {
        s_pending_lock_req.is_pending = false;
        FaultPort_Report(FAULT_ACTUATOR_LOCK, true);
    }
    return res;
}

void BodyStateMgr_OnLockActuatorDone(uint32_t request_id, bool hw_success)
{
    if ((!s_pending_lock_req.is_pending) || (s_pending_lock_req.request_id != request_id)) {
        return; /* Bỏ qua sự kiện không khớp */
    }

    CommandAckMsg_t ack = {
        .sequence = s_pending_lock_req.cmd_seq,
        .cmd_id   = s_pending_lock_req.cmd_id
    };

    if (hw_success) {
        /* Chỉ cập nhật trạng thái chính thức sau khi phần cứng báo thành công (Mục 3 Kiến trúc) */
        OsPort_EnterCritical();
        s_body_state.central_lock  = s_pending_lock_req.target_lock ? LOCK_STATE_LOCKED : LOCK_STATE_UNLOCKED;
        s_body_state.updated_at_ms = TimePort_GetTickMs();
        OsPort_ExitCritical();

        FaultPort_Report(FAULT_ACTUATOR_LOCK, false);
        ack.result          = ACK_RES_SUCCESS;
        ack.resulting_state = (uint8_t)s_body_state.central_lock;
    } else {
        /* Báo lỗi phần cứng xuống DtcManager (Trang 1 Tài liệu Kiến trúc) */
        FaultPort_Report(FAULT_ACTUATOR_LOCK, true);
        ack.result          = ACK_RES_HW_FAILURE;
        ack.resulting_state = (uint8_t)s_body_state.central_lock;
    }

    s_pending_lock_req.is_pending = false;

    /* Gửi bản tin ACK 0x500 phản hồi kết quả cho TCU */
    (void)ComPort_SendCommandAck(&ack);
}
static struct {
    uint16_t remaining_ticks_10ms; /* Tổng thời gian còn lại (10s = 1000 ticks) */
    uint8_t  half_period_cnt;      /* Đếm 25 ticks (250ms) để đảo trạng thái 2 Hz */
} s_light_pattern;

static struct {
    uint8_t  pulses_remaining;     /* Số xung còn lại (3 xung) */
    uint16_t phase_ticks_10ms;     /* Đếm 50 ticks (500ms) cho mỗi pha ON/OFF */
    bool     in_on_phase;
} s_horn_pattern;

void BodyStateMgr_StartFlashLights(void)
{
    OsPort_EnterCritical();
    s_light_pattern.remaining_ticks_10ms = 1000U; /* 10 giây */
    s_light_pattern.half_period_cnt      = 25U;   /* 250 ms */
    s_body_state.light_active            = true;
    OsPort_ExitCritical();

    (void)LightPort_SetOutput(true);
}

void BodyStateMgr_StartHonkPattern(void)
{
    OsPort_EnterCritical();
    s_horn_pattern.pulses_remaining = 3U;
    s_horn_pattern.phase_ticks_10ms = 50U; /* 500 ms */
    s_horn_pattern.in_on_phase      = true;
    s_body_state.horn_active        = true;
    OsPort_ExitCritical();

    (void)HornPort_SetOutput(true);
}

/* Gọi mỗi 10 ms cùng với VehicleStateMgr_Step10ms() trong StartVehicleTask */
void BodyStateMgr_Step10ms(void)
{
    /* 1. Xử lý nháy đèn 2 Hz không chặn (Non-blocking) */
    if (s_light_pattern.remaining_ticks_10ms > 0U) {
        s_light_pattern.remaining_ticks_10ms--;
        if (s_light_pattern.remaining_ticks_10ms == 0U) {
            s_body_state.light_active = false;
            (void)LightPort_SetOutput(false);
        } else {
            s_light_pattern.half_period_cnt--;
            if (s_light_pattern.half_period_cnt == 0U) {
                s_light_pattern.half_period_cnt = 25U; /* Nạp lại 250 ms */
                s_body_state.light_active = !s_body_state.light_active;
                (void)LightPort_SetOutput(s_body_state.light_active);
            }
        }
    }

    /* 2. Xử lý còi 3 xung x 500 ms không chặn (Non-blocking) */
    if (s_horn_pattern.pulses_remaining > 0U) {
        if (s_horn_pattern.phase_ticks_10ms > 0U) {
            s_horn_pattern.phase_ticks_10ms--;
        }
        if (s_horn_pattern.phase_ticks_10ms == 0U) {
            if (s_horn_pattern.in_on_phase) {
                /* Kết thúc 500ms kêu -> Chuyển sang 500ms nghỉ */
                s_horn_pattern.in_on_phase      = false;
                s_horn_pattern.phase_ticks_10ms = 50U;
                s_body_state.horn_active        = false;
                (void)HornPort_SetOutput(false);
            } else {
                /* Kết thúc 1 chu kỳ xung */
                s_horn_pattern.pulses_remaining--;
                if (s_horn_pattern.pulses_remaining > 0U) {
                    s_horn_pattern.in_on_phase      = true;
                    s_horn_pattern.phase_ticks_10ms = 50U;
                    s_body_state.horn_active        = true;
                    (void)HornPort_SetOutput(true);
                }
            }
        }
    }
}

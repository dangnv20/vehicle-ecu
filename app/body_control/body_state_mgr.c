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

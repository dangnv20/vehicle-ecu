#include "vehicle_state_mgr.h"
#include "app_ports.h"
#include <string.h>

/* Dữ liệu nội bộ được đóng kín (Single Writer Ownership - Mục 3 Kiến trúc) */
static EcuState_t              s_ecu_state;
static VehicleMotionSnapshot_t s_motion_state;
/* 0.1 km tương ứng với: (speed_centi_kmh) * (số chu kỳ 10ms) = 3,600,000 */
#define ODO_ACCUM_PER_DECI_KM   (3600000UL)

static uint32_t s_odo_accum = 0UL;

void VehicleStateMgr_Init(void)
{
    OsPort_EnterCritical();
    s_ecu_state = ECU_STATE_RUN; /* Sau khi qua SELF_TEST sẽ vào RUN */
    memset(&s_motion_state, 0, sizeof(s_motion_state));
    s_motion_state.ignition      = IGN_STATE_OFF;
    s_motion_state.speed_quality = SIGNAL_VALID;
    s_motion_state.rpm_quality   = SIGNAL_VALID;
    OsPort_ExitCritical();
}


void VehicleStateMgr_Step10ms(void)
{
    uint32_t now = TimePort_GetTickMs();

    OsPort_EnterCritical();

    /* 1. Thực thi VEH-002: Nếu Ignition == OFF, ép Speed và RPM giảm về 0 trong <= 100ms */
    if (s_motion_state.ignition == IGN_STATE_OFF) {
        s_motion_state.speed_centi_kmh = 0U;
        s_motion_state.rpm_quarter     = 0U;
    }

    /* 2. Thực thi VEH-003: Tích phân Odometer chính xác từ tốc độ hợp lệ */
    if ((s_motion_state.speed_quality == SIGNAL_VALID) && (s_motion_state.speed_centi_kmh > 0U)) {
        s_odo_accum += (uint32_t)s_motion_state.speed_centi_kmh;
        while (s_odo_accum >= ODO_ACCUM_PER_DECI_KM) {
            s_odo_accum -= ODO_ACCUM_PER_DECI_KM;
            s_motion_state.odometer_deci_km++;
        }
    }

    s_motion_state.updated_at_ms = now;
    OsPort_ExitCritical();
}
EcuState_t VehicleStateMgr_GetEcuState(void)
{
    return s_ecu_state;
}

void VehicleStateMgr_SetEcuState(EcuState_t new_state)
{
    s_ecu_state = new_state;
}

void VehicleStateMgr_ReadMotion(VehicleMotionSnapshot_t *out_snapshot)
{
    if (out_snapshot == NULL) {
        return;
    }
    /* Khóa ngắn (< 10 us) để copy toàn bộ struct tránh đọc phải dữ liệu đang ghi dở */
    OsPort_EnterCritical();
    *out_snapshot = s_motion_state;
    OsPort_ExitCritical();
}

void VehicleStateMgr_UpdateSpeed(uint16_t speed_centi_kmh, SignalQuality_t quality)
{
    uint32_t now = TimePort_GetTickMs();
    OsPort_EnterCritical();
    s_motion_state.speed_centi_kmh = speed_centi_kmh;
    s_motion_state.speed_quality   = quality;
    s_motion_state.updated_at_ms   = now;
    OsPort_ExitCritical();
}

void VehicleStateMgr_UpdateIgnition(IgnitionState_t ign)
{
    uint32_t now = TimePort_GetTickMs();
    OsPort_EnterCritical();
    s_motion_state.ignition      = ign;
    s_motion_state.updated_at_ms = now;
    /* Quy tắc VEH-002: Khi tắt máy, tốc độ và RPM về 0 */
    if (ign == IGN_STATE_OFF) {
        s_motion_state.speed_centi_kmh = 0U;
        s_motion_state.rpm_quarter     = 0U;
    }
    OsPort_ExitCritical();
}

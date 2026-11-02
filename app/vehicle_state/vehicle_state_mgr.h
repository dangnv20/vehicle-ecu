#ifndef VEHICLE_STATE_MGR_H
#define VEHICLE_STATE_MGR_H

#include "app_types.h"
void VehicleStateMgr_Step10ms(void);
void      VehicleStateMgr_Init(void);
EcuState_t VehicleStateMgr_GetEcuState(void);
void      VehicleStateMgr_SetEcuState(EcuState_t new_state);

/* API cho các module khác đọc bản sao trạng thái an toàn (Snapshot) */
void      VehicleStateMgr_ReadMotion(VehicleMotionSnapshot_t *out_snapshot);

/* API dành cho bộ mô phỏng hoặc CLI cập nhật tốc độ/Ignition */
void      VehicleStateMgr_UpdateSpeed(uint16_t speed_centi_kmh, SignalQuality_t quality);
void      VehicleStateMgr_UpdateIgnition(IgnitionState_t ign);

#endif /* VEHICLE_STATE_MGR_H */

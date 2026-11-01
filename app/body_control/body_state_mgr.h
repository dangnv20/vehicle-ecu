#ifndef BODY_STATE_MGR_H
#define BODY_STATE_MGR_H

#include "app_types.h"

void            BodyStateMgr_Init(void);
void            BodyStateMgr_ReadState(BodyStateSnapshot_t *out_snapshot);
ServiceResult_t BodyStateMgr_RequestDoorLock(bool lock, uint8_t cmd_seq, uint8_t cmd_id);
void            BodyStateMgr_OnLockActuatorDone(uint32_t request_id, bool hw_success);

#endif /* BODY_STATE_MGR_H */

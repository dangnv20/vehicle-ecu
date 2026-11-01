#ifndef COMMAND_MGR_H
#define COMMAND_MGR_H

#include "app_types.h"

void CommandMgr_Init(void);
void CommandMgr_ProcessRemoteCommand(const RemoteCommandReq_t *req);

#endif /* COMMAND_MGR_H */

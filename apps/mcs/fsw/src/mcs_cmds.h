#ifndef MCS_CMDS_H
#define MCS_CMDS_H

#include "mcs_msg.h"

void MCS_SwitchCmd(const MCS_SwitchCmd_t *Msg);
void MCS_CloseCmd(const MCS_CloseCmd_t *Msg);
void MCS_SetDutyCmd(const MCS_SetDutyCmd_t *Msg);

#endif
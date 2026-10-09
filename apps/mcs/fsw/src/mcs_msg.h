#ifndef MCS_MSG_H
#define MCS_MSG_H

#include "cfe.h"
#include "cfe_config.h"

typedef struct{
    float DutyPercent;
} MCS_SetDuty_Payload_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    MCS_SetDuty_Payload_t Payload;

} MCS_SetDutyCmd_t;

typedef struct{
    uint8 Enable;
}  MCS_Switch_Payload_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    MCS_Switch_Payload_t Payload;

} MCS_SwitchCmd_t;


typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;

} MCS_CloseCmd_t;
#endif
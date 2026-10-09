#ifndef MCS_APP_H
#define MCS_APP_H

#include "cfe.h"
#include "cfe_config.h"
#include "pwm.h"
#include "mcs_msgids.h"
#include "mcs_msg.h"

#define PWM_CHIP        0
#define PWM_CHANNEL     0

// se puede serparar en mcs_fcncodes.h
//----------------------------------------------
#define MCS_SWITCH           0
#define MCS_CLOSE            1
#define MCS_SET_DUTY_CC      2
//----------------------------------------------

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
} MCS_APP_DataTlm_t;

typedef struct{

    uint8 CommandCounter;
    CFE_SB_PipeId_t CommandPipe;
    uint32 RunStatus;

    // MCS_APP_DataTlm_t DataTlm;


    pwm_t pwm;
} MCS_APP_Data_t;

extern MCS_APP_Data_t MCS_APP_Data;

void MCS_AppMain(void);
CFE_Status_t MCS_APP_Init(void);
#endif
#include "cfe.h"
#include "mcs_dispatch.h"
#include "mcs_app.h"

MCS_APP_Data_t MCS_APP_Data;

void MCS_AppMain(void)
{
    CFE_Status_t status;
    status = MCS_APP_Init();
    CFE_SB_Buffer_t *SBBufPtr = NULL;

    
    if(status != CFE_SUCCESS){
        MCS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    CFE_ES_WriteToSysLog("MCS: starting\n");


    if (status == CFE_SUCCESS){
        if (PWM_SetDutyPercent(&MCS_APP_Data.pwm, 25.0f) != 0 || PWM_Enable(&MCS_APP_Data.pwm) != 0){
            CFE_ES_WriteToSysLog("MCS: error configurando o habilitando PWM\n");
            MCS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }
    
    while (CFE_ES_RunLoop(&MCS_APP_Data.RunStatus)){
        
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, MCS_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);
        if(status == CFE_SUCCESS){
            MCS_TaskPipe(SBBufPtr);
        } else{
            MCS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }

    }

    PWM_Close(&MCS_APP_Data.pwm);
    CFE_ES_ExitApp(MCS_APP_Data.RunStatus);
}

CFE_Status_t MCS_APP_Init(void){
    CFE_Status_t status;

    CFE_ES_WriteToSysLog("MCS_APP: Iniciando app\n");
    MCS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0 ,CFE_EVS_EventFilter_BINARY);

    if(status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("MCS: Error registering events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    status = CFE_SB_CreatePipe(&MCS_APP_Data.CommandPipe, 8, "MCS_CMD_PIPE");
    if (status != CFE_SUCCESS){
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MCS_CMD_MID), MCS_APP_Data.CommandPipe);
    
    if (status != CFE_SUCCESS){
        return status;
    }

    if(PWM_Init(&MCS_APP_Data.pwm, PWM_CHIP, PWM_CHANNEL, PWM_DEFAULT_PERIOD_NS) != 0){
        CFE_ES_WriteToSysLog("MCS: Error iniciando pwm\n");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}
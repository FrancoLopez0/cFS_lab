#include "mcs_cmds.h"
#include "mcs_app.h"
#include <math.h>

void MCS_SwitchCmd(const MCS_SwitchCmd_t *Msg)
{
    int result;

    if (Msg->Payload.Enable > 1){
        CFE_ES_WriteToSysLog("MCS: Enable debe ser 0 o 1\n");
        return;
    }

    if (!MCS_APP_Data.pwm.initialized){
        CFE_ES_WriteToSysLog("MCS: PWM no inicializado\n");
        return;
    }

    if (Msg->Payload.Enable == 1){
        result = PWM_Enable(&MCS_APP_Data.pwm);
    } else {
        result = PWM_Disable(&MCS_APP_Data.pwm);
    }

    if (result != 0){
        CFE_ES_WriteToSysLog("MCS: no se pudo cambiar el estado\n");
        return;
    }

    MCS_APP_Data.CommandCounter++;
}

void MCS_CloseCmd(const MCS_CloseCmd_t *Msg){
    (void)Msg;

    if(PWM_Close(&MCS_APP_Data.pwm) != 0){
        CFE_ES_WriteToSysLog("MCS: no se pudo cerrar el PWM\n");
        return;
    }
    MCS_APP_Data.CommandCounter++;
}

void MCS_SetDutyCmd(const MCS_SetDutyCmd_t *Msg){
    float duty = Msg->Payload.DutyPercent;

    if(duty < 0.0f || duty > 100.0f){
        CFE_ES_WriteToSysLog("MCS: Duty Cycle invalido\n");
        return;
    }

    if(!MCS_APP_Data.pwm.initialized){
        CFE_ES_WriteToSysLog("MCS: PWM no inicializado\n");
        return;
    }

    if (PWM_SetDutyPercent(&MCS_APP_Data.pwm, duty) != 0){
        CFE_ES_WriteToSysLog("MCS: no se pudo aplicar el duty\n");
        return;
    }

    MCS_APP_Data.CommandCounter++;
}
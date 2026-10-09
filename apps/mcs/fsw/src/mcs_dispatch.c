#include "mcs_dispatch.h"
#include "mcs_cmds.h"
#include "mcs_app.h"

static bool MCS_VerifyCmdLength(const CFE_MSG_Message_t *Msg, CFE_MSG_Size_t ExpectedLength){
    CFE_MSG_Size_t ActualLength = 0;

    CFE_MSG_GetSize(Msg, &ActualLength);

    if (ActualLength != ExpectedLength){
        CFE_ES_WriteToSysLog("MCS: longitud de comando incorrecta\n");
        return false;
    }

    return true;
}

static void MCS_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t status;
    CFE_MSG_FcnCode_t CommandCode = 0;

    status = CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MCS: no se pudo leer el CC\n");
        return;
    }

    switch (CommandCode)
    {
        case MCS_SWITCH:
            CFE_ES_WriteToSysLog("MCS: comando Switch\n");
            if(MCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(MCS_SwitchCmd_t))){
                MCS_SwitchCmd((const MCS_SwitchCmd_t *)SBBufPtr);
            }
            break;

        case MCS_CLOSE:
            CFE_ES_WriteToSysLog("MCS: Comando Close\n");
            if(MCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(MCS_CloseCmd_t))){
                MCS_CloseCmd((const MCS_CloseCmd_t *)SBBufPtr);
            }
            break;

        case MCS_SET_DUTY_CC:
            CFE_ES_WriteToSysLog("MCS: comando SET_DUTY\n");
            if(MCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(MCS_SetDutyCmd_t))){
                MCS_SetDutyCmd((const MCS_SetDutyCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_ES_WriteToSysLog(
                "MCS: CC desconocido: %u\n",
                (unsigned int)CommandCode);
            break;
    }
}

void MCS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr){
    CFE_Status_t status;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    status = CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    if( status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("MCS: no se pudo leer el MID\n");
        return;
    }

    if(!CFE_SB_MsgId_Equal(MsgId, CFE_SB_ValueToMsgId(MCS_CMD_MID))){
        CFE_ES_WriteToSysLog("MCS: MID no reconocido\n");
        return;
    } 

    CFE_ES_WriteToSysLog("MCS: recibido mensaje de comandos\n");
    MCS_ProcessGroundCommand(SBBufPtr);
}
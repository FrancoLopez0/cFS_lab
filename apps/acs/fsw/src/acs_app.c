#include "acs_app.h"
#include "mpu6050.h"

#define ACS_PERF_ID 6767

ACS_APP_Data_t ACS_APP_Data;

void ACS_AppMain(void){
    CFE_Status_t     status;

    // CFE_ES_PerfLogEntry(HELLO_WORLD_PERF_ID);

    status = ACS_APP_Init();
    if(status != CFE_SUCCESS){
        ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while(CFE_ES_RunLoop(&ACS_APP_Data.RunStatus) == true){

        CFE_ES_WriteToSysLog("ACS_APP: Dentro de loop");
        OS_TaskDelay(1000);
    }
    CFE_ES_ExitApp(ACS_APP_Data.RunStatus);
}

CFE_Status_t ACS_APP_Init(void){
    CFE_ES_WriteToSysLog("ACS_APP: Iniciando app");
    ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    return CFE_SUCCESS;
}
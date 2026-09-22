
#include "hello_world_app.h"

#define HELLO_WORLD_PERF_ID 6767

HELLO_WORLD_APP_Data_t HELLO_WORLD_APP_Data;

void HELLO_WORLD_Main(void){
    CFE_Status_t     status;

    // CFE_ES_PerfLogEntry(HELLO_WORLD_PERF_ID);

    status = HELLO_WORLD_APP_Init();
    if(status != CFE_SUCCESS){
        HELLO_WORLD_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while(CFE_ES_RunLoop(&HELLO_WORLD_APP_Data.RunStatus) == true){

        // CFE_ES_PerfLogExit(HELLO_WORLD_PERF_ID);
        // CFE_ES_PerfLogEntry(HELLO_WORLD_PERF_ID);
        CFE_ES_WriteToSysLog("HELLO_WORLD_APP: Dentro de loop");
        OS_TaskDelay(1000);
    }
    CFE_ES_ExitApp(HELLO_WORLD_APP_Data.RunStatus);
}

CFE_Status_t HELLO_WORLD_APP_Init(void){
    CFE_ES_WriteToSysLog("HELLO_WORLD_APP: Iniciando app");
    HELLO_WORLD_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    return CFE_SUCCESS;
}
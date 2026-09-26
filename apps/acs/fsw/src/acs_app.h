#ifndef ACS_APP_H
#define ACS_APP_H

#include "cfe.h"
#include "cfe_config.h"

typedef struct{
    char msg [50];
} ACS_APP_Msg_t;

typedef struct{
    ACS_APP_Msg_t msg;

    uint8 CommandCounter;
    CFE_SB_PipeId_t CommandPipe;
    uint32 RunStatus;
} ACS_APP_Data_t;


void ACS_AppMain(void);
CFE_Status_t ACS_APP_Init(void);

#endif /* ACS_APP_H */
#ifndef HELLO_WORLD_APP_H
#define HELLO_WORLD_APP_H

#include "cfe.h"
#include "cfe_config.h"

typedef struct{
    char msg [50];
} HELLO_WORLD_APP_Msg_t;

typedef struct{
    HELLO_WORLD_APP_Msg_t msg;

    uint8 CommandCounter;
    CFE_SB_PipeId_t CommandPipe;
    uint32 RunStatus;
} HELLO_WORLD_APP_Data_t;


void HELLO_WORLD_Main(void);
CFE_Status_t HELLO_WORLD_APP_Init(void);

#endif /* HELLO_WORLD_APP_H */
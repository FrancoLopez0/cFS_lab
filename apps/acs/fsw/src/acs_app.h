#ifndef ACS_APP_H
#define ACS_APP_H

#include "cfe.h"
#include "cfe_config.h"
#include "mpu6050.h"

typedef struct{
    char msg [50];
} ACS_APP_Msg_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint32_t SampleCounter;
    float AccelX;
    float AccelY;
    float AccelZ;
    float GyroX;
    float GyroY;
    float GyroZ;
} ACS_APP_DataTlm_t;

typedef struct{
    ACS_APP_Msg_t msg;

    uint8 CommandCounter;
    CFE_SB_PipeId_t CommandPipe;
    uint32 RunStatus;

    ACS_APP_DataTlm_t DataTlm;

    mpu6050_t Mpu;

    bool MpuReady;
} ACS_APP_Data_t;


extern ACS_APP_Data_t ACS_APP_Data;


void ACS_AppMain(void);
CFE_Status_t ACS_APP_Init(void);

#endif /* ACS_APP_H */
#include "acs_app.h"
#include "acs_msgids.h"
#include "acs_tlm.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

#define ACS_PERF_ID 6767
#define ACS_PERIOD_MS 10

ACS_APP_Data_t ACS_APP_Data;

void ACS_AppMain(void)
{
    CFE_Status_t status;
    CFE_SB_Buffer_t *SBBufPtr = NULL;

    status = ACS_APP_Init();
    if (status != CFE_SUCCESS){
        ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&ACS_APP_Data.RunStatus) == true){

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ACS_APP_Data.CommandPipe, CFE_SB_POLL);

        if (status == CFE_SUCCESS){
            /* Coming soon: dispatch de comandos. */
        }
        else if (status != CFE_SB_NO_MESSAGE){
            CFE_ES_WriteToSysLog( "ACS: Error receiving, RC = 0x%08lX\n", (unsigned long)status);
            ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
            break;
        }
        
        if(ACS_APP_Data.MpuReady){
            mpu6050_get_data(&ACS_APP_Data.Mpu);
            status = ACS_APP_PublishCounter();

            if (status != CFE_SUCCESS){
                CFE_ES_WriteToSysLog("ACS: error publicando telemetria\n");
                ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
                break;
            }
        }

        if(OS_TaskDelay(ACS_PERIOD_MS) != OS_SUCCESS){
            CFE_ES_WriteToSysLog("ACS: Error waiting between samples\n");
            ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }

    }

    if (ACS_APP_Data.Mpu.file >= 0) {
        close(ACS_APP_Data.Mpu.file);
        ACS_APP_Data.Mpu.file = -1;
    }

    CFE_ES_ExitApp(ACS_APP_Data.RunStatus);
}



CFE_Status_t ACS_APP_Init(void){
    CFE_Status_t status;

    ACS_APP_Data.Mpu.file = -1;
    ACS_APP_Data.Mpu.addr = MPU6050_I2C_ADDR;
    ACS_APP_Data.Mpu.accel_range = RANGE_2G;
    ACS_APP_Data.Mpu.gyro_range = RANGE_250DPS;
    ACS_APP_Data.MpuReady = false;

    CFE_ES_WriteToSysLog("ACS_APP: Iniciando app\n");
    ACS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);

    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("ACS: Error registering events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    status = CFE_MSG_Init(
        CFE_MSG_PTR(ACS_APP_Data.DataTlm.TelemetryHeader),
        CFE_SB_ValueToMsgId(ACS_DATA_TLM_MID),
        sizeof(ACS_APP_Data.DataTlm));

    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("ACS: Error initializing message, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    //Revisar el depth y pipe name como se hace en el sample_app
    status = CFE_SB_CreatePipe(&ACS_APP_Data.CommandPipe, 8, "ACS_CMD_PIPE");

    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("ACS: Error creating pipe, RC = 0x%08lX\n",(unsigned long)status);
        return status;
    }

    if(status == CFE_SUCCESS){
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ACS_SEND_DATA_MID), ACS_APP_Data.CommandPipe);

        if (status != CFE_SUCCESS){
            CFE_ES_WriteToSysLog("ACS: Error subscribing to data request, RC = 0x%08lX\n", (unsigned long)status);
            return status;
        }
    }

    // encapsular en funcion
    ACS_APP_Data.Mpu.file = open("/dev/i2c-1", O_RDWR);

    if( ACS_APP_Data.Mpu.file < 0 ){
        CFE_ES_WriteToSysLog("ACS: no se pudo abrir /dev/i2c-1\n");
    } else if(ioctl(ACS_APP_Data.Mpu.file, I2C_SLAVE, ACS_APP_Data.Mpu.addr) <0 ){
        CFE_ES_WriteToSysLog("ACS: no se pudo seleccionar el MPU en I2C\n");
        close(ACS_APP_Data.Mpu.file);
        ACS_APP_Data.Mpu.file = -1;
    } else{
        mpu6050_init(&ACS_APP_Data.Mpu);
        ACS_APP_Data.MpuReady = true;
    }

    return CFE_SUCCESS;
}
#include "acs_tlm.h"
#include "acs_app.h"

CFE_Status_t ACS_APP_PublishCounter(void){

    ACS_APP_Data.DataTlm.SampleCounter++;
    ACS_APP_Data.DataTlm.AccelX = ACS_APP_Data.Mpu.accel_x;
    ACS_APP_Data.DataTlm.AccelY = ACS_APP_Data.Mpu.accel_y;
    ACS_APP_Data.DataTlm.AccelZ = ACS_APP_Data.Mpu.accel_z;

    ACS_APP_Data.DataTlm.GyroX = ACS_APP_Data.Mpu.gyro_x;
    ACS_APP_Data.DataTlm.GyroY = ACS_APP_Data.Mpu.gyro_y;
    ACS_APP_Data.DataTlm.GyroZ = ACS_APP_Data.Mpu.gyro_z;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ACS_APP_Data.DataTlm.TelemetryHeader));

    return CFE_SB_TransmitMsg(CFE_MSG_PTR(ACS_APP_Data.DataTlm.TelemetryHeader), true);
}
#include "mpu6050.h"

// Direccion del MPU6050
static uint8_t mpu6050_addr;
// Bus de I2C
static int mpu6050_file;


static inline void mpu6050_write(uint8_t reg, uint8_t *src, uint8_t len) {
    // Array completo para enviar
    uint8_t buff[20] = {0};
    // Registro a escribir
    buff[0] = reg;
    // Copio los otros
    for(uint8_t i = 0; i < len; i++) { buff[i + 1] = src[i]; }
    // Inicio la comunicacion
    write(mpu6050_file, buff, len + 1);
}

static inline void mpu6050_read(uint8_t reg, uint8_t *dst, uint8_t len) {
 
    write(mpu6050_file, &reg, 1);

    read(mpu6050_file, dst, len);
}

void sleep_ms(int time){
    sleep((time / 1000));
}

uint8_t mpu6050_fifo_read(void){
    uint8_t dst;
    mpu6050_read(FIFO_R_W_REG, &dst, 1);
    return dst;
}

void mpu6050_fifo_drain(uint8_t *dst, uint16_t samples){
    int count = mpu6050_get_fifo_count();
    if(samples >= count){
        samples = count;
    }
    for(int i=0 ; i<samples; i++){
        dst[i] = mpu6050_fifo_read();
    }
}

int16_t mpu6050_get_fifo_count(void){
    uint8_t dst[2];

    mpu6050_read(FIFO_COUNT_H_REG, dst, 2);

    return (int16_t)(dst[0] << 8 | dst[1]);
}

void mpu6050_set_fifo(uint8_t fifo_config){
    uint8_t data = fifo_config;
    mpu6050_write(FIFO_EN_REG, &data, 1);
}

void mpu6050_fifo_reset(void) {
    uint8_t user_ctrl = 0;
    // mpu6050_read(USER_CTRL_REG, &user_ctrl, 1);

    user_ctrl = FIFO_RESET_BIT;

    mpu6050_write(USER_CTRL_REG, &user_ctrl, 1);

    user_ctrl = FIFO_EN;

    mpu6050_write(USER_CTRL_REG, &user_ctrl, 1);
}

void mpu6050_init(mpu6050_t *mpu6050) {
    // Guardo bus de I2C y direccion
    mpu6050_file = mpu6050->file;
    mpu6050_addr = mpu6050->addr;
    
    // power management register 0X6B we should write all 0's to wake the sensor up
    uint8_t data = INT_OSC;
    mpu6050_write(PWR_MGMT_1_REG, &data, 1);

    data = SAMPLE_RATE_1KHZ;
    mpu6050_write(SMPLRT_DIV_REG, &data, 1);

    mpu6050_write(ACCEL_CONFIG_REG, &(mpu6050->accel_range), 1);
    
    mpu6050_write(GYRO_CONFIG_REG, &(mpu6050->gyro_range), 1);

    mpu6050_set_fifo(0x00);

    data = INT_FIFO_OFLOW_EN;
    mpu6050_write(INT_ENABLE_REG, &data, 1);

    data = 0x40;
    mpu6050_write(USER_CTRL_REG, &data, 1);
    
    data = 0x02;
    mpu6050_write(0x37, &data, 1);
}

void mpu6050_reset(void) {
    uint8_t rst = 0x80;
    mpu6050_write(PWR_MGMT_1_REG, &rst, 1);
}

uint8_t mpu6050_who_am_i(void) {
    // check device ID WHO_AM_I
    uint8_t who_am_i;
    mpu6050_read(WHO_AM_I_REG, &who_am_i, 1);
    return who_am_i;
}

void mpu6050_read_accel(mpu_accel_t *accel) {
    // Array para guardar valores
	uint8_t dst[6] = {0};
	// Leer 14 bytes del MPU6050
    mpu6050_read(ACCEL_XOUT_H_REG, dst, 6);
    // Ajusto valores de aceleracion
    accel->accel_x = (int16_t)(dst[0] << 8 | dst[1]);
    accel->accel_y = (int16_t)(dst[2] << 8 | dst[3]);
    accel->accel_z = (int16_t)(dst[4] << 8 | dst[5]);
}

void mpu6050_read_gyro(mpu_gyro_t *gyro) {
    // Array para guardar valores
	uint8_t dst[6] = {0};
	// Leer 14 bytes del MPU6050
    mpu6050_read(GYRO_XOUT_H_REG, dst, 6);
    // Ajusto valores de aceleracion
    gyro->gyro_x = (int16_t)(dst[0] << 8 | dst[1]);
    gyro->gyro_y = (int16_t)(dst[2] << 8 | dst[3]);
    // printf("raw %d , %d \n", dst[2], dst[3]);
    gyro->gyro_z = (int16_t)(dst[4] << 8 | dst[5]);
}

void mpu6050_get_accel(mpu6050_t *data){
    // Leer acelerometro
    mpu6050_read_accel(&data->raw_accel);
    
    // Convertir a flotantes
    if(data->accel_range == RANGE_2G) {
        data->accel_x = data->raw_accel.accel_x / LSB_2G;
        data->accel_y = data->raw_accel.accel_y / LSB_2G;
        data->accel_z = data->raw_accel.accel_z / LSB_2G;
    } else if(data->accel_range == RANGE_4G) {
        data->accel_x = data->raw_accel.accel_x / LSB_4G;
        data->accel_y = data->raw_accel.accel_y / LSB_4G;
        data->accel_z = data->raw_accel.accel_z / LSB_4G;
    } else if(data->accel_range == RANGE_8G) {
        data->accel_x = data->raw_accel.accel_x / LSB_8G;
        data->accel_y = data->raw_accel.accel_y / LSB_8G;
        data->accel_z = data->raw_accel.accel_z / LSB_8G;
    } else if(data->accel_range == RANGE_16G) {
        data->accel_x = data->raw_accel.accel_x / LSB_16G;
        data->accel_y = data->raw_accel.accel_y / LSB_16G;
        data->accel_z = data->raw_accel.accel_z / LSB_16G;
    }

    data->accel_x -= data->offset_accel_x;
    data->accel_y -= data->offset_accel_y;
    data->accel_z -= data->offset_accel_z;
}

void mpu6050_get_gyro(mpu6050_t *data){
    // Leer giroscopio
    mpu6050_read_gyro(&data->raw_gyro);
    
    // Convertir a flotantes
    if(data->gyro_range == RANGE_250DPS) {
        data->gyro_x = data->raw_gyro.gyro_x / LSB_250DPS;
        data->gyro_y = data->raw_gyro.gyro_y / LSB_250DPS;
        data->gyro_z = data->raw_gyro.gyro_z / LSB_250DPS;
    } else if(data->gyro_range == RANGE_500DPS) {
        data->gyro_x = data->raw_gyro.gyro_x / (LSB_500DPS);
        data->gyro_y = data->raw_gyro.gyro_y / (LSB_500DPS);
        data->gyro_z = data->raw_gyro.gyro_z / (LSB_500DPS);
    } else if(data->gyro_range == RANGE_1000DPS) {
        data->gyro_x = data->raw_gyro.gyro_x / (LSB_1000DPS);
        data->gyro_y = data->raw_gyro.gyro_y / (LSB_1000DPS);
        data->gyro_z = data->raw_gyro.gyro_z / (LSB_1000DPS);
    } else if(data->gyro_range == RANGE_2000DPS) {
        data->gyro_x = data->raw_gyro.gyro_x / (LSB_2000DPS);
        data->gyro_y = data->raw_gyro.gyro_y / (LSB_2000DPS);
        data->gyro_z = data->raw_gyro.gyro_z / (LSB_2000DPS);
    }

    data->gyro_x -= data->offset_gyro_x;
    data->gyro_y -= data->offset_gyro_y;
    data->gyro_z -= data->offset_gyro_z;
}

void mpu6050_get_data(mpu6050_t *data) {
    // Leer acelerometro
    mpu6050_get_accel(data);
    // Leer giroscopio
    mpu6050_get_gyro(data);
}

void mpu6050_read_temp(int16_t *temp) {
    // Array para guardar valores
	uint8_t dst[2] = {0};
	// Leer 14 bytes del MPU6050
    mpu6050_read(TEMP_OUT_H_REG, dst, 6);
    // Ajusto valores de aceleracion
    *temp = (int16_t)(dst[0] << 8 | dst[1]);
}

void mpu6050_calibrate(mpu6050_t *mpu6050) {
    // Obtengo el offset del sensor en cada eje
    mpu6050->offset_accel_x = 0; 
    mpu6050->offset_accel_y = 0;
    mpu6050->offset_accel_z = 0;
    mpu6050->offset_gyro_x = 0; 
    mpu6050->offset_gyro_y = 0;
    mpu6050->offset_gyro_z = 0;

    float offset_accel_x = 0;
    float offset_accel_y = 0;
    float offset_accel_z = 0;
    float offset_gyro_x = 0;
    float offset_gyro_y = 0;
    float offset_gyro_z = 0;

    for(int i = 0; i < NUM_SAMPLES; i++) {
        mpu6050_get_data(mpu6050);
        offset_accel_x += mpu6050->accel_x;
        offset_accel_y += mpu6050->accel_y;
        offset_accel_z += mpu6050->accel_z;
        offset_gyro_x += mpu6050->gyro_x;
        offset_gyro_y += mpu6050->gyro_y;
        offset_gyro_z += mpu6050->gyro_z;
        sleep_ms(10);
    }

    mpu6050->offset_accel_x = offset_accel_x / (NUM_SAMPLES-1);
    mpu6050->offset_accel_y = offset_accel_y / (NUM_SAMPLES-1);
    mpu6050->offset_accel_z = (offset_accel_z / (NUM_SAMPLES-1)) - 1.0f; // Adjust for gravity
    mpu6050->offset_gyro_x  = offset_gyro_x  / (NUM_SAMPLES-1);
    mpu6050->offset_gyro_y  = offset_gyro_y  / (NUM_SAMPLES-1);
    mpu6050->offset_gyro_z  = offset_gyro_z  / (NUM_SAMPLES-1);

    // Obtengo la varianza de los datos
    mpu6050->variance_accel_x = 0;   
    mpu6050->variance_accel_y = 0;
    mpu6050->variance_accel_z = 0;
    mpu6050->variance_gyro_x  = 0;
    mpu6050->variance_gyro_y  = 0;
    mpu6050->variance_gyro_z  = 0;

    for(int i = 0; i < NUM_SAMPLES; i++) {
        mpu6050_get_data(mpu6050);
        mpu6050->variance_accel_x += (mpu6050->accel_x) * (mpu6050->accel_x);
        mpu6050->variance_accel_y += (mpu6050->accel_y) * (mpu6050->accel_y);
        mpu6050->variance_accel_z += (mpu6050->accel_z) * (mpu6050->accel_z);
        mpu6050->variance_gyro_x  += (mpu6050->gyro_x)  * (mpu6050->gyro_x);
        mpu6050->variance_gyro_y  += (mpu6050->gyro_y)  * (mpu6050->gyro_y);
        mpu6050->variance_gyro_z  += (mpu6050->gyro_z)  * (mpu6050->gyro_z);
        sleep_ms(10);
    }

    mpu6050->variance_accel_x/= NUM_SAMPLES - 1;
    mpu6050->variance_accel_y/= NUM_SAMPLES - 1;
    mpu6050->variance_accel_z/= NUM_SAMPLES - 1;
    mpu6050->variance_gyro_x /= NUM_SAMPLES - 1;
    mpu6050->variance_gyro_y /= NUM_SAMPLES - 1;
    mpu6050->variance_gyro_z /= NUM_SAMPLES - 1;

    // Calculo la desviacion estandar
    mpu6050->std_dev_accel_x = sqrt(mpu6050->variance_accel_x);
    mpu6050->std_dev_accel_y = sqrt(mpu6050->variance_accel_y);
    mpu6050->std_dev_accel_z = sqrt(mpu6050->variance_accel_z);
    mpu6050->std_dev_gyro_x = sqrt(mpu6050->variance_gyro_x);
    mpu6050->std_dev_gyro_y = sqrt(mpu6050->variance_gyro_y);
    mpu6050->std_dev_gyro_z = sqrt(mpu6050->variance_gyro_z);
}
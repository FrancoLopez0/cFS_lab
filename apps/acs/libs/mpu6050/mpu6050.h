#ifndef MPU6050_H
#define MPU6050_H

#include "stdio.h"
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_SAMPLES 200 // Number of samples for calibrations

#define SMPLRT_DIV_REG      0x19
#define GYRO_CONFIG_REG     0x1B
#define ACCEL_CONFIG_REG    0x1C
#define ACCEL_XOUT_H_REG    0x3B
#define TEMP_OUT_H_REG      0x41
#define GYRO_XOUT_H_REG     0x43
#define PWR_MGMT_1_REG      0x6B
#define WHO_AM_I_REG        0x75
#define FIFO_EN_REG         0x23
#define FIFO_R_W_REG		0x74
#define FIFO_COUNT_H_REG    0x72
#define FIFO_COUNT_L_REG    0x73
#define INT_ENABLE_REG      0x38
#define USER_CTRL_REG		0x6A
#define MPU_CONFIG_REG      0x1A

#define MPU6050_I2C_ADDR  	0x69 // Default I2C address for MPU6050
#define INT_OSC           	0x00 // Internal oscillator
#define SAMPLE_RATE_1KHZ  	0x07

#define INT_FIFO_OFLOW_EN 	0x10
#define INT_DATA_RDY_EN     0x01
#define FIFO_EN 			0x40

#define RANGE_2G   			0x00  // AFS_SEL=0 << 3
#define RANGE_4G   			0x08  // AFS_SEL=1 << 3
#define RANGE_8G   			0x10  // AFS_SEL=2 << 3
#define RANGE_16G  			0x18  // AFS_SEL=3 << 3

#define RANGE_250DPS  		0x00
#define RANGE_500DPS  		0x01
#define RANGE_1000DPS 		0x02
#define RANGE_2000DPS 		0x03

#define LSB_2G 				16384.0f // LSB for 2G range
#define LSB_4G 				8192.0f  // LSB for 4G range
#define LSB_8G 				4096.0f  // LSB for 8G range
#define LSB_16G 			2048.0f // LSB for 16G range

#define LSB_250DPS 			131.0f // LSB for 250 DPS
#define LSB_500DPS 			65.5f  // LSB for 500 DPS
#define LSB_1000DPS 		32.8f // LSB for 100
#define LSB_2000DPS 		16.4f // LSB for 2000 DPS

#define ACCEL_FIFO_EN     	0x08
#define FIFO_OFF          	0x00
#define FIFO_RESET_BIT    	0x04

typedef struct {
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
} mpu_accel_t;

typedef struct {
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
} mpu_gyro_t;

typedef struct {
	mpu_accel_t raw_accel;
	mpu_gyro_t raw_gyro;
	uint8_t fifo;
	float accel_x;
	float accel_y;
	float accel_z;
	float gyro_x;
	float gyro_y;
	float gyro_z;
	float offset_accel_x;
	float offset_accel_y;
	float offset_accel_z;
	float offset_gyro_x;
	float offset_gyro_y;
	float offset_gyro_z;
	float variance_accel_x;
	float variance_accel_y;
	float variance_accel_z;
	float variance_gyro_x;
	float variance_gyro_y;
	float variance_gyro_z;
	float std_dev_accel_x;
	float std_dev_accel_y;
	float std_dev_accel_z;
	float std_dev_gyro_x;
	float std_dev_gyro_y;
	float std_dev_gyro_z;
	uint8_t accel_range;
	uint8_t gyro_range;
	int file;
	uint8_t addr;
} mpu6050_t; 

void mpu6050_init(mpu6050_t *mpu6050);
void mpu6050_reset(void);
uint8_t mpu6050_who_am_i(void);
void mpu6050_read_accel(mpu_accel_t *accel);
void mpu6050_read_gyro(mpu_gyro_t *gyro);
void mpu6050_read_temp(int16_t *temp);
void mpu6050_get_accel(mpu6050_t *data);
void mpu6050_get_gyro(mpu6050_t *data);
void mpu6050_get_data(mpu6050_t *data);
void mpu6050_calibrate(mpu6050_t *mpu6050);
int16_t mpu6050_get_fifo_count(void);
void mpu6050_set_fifo(uint8_t fifo_config);
uint8_t mpu6050_fifo_read(void);
void mpu6050_fifo_drain(uint8_t *dst, uint16_t samples);
void mpu6050_fifo_reset(void);

#endif // MPU6050_H

/*
 * mpu6050.h
 *
 *  Created on: Nov 13, 2019
 *  Author: Bulanov Konstantin
 */

#ifndef _INC_GY521_H_
#define _INC_GY521_H_

#include <stdint.h>
#include "i2c.h"

// MPU6050 structure
typedef struct
{

    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;

    float Temperature;

    double KalmanAngleX;
    double KalmanAngleY;
} MPU6050_t;

// Kalman structure
typedef struct
{
    double Q_angle;
    double Q_bias;
    double R_measure;
    double angle;
    double bias;
    double P[2][2];
} Kalman_t;

//define accel range config
typedef enum {
    ACCEL_RANGE_2G = 0x00,
    ACCEL_RANGE_4G = 0x08,
    ACCEL_RANGE_8G = 0x10,
    ACCEL_RANGE_16G = 0x18
} AccelRange;

//static float get_threshold(AccelRange range) {
//    switch (range) {
//        case ACCEL_RANGE_2G: return 2.0f;
//        case ACCEL_RANGE_4G: return 4.0f;
//        case ACCEL_RANGE_8G: return 8.0f;
//        case ACCEL_RANGE_16G: return 16.0f;
//        default: return 2.0f; // Default to 2g in case of an error
//    }
//}


extern uint8_t current_accel_range;

// char array for acc_x, acc_y, acc_z
extern char acc_x_str[13];
extern char acc_y_str[13];
extern char acc_z_str[13];
extern char IMU_Temp[13];

// MPU6050
extern MPU6050_t MPU6050;

// Function Prototyping
uint8_t MPU6050_Init(I2C_HandleTypeDef *I2Cx);

void MPU6050_Read_Accel(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_Read_Gyro(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_Read_Temp(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_Read_All(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt);

void adjust_accel_range(I2C_HandleTypeDef *I2Cx, AccelRange new_range);

#endif /* _INC_GY521_H_ */

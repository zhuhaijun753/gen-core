/*
 * gyro.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */

#ifndef GYRO_H_
#define GYRO_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Drivers/_mpu6050.h"

/* Exported constants --------------------------------------------------------*/
#define GRAVITY_FORCE                    (float) 9.8

#define MOVED_LIMIT                    (uint8_t) (5)
#define GYROSCOPE_LIMIT                (uint8_t) (45)
#define ACCELEROMETER_LIMIT           (uint32_t) (7000)

#define RAD2DEG(rad)                             ((rad) * 180.0 / M_PI)

/* Exported struct ------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
  int8_t yaw;
  int8_t pitch;
  int8_t roll;
} motion_t;

typedef struct {
  float yaw;
  float pitch;
  float roll;
} motion_float_t;

typedef struct {
  int32_t x;
  int32_t y;
  int32_t z;
} coordinate_t;

typedef struct {
  coordinate_t accelerometer;
  coordinate_t gyroscope;
  float temperature;
} mems_t;

typedef struct {
  struct {
    uint8_t state;
    uint32_t value;
  } fall;
  struct {
    uint8_t state;
    int32_t value;
  } crash;
  uint8_t fallen;
} movement_t;

typedef struct {
  MPU6050 mpu;
  uint8_t detector_init;
  I2C_HandleTypeDef *pi2c;
} gyro_t;

/* Public functions prototype ------------------------------------------------*/
void GYRO_Init(void);
void GYRO_DeInit(void);
void GYRO_Decision(movement_t *movement);
void GYRO_MonitorMovement(void);
void GYRO_ResetDetector(void);

#endif /* GYRO_H_ */

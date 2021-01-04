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

/* Exported constants --------------------------------------------------------*/
#define GRAVITY_FORCE                    (float) 9.8

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
} gyro_t;

typedef struct {
	struct {
		uint8_t state;
		uint32_t value;
	} fall;
	struct {
		uint8_t state;
		int32_t value;
	} crash;
} gyro_decision_t;

typedef struct {
	I2C_HandleTypeDef *hi2c;
} gyro_handler_t;

/* Public functions prototype ------------------------------------------------*/
void GYRO_Init(I2C_HandleTypeDef *hi2c);
gyro_decision_t GYRO_Decision(uint16_t sample, motion_t *motion);
void GYRO_Debugger(gyro_decision_t *decider);

#endif /* GYRO_H_ */

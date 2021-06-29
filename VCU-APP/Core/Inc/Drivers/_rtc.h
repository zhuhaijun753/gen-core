/*
 * _rtc.h
 *
 *  Created on: Oct 9, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef RTC_H_
#define RTC_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
#define RTC_CALIBRATION_MINUTES ((uint8_t)60)

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  int8_t tzQuarterHour;
} timestamp_t;

typedef struct __attribute__((packed)) {
  uint8_t Year;
  uint8_t Month;
  uint8_t Date;
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
  uint8_t WeekDay;
} datetime_t;

/* Public functions prototype
 * --------------------------------------------*/
void RTC_Init(void);
datetime_t RTC_Read(void);
void RTC_Write(datetime_t dt);
uint8_t RTC_NeedCalibration(void);
void RTC_Calibrate(timestamp_t *ts);
uint8_t RTC_Daylight(void);

#endif /* RTC_H_ */

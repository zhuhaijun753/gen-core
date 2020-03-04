/*
 * _rtc.h
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

#ifndef RTC_H_
#define RTC_H_

#include "main.h"
#include "_config.h"

typedef struct {
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
} timestamp_t;

timestamp_t RTC_Decode(uint64_t dateTime);
uint64_t RTC_Encode(timestamp_t timestamp);
uint64_t RTC_Read(void);
void RTC_Write(uint64_t dateTime);
void RTC_Read_RAW(timestamp_t *timestamp);
void RTC_Write_RAW(timestamp_t *timestamp);
uint8_t RTC_Daylight(void);

#endif /* RTC_H_ */

/*
 * _utils.h
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#ifndef UTILS_H_
#define UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include "_defines.h"
#include "Drivers/_log.h"

/* Public functions prototype ------------------------------------------------*/
void _DelayMS(uint32_t ms);
uint32_t _GetTickMS(void);
uint8_t _LedRead(void);
void _LedWrite(uint8_t state);
void _LedToggle(void);
void _Error(char msg[50]);
uint32_t _ByteSwap32(uint32_t x);

#if (!BOOTLOADER)
void _BuzzerWrite(uint8_t state);
void _DummyDataGenerator(void);
int8_t _BitPosition(uint64_t event_id);
#endif
#endif /* UTILS_H_ */

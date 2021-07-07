/*
 * _bat.h
 *
 *  Created on: Dec 11, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__BAT_H_
#define INC_LIBS__BAT_H_

/* Includes
 * ---------------------------------- ----------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
#define BAT_SAMPLE_SZ ((uint8_t)100)
#define ADC_MAX_VALUE ((float)4095.0)  // 12 bit
#define REF_MAX_MV ((float)3300.0)
#define BAT_MAX_MV ((float)4150.0)
#define BAT_OFFSET_MV ((float)0.0)

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint16_t voltage;
  uint16_t buf[BAT_SAMPLE_SZ];
  ADC_HandleTypeDef* padc;
} bat_t;

/* Public functions prototype
 * --------------------------------------------*/
void BAT_Init(void);
void BAT_DeInit(void);
uint16_t BAT_ScanValue(void);

#endif /* INC_LIBS__BAT_H_ */

/*
 * _ublox.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef GPS_H_
#define GPS_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_nmea.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define GPS_TIMEOUT (uint16_t)5000 // in ms
#define GPS_LENGTH_MIN (uint8_t)100

/* Exported struct -----------------------------------------------------------*/
typedef struct {
	uint8_t active;
	uint32_t tick;
	nmea_t nmea;
} gps_data_t;

typedef struct {
	gps_data_t d;
	UART_HandleTypeDef *puart;
	DMA_HandleTypeDef *pdma;
} gps_t;

/* Exported variables
 * ----------------------------------------------------------*/
extern gps_t GPS;

/* Public functions prototype ------------------------------------------------*/
uint8_t GPS_Init(void);
void GPS_DeInit(void);
void GPS_Refresh(void);
void GPS_Flush(void);
void GPS_ReceiveCallback(void *ptr, size_t len);

#endif /* GPS_H_ */

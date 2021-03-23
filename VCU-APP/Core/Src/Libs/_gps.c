/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_gps.h"
#include "DMA/_dma_ublox.h"
#include "Nodes/VCU.h"
#include "usart.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osThreadId_t GpsTaskHandle;
extern osMutexId_t GpsMutexHandle;
#endif

/* Public variables
 * ----------------------------------------------------------*/
nmea_t GPS = {0};

/* Private variables
 * ----------------------------------------------------------*/
static gps_t gps = {
		.puart = &huart2,
		.pdma = &hdma_usart2_rx,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t DataInvalid(void);
#if GPS_DEBUG
static void Debugger(char *ptr, size_t len);
#endif

/* Public functions implementation
 * --------------------------------------------*/
void GPS_Init(void) {
	lock();
	MX_USART2_UART_Init();
	UBLOX_DMA_Start(gps.puart, gps.pdma, GPS_ProcessBuffer);

	// Initiate Module
	do {
		printf("GPS:Init\n");

		GATE_GpsReset();
		_DelayMS(5000);
	} while (DataInvalid());

	nmea_init(&GPS);
	unlock();
}

void GPS_DeInit(void) {
	lock();
	GATE_GpsShutdown();
	UBLOX_DMA_Stop();
	HAL_UART_DeInit(gps.puart);
	unlock();
}

void GPS_ProcessBuffer(void *ptr, size_t len) {
	if (DataInvalid())
		return;

#if GPS_DEBUG
	Debugger(ptr, len);
#endif
	nmea_process(&GPS, (char *)ptr, len);
	osThreadFlagsSet(GpsTaskHandle, FLAG_GPS_RECEIVED);
}

uint8_t GPS_CalculateOdometer(void) {
	float speed_mps = nmea_to_speed(GPS.speed, nmea_speed_mps);

	if (speed_mps)
		return (speed_mps * GPS_INTERVAL);
	return 0;
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(GpsMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
	osMutexRelease(GpsMutexHandle);
#endif
}

static uint8_t DataInvalid(void) {
	return strnlen(UBLOX_UART_RX, UBLOX_UART_RX_SZ) < GPS_LENGTH_MIN;
}

#if GPS_DEBUG
static void Debugger(char *ptr, size_t len) {
	printf("\n%.*s\n", len, ptr);
}
#endif

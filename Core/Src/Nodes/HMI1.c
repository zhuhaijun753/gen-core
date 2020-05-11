/*
 * HMI1.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "HMI1.h"

/* Public variables -----------------------------------------------------------*/
hmi1_t HMI1 = {
		.d = { 0 },
		HMI1_Init,
		HMI1_RefreshIndex,
};

/* Public functions implementation --------------------------------------------*/
void HMI1_Init(void) {
	// reset HMI1 data
	HMI1.d.started = 0;
	HMI1.d.status.mirroring = 0;
	HMI1.d.status.warning = 0;
	HMI1.d.status.overheat = 0;
	HMI1.d.status.finger = 0;
	HMI1.d.status.keyless = 0;
	HMI1.d.status.daylight = 0;
	for (uint8_t i = 0; i < HMI1_DEV_MAX; i++) {
		HMI1.d.device[i].started = 0;
		HMI1.d.device[i].tick = 0;
	}
}

void HMI1_RefreshIndex(void) {
	for (uint8_t i = 0; i < HMI1_DEV_MAX; i++) {
		if ((osKernelGetTickCount() - HMI1.d.device[i].tick) > pdMS_TO_TICKS(1000)) {
			HMI1.d.device[i].started = 0;
		}
	}
}

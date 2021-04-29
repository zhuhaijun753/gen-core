/*
 * HMI1.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI1_H_
#define INC_NODES_HMI1_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Drivers/_canbus.h"

/* Exported constants --------------------------------------------------------*/
#define HMI1_TIMEOUT_MS ((uint32_t)10000)

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
	uint8_t active;
	uint32_t tick;
	uint16_t version;
} hmi1_data_t;

typedef struct {
	hmi1_data_t d;
	struct {
		void (*State)(can_rx_t *);
	} r;
	void (*Init)(void);
	void (*Refresh)(void);
	void (*Power)(GPIO_PinState);
} hmi1_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern hmi1_t HMI1;

/* Public functions implementation
 * --------------------------------------------*/
void HMI1_Init(void);
void HMI1_Refresh(void);
void HMI1_RX_State(can_rx_t *Rx);

#endif /* INC_NODES_HMI1_H_ */

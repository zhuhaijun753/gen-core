/*
 * DMA_Ublox.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef DMA_UBLOX_H_
#define DMA_UBLOX_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define UBLOX_UART_RX_SZ     (uint16_t) 512
#define UBLOX_DMA_RX_SZ      (uint16_t) (UBLOX_UART_RX_SZ/(UBLOX_UART_RX_SZ/128))

/* Public functions prototype ------------------------------------------------*/
void UBLOX_DMA_Init(void);
void UBLOX_DMA_IrqHandler(void);
void UBLOX_USART_IrqHandler(void);

#endif /* DMA_UBLOX_H_ */

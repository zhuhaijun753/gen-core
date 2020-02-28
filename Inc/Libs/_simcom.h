/*
 * simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef SIMCOM_H_
#define SIMCOM_H_

#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>							// for: sprintf()
#include "_config.h"
#include "_reporter.h"
#include "_DMA_Simcom.h"

#define SIMCOM_STATUS_SEND 					">"
#define SIMCOM_STATUS_SENT					"SEND OK\r\n"
#define SIMCOM_STATUS_OK 						"OK\r\n"
#define SIMCOM_STATUS_ERROR 				"ERROR\r\n"
#define SIMCOM_STATUS_READY 				"RDY\r"
#define SIMCOM_BOOT_COMMAND					"AT\r"
#define SIMCOM_RESPONSE_IPD					"+IPD,"

/* Public typedef -----------------------------------------------------------*/
typedef struct {
	char CMD_CIPSTART[100];
	char CMD_CSTT[75];
	char CMD_CNMP[12];
} simcom_t;

/* Public functions ---------------------------------------------------------*/
void Ublox_Init(gps_t *hgps);
void Simcom_Init(void);
uint8_t Simcom_Command(char *cmd, uint32_t ms);
uint8_t Simcom_Upload(char *message, uint16_t length);
uint8_t Simcom_Read_Command(command_t *command);
uint8_t Simcom_Read_IPD(void);
uint8_t Simcom_Read_ACK(ack_t *ack, report_header_t *report_header);
uint8_t Simcom_Read_Signal(uint8_t *signal);
uint8_t Simcom_Read_Carrier_Time(timestamp_t *timestamp);

#endif /* SIMCOM_H_ */

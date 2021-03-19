/*
 * VCU.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_VCU_H_
#define INC_NODES_VCU_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Libs/_gps.h"
#include "Libs/_gyro.h"

/* Exported define
 * ------------------------------------------------------------*/
typedef enum {
  EVG_NET_SOFT_RESET = 0,
  EVG_NET_HARD_RESET,
  EVG_REMOTE_MISSING,
  EVG_BIKE_FALLEN,
  EVG_BIKE_MOVED,
	EVG_BMS_ERROR,
	EVG_MCU_ERROR
} EVENTS_GROUP_BIT;

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
  uint8_t wakeup;
  uint16_t stack;
} task_t;

typedef struct __attribute__((packed)) {
  task_t manager;
  task_t iot;
  task_t reporter;
  task_t command;
  task_t gps;
  task_t gyro;
  task_t remote;
  task_t finger;
  task_t audio;
  task_t gate;
  task_t canRx;
  task_t canTx;
  //  task_t hmi2Power;
} rtos_task_t;

typedef struct {
  uint8_t error;
  uint8_t override;
  vehicle_state_t state;
  uint16_t bat;
  uint16_t events;
  uint32_t uptime;
  uint8_t driver_id;
  uint16_t interval;
  struct {
    uint32_t starter;
  } gpio;
  struct {
    uint32_t independent;
  } tick;
  motion_t motion;
  gps_data_t gps;
  rtos_task_t task;
} vcu_data_t;

typedef struct {
  vcu_data_t d;
  struct {
    uint8_t (*Heartbeat)(void);
    uint8_t (*SwitchModeControl)(void);
    uint8_t (*Datetime)(datetime_t);
    uint8_t (*MixedData)(void);
    uint8_t (*TripData)(void);
  } t;
  void (*Init)(void);
  void (*Refresh)(void);
  void (*NodesInit)(void);
  void (*NodesRefresh)(void);
  void (*CheckState)(void);
  uint8_t (*CheckRTOS)(void);
  void (*CheckStack)(void);
  void (*SetEvent)(uint8_t, uint8_t);
  uint8_t (*ReadEvent)(uint8_t);
  void (*SetDriver)(uint8_t);
  void (*SetOdometer)(uint8_t);
} vcu_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern vcu_t VCU;

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void);
void VCU_Refresh(void);
void VCU_NodesInit(void);
void VCU_NodesRefresh(void);
void VCU_SetEvent(uint8_t bit, uint8_t value);
uint8_t VCU_ReadEvent(uint8_t bit);
void VCU_SetDriver(uint8_t driver_id);
void VCU_SetOdometer(uint8_t meter);
void VCU_CheckState(void);
uint8_t VCU_CheckRTOS(void);
void VCU_CheckStack(void);

uint8_t VCU_TX_Heartbeat(void);
uint8_t VCU_TX_SwitchModeControl(void);
uint8_t VCU_TX_Datetime(datetime_t dt);
uint8_t VCU_TX_MixedData(void);
uint8_t VCU_TX_TripData(void);

#endif /* INC_NODES_VCU_H_ */

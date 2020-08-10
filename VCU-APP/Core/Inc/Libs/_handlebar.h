/*
 * _handlebar.h
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

#ifndef LIBS__HANDLEBAR_H_
#define LIBS__HANDLEBAR_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
// EXTI list
#define SW_K_TOTAL                              7
#define SW_K_SELECT                             0
#define SW_K_SET                                1
#define SW_K_SEIN_LEFT                          2
#define SW_K_SEIN_RIGHT                         3
#define SW_K_REVERSE                            4
#define SW_K_ABS                                5
#define SW_K_LAMP                               6

/* Exported enum ----------------------------------------------------------------*/
typedef enum {
    SW_M_DRIVE = 0,
    SW_M_TRIP,
    SW_M_REPORT,
    SW_M_MAX = 2
} SW_MODE;

typedef enum {
    SW_M_DRIVE_ECONOMY = 0,
    SW_M_DRIVE_STANDARD,
    SW_M_DRIVE_SPORT,
    SW_M_DRIVE_PERFORMANCE,
    SW_M_DRIVE_MAX = 3
} SW_MODE_DRIVE;

typedef enum {
    SW_M_TRIP_ODO = 0,
    SW_M_TRIP_A,
    SW_M_TRIP_B,
    SW_M_TRIP_MAX = 2
} SW_MODE_TRIP;

typedef enum {
    SW_M_REPORT_RANGE = 0,
    SW_M_REPORT_AVERAGE,
    SW_M_REPORT_MAX = 1
} SW_MODE_REPORT;

/* Exported struct --------------------------------------------------------------*/
typedef struct {
    uint8_t left;
    uint8_t right;
} sein_state_t;

typedef struct {
    uint8_t max[SW_M_MAX + 1];
    uint8_t val[SW_M_MAX + 1];
    uint8_t report[SW_M_REPORT_MAX + 1];
    uint32_t trip[SW_M_TRIP_MAX + 1];
} sw_sub_t;

typedef struct {
    uint8_t listening;
    uint8_t hazard;
    uint8_t reverse;
    struct {
        SW_MODE sel;
        sw_sub_t sub;
    } mode;
} sw_runner_t;

typedef struct {
    struct {
        GPIO_TypeDef *port;
        char event[20];
        uint16_t pin;
        uint8_t state;
    } list[SW_K_TOTAL];
    struct {
        uint32_t start;
        uint8_t running;
        uint8_t time;
    } timer[2];
    sw_runner_t runner;
} sw_t;

/* Public functions prototype ------------------------------------------------*/
void HBAR_ReadStates(void);
void HBAR_CheckReverse(void);
void HBAR_TimerSelectSet(void);
void HBAR_RunSelect(void);
void HBAR_RunSet(void);
void HBAR_AccumulateSubTrip(void);

sein_state_t HBAR_SeinController(sw_t *sw);
uint8_t HBAR_ModeController(sw_runner_t *runner);

#endif /* LIBS__HANDLEBAR_H_ */

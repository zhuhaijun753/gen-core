/*
 * _utils.h
 *
 *  Created on: Aug 26, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef UTILS_H_
#define UTILS_H_

/* Includes
 * --------------------------------------------*/
#include "_defs.h"
#include "Drivers/_gate.h"
#include "Drivers/_log.h"
#include "App/_iap.h"

#if (APP)
#include "App/_event.h"
#include "Libs/_rtos.h"
#endif

/* Exported macros
 * --------------------------------------------*/
#define BIT(x) (1ULL << x)
#define BV(var, x) (var |= (1ULL << x))
#define BC(var, x) (var &= ~(1ULL << x))
#define BT(var, x) (var ^= (1ULL << x))

#define CHARISNUM(x) ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x) ((x) - '0')

#define MIN(a, b) ((a < b) ? a : b)

#define MAX_U8(X) ((X) > UINT8_MAX ? UINT8_MAX : (X))
#define RAD2DEG(X) ((X)*180.0 / M_PI)

/* Exported unions
 * --------------------------------------------*/
typedef union {
  uint8_t u8[8];
  uint16_t u16[4];
  uint32_t u32[2];
  uint64_t u64;
  int8_t s8[8];
  int16_t s16[4];
  int32_t s32[2];
  int64_t s64;
  char CHAR[8];
  float FLOAT[2];
  double DOUBLE;
} UNION64;

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  VEHICLE_UNKNOWN = -3,
  VEHICLE_LOST = -2,
  VEHICLE_BACKUP = -1,
  VEHICLE_NORMAL = 0,
  VEHICLE_STANDBY = 1,
  VEHICLE_READY = 2,
  VEHICLE_RUN = 3,
} vehicle_state_t;

typedef enum {
  PAYLOAD_RESPONSE = 0,
  PAYLOAD_REPORT,
  PAYLOAD_MAX = 2,
} PAYLOAD_TYPE;

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  double sum;
  uint16_t pos;
  uint16_t len;
} avg_float_t;

/* Public functions prototype
 * --------------------------------------------*/
void _DelayMS(uint32_t ms);
uint32_t _GetTickMS(void);
uint8_t _TickOut(uint32_t tick, uint32_t ms);
uint8_t _TickIn(uint32_t tick, uint32_t ms);
void _Error(char msg[50]);
uint32_t _ByteSwap32(uint32_t x);
float _MovAvgFloat(avg_float_t* m, float* buf, uint16_t sz, float val);
#endif /* UTILS_H_ */
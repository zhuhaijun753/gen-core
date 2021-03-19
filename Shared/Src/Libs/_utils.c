/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions implementation
 * --------------------------------------------*/
#if RTOS_ENABLE
uint8_t _osThreadFlagsWait(uint32_t *notif, uint32_t flags, uint32_t options,
                           uint32_t timeout) {
  *notif = osThreadFlagsWait(flags, options, timeout);

  if (*notif > EVT_MASK)
    return 0;

  return (*notif & flags) > 0;
}
#endif

void _DelayMS(uint32_t ms) {
#if RTOS_ENABLE
  osDelay(ms);
#else
  HAL_Delay(ms);
#endif
}

uint32_t _GetTickMS(void) {
#if RTOS_ENABLE
  return osKernelGetTickCount();
#else
  return HAL_GetTick();
#endif
}

void _Error(char msg[50]) {
#if RTOS_ENABLE
  if (osKernelGetState() == osKernelRunning)
    printf(msg);
#else
  printf(msg);
#endif

  // indicator error
  //	while (1) {
  //		GATE_LedToggle();
  //		HAL_Delay(50);
  //	}
}

uint32_t _ByteSwap32(uint32_t x) {
  uint32_t y = (x >> 24) & 0xff;
  y |= ((x >> 16) & 0xff) << 8;
  y |= ((x >> 8) & 0xff) << 16;
  y |= (x & 0xff) << 24;

  return y;
}

// int8_t _BitPos(uint64_t event_id) {
//  int8_t pos = -1;
//
//  for (int8_t i = 0; i < 64; i++)
//    if (event_id & BIT(i)) {
//      pos = i;
//      break;
//    }
//
//  return pos;
//}

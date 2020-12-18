/*
 * _flash.h
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported macro function ---------------------------------------------------*/
/* NOTE: EEPROM is 32 bytes aligned, do not store variable in intersection */
#define EE_AREA(ad, sz)             (((ad + sz) % 32) >= sz ? (ad) : (ad + (sz - ((ad + sz) % 32))))

/* Exported constants --------------------------------------------------------*/
#define VADDR_RESET                 (uint16_t) EE_AREA(0, 2)
#define VADDR_ODOMETER              (uint16_t) EE_AREA(VADDR_RESET + 2, 4)
#define VADDR_UNITID                (uint16_t) EE_AREA(VADDR_ODOMETER + 4, 4)
#define VADDR_REPORT_SEQ_ID         (uint16_t) EE_AREA(VADDR_UNITID + 4, 2)
#define VADDR_RESPONSE_SEQ_ID       (uint16_t) EE_AREA(VADDR_REPORT_SEQ_ID + 2, 2)
#define VADDR_AES_KEY               (uint16_t) EE_AREA(VADDR_RESPONSE_SEQ_ID + 2, 16)
#define VADDR_DFU_FLAG              (uint16_t) EE_AREA(VADDR_AES_KEY + 16, 4)
#define VADDR_FOTA_VERSION          (uint16_t) EE_AREA(VADDR_DFU_FLAG + 4, 2)
#define VADDR_FOTA_TYPE             (uint16_t) EE_AREA(VADDR_FOTA_VERSION + 2, 4)

#define EE_NULL                      (uint8_t) 0
#define EE_DEV_TOTAL                 (uint8_t) 1

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    EE_CMD_R = 0,
    EE_CMD_W = 1
} EEPROM_COMMAND;

/* Exported variables ---------------------------------------------------------*/
extern uint16_t FOTA_VERSION;
extern IAP_TYPE FOTA_TYPE;
#if (BOOTLOADER)
extern uint32_t DFU_FLAG;
#endif


/* Public functions prototype ------------------------------------------------*/
uint8_t EEPROM_Init(I2C_HandleTypeDef *hi2c);
#if (!BOOTLOADER)
void EEPROM_ResetOrLoad(void);
uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint16_t value);
uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value);
uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value);
uint8_t EEPROM_SequentialID(EEPROM_COMMAND cmd, uint16_t value, PAYLOAD_TYPE type);
uint8_t EEPROM_AesKey(EEPROM_COMMAND cmd, uint32_t *value);
#else
uint8_t EEPROM_FlagDFU(EEPROM_COMMAND cmd, uint32_t value);
#endif
uint8_t EEPROM_FotaVersion(EEPROM_COMMAND cmd, uint16_t value);
uint8_t EEPROM_FotaType(EEPROM_COMMAND cmd, IAP_TYPE value);
#endif /* EEPROM_H_ */

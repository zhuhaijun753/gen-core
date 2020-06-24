/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_eeprom.h"
#include "Drivers/_ee24xx.h"
#if (!BOOTLOADER)
#include "Libs/_reporter.h"
#include "Libs/_keyless.h"
#include "Drivers/_aes.h"
#include "Nodes/VCU.h"

/* External variables -------------------------------------------------------*/
extern osMutexId_t EepromMutexHandle;
extern response_t RESPONSE;
extern report_t REPORT;
extern vcu_t VCU;
extern uint32_t AesKey[4];
#else
#endif

/* Exported variables ---------------------------------------------------------*/
uint32_t DFU_FLAG = 0;

/* Private functions prototype ------------------------------------------------*/
static uint8_t EE_Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(void) {
    const uint8_t MAX_RETRY = 5;
    const EEPROM24_DEVICE EEPROMS[2] = {
            EEPROM24_MAIN,
            EEPROM24_BACKUP
    };
    uint8_t retry, ret = 0;

    lock();
    LOG_StrLn("EEPROM:Init");
    // check each eeprom
    for (uint8_t i = 0; i < 1; i++) {
        if (!ret) {
            retry = MAX_RETRY;
            EEPROM24XX_SetDevice(EEPROMS[i]);
            do {
                if (EEPROM24XX_IsConnected()) {
                    LOG_Str("EEPROM:Device = ");
                    LOG_Int(i + 1);
                    LOG_Enter();

                    ret = 1;
                    break;
                }
                _DelayMS(50);
            } while (retry--);
        }
    }

    // all failed
    if (!ret) {
        LOG_StrLn("EEPROM:Error");
    }
    unlock();

#if (!BOOTLOADER)
    // Load or Reset
    EEPROM_ResetOrLoad();
#else
#endif
    /* Read DFU flag */
    EEPROM_FlagDFU(EE_CMD_R, EE_NULL);
    return ret;
}

#if (!BOOTLOADER)
void EEPROM_ResetOrLoad(void) {
    uint32_t AesKeyNew[4];

    if (!EEPROM_Reset(EE_CMD_R, EEPROM_RESET)) {
        // load from EEPROM
        EEPROM_UnitID(EE_CMD_R, EE_NULL);
        EEPROM_Odometer(EE_CMD_R, EE_NULL);
        for (uint8_t type = 0; type <= PAYLOAD_MAX; type++) {
            EEPROM_SequentialID(EE_CMD_R, EE_NULL, type);
        }
        // load aes key
        EEPROM_AesKey(EE_CMD_R, EE_NULL);

    } else {
        // save to EEPROM, first
        EEPROM_UnitID(EE_CMD_W, VCU_UNITID);
        EEPROM_Odometer(EE_CMD_W, 0);
        for (uint8_t type = 0; type <= PAYLOAD_MAX; type++) {
            EEPROM_SequentialID(EE_CMD_W, 0, type);
        }
        // generate aes key
        KLESS_GenerateAesKey(AesKeyNew);
        EEPROM_AesKey(EE_CMD_W, AesKeyNew);

        // re-write eeprom
        EEPROM_Reset(EE_CMD_W, EEPROM_RESET);
    }

}

uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint16_t value) {
    uint8_t ret;
    uint16_t tmp = value, temp;

    ret = EE_Command(VADDR_RESET, cmd, &value, &temp, sizeof(value));

    if (ret) {
        if (cmd == EE_CMD_R) {
            return tmp != temp;
        }
    }

    return ret;
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value) {
    // reset on overflow
    if (value > VCU_ODOMETER_MAX) {
        value = 0;
    }

    return EE_Command(VADDR_ODOMETER, cmd, &value, &(VCU.d.odometer), sizeof(value));
}

uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value) {
    uint8_t ret;

    ret = EE_Command(VADDR_UNITID, cmd, &value, &(VCU.d.unit_id), sizeof(value));

    // update the NRF Address
    if (cmd == EE_CMD_W) {
        KLESS_Init();
    }

    return ret;
}

uint8_t EEPROM_SequentialID(EEPROM_COMMAND cmd, uint16_t value, PAYLOAD_TYPE type) {
    uint16_t *pSeqId;
    uint32_t vaddr;

    // decide payload type
    if (type == PAYLOAD_REPORT) {
        pSeqId = &(VCU.d.seq_id.report);
        vaddr = VADDR_REPORT_SEQ_ID;
    } else {
        pSeqId = &(VCU.d.seq_id.response);
        vaddr = VADDR_RESPONSE_SEQ_ID;
    }

    return EE_Command(vaddr, cmd, &value, pSeqId, sizeof(value));
}

uint8_t EEPROM_AesKey(EEPROM_COMMAND cmd, uint32_t *value) {
    uint8_t ret;
    uint32_t *ptr, tmp[4];

    // eeprom
    ptr = (cmd == EE_CMD_W ? value : tmp);
    ret = EE_Command(VADDR_AES_KEY, cmd, ptr, AesKey, 16);

    // apply the AES key
    if (cmd == EE_CMD_W) {
        AES_Init();
    }

    return ret;
}
#else
#endif
uint8_t EEPROM_FlagDFU(EEPROM_COMMAND cmd, uint32_t value) {
    return EE_Command(VADDR_DFU_FLAG, cmd, &value, &DFU_FLAG, sizeof(value));
}

/* Private functions implementation --------------------------------------------*/
static uint8_t EE_Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size) {
    uint8_t ret = 0;

    lock();

    // check if new value is same with old value
    if (cmd == EE_CMD_W) {
        // apply the value
        memcpy(ptr, value, size);
        // save the value
        ret = EEPROM24XX_Save(vaddr, value, size);
    } else {
        // load the value
        ret = EEPROM24XX_Load(vaddr, value, size);
        // apply the value
        if (ret) {
            memcpy(ptr, value, size);
        }
    }

    unlock();
    return ret;
}

static void lock(void) {
#if (!BOOTLOADER)
    osMutexAcquire(EepromMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (!BOOTLOADER)
    osMutexRelease(EepromMutexHandle);
#endif
}

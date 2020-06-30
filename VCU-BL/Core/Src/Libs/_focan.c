/*
 * _focan.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_focan.h"
#include "Drivers/_canbus.h"

/* External variables ---------------------------------------------------------*/
extern canbus_t CB;

/* Private variables ----------------------------------------------------------*/
static uint16_t HMI_ADDR;

/* Private functions prototypes -----------------------------------------------*/
static void FOCAN_SetTarget(uint16_t address);
static uint8_t FOCAN_EnterModeIAP(void);
static uint8_t FOCAN_GetVersion(uint16_t *version);

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_Upgrade(void) {
    uint16_t version;
    uint8_t p;

    /* Set HMI target */
    FOCAN_SetTarget(CAND_HMI1_LEFT);

    /* Tell HMI to enter IAP mode */
    p = FOCAN_EnterModeIAP();

    /* Get HMI version via CAN */
    if (p > 0) {
        p = FOCAN_GetVersion(&version);
    }

    return p;
}

/* Private functions implementation ------------------------------------------*/
static void FOCAN_SetTarget(uint16_t address) {
    HMI_ADDR = address;
}

static uint8_t FOCAN_EnterModeIAP(void) {
    CAN_DATA *txd = &(CB.tx.data);
    CAN_DATA *rxd = &(CB.rx.data);
    uint8_t step = 0, reply = 1;
    uint32_t tick;
    uint8_t p;

    // set message
    txd->u16[0] = HMI_ADDR;

    // send message
    p = CANBUS_Write(CAND_ENTER_IAP, 2, 0);

    // wait response
    if (p) {
        tick = _GetTickMS();
        while ((step < reply) && (_GetTickMS() - tick < 1000)) {
            // read
            if (CANBUS_Read()) {
                if (CANBUS_ReadID() == CAND_ENTER_IAP) {
                    switch (step) {
                        case 0: // ack
                            step += (rxd->u8[0] == FOCAN_ACK );
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        p = (step == reply);
    }

    return p;
}

static uint8_t FOCAN_GetVersion(uint16_t *version) {
    //    CAN_DATA *txd = &(CB.tx.data);
    CAN_DATA *rxd = &(CB.rx.data);
    uint8_t step = 0, reply = 4;
    uint32_t tick;
    uint8_t p;

    // send message
    p = CANBUS_Write(CAND_GET_VERSION, 0, 0);

    // wait response
    if (p) {
        tick = _GetTickMS();
        while ((step < reply) && (_GetTickMS() - tick < 1000)) {
            // read
            if (CANBUS_Read()) {
                if (CANBUS_ReadID() == CAND_GET_VERSION) {
                    switch (step) {
                        case 0: // ack
                            step += (rxd->u8[0] == FOCAN_ACK );
                            break;
                        case 1: // version
                            *version = rxd->u16[0];
                            step++;
                            break;
                        case 2: // option message (2 bytes)
                            step += (rxd->u16[0] == 0x0000);
                            break;
                        case 3: // ack
                            step += (rxd->u8[0] == FOCAN_ACK );
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        /* return value */
        p = (step == reply);
    }

    return p;
}


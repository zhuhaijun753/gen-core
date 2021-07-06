/*
 * _finger_ada.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Pudja Mansyurin
 */
/***************************************************
 This is a library for our optical Fingerprint sensor
 Designed specifically to work with the Adafruit Fingerprint sensor
 ----> http://www.adafruit.com/products/751
 These displays use TTL Serial to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef INC_DRIVERS__R307_H_
#define INC_DRIVERS__R307_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
#define FP_RECEIVING_MS ((uint16_t)1000)

#define FP_OK 0x00
#define FP_PACKETRECIEVEERR 0x01
#define FP_NOFINGER 0x02
#define FP_IMAGEFAIL 0x03
#define FP_IMAGEMESS 0x06
#define FP_FEATUREFAIL 0x07
#define FP_NOMATCH 0x08
#define FP_NOTFOUND 0x09
#define FP_ENROLLMISMATCH 0x0A
#define FP_BADLOCATION 0x0B
#define FP_DBRANGEFAIL 0x0C
#define FP_UPLOADFEATUREFAIL 0x0D
#define FP_PACKETRESPONSEFAIL 0x0E
#define FP_UPLOADFAIL 0x0F
#define FP_DELETEFAIL 0x10
#define FP_DBCLEARFAIL 0x11
#define FP_PASSFAIL 0x13
#define FP_INVALIDIMAGE 0x15
#define FP_FLASHERR 0x18
#define FP_INVALIDREG 0x1A
#define FP_ADDRCODE 0x20
#define FP_PASSVERIFY 0x21

#define FP_STARTCODE 0xEF01

#define FP_COMMANDPACKET 0x1
#define FP_DATAPACKET 0x2
#define FP_ACKPACKET 0x7
#define FP_ENDDATAPACKET 0x8

#define FP_TIMEOUT 0xFF
#define FP_BADPACKET 0xFE

#define FP_GETIMAGE 0x01
#define FP_IMAGE2TZ 0x02
#define FP_REGMODEL 0x05
#define FP_STORE 0x06
#define FP_LOAD 0x07
#define FP_UPLOAD 0x08
#define FP_DELETE 0x0C
#define FP_EMPTY 0x0D
#define FP_SETPASSWORD 0x12
#define FP_VERIFYPASSWORD 0x13
#define FP_HISPEEDSEARCH 0x1B
#define FP_TEMPLATECOUNT 0x1D

#define FP_PASSWORD 0x0000000
#define FP_ADDRESS 0xFFFFFFFF

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint16_t start_code;  ///< "Wakeup" code for packet detection
  uint8_t address[4];   ///< 32-bit Fingerprint sensor address
  uint8_t type;         ///< Type of packet
  uint16_t length;      ///< Length of packet
  uint8_t data[64];     ///< The raw buffer for packet payload
} packet_t;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t R307_checkPassword(void);
uint8_t R307_getImage(void);
uint8_t R307_image2Tz(uint8_t slot);
uint8_t R307_createModel(void);
uint8_t R307_emptyDatabase(void);
uint8_t R307_storeModel(uint16_t id);
uint8_t R307_loadModel(uint16_t id);
uint8_t R307_getModel(void);
uint8_t R307_deleteModel(uint16_t id);
uint8_t R307_fingerFastSearch(uint16_t *id, uint16_t *confidence);
uint8_t R307_getTemplateCount(uint16_t *templateCount);
uint8_t R307_setPassword(uint32_t password);

#endif /* INC_DRIVERS__R307_H_ */

/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_reporter.h"

/* External variables ----------------------------------------------------------*/
extern db_t DB;
extern gps_t GPS;

/* Public variables -----------------------------------------------------------*/
report_t REPORT;
response_t RESPONSE;

/* Public functions implementation --------------------------------------------*/
void Reporter_Init(void) {
  // set default data
  LOG_StrLn("Reporter:Init");
  // =============== REPORT ==============
  // header report
  REPORT.header.prefix[0] = PREFIX_REPORT[1];
  REPORT.header.prefix[1] = PREFIX_REPORT[0];
  REPORT.header.crc = 0;
  REPORT.header.size = 0;
  REPORT.header.frame_id = FR_FULL;
  REPORT.header.seq_id = 0;
  REPORT.header.unit_id = 0;
  // (already set)
  // body required
  REPORT.data.req.vcu.rtc.send = 0;
  REPORT.data.req.vcu.rtc.log = 0;
  REPORT.data.req.vcu.driver_id = 1;
  REPORT.data.req.vcu.events_group = 0;
  // body optional
  REPORT.data.opt.vcu.gps.longitude = 0;
  REPORT.data.opt.vcu.gps.latitude = 0;
  REPORT.data.opt.vcu.gps.hdop = 0;
  REPORT.data.opt.vcu.gps.heading = 0;
  REPORT.data.opt.vcu.speed = 1;
  REPORT.data.opt.vcu.odometer = 0;
  REPORT.data.opt.vcu.bat_voltage = 0;
  REPORT.data.opt.vcu.report.range = 0;
  REPORT.data.opt.vcu.report.battery = 99;
  REPORT.data.opt.vcu.trip.a = 0x01234567;
  REPORT.data.opt.vcu.trip.b = 0x89A;

  // =============== RESPONSE ==============
  // header response (copy from report)
  RESPONSE.header = REPORT.header;
  RESPONSE.header.frame_id = FR_RESPONSE;
  // body response
  RESPONSE.data.code = 1;
  strcpy(RESPONSE.data.message, "");
}

void Reporter_SetUnitID(uint32_t unitId) {
  RESPONSE.header.unit_id = unitId;
  REPORT.header.unit_id = unitId;
  EEPROM_UnitID(EE_CMD_W, &unitId);
}

void Reporter_SetOdometer(uint32_t odom) {
  DB.vcu.odometer = odom;
  EEPROM_Odometer(EE_CMD_W, &odom);
}

void Reporter_SetEvents(uint64_t value) {
  REPORT.data.req.vcu.events_group = value;
}

void Reporter_WriteEvent(uint64_t event_id, uint8_t value) {
  if (value & 1) {
    BV(REPORT.data.req.vcu.events_group, _BitPosition(event_id));
  } else {
    BC(REPORT.data.req.vcu.events_group, _BitPosition(event_id));
  }
}

uint8_t Reporter_ReadEvent(uint64_t event_id) {
  return BSR((REPORT.data.req.vcu.events_group & event_id), _BitPosition(event_id));
}

void Reporter_Capture(FRAME_TYPE frame) {
  if (frame == FR_RESPONSE) {
    //Reconstruct the header
    RESPONSE.header.seq_id++;
    RESPONSE.header.frame_id = FR_RESPONSE;
    RESPONSE.header.size = sizeof(RESPONSE.header.frame_id) +
        sizeof(RESPONSE.header.unit_id) +
        sizeof(RESPONSE.header.seq_id) +
        sizeof(RESPONSE.data.code) +
        strlen(RESPONSE.data.message);
    RESPONSE.header.crc = CRC_Calculate8(
        (uint8_t*) &(RESPONSE.header.size),
        RESPONSE.header.size + sizeof(RESPONSE.header.size), 1);

  } else {
    // Reconstruct the header
    REPORT.header.seq_id++;
    REPORT.header.frame_id = frame;
    REPORT.header.size = sizeof(REPORT.header.frame_id) +
        sizeof(REPORT.header.unit_id) +
        sizeof(REPORT.header.seq_id) +
        sizeof(REPORT.data.req);
    // CRC will be recalculated when sending the payload
    // (because RTC_Send_Datetime will be changed later)
    REPORT.header.crc = 0;

    // Reconstruct the body
    // set parameter
    REPORT.data.req.vcu.rtc.log = RTC_Read();
    REPORT.data.req.vcu.rtc.send = 0;

    REPORT.data.req.bms.pack[0].id = DB.bms.pack[0].id;
    REPORT.data.req.bms.pack[0].voltage = DB.bms.pack[0].voltage * 1000;
    REPORT.data.req.bms.pack[0].current = (DB.bms.pack[0].current - 50) * 100;

    // Add more (if full frame)
    if (frame == FR_FULL) {
      REPORT.header.size += sizeof(REPORT.data.opt);
      // set parameter
      REPORT.data.opt.vcu.gps.latitude = (int32_t) (GPS.latitude * 10000000);
      REPORT.data.opt.vcu.gps.longitude = (int32_t) (GPS.longitude * 10000000);
      REPORT.data.opt.vcu.gps.hdop = (uint8_t) (GPS.dop_h * 10);
      REPORT.data.opt.vcu.gps.heading = (uint8_t) (GPS.heading / 2);

      REPORT.data.opt.vcu.speed = GPS.speed_mps;
      REPORT.data.opt.vcu.odometer = DB.vcu.odometer;
      REPORT.data.opt.vcu.bat_voltage = DB.vcu.bat_voltage / 18;

      REPORT.data.opt.bms.pack[0].soc = DB.bms.pack[0].soc;
      REPORT.data.opt.bms.pack[0].temperature = DB.bms.pack[0].id;
    }
  }
}


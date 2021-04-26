/*
 * BMS.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"

/* Public variables
 * -----------------------------------------------------------*/
bms_t BMS = {
		.d = {0},
		.r = {BMS_RX_Param1, BMS_RX_Param2},
		.t = {BMS_TX_Setting},
		.Init = BMS_Init,
		.PowerOverCan = BMS_PowerOverCan,
		.RefreshIndex = BMS_RefreshIndex,
};

/* Private functions prototypes
 * -----------------------------------------------*/
static void ResetIndex(uint8_t i);
static void ResetFaults(void);
static uint8_t GetIndex(uint32_t addr);
static uint16_t MergeFault(void);
static uint8_t MergeSOC(void);
static uint8_t AreActive(void);
static uint8_t AreOverheat(void);
static uint8_t AreRunning(uint8_t on);

/* Public functions implementation
 * --------------------------------------------*/
void BMS_Init(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		ResetIndex(i);
	ResetFaults();

	BMS.d.active = 0;
	BMS.d.run = 0;
	BMS.d.soc = 0;
	BMS.d.overheat = 0;
}

void BMS_PowerOverCan(uint8_t on) {
	static uint8_t lastState = 0;
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;
	uint8_t sc = on && (BMS.d.fault & BIT(BMSF_SHORT_CIRCUIT));

	if (lastState != on) {
		lastState = on;
		if (on) ResetFaults();
	}
	BMS.t.Setting(state, sc);
}

void BMS_RefreshIndex(void) {
	BMS.d.active = AreActive();
	BMS.d.run = BMS.d.active && AreRunning(1);

	BMS.d.soc = MergeSOC();
	BMS.d.fault = MergeFault();
	BMS.d.overheat = AreOverheat();

	VCU.SetEvent(EVG_BMS_ERROR, BMS.d.fault > 0);
}

/* ====================================== CAN RX
 * =================================== */
void BMS_RX_Param1(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);

	// read the content
	BMS.d.pack[i].voltage = Rx->data.u16[0] * 0.01;
	BMS.d.pack[i].current = Rx->data.u16[1] * 0.1;
	BMS.d.pack[i].soc = Rx->data.u16[2];
	BMS.d.pack[i].temperature = Rx->data.u16[3] - 40;

	// update index
	BMS.d.pack[i].id = BMS_ID(Rx->header.ExtId);
	BMS.d.pack[i].tick = _GetTickMS();
}

void BMS_RX_Param2(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);

	// read content
	BMS.d.pack[i].capacity = Rx->data.u16[0] * 0.1;
	BMS.d.pack[i].soh = Rx->data.u16[1];
	BMS.d.pack[i].cycle = Rx->data.u16[2];
	BMS.d.pack[i].fault = Rx->data.u16[3] & 0x0FFF;
	BMS.d.pack[i].state =
			(((Rx->data.u8[7] >> 4) & 0x01) << 1) | ((Rx->data.u8[7] >> 5) & 0x01);

	// update index
	BMS.d.pack[i].id = BMS_ID(Rx->header.ExtId);
	BMS.d.pack[i].tick = _GetTickMS();
}

/* ====================================== CAN TX
 * =================================== */
uint8_t BMS_TX_Setting(BMS_STATE state, uint8_t sc) {
	can_tx_t Tx = {0};

	// set message
	Tx.data.u8[0] = state << 1;
	Tx.data.u8[1] = sc;
	Tx.data.u8[2] = BMS_SCALE_15_85 & 0x03;

	// send message
	return CANBUS_Write(&Tx, CAND_BMS_SETTING, 8, 1);
}

/* Private functions implementation
 * --------------------------------------------*/
static void ResetIndex(uint8_t i) {
	BMS.d.pack[i].run = 0;
	BMS.d.pack[i].tick = 0;
	BMS.d.pack[i].state = BMS_STATE_OFF;
	BMS.d.pack[i].id = BMS_ID_NONE;
	BMS.d.pack[i].voltage = 0;
	BMS.d.pack[i].current = 0;
	BMS.d.pack[i].soc = 0;
	BMS.d.pack[i].temperature = 0;
	BMS.d.pack[i].capacity = 0;
	BMS.d.pack[i].soh = 0;
	BMS.d.pack[i].cycle = 0;
}

static void ResetFaults(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		BMS.d.pack[i].fault = 0;
	BMS.d.fault = 0;
}

static uint8_t GetIndex(uint32_t addr) {
	// find index (if already exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.pack[i].id == BMS_ID(addr))
			return i;

	// find index (if not exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.pack[i].id == BMS_ID_NONE)
			return i;

	// force replace first index (if already full)
	return 0;
}

static uint16_t MergeFault(void) {
	uint16_t fault = 0;

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		fault |= BMS.d.pack[i].fault;

	return fault;
}

static uint8_t MergeSOC(void) {
	uint8_t soc = 100;

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.pack[i].soc < soc)
			soc = BMS.d.pack[i].soc;

	return soc;
}

static uint8_t AreActive(void) {
	uint8_t active = 1;
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		BMS.d.pack[i].active = BMS.d.pack[i].tick && (_GetTickMS() - BMS.d.pack[i].tick) < BMS_TIMEOUT;
		if (!BMS.d.pack[i].active) {
			ResetIndex(i);
			active = 0;
		}
	}
	return active;
}

static uint8_t AreOverheat(void) {
	BMS_FAULT_BIT overheat[] = {
			BMSF_DISCHARGE_OVER_TEMPERATURE,
			BMSF_DISCHARGE_UNDER_TEMPERATURE,
			BMSF_CHARGE_OVER_TEMPERATURE,
			BMSF_CHARGE_UNDER_TEMPERATURE,
	};

	uint8_t temp = 0;
	for (uint8_t i = 0; i < sizeof(overheat); i++)
		temp |= BMS.d.fault & BIT(overheat[i]);

	return temp;
}

static uint8_t AreRunning(uint8_t on) {
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;

	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].state != state)
			return 0;
		if (on && BMS.d.pack[i].fault)
			return 0;
	}
	return 1;
}

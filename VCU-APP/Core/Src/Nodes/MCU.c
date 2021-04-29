/*
 * MCU.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"
#include "Nodes/VCU.h"

/* Public variables
 * -----------------------------------------------------------*/
mcu_t MCU = {
		.d = {0},
		.set = {0},
		.synced = {0},
		.r = {
				MCU_RX_CurrentDC,
				MCU_RX_VoltageDC,
				MCU_RX_TorqueSpeed,
				MCU_RX_FaultCode,
				MCU_RX_State,
				MCU_RX_Template
		},
		.t = {
				MCU_TX_Setting,
				MCU_TX_Template,
				MCU_RpmMax,
				MCU_Templates,
		},
		.Init = MCU_Init,
		.Power12v = MCU_Power12v,
		.PowerOverCan = MCU_PowerOverCan,
		.Refresh = MCU_Refresh,
		.SetSpeedMax = MCU_SetSpeedMax,
		.SetTemplates = MCU_SetTemplates,
		.RpmToSpeed = MCU_RpmToSpeed,
		.SpeedToRpm = MCU_SpeedToRpm,
		.SpeedToVolume = MCU_SpeedToVolume,
		.Reversed = MCU_Reversed,
		.Running = MCU_Running,
		.Sync = MCU_Sync
};

/* Private variables
 * -----------------------------------------------------------*/
static mcu_template_addr_t tplAddr[HBAR_M_DRIVE_MAX];

/* Private functions prototypes
 * -----------------------------------------------*/
static void Reset(void);
static void ResetFault(void);
static void SetTemplateAddr(void);
static void ResetTemplates(void);
static uint8_t SyncedSpeedMax(void);
static uint8_t SyncedTemplates(void);
static uint8_t IsOverheat(void);
//static uint8_t HandleReverse(uint8_t *on);

/* Public functions implementation
 * --------------------------------------------*/
void MCU_Init(void) {
	SetTemplateAddr();
	Reset();
	ResetFault();
}

void MCU_Power12v(uint8_t on) {
	if (MCU.d.run) return;

	if (on) {
		if (!MCU.d.active) {
			ResetFault();
			GATE_McuPower(0); _DelayMS(100);
			GATE_McuPower(1); _DelayMS(500);
		}
	} else {
		if (MCU.d.active) {
			GATE_McuPower(0);
		}
	}
}

void MCU_PowerOverCan(uint8_t on) {
//	static uint8_t lastOn = 0;
//
//	// make sure BMS is power on
//	if (lastOn != on) {
//		lastOn = on;
//		if (on) _DelayMS(500);
//	}

	if (on) {
		if (MCU.d.inv.lockout) {
			MCU.t.Setting(0); _DelayMS(5);
		} else {
			MCU.t.Setting(1);
		}
	} else {
		MCU.t.Setting(0);
	}
}

void MCU_Refresh(void) {
	MCU.d.active = MCU.d.tick && (_GetTickMS() - MCU.d.tick) < MCU_TIMEOUT_MS;
	if (MCU.d.active) {
		if (MCU.set.rpm_max && SyncedSpeedMax()) {
			MCU.set.rpm_max = 0;
			MCU.Power12v(0); _DelayMS(50);
		}
		if (MCU.set.template && SyncedTemplates()) {
			MCU.set.template = 0;
			MCU.Power12v(0); _DelayMS(50);
		}
	} else {
		Reset();
	}

	MCU.d.overheat = IsOverheat();
	MCU.d.run = MCU.d.active && MCU.d.inv.enabled;

	VCU.SetEvent(EVG_MCU_ERROR, (MCU.d.fault.post | MCU.d.fault.run) > 0);
}


void MCU_SetSpeedMax(uint8_t speed_max) {
	MCU.set.par.rpm_max = MCU.SpeedToRpm(speed_max);
	MCU.set.rpm_max = 1;
}

void MCU_SetTemplates(mcu_templates_t t) {
	memcpy(MCU.set.par.tpl, t.tpl, sizeof(t.tpl));
	MCU.set.template = 1;
}

void MCU_RpmMax(uint8_t write) {
	MCU.t.Template(MTP_RPM_MAX, write, write ? MCU.set.par.rpm_max : 0);
}

void MCU_Templates(uint8_t write) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		MCU.t.Template(tplAddr[m].discur_max, write, write ? MCU.set.par.tpl[m].discur_max : 0);
		MCU.t.Template(tplAddr[m].torque_max, write, write ? MCU.set.par.tpl[m].torque_max * 10 : 0);
		//		MCU.t.Template(tplAddr[m].rbs_switch, write, write ? MCU.set.par.tpl[m].rbs_switch : 0);
	}
}

uint8_t MCU_RpmToSpeed(int16_t rpm) {
	return (abs(rpm) * 60 * 1.58) / (8.26 * 1000);
}

int16_t MCU_SpeedToRpm(uint8_t speed) {
	return  (speed * 8.26 * 1000) / (60 * 1.58);
}

uint8_t MCU_SpeedToVolume(void) {
	//	uint8_t vol = MCU.RpmToSpeed(MCU.d.rpm) * 100 / MCU_SPEED_MAX_KPH;
	uint8_t vol = MCU.RpmToSpeed(MCU.d.rpm);

	return vol > 100 ? 100 : vol;
}

uint8_t MCU_Reversed(void) {
	return MCU.d.reverse && !MCU.d.inv.lockout;
}

uint8_t MCU_Running(void) {
	return MCU.RpmToSpeed(MCU.d.rpm) > 0;
}

void MCU_Sync(void) {
	MCU.t.Templates(MCU.set.template && !SyncedTemplates());
	MCU.t.RpmMax(MCU.set.rpm_max && !SyncedSpeedMax());
}

/* ====================================== CAN RX
 * =================================== */
void MCU_RX_CurrentDC(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	MCU.d.dcbus.current = d->s16[3] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_VoltageDC(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	MCU.d.dcbus.voltage = d->s16[1] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_TorqueSpeed(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	MCU.d.temperature = d->s16[0] * 0.1;
	MCU.d.rpm = d->s16[1];
	MCU.d.torque.commanded = d->s16[2] * 0.1;
	MCU.d.torque.feedback = d->s16[3] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_FaultCode(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	MCU.d.fault.post = d->u32[0];
	MCU.d.fault.run = d->u32[1];

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_State(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	MCU.d.drive_mode = d->u8[4] & 0x03;
	MCU.d.inv.discharge = (d->u8[4] >> 5) & 0x07;
	//	MCU.d.inv.can_mode = d->u8[5] & 0x01;
	MCU.d.inv.enabled = d->u8[6] & 0x01;
	MCU.d.inv.lockout = (d->u8[6] >> 7) & 0x01;
	MCU.d.reverse = d->u8[7] & 0x01;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_Template(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	uint16_t param = d->u16[0];
	//	uint8_t write = d->u8[2];
	int16_t data = d->s16[2];

	if (param == MTP_RPM_MAX) {
		MCU.d.par.rpm_max = data;
		return;
	}

	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		if (param == tplAddr[m].discur_max) {
			MCU.d.par.tpl[m].discur_max = data;
			return;
		}
		else if (param == tplAddr[m].torque_max) {
			MCU.d.par.tpl[m].torque_max = data * 0.1;
			return;
		}
		//		else if (param == tplAddr[m].rbs_switch) {
		//			MCU.d.par.tpl[m].rbs_switch = data;
		//			return;
		//		}
	}
}

/* ====================================== CAN TX
 * =================================== */
uint8_t MCU_TX_Setting(uint8_t on) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	// set message
//	d->u8[4] = HandleReverse();
	d->u8[4] = HBAR.state[HBAR_K_REVERSE];
	d->u8[5] = on & 0x01;
	d->u8[5] |= (0 & 0x01) << 1;
	d->u8[5] |= (HBAR.d.mode[HBAR_M_DRIVE] & 0x03) << 2;

	// send message
	return CANBUS_Write(&Tx, CAND_MCU_SETTING, 6, 0);
}

uint8_t MCU_TX_Template(uint16_t param, uint8_t write, int16_t data) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);
	uint8_t ok;

	// set message
	d->u16[0] = param;
	d->u8[2] = write;
	d->s16[2] = data;

	// send message
	ok = CANBUS_Write(&Tx, CAND_MCU_TEMPLATE_W, 6, 0);
	_DelayMS(100);

	return ok;
}

/* Private functions implementation
 * --------------------------------------------*/
static void Reset(void) {
	MCU.d.run = 0;
	MCU.d.tick = 0;
	MCU.d.overheat = 0;

	MCU.d.rpm = 0;
	MCU.d.reverse = 0;
	MCU.d.temperature = 0;
	MCU.d.drive_mode = HBAR_M_DRIVE_STANDARD;
	MCU.d.torque.commanded = 0;
	MCU.d.torque.feedback = 0;
	MCU.d.dcbus.current = 0;
	MCU.d.dcbus.voltage = 0;

	MCU.d.inv.enabled = 0;
	MCU.d.inv.lockout = 0;
	MCU.d.inv.discharge = INV_DISCHARGE_DISABLED;

	MCU.d.par.rpm_max = 0;
	ResetTemplates();
}

static void ResetFault(void) {
	MCU.d.fault.post = 0;
	MCU.d.fault.run = 0;
}

static void SetTemplateAddr(void) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		tplAddr[m].discur_max = (m*3) + MTP_1_DISCUR_MAX;
		tplAddr[m].torque_max = (m*3) + MTP_1_TORQUE_MAX;
		//		tplAddr[m].rbs_switch = (m*3) + MTP_1_RBS_SWITCH;
	}
}

static void ResetTemplates(void) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		MCU.d.par.tpl[m].discur_max = 0;
		MCU.d.par.tpl[m].torque_max = 0;
		//		MCU.d.par.tpl[m].rbs_switch = 0;
	}
}

static uint8_t SyncedSpeedMax(void) {
	MCU.synced.rpm_max = !MCU.set.rpm_max || (MCU.d.par.rpm_max == MCU.set.par.rpm_max);
	return MCU.synced.rpm_max;
}

static uint8_t SyncedTemplates(void) {
	MCU.synced.template = !MCU.set.template || (memcmp(MCU.d.par.tpl, MCU.set.par.tpl, sizeof(MCU.d.par.tpl)) == 0);
	return MCU.synced.template;
}

static uint8_t IsOverheat(void) {
	MCU_POST_FAULT_BIT overheat_post[] = {
			MPF_MOD_TEMP_LOW, MPF_MOD_TEMP_HIGH,  MPF_PCB_TEMP_LOW,
			MPF_PCB_TEMP_HIGH, MPF_GATE_TEMP_LOW, MPF_GATE_TEMP_HIGH,
	};
	MCU_RUN_FAULT_BIT overheat_run[] = {
			MRF_INV_OVER_TEMP,   MRF_MOTOR_OVER_TEMP, MRF_MODA_OVER_TEMP,
			MRF_MODB_OVER_TEMP,  MRF_MODC_OVER_TEMP,  MRF_PCB_OVER_TEMP,
			MRF_GATE1_OVER_TEMP, MRF_GATE2_OVER_TEMP, MRF_GATE3_OVER_TEMP,
	};

	uint8_t temp = 0;
	for (uint8_t i = 0; i < sizeof(overheat_post); i++)
		temp |= (MCU.d.fault.post & BIT(overheat_post[i]));
	for (uint8_t i = 0; i < sizeof(overheat_run); i++)
		temp |= (MCU.d.fault.run & BIT(overheat_run[i]));

	return temp;
}

//static uint8_t HandleReverse(uint8_t *on) {
//	static uint8_t reverse = 0;
//
//	if (HBAR.state[HBAR_K_REVERSE] && !MCU.d.reverse) {
//		on = 0;
//		if (!MCU.d.inv.enabled)
//			reverse = 1;
//	} else if (!HBAR.state[HBAR_K_REVERSE] && MCU.d.reverse) {
//		on = 0;
//		if (!MCU.d.inv.enabled)
//			reverse = 0;
//	} else {
//		reverse = HBAR.state[HBAR_K_REVERSE];
//	}
//
//	return reverse;
//}

/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/VCU.h"
#include "Drivers/_bat.h"
#include "Drivers/_simcom.h"
#include "Libs/_hbar.h"
#include "Libs/_remote.h"
#include "Libs/_finger.h"
#include "Nodes/NODE.h"
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI2.h"

/* Public variables
 * -----------------------------------------------------------*/
vcu_t VCU = {
		.d = {0},
};

/* External variables -----------------------------------------*/
extern osMessageQueueId_t ReportQueueHandle;

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void) {
	memset(&(VCU.d), 0, sizeof(vcu_data_t));
	VCU.d.state = VEHICLE_BACKUP;
}

void VCU_Refresh(void) {
	BAT_ScanValue();

	VCU.d.uptime++;
	VCU.d.buffered = osMessageQueueGetCount(ReportQueueHandle);
	VCU.d.error = VCU_GetEvent(EVG_BIKE_FALLEN);
}

void VCU_CheckState(void) {
	static vehicle_state_t lastState = VEHICLE_UNKNOWN;
	static uint32_t lastRemoteReset = 0;
	vehicle_state_t initialState;
	uint8_t normalize = 0, start = 0;

	HBAR_STARTER starter = HBAR.ctl.starter;
	if (starter != HBAR_STARTER_UNKNOWN) {
		HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
		normalize = starter == HBAR_STARTER_OFF;
		start = starter == HBAR_STARTER_ON;
	}

	do {
		initialState = VCU.d.state;

		switch (VCU.d.state) {
		case VEHICLE_LOST:
			if (lastState != VEHICLE_LOST) {
				lastState = VEHICLE_LOST;
				osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
			}

			if (GATE_ReadPower5v())
				VCU.d.state += 2;
			break;

		case VEHICLE_BACKUP:
			if (lastState != VEHICLE_BACKUP) {
				lastState = VEHICLE_BACKUP;
				osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
				osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_TASK_STOP);
				osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_TASK_STOP);
				osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_TASK_STOP);
				osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_STOP);
				osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_STOP);
				osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_STOP);

				VCU.d.tick.independent = _GetTickMS();
				VCU_SetEvent(EVG_REMOTE_MISSING, 1);
			}

			if ((_GetTickMS() - VCU.d.tick.independent) > (VCU_LOST_MODE_S*1000))
				VCU.d.state--;
			else if (GATE_ReadPower5v())
				VCU.d.state++;
			break;

		case VEHICLE_NORMAL:
			if (lastState != VEHICLE_NORMAL) {
				lastState = VEHICLE_NORMAL;
				osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
				osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_TASK_START);
				osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_TASK_START);
				osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_TASK_START);
				osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_START);
				osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_START);
				osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_START);

				HBAR_Init();
				BMS_Init();
				MCU_Init();
				_DelayMS(500);
				normalize = 0;
				start = 0;
			}

			if (!GATE_ReadPower5v())
				VCU.d.state--;
			else if (start) {
				if (RMT.d.nearby) VCU.d.state++;
				else {
					if (_GetTickMS() - lastRemoteReset > REMOTE_RESET_GUARD_MS) {
						osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RESET);
						lastRemoteReset = _GetTickMS();
					}
				}
			}
			break;

		case VEHICLE_STANDBY:
			if (lastState != VEHICLE_STANDBY) {
				lastState = VEHICLE_STANDBY;
				start = 0;
				FGR.d.id = 0;
			}

			if (!GATE_ReadPower5v() || normalize)
				VCU.d.state--;
			else if (FGR.d.id)
				VCU.d.state++;
			break;

		case VEHICLE_READY:
			if (lastState != VEHICLE_READY) {
				lastState = VEHICLE_READY;
				start = 0;
				VCU.d.tick.ready = _GetTickMS();
			}

			if (!GATE_ReadPower5v() || normalize)
				VCU.d.state--;
			else if (!NODE.d.error && (start))
				VCU.d.state++;
			break;

		case VEHICLE_RUN:
			if (lastState != VEHICLE_RUN) {
				lastState = VEHICLE_RUN;
				start = 0;
			}

			if (!GATE_ReadPower5v() || (start && !MCU_Running()) || normalize || NODE.d.error)
				VCU.d.state--;
			break;

		default:
			break;
		}
		_DelayMS(100);
	} while (initialState != VCU.d.state);
}

void VCU_SetEvent(uint8_t bit, uint8_t value) {
	if (value & 1) BV(VCU.d.events, bit);
	else BC(VCU.d.events, bit);
}

uint8_t VCU_GetEvent(uint8_t bit) {
	return (VCU.d.events & BIT(bit)) == BIT(bit);
}

uint8_t VCU_CalcDistance(void) {
	static uint32_t tick = 0;
	uint8_t meter = 0;
	float mps;

	if (tick > 0) {
		mps = (float) MCU_RpmToSpeed(MCU.d.rpm) / 3.6;
		meter = ((_GetTickMS() - tick) * mps) / 1000;
	}
	tick = _GetTickMS();

	return meter;
}

/* ====================================== CAN TX
 * =================================== */
uint8_t VCU_TX_Heartbeat(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	d->u16[0] = VCU_VERSION;

	return CANBUS_Write(&Tx, CAND_VCU, 2, 0);
}

uint8_t VCU_TX_SwitchControl(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	d->u8[0] = HBAR.d.pin[HBAR_K_ABS];
	d->u8[0] |= HMI2.d.mirroring << 1;
	d->u8[0] |= HBAR.d.pin[HBAR_K_LAMP] << 2;
	d->u8[0] |= NODE.d.error << 3;
	d->u8[0] |= NODE.d.overheat << 4;
	d->u8[0] |= !FGR.d.id << 5;
	d->u8[0] |= !RMT.d.nearby << 6;
	d->u8[0] |= RTC_Daylight() << 7;

	// sein value
	HBAR_RefreshSein();
	d->u8[1] = HBAR.ctl.sein.left;
	d->u8[1] |= HBAR.ctl.sein.right << 1;
	//	d->u8[1] |= HBAR.d.pin[HBAR_K_REVERSE] << 2;
	d->u8[1] |= MCU_Reversed() << 2;
	d->u8[1] |= BMS.d.run << 3;
	d->u8[1] |= MCU.d.run << 4;
	d->u8[1] |= (FGR.d.registering & 0x03) << 5;

	// mode
	d->u8[2] = HBAR.d.mode[HBAR_M_DRIVE];
	//	d->u8[2] = MCU.d.drive_mode;
	d->u8[2] |= HBAR.d.mode[HBAR_M_TRIP] << 2;
	d->u8[2] |= HBAR.d.mode[HBAR_M_REPORT] << 4;
	d->u8[2] |= HBAR.d.m << 5;
	d->u8[2] |= (HBAR.ctl.session > 0) << 7;

	// others
	d->u8[3] = MCU_RpmToSpeed(MCU.d.rpm);
	d->u8[4] = (uint8_t) MCU.d.dcbus.current;
	d->u8[5] = BMS.d.soc;
	d->u8[6] = SIM.d.signal;
	d->u8[7] = (int8_t) VCU.d.state;

	// send message
	return CANBUS_Write(&Tx, CAND_VCU_SWITCH_CTL, 8, 0);
}

uint8_t VCU_TX_Datetime(datetime_t dt) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	uint8_t hmi2shutdown = VCU.d.state < VEHICLE_STANDBY;

	d->u8[0] = dt.Seconds;
	d->u8[1] = dt.Minutes;
	d->u8[2] = dt.Hours;
	d->u8[3] = dt.Date;
	d->u8[4] = dt.Month;
	d->u8[5] = dt.Year;
	d->u8[6] = dt.WeekDay;
	d->u8[7] = hmi2shutdown;

	return CANBUS_Write(&Tx, CAND_VCU_DATETIME, 8, 0);
}

uint8_t VCU_TX_ModeData(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	d->u16[0] = HBAR.d.trip[HBAR_M_TRIP_A];
	d->u16[1] = HBAR.d.trip[HBAR_M_TRIP_B];
	d->u16[2] = HBAR.d.trip[HBAR_M_TRIP_ODO];
	d->u8[6] = HBAR.d.report[HBAR_M_REPORT_RANGE];
	d->u8[7] = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

	return CANBUS_Write(&Tx, CAND_VCU_MODE_DATA, 8, 0);
}
/* Private functions implementation -------------------------------------------*/

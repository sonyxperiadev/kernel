/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/pm_prm_sysparm.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

////////////////////////////////////////////////////////
//      CCU System Parameters starts from here         //
////////////////////////////////////////////////////////

// CCU auto-gating enable/disable
#define KHUB_AUTO_GATING							FALSE
#define ROOT_AUTO_GATING							FALSE
#define KHUBAON_AUTO_GATING							FALSE
#define MM_AUTO_GATING								FALSE
#define KPM_AUTO_GATING								FALSE
#define KPS_AUTO_GATING								FALSE
#define KPROC_AUTO_GATING							FALSE
#define BMDM_AUTO_GATING							FALSE
#define DSP_AUTO_GATING								FALSE

// DIV settings

////////////////////////////////////////////////////////
//  Power Manager System Parameters starts from here   //
////////////////////////////////////////////////////////

// VO0_SPECIFIC_I2C_COMMAND_POINTER: VO0 Specific I2C Command Pointer Register
#define	VO0_SPECIFIC_I2C_COMMAND_SET2VALUE			0x0
#define	VO0_SPECIFIC_I2C_COMMAND_SET2PTR			0x0
#define	VO0_SPECIFIC_I2C_COMMAND_SET1VALUE			0x0
#define	VO0_SPECIFIC_I2C_COMMAND_SET1PTR			0x0
#define	VO0_SPECIFIC_I2C_COMMAND_ZEROPTR			0x0
#define	VO0_SPECIFIC_I2C_COMMAND_CMDPTR				0x0

// FIXED_VOLTAGE_MAP: Fixed Voltage Register
#define	FIXED_VOLTAGE_MAP_MODEM						0x0
#define	FIXED_VOLTAGE_MAP_ARMSUB					0x0
#define	FIXED_VOLTAGE_MAP_HUB						0x0
#define	FIXED_VOLTAGE_MAP_MM						0x0
#define	FIXED_VOLTAGE_MAP_ARMCORE					0x0

// PI_DEFAULT_POWER_STATE: This register sets the defualt states of the power islands (power on/off)
#define	PI_DEFAULT_POWER_STATE						0x00000006	// to be or'ed with content in PI_DEFAULT_POWER_STATE register

// PC_PIN_OVERRIDE_CONTROL: PC pin override controls
#define	PC0_SW_OVERRIDE_VALUE						0x0
#define	PC1_SW_OVERRIDE_VALUE						0x0
#define	PC2_SW_OVERRIDE_VALUE						0x0
#define	PC3_SW_OVERRIDE_VALUE						0x0
#define	PC0_SW_OVERRIDE_ENABLE						0x0
#define	PC1_SW_OVERRIDE_ENABLE						0x0
#define	PC2_SW_OVERRIDE_ENABLE						0x0
#define	PC3_SW_OVERRIDE_ENABLE						0x0
#define	PC0_CLKREQ_OVERRIDE_VALUE					0x0
#define	PC1_CLKREQ_OVERRIDE_VALUE					0x0
#define	PC2_CLKREQ_OVERRIDE_VALUE					0x0
#define	PC3_CLKREQ_OVERRIDE_VALUE					0x0
#define	PC0_CLKREQ_OVERRIDE_ENABLE					0x0
#define	PC1_CLKREQ_OVERRIDE_ENABLE					0x0
#define	PC2_CLKREQ_OVERRIDE_ENABLE					0x0
#define	PC3_CLKREQ_OVERRIDE_ENABLE					0x0

// POWER_MANAGER_I2C_ENABLE
#define	POWER_MANAGER_I2C_ENABLE					1

// POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_xx: These registers make up the i2c command/data stream for the vo2i2c block
CHAL_PM_I2C_CMD_t i2c_command_data = {
	// command[64]
	0x0, 0x0, 0x6, 0x7, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

	// command_data[64]
	0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

	// length
	64
};

// POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01: These registers make up the i2c variable data stream for the vo2i2c block
unsigned char i2c_variable_data[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef struct {
	CHAL_PM_EVENT_t event_id;
	int postiveEdge;
	int negtiveEdge;
	int modem;
	int modemSub;
	int ARMCore;
	int ARMSub;
	int hubAON;
	int hubSwitchable;
	int MM;
	int MMSub;
} SYSPARM_PM_EVENT_t;

// Event Configuration
SYSPARM_PM_EVENT_t event_table[] = {
// to enable an event, please choose event ID from event list below and enter pos/neg trigger and domain policies, AC/ATL will be hardcoded to 1/0

//      event_id                        postiveEdge     negtiveEdge     modem   modemSub        core    coreSub         hubAON  hubSwitch       MM              MMsub
	SOFTWARE_0_EVENT, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
	SOFTWARE_1_EVENT, 1, 1, 0, 0, 5, 5, 5, 5, 5, 5,
	SOFTWARE_2_EVENT, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
	(CHAL_PM_EVENT_t) - 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

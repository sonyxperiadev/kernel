/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/pmu_chip.h
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

/*
*
*****************************************************************************
*
*  pmu_chip.h
*
*  PURPOSE:
*
*  This file defines the common AT ccomamnd interface to PMU chip
*
*  NOTES:
*
*****************************************************************************/

#if !defined( BCM_PMU_CHIP_H )
#define BCM_PMU_CHIP_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>
#include <linux/fs.h>

/* ---- Constants and Types ---------------------------------------------- */

#define BCM_PMU_MAGIC   'P'

#define BCM_PMU_CMD_FIRST               0x80

#define BCM_PMU_CMD_ENABLE_INTS         0x80
#define BCM_PMU_CMD_DISABLE_INTS        0x81
#define BCM_PMU_CMD_READ_REG            0x83
#define BCM_PMU_CMD_WRITE_REG           0x84
#define BCM_PMU_CMD_ACTIVATESIM         0x85
#define BCM_PMU_CMD_DEACTIVATESIM       0x86
#define BCM_PMU_CMD_GET_REGULATOR_STATE 0x87
#define BCM_PMU_CMD_SET_REGULATOR_STATE 0x88
#define BCM_PMU_CMD_SET_PWM_LED_CTRL    0x89
#define BCM_PMU_CMD_POWERONOFF		0x00
#define BCM_PMU_CMD_SET_PWM_HI_PER      0x8a
#define BCM_PMU_CMD_SET_PWM_LO_PER      0x8b
#define BCM_PMU_CMD_SET_PWM_PWR_CTRL    0x8c
#define BCM_PMU_CMD_GET_VOLTAGE		0x8d
#define BCM_PMU_CMD_SET_VOLTAGE		0x8e

#define BCM_PMU_CMD_LAST                0x8e

#define BCM_PMU_IOCTL_ENABLE_INTS       _IO(  BCM_PMU_MAGIC, BCM_PMU_CMD_ENABLE_INTS )	// arg is unused
#define BCM_PMU_IOCTL_DISABLE_INTS      _IO(  BCM_PMU_MAGIC, BCM_PMU_CMD_DISABLE_INTS )	// arg is unused
#define BCM_PMU_IOCTL_READ_REG          _IOWR(BCM_PMU_MAGIC, BCM_PMU_CMD_READ_REG, BCM_PMU_Reg_t )
#define BCM_PMU_IOCTL_WRITE_REG         _IOW( BCM_PMU_MAGIC, BCM_PMU_CMD_WRITE_REG, BCM_PMU_Reg_t )
#define BCM_PMU_IOCTL_ACTIVATESIM       _IOW( BCM_PMU_MAGIC, BCM_PMU_CMD_ACTIVATESIM, PM_SimVoltage_t )
#define BCM_PMU_IOCTL_DEACTIVATESIM 	_IO( BCM_PMU_MAGIC, BCM_PMU_CMD_DEACTIVATESIM )	// arg is unused
#define BCM_PMU_IOCTL_GET_REGULATOR_STATE _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_REGULATOR_STATE, BCM_PMU_Regulator_t)
#define BCM_PMU_IOCTL_SET_REGULATOR_STATE _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_REGULATOR_STATE, BCM_PMU_Regulator_t)
#define BCM_PMU_IOCTL_GET_VOLTAGE	 _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_GET_VOLTAGE, BCM_PMU_Regulator_Volt_t)
#define BCM_PMU_IOCTL_SET_VOLTAGE	 _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_VOLTAGE, BCM_PMU_Regulator_Volt_t)
#define BCM_PMU_IOCTL_SET_PWM_LED_CTRL    _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LED_CTRL, BCM_PMU_PWM_ctrl_t)
#define BCM_PMU_IOCTL_POWERONOFF    	  _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_POWERONOFF, BCM_PMU_Reg_t)
#define BCM_PMU_IOCTL_SET_PWM_HI_PER      _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_HI_PER, BCM_PMU_PWM_hi_per_t)
#define BCM_PMU_IOCTL_SET_PWM_LO_PER      _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_LO_PER, BCM_PMU_PWM_lo_per_t)
#define BCM_PMU_IOCTL_SET_PWM_PWR_CTRL    _IOW(BCM_PMU_MAGIC, BCM_PMU_CMD_SET_PWM_PWR_CTRL, BCM_PMU_PWM_pwr_ctrl_t)

//per lori: at_pmu.c requires an updated kernel/common/include/linux/broadcom/pmu_chip.h version.  For now, I just added the required defines to pmu_chip.h, and not the corresponding implementation. So, AT*MPMUTST commands won.t work properly.
typedef struct {
	unsigned long reg;
	unsigned short val;
} pmu_reg;

typedef enum {
	SIM_3POINT0VOLT = 0,
	SIM_2POINT5VOLT,
	SIM_3POINT1VOLT,
	SIM_1POINT8VOLT,
	SIM_MAX_VOLTAGE
} PM_SimVoltage_t;

typedef struct {
	unsigned char reg;
	unsigned char val;
} BCM_PMU_Reg_t;

typedef struct {
	int regulatorID;
	int voltage;
	int min;
	int max;
	int step;
} BCM_PMU_Regulator_Volt_t;

typedef enum {
	PMU_PCF50603 = 0,
	PMU_PCF50611,
	PMU_BCM59001,
	PMU_BCM59035,
	PMU_NONE,
	PMU_NUM_CHIPS,
} BCM_PMU_Chip_t;

typedef enum {
	PMU_Regulator_Off,
	PMU_Regulator_On,
	PMU_Regulator_Eco
} BCM_PMU_Regulator_State_t;

typedef struct {
	int regulatorID;
	BCM_PMU_Regulator_State_t state;
} BCM_PMU_Regulator_t;

typedef struct {
	unsigned int regoffset;
	unsigned int pwmled_ctrl;
	unsigned int pwmdiv;	// divider value. fsys/x value.    
} BCM_PMU_PWM_ctrl_t;

typedef struct {
	unsigned int hi_per;
} BCM_PMU_PWM_hi_per_t;

typedef struct {
	unsigned int lo_per;
} BCM_PMU_PWM_lo_per_t;

typedef struct {
	unsigned int pwr_ctrl;
} BCM_PMU_PWM_pwr_ctrl_t;

typedef enum {
	PMU_Power_On_By_On_Button,	// on button
	PMU_Power_On_By_Charger,	// charger insertion and no on button
	PMU_Power_On_By_Restart	// re-started while power on
} BCM_PMU_Power_On_State_t;

typedef enum			// If you change this, please update PM_EventTable as well
{
	PMU_EVENT_ATTACHED = 0,

	PMU_EVENT_BATTERY_LOW,
	PMU_EVENT_BATTERY_FULL,

	PMU_EVENT_BATTERY_TEMPERATURE_FAULT,
	PMU_EVENT_BATTERY_TEMPERATURE_OK,

	PMU_EVENT_ONKEY_RISE,
	PMU_EVENT_ONKEY_FALL,
	PMU_EVENT_ONKEY_1S_HOLD,

	PMU_EVENT_HIGH_TEMPERATURE,

	PMU_EVENT_CHARGER_INSERT,
	PMU_EVENT_CHARGER_REMOVE,
	PMU_EVENT_CHARGER_ERROR,

	PMU_NUM_EVENTS,
} BCM_PMU_Event_t;

#endif /* BCM_PMU_CHIP_H */

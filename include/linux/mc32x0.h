/*******************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/mc32x0.h
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

#ifndef LINUX_MC32X0_MODULE_H
#define LINUX_MC32X0_MODULE_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

#define G_0		ABS_Y
#define G_1		ABS_X
#define G_2		ABS_Z
#define G_0_REVERSE	1
#define G_1_REVERSE	1
#define G_2_REVERSE	1

#define SENSOR_DMARD_IOCTL_BASE 234

#define IOCTL_SENSOR_SET_DELAY_ACCEL _IO(SENSOR_DMARD_IOCTL_BASE, 100)
#define IOCTL_SENSOR_GET_DELAY_ACCEL _IO(SENSOR_DMARD_IOCTL_BASE, 101)
#define IOCTL_SENSOR_GET_STATE_ACCEL _IO(SENSOR_DMARD_IOCTL_BASE, 102)
#define IOCTL_SENSOR_SET_STATE_ACCEL _IO(SENSOR_DMARD_IOCTL_BASE, 103)
#define IOCTL_SENSOR_GET_DATA_ACCEL _IO(SENSOR_DMARD_IOCTL_BASE, 104)

#define IOCTL_MSENSOR_SET_DELAY_MAGNE   _IO(SENSOR_DMARD_IOCTL_BASE, 200)
#define IOCTL_MSENSOR_GET_DATA_MAGNE	_IO(SENSOR_DMARD_IOCTL_BASE, 201)
#define IOCTL_MSENSOR_GET_STATE_MAGNE   _IO(SENSOR_DMARD_IOCTL_BASE, 202)
#define IOCTL_MSENSOR_SET_STATE_MAGNE	_IO(SENSOR_DMARD_IOCTL_BASE, 203)

#define IOCTL_SENSOR_GET_NAME   _IO(SENSOR_DMARD_IOCTL_BASE, 301)
#define IOCTL_SENSOR_GET_VENDOR   _IO(SENSOR_DMARD_IOCTL_BASE, 302)

#define IOCTL_SENSOR_GET_CONVERT_PARA   _IO(SENSOR_DMARD_IOCTL_BASE, 401)

#define SENSOR_CALIBRATION   _IOWR(SENSOR_DMARD_IOCTL_BASE,  402, \
				int[SENSOR_DATA_SIZE])

#define mc32x0_CONVERT_PARAMETER       (1.5f * (9.80665f) / 256.0f)
#define mc32x0_DISPLAY_NAME         "mc32x0"
#define mc32x0_DIPLAY_VENDOR        "domintech"

#define X_OUT 0x41
#define CONTROL_REGISTER 0x44
#define SW_RESET 0x53
#define WHO_AM_I 0x0f
#define WHO_AM_I_VALUE 0x06

#define MC32X0_AXIS_X 0
#define MC32X0_AXIS_Y 1
#define MC32X0_AXIS_Z 2
#define MC32X0_AXES_NUM 3
#define MC32X0_DATA_LEN 6

#define MC32X0_XOUT_REG		0x00
#define MC32X0_YOUT_REG		0x01
#define MC32X0_ZOUT_REG		0x02
#define MC32X0_Tilt_Status_REG	0x03
#define MC32X0_Sampling_Rate_Status_REG 0x04
#define MC32X0_Sleep_Count_REG 0x05
#define MC32X0_Interrupt_Enable_REG 0x06
#define MC32X0_Mode_Feature_REG 0x07
#define MC32X0_Sample_Rate_REG 0x08
#define MC32X0_Tap_Detection_Enable_REG 0x09
#define MC32X0_TAP_Dwell_Reject_REG 0x0a
#define MC32X0_DROP_Control_Register_REG 0x0b
#define MC32X0_SHAKE_Debounce_REG 0x0c
#define MC32X0_XOUT_EX_L_REG 0x0d
#define MC32X0_XOUT_EX_H_REG 0x0e
#define MC32X0_YOUT_EX_L_REG 0x0f
#define MC32X0_YOUT_EX_H_REG 0x10
#define MC32X0_ZOUT_EX_L_REG 0x11
#define MC32X0_ZOUT_EX_H_REG 0x12
#define MC32X0_CHIP_ID_REG 0x18
#define MC32X0_RANGE_Control_REG 0x20
#define MC32X0_SHAKE_Threshold_REG 0x2B
#define MC32X0_UD_Z_TH_REG 0x2C
#define MC32X0_UD_X_TH_REG 0x2D
#define MC32X0_RL_Z_TH_REG 0x2E
#define MC32X0_RL_Y_TH_REG 0x2F
#define MC32X0_FB_Z_TH_REG 0x30
#define MC32X0_DROP_Threshold_REG 0x31
#define MC32X0_TAP_Threshold_REG 0x32
#define MC32X0_HIGH_END 0x01
/*******MC3210/20 define this**********/

#define MCUBE_8G_14BIT 0x10

#define DOT_CALI

#define MC32X0_LOW_END 0x02
/*******mc32x0 define this**********/

#define MCUBE_1_5G_8BIT 0x20
#define MC32X0_MODE_DEF 0x43

#define mc32x0_I2C_NAME "mc32x0"
#define A10ASENSOR_DEV_COUNT 1
#define A10ASENSOR_DURATION_MAX 200
#define A10ASENSOR_DURATION_MIN 10
#define A10ASENSOR_DURATION_DEFAULT 100

#define MAX_RETRY 20
#define INPUT_FUZZ 0
#define INPUT_FLAT 0


#endif /*LINUX_MC32X0_MODULE_H */

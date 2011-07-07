/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include <linux/mfd/bcmpmu.h>

#define PMU_IRG_REG_MAX		14
#define PMU_ENV_REG_MAX		7

static const struct bcmpmu_reg_map bcm59055_reg_map[PMU_REG_MAX] = {//revisit
	[PMU_REG_SMPLCTRL] =		{.map = 0x00, .addr = 0x06, .mask = 0xFF, .ro = 0},
	[PMU_REG_WRLOCKKEY] =		{.map = 0x00, .addr = 0x07, .mask = 0xFF, .ro = 0},
	[PMU_REG_WRPROEN] =		{.map = 0x00, .addr = 0x08, .mask = 0xFF, .ro = 0},
	[PMU_REG_PMUGID] =		{.map = 0x00, .addr = 0x09, .mask = 0xFF, .ro = 0},
	[PMU_REG_PONKEYCTRL1] =		{.map = 0x00, .addr = 0x0C, .mask = 0xFF, .ro = 0},
	[PMU_REG_PONKEYCTRL2] =		{.map = 0x00, .addr = 0x0D, .mask = 0xFF, .ro = 0},
	[PMU_REG_PONKEYCTRL3] =		{.map = 0x00, .addr = 0x0E, .mask = 0xFF, .ro = 0},
	[PMU_REG_AUXCTRL] =		{.map = 0x00, .addr = 0x0F, .mask = 0xFF, .ro = 0},
	[PMU_REG_RTCSC] =		{.map = 0x00, .addr = 0x20, .mask = 0x3F, .ro = 0},
	[PMU_REG_RTCMN] =		{.map = 0x00, .addr = 0x21, .mask = 0x3F, .ro = 0},
	[PMU_REG_RTCHR] =		{.map = 0x00, .addr = 0x22, .mask = 0x1F, .ro = 0},
	[PMU_REG_RTCDT] =		{.map = 0x00, .addr = 0x23, .mask = 0x1F, .ro = 0},
	[PMU_REG_RTCMT] =		{.map = 0x00, .addr = 0x24, .mask = 0x0F, .ro = 0},
	[PMU_REG_RTCYR] =		{.map = 0x00, .addr = 0x25, .mask = 0xFF, .ro = 0},
	[PMU_REG_RTCSC_ALM] =		{.map = 0x00, .addr = 0x26, .mask = 0x3F, .ro = 0},
	[PMU_REG_RTCMN_ALM] =		{.map = 0x00, .addr = 0x27, .mask = 0x3F, .ro = 0},
	[PMU_REG_RTCHR_ALM] =		{.map = 0x00, .addr = 0x28, .mask = 0x1F, .ro = 0},
	[PMU_REG_RTCDT_ALM] =		{.map = 0x00, .addr = 0x2A, .mask = 0x1F, .ro = 0},
	[PMU_REG_RTCMT_ALM] =		{.map = 0x00, .addr = 0x2B, .mask = 0x0F, .ro = 0},
	[PMU_REG_RTCYR_ALM] =		{.map = 0x00, .addr = 0x2C, .mask = 0xFF, .ro = 0},
	[PMU_REG_RTC_CORE] =		{.map = 0x00, .addr = 0x2D, .mask = 0xFF, .ro = 0},
	[PMU_REG_RTC_C2C1_XOTRIM] =	{.map = 0x00, .addr = 0x2E, .mask = 0xFF, .ro = 0},
	[PMU_REG_RFOPMODCTRL] =		{.map = 0x00, .addr = 0xA0, .mask = 0xFF, .ro = 0},
	[PMU_REG_CAMOPMODCTRL] =	{.map = 0x00, .addr = 0xA1, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV1OPMODCTRL] =	{.map = 0x00, .addr = 0xA2, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV2OPMODCTRL] =	{.map = 0x00, .addr = 0xA3, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV3OPMODCTRL] =	{.map = 0x00, .addr = 0xA4, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV4OPMODCTRL] =	{.map = 0x00, .addr = 0xA5, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV5OPMODCTRL] =	{.map = 0x00, .addr = 0xA6, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV6OPMODCTRL] =	{.map = 0x00, .addr = 0xA7, .mask = 0xFF, .ro = 0},
	[PMU_REG_HV7OPMODCTRL] =	{.map = 0x00, .addr = 0xA8, .mask = 0xFF, .ro = 0},
	[PMU_REG_SIMOPMODCTRL] =	{.map = 0x00, .addr = 0xA9, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSROPMODCTRL] =	{.map = 0x00, .addr = 0xAA, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSROPMODCTRL] =	{.map = 0x00, .addr = 0xAB, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSROPMODCTRL] =	{.map = 0x00, .addr = 0xAC, .mask = 0xFF, .ro = 0},
	[PMU_REG_RFLDOCTRL] =		{.map = 0x00, .addr = 0xB0, .mask = 0xFF, .ro = 0},
	[PMU_REG_CAMLDOCTRL] =		{.map = 0x00, .addr = 0xB1, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO1CTRL] =		{.map = 0x00, .addr = 0xB2, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO2CTRL] =		{.map = 0x00, .addr = 0xB3, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO3CTRL] =		{.map = 0x00, .addr = 0xB4, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO4CTRL] =		{.map = 0x00, .addr = 0xB5, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO5CTRL] =		{.map = 0x00, .addr = 0xB6, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO6CTRL] =		{.map = 0x00, .addr = 0xB7, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO7CTRL] =		{.map = 0x00, .addr = 0xB8, .mask = 0xFF, .ro = 0},
	[PMU_REG_SIMLDOCTRL] =		{.map = 0x00, .addr = 0xB9, .mask = 0xFF, .ro = 0},
	[PMU_REG_PWR_GRP_DLY] =		{.map = 0x00, .addr = 0xBB, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL1] =		{.map = 0x00, .addr = 0xC0, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL2] =		{.map = 0x00, .addr = 0xC1, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL3] =		{.map = 0x00, .addr = 0xC2, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL4] =		{.map = 0x00, .addr = 0xC3, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL5] =		{.map = 0x00, .addr = 0xC4, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL6] =		{.map = 0x00, .addr = 0xC5, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL7] =		{.map = 0x00, .addr = 0xC6, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL8] =		{.map = 0x00, .addr = 0xC7, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL1] =		{.map = 0x00, .addr = 0xC8, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL2] =		{.map = 0x00, .addr = 0xC9, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL3] =		{.map = 0x00, .addr = 0xCA, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL4] =		{.map = 0x00, .addr = 0xCB, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL5] =		{.map = 0x00, .addr = 0xCC, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL6] =		{.map = 0x00, .addr = 0xCD, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL7] =		{.map = 0x00, .addr = 0xCE, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL8] =		{.map = 0x00, .addr = 0xCF, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL1] =		{.map = 0x00, .addr = 0xD0, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL2] =		{.map = 0x00, .addr = 0xD1, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL3] =		{.map = 0x00, .addr = 0xD2, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL4] =		{.map = 0x00, .addr = 0xD3, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL5] =		{.map = 0x00, .addr = 0xD4, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL6] =		{.map = 0x00, .addr = 0xD5, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL7] =		{.map = 0x00, .addr = 0xD6, .mask = 0xFF, .ro = 0},
	[PMU_REG_SDSRCTRL8] =		{.map = 0x00, .addr = 0xD7, .mask = 0xFF, .ro = 0},
	[PMU_REG_ENV1] =		{.map = 0x00, .addr = 0xE0, .mask = 0xFF, .ro = 1},
	[PMU_REG_ENV2] =		{.map = 0x00, .addr = 0xE1, .mask = 0xFF, .ro = 1},
	[PMU_REG_ENV3] =		{.map = 0x00, .addr = 0xE2, .mask = 0xFF, .ro = 1},
	[PMU_REG_ENV4] =		{.map = 0x00, .addr = 0xE3, .mask = 0xFF, .ro = 1},
	[PMU_REG_ENV5] =		{.map = 0x00, .addr = 0xE4, .mask = 0xFF, .ro = 1},
	[PMU_REG_ENV6] =		{.map = 0x00, .addr = 0xE5, .mask = 0xFF, .ro = 1},
	[PMU_REG_ENV7] =		{.map = 0x00, .addr = 0xE6, .mask = 0xFF, .ro = 1},
	[PMU_REG_IHFTOP_IHF_IDDQ] =	{.map = 0x01, .addr = 0x80, .mask = 0x01, .ro = 0},
	[PMU_REG_IHFLDO_PUP] =		{.map = 0x01, .addr = 0x82, .mask = 0x01, .ro = 0},
	[PMU_REG_IHFPOP_PUP] =		{.map = 0x01, .addr = 0x83, .mask = 0x08, .ro = 0},
	[PMU_REG_IHFPGA2_GAIN] =	{.map = 0x01, .addr = 0x8A, .mask = 0x3F, .ro = 0},
	[PMU_REG_HSPUP1_IDDQ_PWRDWN] =	{.map = 0x01, .addr = 0xA2, .mask = 0x10, .ro = 0},
	[PMU_REG_HSPUP2_HS_PWRUP] =	{.map = 0x01, .addr = 0xA3, .mask = 0x80, .ro = 0},
	[PMU_REG_HSPGA1_GAIN] =		{.map = 0x01, .addr = 0x9E, .mask = 0x3F, .ro = 0},
	[PMU_REG_HSPGA2_GAIN] =		{.map = 0x01, .addr = 0x9F, .mask = 0x3F, .ro = 0},
	[PMU_REG_PMUID] =		{.map = 0x01, .addr = 0xF7, .mask = 0xFF, .ro = 0},
	[PMU_REG_PMUREV] =		{.map = 0x01, .addr = 0xF8, .mask = 0xFF, .ro = 0},
};

static const struct bcmpmu_irq_map bcm59055_irq_map[PMU_IRQ_MAX] = {//revisit
	[PMU_IRQ_RTC_ALARM] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x01},
	[PMU_IRQ_RTC_SEC] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x02},
	[PMU_IRQ_RTC_MIN] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x04},
	[PMU_IRQ_RTCADJ] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x08},
	[PMU_IRQ_BATINS] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x10},
	[PMU_IRQ_BATRM] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x20},
	[PMU_IRQ_GBAT_PLUG_IN] =	{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x40},
	[PMU_IRQ_SMPL_INT] =		{.int_addr = 0x30, .mask_addr = 0x40, .bit_mask = 0x80},
	[PMU_IRQ_USBINS] =		{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x01},
	[PMU_IRQ_USBRM] =		{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x02},
	[PMU_IRQ_USBOV] =		{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x04},
	[PMU_IRQ_EOC] =			{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x08},
	[PMU_IRQ_RESUME_VBUS] =		{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x10},
	[PMU_IRQ_CHG_HW_TTR_EXP] =	{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x20},
	[PMU_IRQ_CHG_HW_TCH_EXP] =	{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x40},
	[PMU_IRQ_CHG_SW_TMR_EXP] =	{.int_addr = 0x31, .mask_addr = 0x41, .bit_mask = 0x80},
	[PMU_IRQ_CHGDET_LATCH] =	{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x01},
	[PMU_IRQ_CHGDET_TO] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x02},
	[PMU_IRQ_MBTEMPLOW] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x04},
	[PMU_IRQ_MBTEMPHIGH] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x08},
	[PMU_IRQ_MBOV] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x10},
	[PMU_IRQ_MBOV_DIS] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x20},
	[PMU_IRQ_USBOV_DIS] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x40},
	[PMU_IRQ_CHGERRDIS] =		{.int_addr = 0x32, .mask_addr = 0x42, .bit_mask = 0x80},
	[PMU_IRQ_VBUS_1V5_R] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x01},
	[PMU_IRQ_VBUS_4V5_R] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x02},
	[PMU_IRQ_VBUS_1V5_F] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x03},
	[PMU_IRQ_VBUS_4V5_F] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x04},
	[PMU_IRQ_MBWV_R_10S_WAIT] =	{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x10},
	[PMU_IRQ_BBLOW] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x20},
	[PMU_IRQ_LOWBAT] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x40},
	[PMU_IRQ_VERYLOWBAT] =		{.int_addr = 0x33, .mask_addr = 0x43, .bit_mask = 0x80},
	[PMU_IRQ_RTM_DATA_RDY] =	{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x01},
	[PMU_IRQ_RTM_IN_CON_MEAS] =	{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x02},
	[PMU_IRQ_RTM_UPPER] =		{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x04},
	[PMU_IRQ_RTM_IGNORE] =		{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x08},
	[PMU_IRQ_RTM_OVERRIDDEN] =	{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x10},
	[PMU_IRQ_AUD_HSAB_SHCKT] =	{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x20},
	[PMU_IRQ_AUD_IHFD_SHCKT] =	{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x40},
	[PMU_IRQ_MBC_TF] =		{.int_addr = 0x34, .mask_addr = 0x44, .bit_mask = 0x80},
	[PMU_IRQ_CSROVRI] =		{.int_addr = 0x35, .mask_addr = 0x45, .bit_mask = 0x01},
	[PMU_IRQ_IOSROVRI] =		{.int_addr = 0x35, .mask_addr = 0x45, .bit_mask = 0x02},
	[PMU_IRQ_SDSROVRI] =		{.int_addr = 0x35, .mask_addr = 0x45, .bit_mask = 0x04},
	[PMU_IRQ_ASROVRI] =		{.int_addr = 0x35, .mask_addr = 0x45, .bit_mask = 0x08},
	[PMU_IRQ_UBPD_CHG_F] =		{.int_addr = 0x35, .mask_addr = 0x45, .bit_mask = 0x10},
	[PMU_IRQ_ACD_INS] =		{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x01},
	[PMU_IRQ_ACD_RM] =		{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x02},
	[PMU_IRQ_PONKEYB_HOLD] =	{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x04},
	[PMU_IRQ_PONKEYB_F] =		{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x08},
	[PMU_IRQ_PONKEYB_R] =		{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x10},
	[PMU_IRQ_PONKEYB_OFFHOLD] =	{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x20},
	[PMU_IRQ_PONKEYB_RESTART] =	{.int_addr = 0x36, .mask_addr = 0x46, .bit_mask = 0x40},
	[PMU_IRQ_IDCHG] =		{.int_addr = 0x37, .mask_addr = 0x47, .bit_mask = 0x01},
	[PMU_IRQ_JIG_USB_INS] =		{.int_addr = 0x37, .mask_addr = 0x47, .bit_mask = 0x02},
	[PMU_IRQ_UART_INS] =		{.int_addr = 0x37, .mask_addr = 0x47, .bit_mask = 0x04},
	[PMU_IRQ_ID_INS] =		{.int_addr = 0x37, .mask_addr = 0x47, .bit_mask = 0x08},
	[PMU_IRQ_ID_RM] =		{.int_addr = 0x37, .mask_addr = 0x47, .bit_mask = 0x10},
};

static const struct bcmpmu_env_info bcm59055_env_reg_map[PMU_ENV_MAX] = {//revisit
	[PMU_ENV_MBWV_DELTA] = 		{.regmap = {.addr = 0xE0, .mask = 0x02, .shift = 1, .ro = 1}, .bitmask = PMU_ENV_BITMASK_MBWV_DELTA},
	[PMU_ENV_CGPD_ENV] = 		{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_CGPD_ENV},
	[PMU_ENV_UBPD_ENV] = 		{.regmap = {.addr = 0xE1, .mask = 0x01, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_UBPD_ENV},
	[PMU_ENV_UBPD_USBDET] = 	{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_UBPD_USBDET},
	[PMU_ENV_CGPD_PRI] = 		{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_CGPD_PRI},
	[PMU_ENV_UBPD_PRI] = 		{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_UBPD_PRI},
	[PMU_ENV_WAC_VALID] = 		{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_WAC_VALID},
	[PMU_ENV_USB_VALID] = 		{.regmap = {.addr = 0xE1, .mask = 0x08, .shift = 3, .ro = 1}, .bitmask = PMU_ENV_BITMASK_USB_VALID},
	[PMU_ENV_P_CGPD_CHG] = 		{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_P_CGPD_CHG},
	[PMU_ENV_P_UBPD_CHR] = 		{.regmap = {.addr = 0xE1, .mask = 0x04, .shift = 2, .ro = 1}, .bitmask = PMU_ENV_BITMASK_P_UBPD_CHR},
	[PMU_ENV_PORT_DISABLE] = 	{.regmap = {.addr = 0x00, .mask = 0x00, .shift = 0, .ro = 1}, .bitmask = PMU_ENV_BITMASK_PORT_DISABLE},
};

static const struct bcmpmu_adc_map bcm59055_adc_map[PMU_ADC_MAX] = {//revisit
	[PMU_ADC_VMBATT] =		{.addr0 = 0x83, .addr1 = 0x82, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x00, .vrng = 4800},
	[PMU_ADC_VBBATT] =		{.addr0 = 0x85, .addr1 = 0x84, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x01, .vrng = 4800},
	[PMU_ADC_VBUS] =		{.addr0 = 0x89, .addr1 = 0x88, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x03, .vrng = 5400},
	[PMU_ADC_ID] =			{.addr0 = 0x8B, .addr1 = 0x8A, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x04, .vrng = 4800},
	[PMU_ADC_NTC] =			{.addr0 = 0x8D, .addr1 = 0x8C, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x05, .vrng = 1200},
	[PMU_ADC_BSI] =			{.addr0 = 0x8F, .addr1 = 0x8E, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x06, .vrng = 1200},
	[PMU_ADC_32KTEMP] =		{.addr0 = 0x93, .addr1 = 0x92, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x08, .vrng = 4800},
	[PMU_ADC_RTM] =			{.addr0 = 0x99, .addr1 = 0x98, .dmask = 0x3FF, .vmask = 0x0000, .rtmsel = 0x00, .vrng = 0000},
};

static const struct bcmpmu_reg_map bcm59055_adc_ctrl_map[PMU_ADC_CTRL_MAX] = {//revisit
	[PMU_ADC_RST_CNT] =		{.map = 0, .addr = 0x80, .mask = 0x03, .shift = 0},
	[PMU_ADC_RTM_START] =		{.map = 0, .addr = 0x80, .mask = 0x04, .shift = 2},
	[PMU_ADC_RTM_MASK] =		{.map = 0, .addr = 0x80, .mask = 0x08, .shift = 3},
	[PMU_ADC_RTM_SEL] =		{.map = 0, .addr = 0x80, .mask = 0xF0, .shift = 4},
	[PMU_ADC_RTM_DLY] =		{.map = 0, .addr = 0x81, .mask = 0x1F, .shift = 0},
	[PMU_ADC_GSM_DBNC] =		{.map = 0, .addr = 0x81, .mask = 0x20, .shift = 5},
};

static const struct bcmpmu_reg_map bcm59055_irq_reg_map[PMU_IRG_REG_MAX] = {
	[PMU_REG_INT1] =		{.map = 0, .addr = 0x30, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT2] =		{.map = 0, .addr = 0x31, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT3] =		{.map = 0, .addr = 0x32, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT4] =		{.map = 0, .addr = 0x33, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT5] =		{.map = 0, .addr = 0x34, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT6] =		{.map = 0, .addr = 0x35, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT7] =		{.map = 0, .addr = 0x36, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT8] =		{.map = 0, .addr = 0x37, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT9] =		{.map = 0, .addr = 0x38, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT10] =		{.map = 0, .addr = 0x39, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT11] =		{.map = 0, .addr = 0x3a, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT12] =		{.map = 0, .addr = 0x3b, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT13] =		{.map = 0, .addr = 0x3c, .mask = 0xFF, .ro = 0},
	[PMU_REG_INT14] =		{.map = 0, .addr = 0x3d, .mask = 0xFF, .ro = 0},
};

const struct bcmpmu_reg_map *bcmpmu_get_regmap(void)
{
	return bcm59055_reg_map;
}

const struct bcmpmu_irq_map *bcmpmu_get_irqmap(void)
{
	return bcm59055_irq_map;
}

const struct bcmpmu_adc_map *bcmpmu_get_adcmap(void)
{
	return bcm59055_adc_map;
}

const struct bcmpmu_reg_map *bcmpmu_get_irqregmap(int *len)
{
	*len = PMU_IRG_REG_MAX;
	return bcm59055_irq_reg_map;
}

const struct bcmpmu_reg_map *bcmpmu_get_adc_ctrl_map(void)
{
	return bcm59055_adc_ctrl_map;
}

int bcmpmu_clear_irqs(struct bcmpmu *bcmpmu)
{
	return 0;
}

const struct bcmpmu_env_info *bcmpmu_get_envregmap(int *len)
{
	*len = PMU_ENV_REG_MAX;
	return bcm59055_env_reg_map;
}

int bcmpmu_sel_adcsync(enum bcmpmu_adc_timing_t timing)
{
	return 0;
}


MODULE_DESCRIPTION("BCM590XX PMIC bcm59055 driver");
MODULE_LICENSE("GPL");

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

#define PMU_IRQ_REG_MAX		3
#define PMU_ENV_REG_MAX		1

/* Use this for the regulator stuff */
static const struct bcmpmu_reg_map samoa_reg_map[PMU_REG_MAX] = {
	[PMU_REG_PMUID] =		{.map = 0x00, .addr = 0x00000024, .mask = 0xFFFFFFFF, .ro = 0},
	[PMU_REG_RFOPMODCTRL] =		{.map = 0x00, .addr = 0x00000284, .mask = 0xFF, .ro = 0},
	[PMU_REG_RFLDOCTRL] =		{.map = 0x00, .addr = 0x00000284, .mask = 0x3800, .ro = 0},
	[PMU_REG_CAMOPMODCTRL] =	{.map = 0x00, .addr = 0x00000288, .mask = 0xFF, .ro = 0},
	[PMU_REG_CAMLDOCTRL] =		{.map = 0x00, .addr = 0x00000288, .mask = 0x7800, .ro = 0},
	[PMU_REG_HV1OPMODCTRL] =	{.map = 0x00, .addr = 0x0000028c, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO1CTRL] =		{.map = 0x00, .addr = 0x0000028c, .mask = 0x3800, .ro = 0},
	[PMU_REG_HV2OPMODCTRL] =	{.map = 0x00, .addr = 0x00000290, .mask = 0xFF, .ro = 0},
	[PMU_REG_HVLDO2CTRL] =		{.map = 0x00, .addr = 0x00000290, .mask = 0x3800, .ro = 0},
	[PMU_REG_SIMOPMODCTRL] =	{.map = 0x00, .addr = 0x00000294, .mask = 0xFF, .ro = 0},
	[PMU_REG_SIMLDOCTRL] =		{.map = 0x00, .addr = 0x00000294, .mask = 0x3800, .ro = 0},
	[PMU_REG_SIMLDOEN] =		{.map = 0x00, .addr = 0x00000294, .mask = 0x4000, .ro = 0},
	[PMU_REG_SIM2OPMODCTRL] =	{.map = 0x00, .addr = 0x00000298, .mask = 0xFF, .ro = 0},
	[PMU_REG_SIMLDO2CTRL] =		{.map = 0x00, .addr = 0x00000298, .mask = 0x3800, .ro = 0},
	[PMU_REG_SIMLDO2EN] =		{.map = 0x00, .addr = 0x00000298, .mask = 0x4000, .ro = 0},
	[PMU_REG_USBOPMODCTRL] =	{.map = 0x00, .addr = 0x0000029c, .mask = 0xFF, .ro = 0},
	[PMU_REG_USBLDOCTRL] =		{.map = 0x00, .addr = 0x0000029c, .mask = 0x3800, .ro = 0},
	[PMU_REG_BCDLDOCTRL] =		{.map = 0x00, .addr = 0x0000029c, .mask = 0x1C000, .ro = 0},
	[PMU_REG_DVS1OPMODCTRL] =	{.map = 0x00, .addr = 0x000002a0, .mask = 0xFF, .ro = 0},
	[PMU_REG_DVS2OPMODCTRL] =	{.map = 0x00, .addr = 0x000002a4, .mask = 0xFF, .ro = 0},
	[PMU_REG_DVSLDO1VSEL1] =	{.map = 0x00, .addr = 0x000002a8, .mask = 0x1F, .ro = 0},
	[PMU_REG_DVSLDO1VSEL2] =	{.map = 0x00, .addr = 0x000002ac, .mask = 0x1F, .ro = 0},
	[PMU_REG_DVSLDO1VSEL3] =	{.map = 0x00, .addr = 0x000002b0, .mask = 0x1F, .ro = 0},
	[PMU_REG_DVSLDO2VSEL1] =	{.map = 0x00, .addr = 0x000002b4, .mask = 0x1F, .ro = 0},
	[PMU_REG_DVSLDO2VSEL2] =	{.map = 0x00, .addr = 0x000002b8, .mask = 0x1F, .ro = 0},
	[PMU_REG_DVSLDO2VSEL3] =	{.map = 0x00, .addr = 0x000002bc, .mask = 0x1F, .ro = 0},
	[PMU_REG_CSROPMODCTRL] =	{.map = 0x00, .addr = 0x00000300, .mask = 0xFF, .ro = 0},
	[PMU_REG_CSRCTRL1] =		{.map = 0x00, .addr = 0x00000308, .mask = 0x1F, .ro = 0},
	[PMU_REG_CSRCTRL2] =		{.map = 0x00, .addr = 0x0000030c, .mask = 0x1F, .ro = 0},
	[PMU_REG_CSRCTRL3] =		{.map = 0x00, .addr = 0x00000310, .mask = 0x1F, .ro = 0},
	[PMU_REG_IOSROPMODCTRL] =	{.map = 0x00, .addr = 0x00000304, .mask = 0xFF, .ro = 0},
	[PMU_REG_IOSRCTRL1] =		{.map = 0x00, .addr = 0x00000314, .mask = 0x7, .ro = 0},
	[PMU_REG_IOSRCTRL2] =		{.map = 0x00, .addr = 0x00000318, .mask = 0x7, .ro = 0},
	[PMU_REG_IOSRCTRL3] =		{.map = 0x00, .addr = 0x0000031c, .mask = 0x7, .ro = 0},
};

static const struct bcmpmu_irq_map samoa_irq_map[PMU_IRQ_MAX] = {
	[PMU_IRQ_GBAT_PLUG_IN] =	{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00000080},

	[PMU_IRQ_USBINS] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00001000},
	[PMU_IRQ_USBRM] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00002000},
	[PMU_IRQ_CHGERRDIS] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00400000},
	[PMU_IRQ_VBUS_1V5_R] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x01000000},
	[PMU_IRQ_VBUS_4V5_R] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x02000000},
	[PMU_IRQ_UBPD_CHG_F] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00200000},
	/* [PMU_IRQ_CGPD_CHG_F] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00100000}, */
	[PMU_IRQ_PONKEYB_HOLD] =	{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00000010},
	[PMU_IRQ_PONKEYB_F] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00000001},
	[PMU_IRQ_PONKEYB_R] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00000002},
	/* [PMU_IRQ_PONKEYB_VLD] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x00000008}, */
	[PMU_IRQ_SMPL_INT] =		{.int_addr = 0x000000c0, .mask_addr = 0x00000100, .bit_mask = 0x10000000},
	
	[PMU_IRQ_RTC_ALARM] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x01000000},
	[PMU_IRQ_RTC_SEC] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x02000000},
	[PMU_IRQ_RTC_MIN] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x04000000},
	[PMU_IRQ_RTCADJ] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x08000000},
	[PMU_IRQ_BATINS] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000080},
	[PMU_IRQ_BATRM] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000100},
	[PMU_IRQ_USBOV] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00002000},
	[PMU_IRQ_RESUME_VBUS] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000040},
	[PMU_IRQ_CHG_HW_TTR_EXP] =	{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00020000},
	[PMU_IRQ_CHG_SW_TMR_EXP] =	{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00010000},
	[PMU_IRQ_CHGDET_LATCH] =	{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000002},
	[PMU_IRQ_CHGDET_TO] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000003},
	[PMU_IRQ_MBTEMPLOW] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000400},
	[PMU_IRQ_MBTEMPHIGH] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00000800},
	[PMU_IRQ_MBOV] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00200000},
	[PMU_IRQ_MBOV_DIS] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00004000},
	[PMU_IRQ_USBOV_DIS] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00008000},
	[PMU_IRQ_VBUS_1V5_F] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x04000000},
	[PMU_IRQ_VBUS_4V5_F] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x08000000},
	[PMU_IRQ_MBWV_R_10S_WAIT] =	{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00100000},
	[PMU_IRQ_BBLOW] =		{.int_addr = 0x000000c4, .mask_addr = 0x00000104, .bit_mask = 0x00400000},

	[PMU_IRQ_RTM_DATA_RDY] =	{.int_addr = 0x000000c8, .mask_addr = 0x00000108, .bit_mask = 0x00000001},
	[PMU_IRQ_RTM_IN_CON_MEAS] =	{.int_addr = 0x000000c8, .mask_addr = 0x00000108, .bit_mask = 0x00000002},
	[PMU_IRQ_RTM_UPPER] =		{.int_addr = 0x000000c8, .mask_addr = 0x00000108, .bit_mask = 0x00000004},
	[PMU_IRQ_RTM_OVERRIDDEN] =	{.int_addr = 0x000000c8, .mask_addr = 0x00000108, .bit_mask = 0x00000010},
	[PMU_IRQ_CSROVRI] =		{.int_addr = 0x000000c8, .mask_addr = 0x00000108, .bit_mask = 0x00000100},
	[PMU_IRQ_IOSROVRI] =		{.int_addr = 0x000000c8, .mask_addr = 0x00000108, .bit_mask = 0x00000200},
};

static const struct bcmpmu_env_info samoa_env_reg_map[PMU_ENV_MAX] = {//revisit
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

static const struct bcmpmu_adc_map samoa_adc_map[PMU_ADC_MAX] = {//revisit
	[PMU_ADC_VMBATT] =		{.addr0 = 0x83, .addr1 = 0x82, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x00, .vrng = 4800},
	[PMU_ADC_VBBATT] =		{.addr0 = 0x85, .addr1 = 0x84, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x01, .vrng = 4800},
	[PMU_ADC_VBUS] =		{.addr0 = 0x89, .addr1 = 0x88, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x03, .vrng = 5400},
	[PMU_ADC_ID] =			{.addr0 = 0x8B, .addr1 = 0x8A, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x04, .vrng = 4800},
	[PMU_ADC_NTC] =			{.addr0 = 0x8D, .addr1 = 0x8C, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x05, .vrng = 1200},
	[PMU_ADC_BSI] =			{.addr0 = 0x8F, .addr1 = 0x8E, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x06, .vrng = 1200},
	[PMU_ADC_32KTEMP] =		{.addr0 = 0x93, .addr1 = 0x92, .dmask = 0x3FF, .vmask = 0x0400, .rtmsel = 0x08, .vrng = 4800},
	[PMU_ADC_RTM] =			{.addr0 = 0x99, .addr1 = 0x98, .dmask = 0x3FF, .vmask = 0x0000, .rtmsel = 0x00, .vrng = 0000},
};

static const struct bcmpmu_reg_map samoa_adc_ctrl_map[PMU_ADC_CTRL_MAX] = {//revisit
	[PMU_ADC_RST_CNT] =		{.map = 0, .addr = 0x80, .mask = 0x03, .shift = 0},
	[PMU_ADC_RTM_START] =		{.map = 0, .addr = 0x80, .mask = 0x04, .shift = 2},
	[PMU_ADC_RTM_MASK] =		{.map = 0, .addr = 0x80, .mask = 0x08, .shift = 3},
	[PMU_ADC_RTM_SEL] =		{.map = 0, .addr = 0x80, .mask = 0xF0, .shift = 4},
	[PMU_ADC_RTM_DLY] =		{.map = 0, .addr = 0x81, .mask = 0x1F, .shift = 0},
	[PMU_ADC_GSM_DBNC] =		{.map = 0, .addr = 0x81, .mask = 0x20, .shift = 5},
};

static const struct bcmpmu_reg_map samoa_irq_reg_map[PMU_IRQ_REG_MAX] = {
	[PMU_REG_INT1] =		{.map = 0, .addr = 0xc0, .mask = 0x1FFFFFFF, .ro = 0},
	[PMU_REG_INT2] =		{.map = 0, .addr = 0xc4, .mask = 0xFFFFFFF, .ro = 0},
	[PMU_REG_INT3] =		{.map = 0, .addr = 0xc8, .mask = 0x3FFFFFF, .ro = 0},
};

const struct bcmpmu_reg_map *bcmpmu_get_regmap(void)
{
	return samoa_reg_map;
}

const struct bcmpmu_irq_map *bcmpmu_get_irqmap(void)
{
	return samoa_irq_map;
}

const struct bcmpmu_adc_map *bcmpmu_get_adcmap(void)
{
	return samoa_adc_map;
}

const struct bcmpmu_reg_map *bcmpmu_get_irqregmap(int *len)
{
	*len = PMU_IRQ_REG_MAX;
	return samoa_irq_reg_map;
}

const struct bcmpmu_reg_map *bcmpmu_get_adc_ctrl_map(void)
{
	return samoa_adc_ctrl_map;
}

int bcmpmu_clear_irqs(struct bcmpmu *bcmpmu)
{
	return 0;
}

const struct bcmpmu_env_info *bcmpmu_get_envregmap(int *len)
{
	*len = PMU_ENV_REG_MAX;
	return samoa_env_reg_map;
}

int bcmpmu_sel_adcsync(enum bcmpmu_adc_timing_t timing)
{
	return 0;
}


MODULE_DESCRIPTION("BCMSAMOA PMIC PMU driver");
MODULE_LICENSE("GPL");

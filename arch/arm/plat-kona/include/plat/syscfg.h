/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_spi.h
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

#ifndef ASM_PLAT_SYSCFG_H
#define ASM_PLAT_SYSCFG_H

enum syscfg_mod {
	SYSCFG_CAMERA,
	SYSCFG_HEADSET,
	SYSCFG_AUXMIC,
	SYSCFG_I2S,
	SYSCFG_LCD,
	SYSCFG_LCDBACKLIGHT,
	SYSCFG_RESETREASON,
	SYSCFG_SDHC1,
	SYSCFG_SDHC2,
	SYSCFG_SDHC3,
	SYSCFG_SPI1,
	SYSCFG_SPI2,
	SYSCFG_SYSTEMINIT,
	SYSCFG_USB,
	SYSCFG_VIBRATOR,
	SYSCFG_V3D,
	SYSCFG_LED,
	SYSCFG_PWM0,
	SYSCFG_PWM5 = SYSCFG_PWM0 + 5,
	SYSCFG_CSL_DMA,
};

enum syscfg_op {
	SYSCFG_INIT,
	SYSCFG_ENABLE,
	SYSCFG_DISABLE,
};

enum syscfg_reset_reason {
	POWER_ON_RESET = 0x0,
	SOFT_RESET = 0x100,
	POWEROFF_CHARGING = 0x200,
};

int board_sysconfig(uint32_t syscfg_mod, uint32_t syscfg_op);

#endif

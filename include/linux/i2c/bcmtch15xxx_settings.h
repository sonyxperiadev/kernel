/*****************************************************************************
* Copyright (c) 2013 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/

#ifndef BCMTCH15XXX_SETTINGS_H
#define BCMTCH15XXX_SETTINGS_H

/* chip constants */
#define	BCMTCH_HW_BCM15200A0		0x015200a0
#define	BCMTCH_HW_BCM15500A0		0x015500a0
#define	BCMTCH_HW_BCM15500A1		0x015500a1
#define	BCMTCH_HW_BCM15200A1		0x015200a1

/* set to one of the chip constants above */
#define	BCMTCH_HW_CHIP_VERSION		BCMTCH_HW_BCM15200A1

/* ---------------------------------------------------------- */
/* - (Host) Platform Specific Definitions and Platform Data - */
/* ---------------------------------------------------------- */

/*
 * host I2C bus/adapter id that the BCM Touch Controller is on
 * - MUST BE SET
 */
#define BCMTCH_HW_I2C_BUS_ID            3

/*
 * host gpio pin used to reset the BCM Touch Controller
 * - MUST be set to -1 if not used
 */
#define BCMTCH_HW_GPIO_RESET_PIN        0x46

#define BCMTCH_HW_POLARITY_ACTIVE_HIGH  1
#define BCMTCH_HW_POLARITY_ACTIVE_LOW   0

#define BCMTCH_HW_GPIO_RESET_POLARITY   BCMTCH_HW_POLARITY_ACTIVE_LOW
#define BCMTCH_HW_GPIO_RESET_TIME_MS    100

/*
 * host gpio pin used to receive interrupts from the BCM Touch Controller
 * - MUST be set to -1 if not used
 */
#define BCMTCH_HW_GPIO_INTERRUPT_PIN        0x49

#define BCMTCH_HW_GPIO_INTERRUPT_TRIGGER    IRQF_TRIGGER_FALLING

/* I2C addresses of the BCM Touch Controller */
#if (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15200A0)
#define BCMTCH_HW_I2C_ADDR_SPM      0x67 /* SPM address on I2C bus */
#elif (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15200A1)
#define BCMTCH_HW_I2C_ADDR_SPM      0x67 /* SPM address on I2C bus */
#elif (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15500A0)
#define BCMTCH_HW_I2C_ADDR_SPM      0x66 /* SPM address on I2C bus */
#elif (BCMTCH_HW_CHIP_VERSION == BCMTCH_HW_BCM15500A1)
#define BCMTCH_HW_I2C_ADDR_SPM      0x66 /* SPM address on I2C bus */
#else
#error BCMTCH_HW_CHIP_VERSION is not set, cannot build driver
#endif

#define BCMTCH_HW_I2C_ADDR_SYS       0x68 /* SYS/AHB address on I2C bus */

/* Orientation of the LCD/touch panel to the BCM Touch Controller */
#define BCMTCH_HW_AXIS_SWAP_X_Y		0	/* swap x <-> y axes */

#define BCMTCH_HW_AXIS_REVERSE_Y	0	/* reverse x-axis */

#define BCMTCH_HW_AXIS_REVERSE_X	0	/* reverse y-axis */

/*
 * Support for extended TouchScreen/Panel buttons/keys
 *
 * If the device has buttons which are extensions of the touch panel,
 * set the values below:
 *   a) define BCMTCH_HW_BUTTON_COUNT equal to the number of buttons
 *   b) set the entries in the bcmtch_hw_button_map array to the correct
 *      key/button codes
 *
 */
#define	BCMTCH_HW_BUTTON_COUNT		0

static const int32_t bcmtch_hw_button_map[] = {
		KEY_MENU,
		KEY_BACK
		};

#endif /* BCMTCH15XXX_SETTINGS_H */

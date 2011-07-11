/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef HALAUDIO_PCM_SETTINGS_H_
#define HALAUDIO_PCM_SETTINGS_H_

/* ---- Include Files ---------------------------------------- */
/* ---- Constants and Types ---------------------------------- */

/* PCM Configurations */
#define HALAUDIO_PCM_SETTINGS_CORE_ID_SELECT  0 /* Selected SSPI Core for PCM */
#define HALAUDIO_PCM_SETTINGS_CHANS_SUPPORTED 1 /* Selected 1 channel for BT support */
#define HALAUDIO_PCM_SETTINGS_CHAN_SELECT     0 /* BT support using PCM 0 */

/* Bluetooth GPIO pins */
#define HALAUDIO_PCM_SETTINGS_GPIO_BT_RST_B       -1
#define HALAUDIO_PCM_SETTINGS_GPIO_BT_VREG_CTL    -1
#define HALAUDIO_PCM_SETTINGS_GPIO_BT_WAKE        -1

/* Bluetooth UART GPIO group */
#define HALAUDIO_PCM_SETTINGS_BT_REQ_UART_GPIO_GROUP     -1

/* ---- Variable Externs ------------------------------------- */
/* ---- Function Prototypes ---------------------------------- */

#endif /* HALAUDIO_PCM_SETTINGS_H_ */

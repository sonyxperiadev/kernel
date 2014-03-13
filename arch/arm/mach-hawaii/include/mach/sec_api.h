/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#ifndef SEC_API_H
#define SEC_API_H

#include <linux/types.h>
#define FLAGS	(SEC_ROM_ICACHE_ENABLE_MASK | SEC_ROM_DCACHE_ENABLE_MASK | \
			SEC_ROM_IRQ_ENABLE_MASK | SEC_ROM_FIQ_ENABLE_MASK)

/*!
 * Definitions for IRQ & FIQ Mask for ARM
 */

#define FIQ_IRQ_MASK						0xC0
#define FIQ_MASK						0x40
#define IRQ_MASK						0x80

/*!
 * Secure Mode FLAGs
 */

/* When When set, enables ICache within the secure mode */
#define SEC_ROM_ICACHE_ENABLE_MASK                        0x00000001

/* When When set, enables DCache within the secure mode */
#define SEC_ROM_DCACHE_ENABLE_MASK                        0x00000002

/* When When set, enables IRQ within the secure mode */
#define SEC_ROM_IRQ_ENABLE_MASK                           0x00000004

/* When set, enables FIQ within the secure mode */
#define SEC_ROM_FIQ_ENABLE_MASK                           0x00000008

/* When When set, enables Unified L2 cache within the secure mode */
#define SEC_ROM_UL2_CACHE_ENABLE_MASK                     0x00000010

/* Following are BRCM ROM Secure Service API */
#define SSAPI_DORMANT_ENTRY_SERV                          0x01000000
#define SSAPI_PUBLIC_OTP_SERV                             0x01000001
#define SSAPI_ENABLE_L2_CACHE                             0x01000002
#define SSAPI_DISABLE_L2_CACHE                            0x01000003
#define SSAPI_WRITE_SCU_STATUS                            0x01000004
#define SSAPI_WRITE_PWR_GATE                              0x01000005

#define SSAPI_ROW_AES			0x0E000006
#define SSAPI_BRCM_START_VC_CORE	0x0E000008
#define SSAPI_BRCM_SET_M4U		0x0E000012
#define SSAPI_CHIP_M_TYPE		0x0E000017

#ifdef CONFIG_MOBICORE_DRIVER
u32 mobicore_smc(u32 cmd, u32 arg1, u32 arg2, u32 arg3);
#define SMC_CMD_SLEEP		(-3)
#define SMC_CMD_L2X0CTRL	(-21)
#define SMC_CMD_L2X0SETUP1	(-22)
#define SMC_CMD_L2X0SETUP2	(-23)
#define SMC_CMD_L2X0INVALL	(-24)
#define SMC_CMD_L2X0DEBUG	(-25)

/* VC Core setup */
#define SMC_BCM_VC_CORE_START	(-301)
#endif

#ifdef CONFIG_KONA_SECURE_MONITOR_CALL
extern void secure_api_call_init(void);

extern unsigned secure_api_call(unsigned service_id, unsigned arg0,
	unsigned arg1, unsigned arg2, unsigned arg3);

extern unsigned get_secure_buffer(void);

extern unsigned get_secure_buffer_size(void);
#else
static inline void secure_api_call_init(void) { };

static inline unsigned secure_api_call(unsigned service_id, unsigned arg0,
	unsigned arg1, unsigned arg2, unsigned arg3) { return 0; };

static inline unsigned get_secure_buffer(void) { return 0; }

static inline unsigned get_secure_buffer_size(void) { return 0; }

#endif /* CONFIG_KONA_SECURE_MONITOR_CALL */

#endif /* SEC_API_H */

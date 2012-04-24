#ifndef SEC_API_H
#define SEC_API_H

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

extern unsigned secure_api_call(unsigned service_id, unsigned arg0,
	unsigned arg1, unsigned arg2);
#endif /* SEC_API_H */

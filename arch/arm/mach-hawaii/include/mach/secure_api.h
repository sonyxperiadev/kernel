/******************************************************************************/
/* (c) 2011 Broadcom Corporation                                              */
/*                                                                            */
/* Unless you and Broadcom execute a separate written software license        */
/* agreement governing use of this software, this software is licensed to you */
/* under the terms of the GNU General Public License version 2, available at  */
/* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").                    */
/*                                                                            */
/******************************************************************************/

#define SEC_FLAGS                   0xB
#define SEC_DORMANT_MODE            0x2

/*
 * Secure API identifiers.
 *
 * The ordering and number of identifiers in this list must
 * match the ones defined in smc API list in smc.c.
 */
#define SEC_API_ENTER_DORMANT       0
#define SEC_API_ENABLE_L2_CACHE     1
#define SEC_API_DISABLE_L2_CACHE    2
#define SEC_API_AES                 3
/* Set SEC_API_MAX to one more than the value of last sec API ID */
#define SEC_API_MAX                 4

#ifndef __ASSEMBLY__
u32 hw_sec_pub_dispatcher(u32 service, u32 flags, ...);
u32 hw_sec_rom_pub_bridge(u32 appl_id, u32 flags, va_list v);
int smc_init(void);
#endif

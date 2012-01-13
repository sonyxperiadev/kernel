/******************************************************************************/
/* (c) 2011 Broadcom Corporation                                              */
/*                                                                            */
/* Unless you and Broadcom execute a separate written software license        */
/* agreement governing use of this software, this software is licensed to you */
/* under the terms of the GNU General Public License version 2, available at  */
/* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").                    */
/*                                                                            */
/******************************************************************************/

#define SEC_API_ENTER_DORMANT       0x01000000
#define SEC_API_ENABLE_L2_CACHE     0x01000002
#define SEC_API_DISABLE_L2_CACHE    0x01000003
#define SEC_FLAGS                   0xB
#define SEC_DORMANT_MODE            0x2

#ifndef __ASSEMBLY__
u32 hw_sec_pub_dispatcher(u32 service, u32 flags, ...);
u32 hw_sec_rom_pub_bridge(u32 appl_id, u32 flags, va_list v);
#endif


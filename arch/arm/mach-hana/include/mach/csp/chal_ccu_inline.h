/*****************************************************************************
* Copyright 2008 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _CHAL_CCU_INLINE_H_
#define _CHAL_CCU_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/csp/reg.h>
#include <mach/csp/chipregHw_inline.h>

#include "chal_ccu_util_inline.h"

#define CLK_MGR_REG_WR_ACCESS_OFFSET                                 0x00000000
#define CLK_MGR_REG_LVM_EN_OFFSET                                    0x00000034
#define CLK_MGR_REG_POLICY_FREQ_OFFSET                               0x00000008
#define    CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK                 0x00000007
#define CLK_MGR_REG_POLICY_CTL_OFFSET                                0x0000000C
#define    CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK                        0x00000004
#define    CLK_MGR_REG_POLICY_CTL_GO_MASK                            0x00000001
#define    CLK_MGR_REG_POLICY_CTL_GO_SHIFT                           0
#define CLK_MGR_REG_POLICY0_MASK_OFFSET                              0x00000010

#define  CHAL_CCU_WR_ACCESS_MODE_MASK        0x80000000
#define  CHAL_CCU_WR_ACCESS_PASSWORD         0x00A5A500
#define  CHAL_CCU_WR_ACCESS_ENABLE           0x00000001

#define  CHAL_CCU_POLICY_CONFIG_EN_MASK      0x00000001

/* Generic policy numbers */
typedef enum
{
   chal_ccu_policy_0 = 0,
   chal_ccu_policy_1,
   chal_ccu_policy_2,
   chal_ccu_policy_3   
} chal_ccu_policy_num_e;

typedef enum
{
   /* AXI/AP1=APB2=APB3=APB_HSM frequencies */
   chal_ccu_kps_policy_freq_xtal = 0,
   chal_ccu_kps_policy_freq_52_26,
   chal_ccu_kps_policy_freq_78_39,
   chal_ccu_kps_policy_freq_104_52,
   chal_ccu_kps_policy_freq_156_52,
   chal_ccu_kps_policy_freq_156_78,
   chal_ccu_kps_policy_freq_not_supported
} chal_ccu_kps_policy_freq_e;

typedef enum
{
   chal_ccu_kps_policy_mask_switch = 0,
   chal_ccu_kps_policy_mask_ext,
   chal_ccu_kps_policy_mask_spum_open,
   chal_ccu_kps_policy_mask_spum_sec,
   chal_ccu_kps_policy_mask_crc,
   chal_ccu_kps_policy_mask_magic,
   chal_ccu_kps_policy_mask_mspro,
   chal_ccu_kps_policy_mask_timers,
   chal_ccu_kps_policy_mask_dmac_mux,
   chal_ccu_kps_policy_mask_reserved9,
   chal_ccu_kps_policy_mask_i2s,
   chal_ccu_kps_policy_mask_irda,
   chal_ccu_kps_policy_mask_reserved12,
   chal_ccu_kps_policy_mask_reserved13,
   chal_ccu_kps_policy_mask_ssp2,
   chal_ccu_kps_policy_mask_ssp1,
   chal_ccu_kps_policy_mask_ssp0,
   chal_ccu_kps_policy_mask_uartb4,
   chal_ccu_kps_policy_mask_uartb3,
   chal_ccu_kps_policy_mask_uartb2,
   chal_ccu_kps_policy_mask_uartb,
   chal_ccu_kps_policy_mask_pwm,
   chal_ccu_kps_policy_mask_auxadc,
   chal_ccu_kps_policy_mask_bsc2,
   chal_ccu_kps_policy_mask_bsc1,
   chal_ccu_kps_policy_mask_d1w,
   chal_ccu_kps_policy_mask_hdmikey,
   chal_ccu_kps_policy_mask_mphi,
   chal_ccu_kps_policy_mask_bbl,
   chal_ccu_kps_policy_mask_not_supported
} chal_ccu_kps_policy_mask_e;

#define chal_ccu_unlock_kps_clk_mgr()           chal_ccu_unlock_clk_manager( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_WR_ACCESS_OFFSET )
#define chal_ccu_lock_kps_clk_mgr()             chal_ccu_lock_clk_manager( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_WR_ACCESS_OFFSET )
#define chal_ccu_restore_kps_clk_mgr(old_en)    chal_ccu_restore_clk_manager( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_WR_ACCESS_OFFSET, old_en )
#define chal_ccu_set_kps_lvm_enable()           chal_ccu_set_lvm_enable( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_LVM_EN_OFFSET )
#define chal_ccu_wait_kps_lvm_ready()           chal_ccu_wait_lvm_ready( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_LVM_EN_OFFSET )
#define chal_ccu_is_kps_lvm_ready(timeout)      chal_ccu_is_lvm_ready( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_LVM_EN_OFFSET, timeout )

/* Functions */
static inline uint32_t chal_ccu_unlock_clk_manager( uint32_t addr )
{
   uint32_t old_en;
   uint32_t access;
   access = REG32_READ( addr );
   old_en = access & CHAL_CCU_WR_ACCESS_ENABLE;    /* save the existing ACCESS ENABLE bit */
   access &= CHAL_CCU_WR_ACCESS_MODE_MASK;         /* retaining the access mode bit       */
   access |= CHAL_CCU_WR_ACCESS_PASSWORD | CHAL_CCU_WR_ACCESS_ENABLE;
   REG32_WRITE( addr, access );
   return old_en;
}

static inline void chal_ccu_lock_clk_manager( uint32_t addr )
{
   uint32_t access;
   
   access = REG32_READ( addr );
   access &= CHAL_CCU_WR_ACCESS_MODE_MASK;   /* retaining the access mode bit */
   access |= CHAL_CCU_WR_ACCESS_PASSWORD;
   REG32_WRITE( addr, access );
}

static inline void chal_ccu_restore_clk_manager( uint32_t addr, uint32_t old_en )
{
   uint32_t access;
   
   access = REG32_READ( addr );
   access &= CHAL_CCU_WR_ACCESS_MODE_MASK;         /* retaining the access mode bit       */
   access |= CHAL_CCU_WR_ACCESS_PASSWORD;
   access |= (old_en & CHAL_CCU_WR_ACCESS_ENABLE); /* restore the given ACCESS ENABLE bit */
   REG32_WRITE( addr, access );
}

static inline void chal_ccu_set_lvm_enable( uint32_t addr )
{
   chal_ccu_set_reg_field( addr, CHAL_CCU_POLICY_CONFIG_EN_MASK, 0, CHAL_CCU_POLICY_CONFIG_EN_MASK);
}

static inline void chal_ccu_wait_lvm_ready( uint32_t addr )
{
   /* wait until all existing policy configurations are complete */
   while ( (REG32_READ(addr) & CHAL_CCU_POLICY_CONFIG_EN_MASK ) != 0 );
}

static inline uint32_t chal_ccu_is_lvm_ready( uint32_t addr, int timeout )
{
   return( chal_ccu_wait_for_mask_clear( addr, CHAL_CCU_POLICY_CONFIG_EN_MASK, timeout ) );
}

static inline void chal_ccu_set_kps_policy_freq( chal_ccu_policy_num_e policy_num, chal_ccu_kps_policy_freq_e freq )
{
   uint32_t access;
   
   /* enable access */
   access = chal_ccu_unlock_kps_clk_mgr();
   
   /* enable sw update */
   chal_ccu_set_kps_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   chal_ccu_wait_kps_lvm_ready();
   
   /* set frequency policy */
   chal_ccu_set_reg_field( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY_FREQ_OFFSET,             /* addr  */
                           CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),   /* mask  */
                           8 * policy_num,                                                      /* shift */
                           (uint32_t)freq ) ;                                                   /* value */

   /* trigger go */
   chal_ccu_set_reg_field( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY_CTL_OFFSET,
                           CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( chal_ccu_get_bit( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY_CTL_OFFSET, CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   chal_ccu_restore_kps_clk_mgr(access);
}

static inline chal_ccu_kps_policy_freq_e chal_ccu_get_kps_policy_freq( chal_ccu_policy_num_e policy_num )
{
   uint32_t access;
   uint32_t freq_id;
   
   /* enable access */
   access = chal_ccu_unlock_kps_clk_mgr();
   
   /* set frequency policy */
   freq_id = chal_ccu_get_reg_field( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY_FREQ_OFFSET,            /* addr  */
                                     CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK << (8 * policy_num),  /* mask  */
                                     8 * policy_num ) ;                                                  /* shift */
   
   /* restore access */
   chal_ccu_restore_kps_clk_mgr(access);
   
   return (chal_ccu_kps_policy_freq_e)freq_id;
}

static inline void chal_ccu_cfg_kps_policy_mask( chal_ccu_policy_num_e policy_num, chal_ccu_kps_policy_mask_e policy_type, uint32_t val )
{
   uint32_t addr;
   uint32_t access;
   
   /* enable access */
   access = chal_ccu_unlock_kps_clk_mgr();
   
   /* enable sw update */
   chal_ccu_set_kps_lvm_enable();
   
   /* wait until all existing policy configurations are complete */
   chal_ccu_wait_kps_lvm_ready();

   addr  = MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);

   chal_ccu_set_bit( addr, policy_type, val );
   
   /* trigger go */
   chal_ccu_set_reg_field( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY_CTL_OFFSET,
                           CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | CLK_MGR_REG_POLICY_CTL_GO_MASK,
                           0,
                           CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK | CLK_MGR_REG_POLICY_CTL_GO_MASK );
   
   /* wait until complete */
   while( chal_ccu_get_bit( MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY_CTL_OFFSET, CLK_MGR_REG_POLICY_CTL_GO_SHIFT ) != 0 );
   
   /* restore access */
   chal_ccu_restore_kps_clk_mgr(access);
}

static inline void chal_ccu_set_kps_policy_mask( chal_ccu_policy_num_e policy_num, chal_ccu_kps_policy_mask_e policy_type )
{
   chal_ccu_cfg_kps_policy_mask( policy_num, policy_type, 1 );
}

static inline void chal_ccu_clr_kps_policy_mask( chal_ccu_policy_num_e policy_num, chal_ccu_kps_policy_mask_e policy_type )
{
   chal_ccu_cfg_kps_policy_mask( policy_num, policy_type, 0 );
}

static inline uint32_t chal_ccu_get_kps_policy_mask( chal_ccu_policy_num_e policy_num, chal_ccu_kps_policy_mask_e policy_type )
{
   uint32_t addr;
   uint32_t bit;
   
   addr  = MM_IO_BASE_SLV_CLK + CLK_MGR_REG_POLICY0_MASK_OFFSET;
   addr += (sizeof(uint32_t) * policy_num);
   bit   = chal_ccu_get_bit( addr, policy_type );

   return bit;
}

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_CCU_INLINE_H_*/

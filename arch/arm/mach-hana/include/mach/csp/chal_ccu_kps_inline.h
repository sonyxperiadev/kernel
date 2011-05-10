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

#ifndef _CHAL_CCU_KPS_INLINE_H_
#define _CHAL_CCU_KPS_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/csp/chal_ccu_inline.h>
#include <mach/csp/chal_ccu_util_inline.h>

/* type definitions */

#define IKPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET                            0x00000408
#define IKPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET                            0x0000040C
#define    IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK               0x00000003
#define    IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK                      0x0000FFF0
#define IKPS_CLK_MGR_REG_UARTB_DIV_OFFSET                                 0x00000A10
#define IKPS_CLK_MGR_REG_UARTB2_DIV_OFFSET                                0x00000A14
#define IKPS_CLK_MGR_REG_UARTB3_DIV_OFFSET                                0x00000A18
#define IKPS_CLK_MGR_REG_UARTB4_DIV_OFFSET                                0x00000A1C
#define IKPS_CLK_MGR_REG_TIMERS_DIV_OFFSET                                0x00000A78
#define    IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK             0x00000003
#define IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET                                  0x00000AFC

typedef enum
{
	chal_ccu_kps_uart_clkgate_apb_clk_en            = 0,
	chal_ccu_kps_uart_clkgate_apb_hw_sw_gating_sel  = 1,
	chal_ccu_kps_uart_clkgate_clk_en                = 2,
	chal_ccu_kps_uart_clkgate_hw_sw_gating_sel      = 3,
	chal_ccu_kps_uart_clkgate_voltage_level         = 28

} chal_ccu_kps_uart_clkgate_e;

typedef enum
{
	chal_ccu_kps_uart_div_pll_select = 0,
	chal_ccu_kps_uart_div_div        = 4,

} chal_ccu_kps_uart_div_e;

typedef enum
{
	chal_ccu_kps_timers_div_pll_sel  = 0,

} chal_ccu_kps_timers_div_e;

typedef enum
{
	chal_ccu_kps_div_trig_uartb_trig          = 2,
	chal_ccu_kps_div_trig_uartb2_trig         = 3,
	chal_ccu_kps_div_trig_uartb3_trig         = 4,
	chal_ccu_kps_div_trig_uartb4_trig         = 5,
	chal_ccu_kps_div_trig_timers_trig         = 20,

} chal_ccu_kps_div_trig_e;

/* Local macros */
#define chal_ccu_set_kps_bit(offset, field, val)  \
            chal_ccu_set_bit( (MM_IO_BASE_SLV_CLK + (offset)), (field), (val) )

#define chal_ccu_get_kps_bit(offset, field)       \
            chal_ccu_get_bit( (MM_IO_BASE_SLV_CLK + (offset)), (field) )

#define chal_ccu_set_kps_reg_field(offset, mask, shift, val)  \
            chal_ccu_set_reg_field( (MM_IO_BASE_SLV_CLK + (offset)), (mask), (shift), (val) );

#define chal_ccu_get_kps_reg_field(offset, mask, shift)       \
            chal_ccu_get_reg_field( (MM_IO_BASE_SLV_CLK + (offset)), (mask), (shift) )

/* Functions */
static inline void chal_ccu_set_kps_uartb3_clkgate( chal_ccu_kps_uart_clkgate_e field, uint32_t val )
{
   chal_ccu_set_kps_bit( IKPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET, field, val );
}

static inline uint32_t chal_ccu_get_kps_uartb3_clkgate( chal_ccu_kps_uart_clkgate_e field )
{
   return chal_ccu_get_kps_bit( IKPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET, field );
}

static inline void chal_ccu_set_kps_uartb4_clkgate( chal_ccu_kps_uart_clkgate_e field, uint32_t val )
{
   chal_ccu_set_kps_bit( IKPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET, field, val );
}

static inline uint32_t chal_ccu_get_kps_uartb4_clkgate( chal_ccu_kps_uart_clkgate_e field )
{
   return chal_ccu_get_kps_bit( IKPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET, field );
}

static inline void chal_ccu_set_kps_uartb_div( chal_ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   chal_ccu_set_kps_reg_field( IKPS_CLK_MGR_REG_UARTB_DIV_OFFSET, mask, field, val );
}

static inline uint32_t chal_ccu_get_kps_uartb_div( chal_ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   return chal_ccu_get_kps_reg_field( IKPS_CLK_MGR_REG_UARTB_DIV_OFFSET, mask, field );
}

static inline void chal_ccu_set_kps_uartb2_div( chal_ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   chal_ccu_set_kps_reg_field( IKPS_CLK_MGR_REG_UARTB2_DIV_OFFSET, mask, field, val );
}

static inline uint32_t chal_ccu_get_kps_uartb2_div( chal_ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   return chal_ccu_get_kps_reg_field( IKPS_CLK_MGR_REG_UARTB2_DIV_OFFSET, mask, field );
}

static inline void chal_ccu_set_kps_uartb3_div( chal_ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   chal_ccu_set_kps_reg_field( IKPS_CLK_MGR_REG_UARTB3_DIV_OFFSET, mask, field, val );
}

static inline uint32_t chal_ccu_get_kps_uartb3_div( chal_ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   return chal_ccu_get_kps_reg_field( IKPS_CLK_MGR_REG_UARTB3_DIV_OFFSET, mask, field );
}

static inline void chal_ccu_set_kps_uartb4_div( chal_ccu_kps_uart_div_e field, uint32_t val )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   chal_ccu_set_kps_reg_field( IKPS_CLK_MGR_REG_UARTB4_DIV_OFFSET, mask, field, val );
}

static inline uint32_t chal_ccu_get_kps_uartb4_div( chal_ccu_kps_uart_div_e field )
{
   uint32_t mask;

   switch( (uint32_t)field )
   {
      case chal_ccu_kps_uart_div_pll_select:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK;
         break;

      case chal_ccu_kps_uart_div_div:
         mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK;
         break;

      default:
         mask = 1 << field;
         break;
   }
   return chal_ccu_get_kps_reg_field( IKPS_CLK_MGR_REG_UARTB4_DIV_OFFSET, mask, field );
}

static inline void chal_ccu_set_kps_timers_div( chal_ccu_kps_timers_div_e field, uint32_t val )
{
   uint32_t mask;

   if (field == chal_ccu_kps_timers_div_pll_sel)
   {
      mask = IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   chal_ccu_set_kps_reg_field( IKPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, mask, field, val );
}

static inline uint32_t chal_ccu_get_kps_timers_div( chal_ccu_kps_timers_div_e field )
{
   uint32_t mask;

   if (field == chal_ccu_kps_timers_div_pll_sel)
   {
      mask = IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
   }
   else
   {
      mask = 1 << field;
   }
   return chal_ccu_get_kps_reg_field( IKPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, mask, field );
}

static inline void chal_ccu_set_kps_div_trig( chal_ccu_kps_div_trig_e field, uint32_t val )
{
   chal_ccu_set_kps_bit( IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET, field, val );
}

static inline uint32_t chal_ccu_get_kps_div_trig( chal_ccu_kps_div_trig_e field )
{
   return chal_ccu_get_kps_bit( IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET, field );
}

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_CCU_KPS_INLINE_H_*/

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
/**
*
*  @file  chal_dmux.h
*
*  @brief cHAL interface for DMUX block
*
* \note
*****************************************************************************/
#ifndef _CHAL_DMUX_H_
#define _CHAL_DMUX_H_

#include <plat/chal/chal_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
* DMUX end point for peripheral
*     - based on Kona Addr 062409 "Hera Interrupts & DMA"
******************************************************************************/
typedef enum {
    CHAL_DMUX_EPT_INVALID      = 0x7F,
    CHAL_DMUX_EPT_MPHI         = 5,
    CHAL_DMUX_EPT_PWM_0        = 6,
    CHAL_DMUX_EPT_PWM_1        = 7,
    CHAL_DMUX_EPT_UARTB0_A     = 8,
    CHAL_DMUX_EPT_UARTB0_B     = 9,
    CHAL_DMUX_EPT_UARTB1_A     = 10,
    CHAL_DMUX_EPT_UARTB1_B     = 11,
    CHAL_DMUX_EPT_UARTB2_A     = 12,
    CHAL_DMUX_EPT_UARTB2_B     = 13,
    CHAL_DMUX_EPT_UARTB3_A     = 14,
    CHAL_DMUX_EPT_UARTB3_B     = 15,
    CHAL_DMUX_EPT_SSP_0A_RX0   = 16,
    CHAL_DMUX_EPT_SSP_0B_TX0   = 17,
    CHAL_DMUX_EPT_SSP_0C_RX1   = 18,
    CHAL_DMUX_EPT_SSP_0D_TX1   = 19,
    CHAL_DMUX_EPT_SSP_1A_RX0   = 20,
    CHAL_DMUX_EPT_SSP_1B_TX0   = 21,
    CHAL_DMUX_EPT_SSP_1C_RX1   = 22,
    CHAL_DMUX_EPT_SSP_1D_TX1   = 23,
    CHAL_DMUX_EPT_SSP_2A_RX0   = 24,
    CHAL_DMUX_EPT_SSP_2B_TX0   = 25,
    CHAL_DMUX_EPT_SSP_2C_RX1   = 26,
    CHAL_DMUX_EPT_SSP_2D_TX1   = 27,    
    /* Capri BD 03-28-11. 
     * Note: UARTB0 here is UARTB, 
     *       UARTB1 here is UARTB2,
     *       UARTB2 here is UARTB3,
     *       UARTB3 here is UARTB4,
     *       UARTB4 here is UARTB5,
     *       UARTB5 here is UARTB6 in Capri */
    CHAL_DMUX_EPT_UARTB4_A     = 28,  
    CHAL_DMUX_EPT_UARTB4_B     = 29,
    CHAL_DMUX_EPT_UARTB5_A     = 30,
    CHAL_DMUX_EPT_UARTB5_B     = 31,
    CHAL_DMUX_EPT_HSIA         = 32,
    CHAL_DMUX_EPT_HSIB         = 33,
    CHAL_DMUX_EPT_HSIC         = 34,
    CHAL_DMUX_EPT_HSID         = 35,
    CHAL_DMUX_EPT_EANC         = 40,
    CHAL_DMUX_EPT_STEREO       = 41,
    CHAL_DMUX_EPT_NVIN         = 42,
    CHAL_DMUX_EPT_VIN          = 43,
    CHAL_DMUX_EPT_VIBRA        = 44,
    CHAL_DMUX_EPT_IHF_0        = 45,
    CHAL_DMUX_EPT_VOUT         = 46,
    CHAL_DMUX_EPT_SLIMA        = 47,
    CHAL_DMUX_EPT_SLIMB        = 48,
    CHAL_DMUX_EPT_SLIMC        = 49,
    CHAL_DMUX_EPT_SLIMD        = 50,
    CHAL_DMUX_EPT_SIM_A        = 51,
    CHAL_DMUX_EPT_SIM_B        = 52,
    CHAL_DMUX_EPT_SIM2_A       = 53,
    CHAL_DMUX_EPT_SIM2_B       = 54,
    CHAL_DMUX_EPT_IHF_1        = 55,
    CHAL_DMUX_EPT_SSP_3A_RX0   = 56,
    CHAL_DMUX_EPT_SSP_3B_TX0   = 57,
    CHAL_DMUX_EPT_SSP_3C_RX1   = 58,
    CHAL_DMUX_EPT_SSP_3D_TX1   = 59, 
    CHAL_DMUX_EPT_CIR_RX       = 62,
    CHAL_DMUX_EPT_CIR_TX       = 63,
    CHAL_DMUX_EPT_PWM_2        = 64,
    CHAL_DMUX_EPT_SPUM_SecureA = 65,
    CHAL_DMUX_EPT_SPUM_SecureB = 66,
    CHAL_DMUX_EPT_SPUM_OpenA   = 67,
    CHAL_DMUX_EPT_SPUM_OpenB   = 68, 
    CHAL_DMUX_EPT_I2SA         = 71,
    CHAL_DMUX_EPT_I2SB         = 72,
    CHAL_DMUX_EPT_PWM_3        = 73,
    CHAL_DMUX_EPT_PWM_4        = 74,
    CHAL_DMUX_EPT_PWM_5        = 75,
    CHAL_DMUX_EPT_SSP_4A_RX0   = 76,
    CHAL_DMUX_EPT_SSP_4B_TX0   = 77,
    CHAL_DMUX_EPT_SSP_4C_RX1   = 78,
    CHAL_DMUX_EPT_SSP_4D_TX1   = 79,
    CHAL_DMUX_EPT_SSP_5A_RX0   = 80,
    CHAL_DMUX_EPT_SSP_5B_TX0   = 81,
    CHAL_DMUX_EPT_SSP_5C_RX1   = 82,
    CHAL_DMUX_EPT_SSP_5D_TX1   = 83,
    CHAL_DMUX_EPT_SSP_6A_RX0   = 84,
    CHAL_DMUX_EPT_SSP_6B_TX0   = 85,
    CHAL_DMUX_EPT_SSP_6C_RX1   = 86,
    CHAL_DMUX_EPT_SSP_6D_TX1   = 87
} CHAL_DMUX_END_POINT_t;

/**
* DMUX status
*****************************************************************************/
typedef enum{
    CHAL_DMUX_STATUS_SUCCESS,       /*< DMUX sucess */
    CHAL_DMUX_STATUS_FAILURE        /*< DMUX failure */
} CHAL_DMUX_STATUS_t;


/**
*
*  @brief  Initialize CHAL DMUX for the passed DMUX instance
*
*  @param  baseAddr (in) mapped address of DMUX block
*
*  @return handle of this DMUX instance
*
*  @note
*****************************************************************************/
CHAL_HANDLE chal_dmux_init( uint32_t baseAddr );

/**
*
*  @brief  De-Initialize CHAL DMUX for the passed DMUX instance
*
*  @param  handle (in) handle of this DMUX instance
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_deinit ( CHAL_HANDLE handle );

/**
*
*  @brief  DMUX allocate channel
*
*  @param  handle (in) handle of this DMUX instance
*  @param  chan (in) buffer to store allocated channel
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_alloc_channel ( CHAL_HANDLE handle, uint32_t *chan );

/**
*
*  @brief  DMUX de-allocate channel
*
*  @param  handle (in) handle of this DMUX instance
*  @param  channel (in) channel to be de-allocated
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_dealloc_channel (
    CHAL_HANDLE handle,
    uint32_t channel
);

/**
*
*  @brief  DMUX allocate peripheral
*
*  @param  handle (in) handle of this DMUX instance
*  @param  channel (in) channel to be associated with peripharals
*  @param  peri_a (in) the first peripheral to be allocated
*  @param  peri_b (in) the second peripheral to be allocated
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_alloc_peripheral (
    CHAL_HANDLE handle,
    uint32_t channel,
    CHAL_DMUX_END_POINT_t peri_a,
    CHAL_DMUX_END_POINT_t peri_b,
    uint8_t *src_id,
    uint8_t *dst_id
);

/**
*
*  @brief  DMUX de-allocate peripheral
*
*  @param  handle (in) handle of this DMUX instance
*  @param  channel (in) channel associated with peripheral to be de-allocated
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_dealloc_peripheral (
    CHAL_HANDLE handle,
    uint32_t     channel
);

/**
*
*  @brief  DMUX protect
*
*  @param  handle (in) handle of this DMUX instance
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_protect( CHAL_HANDLE handle );

/**
*
*  @brief  DMUX unprotect
*
*  @param  handle (in) handle of this DMUX instance
*
*  @return DMUX status
*
*  @note
*****************************************************************************/
CHAL_DMUX_STATUS_t chal_dmux_unprotect( CHAL_HANDLE handle );



/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_DMUX_H_ */


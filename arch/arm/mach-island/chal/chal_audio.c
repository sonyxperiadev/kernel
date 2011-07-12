/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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

/*****************************************************************************
*
*    (c) 2001-2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*  @file   chal_audio.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <chal/chal_audio.h>
#include <chal/chal_audio_int.h>

//#include <mach/csp/chal_reg.h>
//#include <mach/csp/mm_io.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_util.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_auxmic.h>

/*
 * ****************************************************************************
 *                         G L O B A L   S E C T I O N
 * ****************************************************************************
 */

/*
 * ****************************************************************************
 *  global variable definitions
 * ****************************************************************************
 */

/*
 * ****************************************************************************
 *                          L O C A L   S E C T I O N
 * ****************************************************************************
 */

/*
 * ****************************************************************************
 *  local macro declarations
 * ****************************************************************************
 */

#define AUDIOH_HW_DACPWRUP_SETTLE_TIME              400         /* Milli Seconds */
#define AUDIOH_HW_SLOWRAMP_PWRDN_PULSE_TIME         150         /* Micro Seconds */
#define AUDIOH_HW_SLOWRAMP_RAMP1UP_TIME             40          /* Milli Seconds */
#define AUDIOH_HW_SLOWRAMP_RAMP2UP_TIME             5           /* Milli Seconds */
#define AUDIOH_HW_SLOWRAMP_RAMP2DOWN_TIME           35          /* Milli Seconds */
#define AUDIOH_HW_SLOWRAMP_RAMP1DOWN_TIME           10          /* Milli Seconds */
#define AUDIOH_HW_PATHENDIS_SETTLING_TIME           1           /* Milli Seconds */

/*
 * ****************************************************************************
 *  local typedef declarations
 * ****************************************************************************
 */



/*
 * ****************************************************************************
 *  local variable definitions
 * ****************************************************************************
 */


/*
 * ****************************************************************************
 *  local function declarations
 * ****************************************************************************
 */



/*
 * ******************************************************************************
 *  local function definitions
 * ******************************************************************************
 */




/*
 * ============================================================================
 * 
 *  Function Name: CHAL_HANDLE chal_audio_init(uint32_t baseAddr)
 * 
 *  Description:   Standard Init entry point for cHal
 *                 first function to call.
 * 
 *  Parameters:
 *                 audioh_base,    mapped address of Hera audio register files
 *                 sdt_base,         mapped address of Hera sidetone register files
 * 
 *  Return:
 *              CHAL handle for Hera audio
 * 
 * ============================================================================
 */

void chal_audio_init(CHAL_HANDLE handle, uint32_t audioh_base, uint32_t sdt_base)
{
    ChalAudioCtrlBlk_t  *pCHALCb = (ChalAudioCtrlBlk_t*)handle;

    pCHALCb->audioh_base = audioh_base;
    pCHALCb->sdt_base = sdt_base;
}

void chal_audio_enable_aci_auxmic( CHAL_HANDLE handle, int enable )
{
    uint32_t regVal;
    (void)handle;

    if( enable )
    {
       /* Set ACI */
       regVal = CHAL_REG_READ32((ACI_BASE_ADDR+ACI_ADC_CTRL_OFFSET));
       regVal |= ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK;
       regVal |= ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
       CHAL_REG_WRITE32((ACI_BASE_ADDR+ACI_ADC_CTRL_OFFSET), regVal);

       /* enable AUXMIC */
       regVal = CHAL_REG_READ32((AUXMIC_BASE_ADDR+AUXMIC_AUXEN_OFFSET));
       regVal |= AUXMIC_AUXEN_MICAUX_EN_MASK;
       CHAL_REG_WRITE32((AUXMIC_BASE_ADDR+AUXMIC_AUXEN_OFFSET), regVal);

       /* Set probe cycle to continuous */
       regVal = CHAL_REG_READ32((AUXMIC_BASE_ADDR+AUXMIC_CMC_OFFSET));
       regVal |= AUXMIC_CMC_CONT_MSR_CTRL_MASK;
       CHAL_REG_WRITE32((AUXMIC_BASE_ADDR+AUXMIC_CMC_OFFSET), regVal);

       /* disable AUXMIC force power down CHAL_REG_WRITE32(0x3500E028, */
       regVal = CHAL_REG_READ32((AUXMIC_BASE_ADDR+AUXMIC_F_PWRDWN_OFFSET));
       regVal &= ~AUXMIC_F_PWRDWN_FORCE_PWR_DWN_MASK;
       CHAL_REG_WRITE32((AUXMIC_BASE_ADDR+AUXMIC_F_PWRDWN_OFFSET), regVal);
    }
    else
    {
       /* Enable AUXMIC force power down */
       regVal = CHAL_REG_READ32((AUXMIC_BASE_ADDR+AUXMIC_F_PWRDWN_OFFSET));
       regVal |= AUXMIC_F_PWRDWN_FORCE_PWR_DWN_MASK;
       CHAL_REG_WRITE32((AUXMIC_BASE_ADDR+AUXMIC_F_PWRDWN_OFFSET), regVal);

       /* Set probe cycle to discontinuous */
       regVal = CHAL_REG_READ32((AUXMIC_BASE_ADDR+AUXMIC_CMC_OFFSET));
       regVal &= ~AUXMIC_CMC_CONT_MSR_CTRL_MASK;
       CHAL_REG_WRITE32((AUXMIC_BASE_ADDR+AUXMIC_CMC_OFFSET), regVal);

       /* Disable AUXMIC */
       regVal = CHAL_REG_READ32((AUXMIC_BASE_ADDR+AUXMIC_AUXEN_OFFSET));
       regVal &= ~AUXMIC_AUXEN_MICAUX_EN_MASK;
       CHAL_REG_WRITE32((AUXMIC_BASE_ADDR+AUXMIC_AUXEN_OFFSET), regVal);

       /* Analog MIC */
       regVal = CHAL_REG_READ32((ACI_BASE_ADDR+ACI_ADC_CTRL_OFFSET));
       regVal &= ~ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK;
       regVal &= ~ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
       CHAL_REG_WRITE32((ACI_BASE_ADDR+ACI_ADC_CTRL_OFFSET), regVal);
    }
}

/*
 * ============================================================================
 * 
 *  Function Name: CHAL_HANDLE chal_audio_deinit(CHAL_HANDLE handle)
 * 
 *  Description:   Deinit Hera audio
 * 
 *  Parameters:
 *                 handle,    the Hera audio handle
 * 
 *  Return:
 *              none
 * 
 * ============================================================================
 */
void chal_audio_deinit(CHAL_HANDLE handle)
{
    ChalAudioCtrlBlk_t  *pCHALCb = (ChalAudioCtrlBlk_t*)handle;

    pCHALCb->audioh_base  = 0;
    pCHALCb->sdt_base  = 0;

    return;
}

uint32_t chal_audio_read_int_status(CHAL_HANDLE handle)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

   /*Read Interrupt status of all FIFO paths*/
   return BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);
}

void chal_audio_int_clear(CHAL_HANDLE handle, uint32_t reg_val)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);
}

void chal_audio_mic_sync( CHAL_HANDLE handle, _Bool on_off )
{

    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    /* Read and make room for change */
    uint32_t val = ( BRCM_READ_REG(base, AUDIOH_ADCPATH_GLOBAL_CTRL) & ~ AUDIOH_ADCPATH_GLOBAL_CTRL_GLOABAL_EN_MASK );

    /* Set the required setting */
    BRCM_WRITE_REG ( base,  AUDIOH_ADCPATH_GLOBAL_CTRL, val | (on_off << AUDIOH_ADCPATH_GLOBAL_CTRL_GLOABAL_EN_SHIFT) );
}


void chal_audio_digi_mic_all( CHAL_HANDLE handle, uint32_t mode )
{

    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);

    if (mode == CHAL_AUDIO_ENABLE)
    {
       reg_val |= (AUDIOH_ADC_CTL_DMIC1_EN_MASK | AUDIOH_ADC_CTL_DMIC2_EN_MASK | AUDIOH_ADC_CTL_DMIC3_EN_MASK |AUDIOH_ADC_CTL_DMIC4_EN_MASK  );
    }
    else
    {
       reg_val &= ~(AUDIOH_ADC_CTL_DMIC1_EN_MASK | AUDIOH_ADC_CTL_DMIC2_EN_MASK | AUDIOH_ADC_CTL_DMIC3_EN_MASK |AUDIOH_ADC_CTL_DMIC4_EN_MASK );
    }

    /* Set the required setting */
    BRCM_WRITE_REG ( base,  AUDIOH_ADC_CTL, reg_val);
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_audio_init);
EXPORT_SYMBOL(chal_audio_enable_aci_auxmic);
EXPORT_SYMBOL(chal_audio_deinit);
EXPORT_SYMBOL(chal_audio_read_int_status);
EXPORT_SYMBOL(chal_audio_int_clear);
EXPORT_SYMBOL(chal_audio_mic_sync);
EXPORT_SYMBOL(chal_audio_digi_mic_all);

#endif

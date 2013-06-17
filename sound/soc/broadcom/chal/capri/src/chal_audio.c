/*
 * Copyright 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
 /**
*
*  @file   chal_audio.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/
#include "chal_caph_audioh.h"
#include "chal_audio_int.h"
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_auxmic.h>
#include <chal/chal_util.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb-fixups.h>


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
static ChalAudioCtrlBlk_t  cHALAudioCB = {0,0,0,0};

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
 *  Function Name: CHAL_HANDLE chal_audio_init(cUInt32 audioh_base, cUInt32 sdt_base)
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
CHAL_HANDLE chal_audio_init(cUInt32 audioh_base, cUInt32	sdt_base)
{
	cHALAudioCB.audioh_base = audioh_base;
	cHALAudioCB.sdt_base = sdt_base;

	return (CHAL_HANDLE)(&cHALAudioCB);
}


/* not being called currently. Should be called after chal_audio_init() function for the address to get initialized*/
void chal_aci_auxmic_init(CHAL_HANDLE handle, cUInt32 aci_base, cUInt32 auxmic_base)
{
    ChalAudioCtrlBlk_t  *pCHALCb = (ChalAudioCtrlBlk_t*)handle;

    pCHALCb->aci_base = aci_base;
    pCHALCb->auxmic_base = auxmic_base;
}



void chal_audio_enable_aci_auxmic( CHAL_HANDLE handle, int enable )
{
    cUInt32 regVal;
    ChalAudioCtrlBlk_t  *pCHALCb = (ChalAudioCtrlBlk_t*)handle;

    if( enable )
    {
       /* Set ACI */
       regVal = CHAL_REG_READ32((pCHALCb->aci_base+ACI_ADC_CTRL_OFFSET));
       regVal |= ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK;
       regVal |= ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
       CHAL_REG_WRITE32((pCHALCb->aci_base+ACI_ADC_CTRL_OFFSET), regVal);

       /* enable AUXMIC */
       regVal = CHAL_REG_READ32((pCHALCb->auxmic_base+AUXMIC_AUXEN_OFFSET));
       regVal |= AUXMIC_AUXEN_MICAUX_EN_MASK;
       CHAL_REG_WRITE32((pCHALCb->auxmic_base+AUXMIC_AUXEN_OFFSET), regVal);

       /* Set probe cycle to continuous */
       regVal = CHAL_REG_READ32((pCHALCb->auxmic_base+AUXMIC_CMC_OFFSET));
       regVal |= AUXMIC_CMC_CONT_MSR_CTRL_MASK;
       CHAL_REG_WRITE32((pCHALCb->auxmic_base+AUXMIC_CMC_OFFSET), regVal);

       /* disable AUXMIC force power down CHAL_REG_WRITE32(0x3500E028, */
       regVal = CHAL_REG_READ32((pCHALCb->auxmic_base+AUXMIC_F_PWRDWN_OFFSET));
       regVal &= ~AUXMIC_F_PWRDWN_FORCE_PWR_DWN_MASK;
       CHAL_REG_WRITE32((pCHALCb->auxmic_base+AUXMIC_F_PWRDWN_OFFSET), regVal);
    }
    else
    {
       /* Enable AUXMIC force power down */
       regVal = CHAL_REG_READ32((pCHALCb->auxmic_base+AUXMIC_F_PWRDWN_OFFSET));
       regVal |= AUXMIC_F_PWRDWN_FORCE_PWR_DWN_MASK;
       CHAL_REG_WRITE32((pCHALCb->auxmic_base+AUXMIC_F_PWRDWN_OFFSET), regVal);

       /* Set probe cycle to discontinuous */
       regVal = CHAL_REG_READ32((pCHALCb->auxmic_base+AUXMIC_CMC_OFFSET));
       regVal &= ~AUXMIC_CMC_CONT_MSR_CTRL_MASK;
       CHAL_REG_WRITE32((pCHALCb->auxmic_base+AUXMIC_CMC_OFFSET), regVal);

       /* Disable AUXMIC */
       regVal = CHAL_REG_READ32((pCHALCb->auxmic_base+AUXMIC_AUXEN_OFFSET));
       regVal &= ~AUXMIC_AUXEN_MICAUX_EN_MASK;
       CHAL_REG_WRITE32((pCHALCb->auxmic_base+AUXMIC_AUXEN_OFFSET), regVal);

       /* Analog MIC */
       regVal = CHAL_REG_READ32((pCHALCb->aci_base+ACI_ADC_CTRL_OFFSET));
       regVal &= ~ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK;
       regVal &= ~ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
       CHAL_REG_WRITE32((pCHALCb->aci_base+ACI_ADC_CTRL_OFFSET), regVal);
    }
}

void chal_audio_set_aci_auxmic_datab(CHAL_HANDLE handle,
					_Bool micOutEnable)
{
	cUInt32 regVal;
	ChalAudioCtrlBlk_t  *pCHALCb = (ChalAudioCtrlBlk_t *)handle;
	regVal = CHAL_REG_READ32((pCHALCb->aci_base+ACI_ACI_CTRL_OFFSET));
	if (micOutEnable)
		regVal |= ACI_ACI_CTRL_SW_MIC_DATAB_MASK;
	else
		regVal &= ~ACI_ACI_CTRL_SW_MIC_DATAB_MASK;
	CHAL_REG_WRITE32((pCHALCb->aci_base+ACI_ACI_CTRL_OFFSET),
					regVal);
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

cUInt32 chal_audio_read_int_status(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

   /*Read Interrupt status of all FIFO paths*/
   return BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);
}

void chal_audio_int_clear(CHAL_HANDLE handle, cUInt32 reg_val)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);
}
//function renamed from chal_audio_mic_sync
void chal_audio_adcpath_global_enable( CHAL_HANDLE handle, _Bool on_off )
{

    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    /* Read and make room for change */
    cUInt32 val = ( BRCM_READ_REG(base, AUDIOH_ADCPATH_GLOBAL_CTRL) & ~ AUDIOH_ADCPATH_GLOBAL_CTRL_GLOABAL_EN_MASK );

    /* Set the required setting */
    BRCM_WRITE_REG ( base,  AUDIOH_ADCPATH_GLOBAL_CTRL, val | (on_off << AUDIOH_ADCPATH_GLOBAL_CTRL_GLOABAL_EN_SHIFT) );
}

//added newly for building CAPH
_Bool chal_audio_adcpath_global_enable_status(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADCPATH_GLOBAL_CTRL);

	if (reg_val & AUDIOH_ADCPATH_GLOBAL_CTRL_GLOABAL_EN_MASK)
    	return TRUE;
	else
		return FALSE;

}

void chal_audio_adcpath_fifo_global_clear( CHAL_HANDLE handle,  _Bool clear )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADCPATH_GLOBAL_CTRL);

    if(clear)
    {
       reg_val |= AUDIOH_ADCPATH_GLOBAL_CTRL_FIFO_GLOBAL_CLEAR_MASK;
    }
    else
    {
       reg_val &= ~AUDIOH_ADCPATH_GLOBAL_CTRL_FIFO_GLOBAL_CLEAR_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADCPATH_GLOBAL_CTRL, reg_val);

    return;

}

void chal_audio_digi_mic_all( CHAL_HANDLE handle, cUInt32 mode )
{

    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

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
EXPORT_SYMBOL(chal_aci_auxmic_init);
EXPORT_SYMBOL(chal_audio_enable_aci_auxmic);
EXPORT_SYMBOL(chal_audio_set_aci_auxmic_datab);
EXPORT_SYMBOL(chal_audio_deinit);
EXPORT_SYMBOL(chal_audio_read_int_status);
EXPORT_SYMBOL(chal_audio_int_clear);
EXPORT_SYMBOL(chal_audio_adcpath_global_enable);
EXPORT_SYMBOL(chal_audio_digi_mic_all);
EXPORT_SYMBOL(chal_audio_adcpath_global_enable_status);
EXPORT_SYMBOL(chal_audio_adcpath_fifo_global_clear);

#endif

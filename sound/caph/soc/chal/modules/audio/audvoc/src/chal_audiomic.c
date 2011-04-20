/*******************************************************************************************
Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
* @file   chal_audiomic.c
* @brief  Audio cHAL layer of microphone
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "audio_consts.h"
#include "brcm_rdb_sysmap.h"
#include "chal_types.h"
#include "brcm_rdb_auxmic.h"
#include "brcm_rdb_syscfg.h"
#include "brcm_rdb_dsp_audio.h"
#include "brcm_rdb_util.h"
#include "chal_audiomisc.h"
#include "chal_audiomic.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    cUInt32		syscfg_base;         // may be mapped
	cUInt32		auxmic_base;         // may be mapped
	cUInt32			audio_base;         // may be mapped
	UInt32		inited;
} chal_audio_mic_t;

// rely on zero inited
static chal_audio_mic_t sMic = {SYSCFG_BASE_ADDR, AUXMIC_BASE_ADDR, AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiomic_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiomic_Init(
				cUInt32 syscfg_baseAddr,
				cUInt32 auxmic_baseAddr,
				cUInt32 audio_baseAddr
				)
{
	if (sMic.inited == 0) 
    {
        sMic.syscfg_base = syscfg_baseAddr;
		sMic.auxmic_base = auxmic_baseAddr;
		sMic.audio_base = audio_baseAddr;
		sMic.inited = 1;
        return (CHAL_HANDLE)&sMic;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audiomic_SelectInput
//
// Description:   Select Audio RX Microphone Input
//
//============================================================================
cVoid chal_audiomic_SelectInput(
		CHAL_HANDLE handle, 
		UInt16 mic_input
		)
{
	BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR3, I_PGA_MUX_SEL, mic_input);
}

//============================================================================
//
// Function Name: chal_audiomic_SetPGA
//
// Description:   Set Mic PGA
//
//============================================================================
cVoid chal_audiomic_SetPGA(
		CHAL_HANDLE handle,
		cInt32		gain
		)
{
	/**
	000000		 0dB			0.6026
	000011		 3dB			0.8482
	000110		 6dB			1.2053
	001001		 9dB			1.6964
	001100		 12dB		   2.4107
	001111		 15dB		   3.3928
	010010		 18dB		   4.8214
	010101		 21dB		   6.80
	010110		 22dB		   7.60
	010111		 23dB		   8.40
	011000		 24dB		   9.40
	011001		 25dB		   10.60
	011010		 26dB		   12.00
	011011		 27dB		   13.40
	011100		 28dB		   15.00
	011101		 29dB		   16.80
	011110		 30dB		   19.00
	011111 :	 31dB			21.20
	100000 :	 32dB			24.00
	100001 :	 33dB			27.20
	100010 :	 34dB			30.40
	100011 :	 35dB			33.60
	100100 :	 36dB			37.60
	100101 :	 37dB			42.40
	100110 :	 38dB			48.00
	100111 :	 39dB			53.60
	101000 :	 40dB			60.00
	101001 :	 41dB			67.20
	111111-101010 : 	42dB		   76.00
	*/

	if( gain < 0 || gain > 4200 )
		return; //error

	BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR3, I_PGA_GAIN, gain/100 );
}

//============================================================================
//
// Function Name: chal_audiomic_SetPGAHex
//
// Description:   Set Mic PGA in hex register value.
//
//============================================================================
cVoid chal_audiomic_SetPGAHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR3, I_PGA_GAIN, gain_hex );
}

//============================================================================
//
// Function Name: chal_audiomic_PowerOnADC
//
// Description:   Power up Mic ADC
//
//============================================================================
cVoid chal_audiomic_PowerOnADC(
		CHAL_HANDLE handle,
		Boolean		on
		)
{
	UInt32 reg_val ;

//Note: when ANACR1[7]==0, AMCR[5] can overwrite this ANACR2 in HW.
//Note: when ANACR1[7]==1, AMCR[5] does not overwrite this ANACR2 in HW. Digital and analog are separate.
	if (on)  //Normal Operation
	{
		//BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, Pga_adc_standby_pwrup, 0);
		//BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_MIC_VOICE_PWRDN, 0);
		//BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_LDO_PWRDN, 0);

		reg_val = *( (volatile SYSCFG_ANACR2_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR2_OFFSET) );
		reg_val &= ~(0x000F);
		reg_val |= (0x0004);
		*(volatile SYSCFG_ANACR2_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR2_OFFSET) = reg_val;
	}
	else  //Power Down
	{
		//BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, Pga_adc_standby_pwrup, 0);
		//BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_MIC_VOICE_PWRDN, 1);
		//BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_LDO_PWRDN, 1);

		reg_val = *( (volatile SYSCFG_ANACR2_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR2_OFFSET) );
		reg_val &= ~(0x000F);
		//reg_val |= (0x0003);
        reg_val |= (0x0002);  //Kishore - change
		*(volatile SYSCFG_ANACR2_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR2_OFFSET) = reg_val;
	}
}

//============================================================================
//
// Function Name: chal_audiomic_SetADCStandby
//
// Description:   Set sMic adc to standby
//
//============================================================================
cVoid chal_audiomic_SetADCStandby(
		CHAL_HANDLE handle,
		Boolean		standby
		)
{
	if (FALSE==standby)  //Normal Operation
	{
		BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_PGA_ADC_STANDBY, 0);
	}
	else  // ADC in Standby
	{
		BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_PGA_ADC_STANDBY, 1);
	}
}

//============================================================================
//
// Function Name: chal_audiomic_PwrdnLDO
//
// Description:   Audio RX LDO Power Down control
//
//============================================================================
cVoid chal_audiomic_PwrdnLDO(
		CHAL_HANDLE handle,
		Boolean		pwrdn
		)
{
	if (FALSE==pwrdn)  //Normal Operation
	{
		BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_LDO_PWRDN, 0);
	}
	else  //Power Down
	{
		BRCM_WRITE_REG_FIELD(sMic.syscfg_base, SYSCFG_ANACR2, I_LDO_PWRDN, 1);
	}
}

//============================================================================
//
// Function Name: chal_audiomic_SetAuxBiasLevelHigh
//
// Description:   Mic bias level control
//
//============================================================================
cVoid chal_audiomic_SetAuxBiasLevelHigh(
		CHAL_HANDLE handle,
		Boolean		high
		)
{
	BRCM_WRITE_REG_FIELD(sMic.auxmic_base, AUXMIC_AUXEN, MICAUX_EN, high);
}

//============================================================================
//
// Function Name: chal_audiomic_SetAuxBiasContinuous
//
// Description:   Mic bias voltage control
//
//============================================================================
cVoid chal_audiomic_SetAuxBiasContinuous(
		CHAL_HANDLE handle, 
		Boolean		continuous
		)
{
	BRCM_WRITE_REG_FIELD(sMic.auxmic_base, AUXMIC_CMC , CONT_MSR_CTRL, continuous );
}

//============================================================================
//
// Function Name: chal_audiomic_EnableDmic
//
// Description:   Enable DMIC path (stereo)
//
//============================================================================
cVoid chal_audiomic_EnableDmic(
		CHAL_HANDLE	handle,
		Boolean		on
		)
{
/**
The ASIC pin name is DIGMICCLK. The default output from this pin is DigMicClk (when 0x0888_0000[24] = ‘0’).
When 0x0888_0000[24] =’1’ the output from DIGMICCLK is GPIO62.

The power to the digital Mic is not supplied by the ASIC so it will not get affected by setting ASIC registers.

When 0x0888_0000[24] = ‘0’ you should get the MIC clk if you set 0x3080_0a80[5] = ‘1’ and
0x3080_0f58[6] = ‘1’.
*/
		if (on) 
		{
/**
Yes you need to also set:

0x3080_0a80[5] = ‘1’
0x3080_0f58[6] = ‘1’
*/
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_AMCR_R, AUDEN, 1 );
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, DIGIMIC_EN, 1 );

//this is for voice in path
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, RXANA2_EN, 1 ); //always enable DMIC2 path
		}
		else
		{
			//leave AMCR.AUDEN for audio driver code to coordinate and control.
			//BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_AMCR_R, AUDEN, 0 ); 
			
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, DIGIMIC_EN, 0 );

//this is for voice in path
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, RXANA2_EN, 0 );
		}
}

//============================================================================
//
// Function Name: chal_audiomic_UseDmicClock3M
//
// Description:   Set DMIC clock to 3.25MHz
//
//============================================================================
cVoid chal_audiomic_UseDmicClock3M(
		CHAL_HANDLE	handle,
		Boolean		use
		)
{
		if (use) // use 3.25MHz clock
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, DIGMIC_CLK_SEL, 0 );
		else //use 1.625MHz clock
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, DIGMIC_CLK_SEL, 1 );

}

//============================================================================
//
// Function Name: chal_audiomic_SetDmicPhase
//
// Description:   DMIC phase select
//
//============================================================================
cVoid chal_audiomic_SetDmicPhase(
		CHAL_HANDLE	handle,
		Boolean		pos
		)
{
		if (pos) // L+ R-
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, DIGIMIC_PH_SEL, 1 ); //match to Ray board DMIC
		else // R+ L-
			BRCM_WRITE_REG_FIELD( sMic.audio_base, DSP_AUDIO_ADCCONTROL_R, DIGIMIC_PH_SEL, 0 );			
}	



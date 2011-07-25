/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*  @file   csl_caph_audioh.c
*
*  @brief  cls layer driver for audioH device driver
*
****************************************************************************/

#include "mobcom_types.h"
#include "ostask.h"
#include "xassert.h"
#include "chal_types.h"
#include "chal_caph.h"
#include "chal_caph_audioh.h"
#include "auddrv_def.h"
#include "csl_caph.h"
#include "csl_caph_audioh.h"
#include "csl_caph_gain.h"
#include "log.h"

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************
CHAL_HANDLE lp_handle = 0x0;
extern const unsigned int eancAIIRFilterCoeff[];
extern const unsigned int eancAFIRFilterCoeff[];
extern const unsigned int eanc96kIIRFilterCoeff[];
extern const unsigned int eanc48kIIRFilterCoeff[];
extern const unsigned int stoneFirCoeff[];
//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// local macro declarations
//****************************************************************************


//****************************************************************************
// local typedef declarations
//****************************************************************************


//****************************************************************************
// local variable definitions
//****************************************************************************
static CSL_CAPH_AUDIOH_Path_t path[AUDDRV_PATH_TOTAL];
static CHAL_HANDLE handle = 0x0;

//Microphone status:
//Bit 0: DMIC1
//Bit 1: DMIC2
//Bit 2: DMIC3
//Bit 3: DMIC4
//Bit 4-7: Reserved
static UInt8 micStatus = 0x0;


//****************************************************************************
// local function declarations
//****************************************************************************



//******************************************************************************
// local function definitions
//******************************************************************************


//============================================================================
//
// Function Name: void csl_caph_audioh_init(UInt32 baseAddr, UInt32 sdtBaseAddr)
//
// Description:  Initialize audio path on CSL layer
//
// Parameters:   baseAddr : The base address of the audioH registers.
//				 
// Return:      
//
//============================================================================

void csl_caph_audioh_init (UInt32 baseAddr, UInt32 sdtBaseAddr)
{

	handle = (CHAL_HANDLE)chal_audio_init(baseAddr, sdtBaseAddr);
    lp_handle = handle;
//	chal_audio_earpath_clr_fifo(handle);	
	return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_deinit(void)
//
// Description:  Deinitialize audio path on CSL layer
//
// Parameters: 
//				 
// Return:       
//
//============================================================================

void csl_caph_audioh_deinit(void)
{
	chal_audio_deinit(handle);
	return;
}


//============================================================================
//
// Function Name: void csl_caph_audioh_unconfig(int path_id)
//
// Description:  Clean the table for the audio path on CSL layer
//
// Parameters:   path_id : audio path
//				 
// Return:      
//
//============================================================================

void csl_caph_audioh_unconfig(int path_id)
{
	path[path_id].sample_rate = 0;
	path[path_id].sample_size = 0;
	path[path_id].sample_pack = 0;
	path[path_id].sample_mode = 0;
	path[path_id].eanc_input  = 0;
	path[path_id].eanc_output = 0;
	return;
}	
//============================================================================
//
// Function Name: void csl_caph_audioh_config(int path_id)
//
// Description:  Configure the audio path on CSL layer
//
// Parameters:   path_id : audio path
//				 
// Return:      
//
//============================================================================

void csl_caph_audioh_config(int path_id, void *p)
{
	audio_config_t *pcfg = (void *)p;

	path[path_id].sample_rate = pcfg->sample_rate;
	path[path_id].sample_size = pcfg->sample_size;
	path[path_id].sample_pack = pcfg->sample_pack;
	path[path_id].sample_mode = pcfg->sample_mode;
	path[path_id].eanc_input  = pcfg->eanc_input;
	path[path_id].eanc_output = pcfg->eanc_output;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chal_audio_vibra_clr_fifo(handle);

			chal_audio_vibra_set_bits_per_sample(handle, pcfg->sample_size); 		// set the sample size
			chal_audio_vibra_set_pack(handle, pcfg->sample_pack);					// set FIFO pack/unpack
			chal_audio_vibra_int_enable(handle, TRUE, FALSE);			            // enable vin path interrupt

			/* : add threshold at here  */
			chal_audio_vibra_set_fifo_thres(handle, 0x4, 0x2); //the fifo size is 8.

			/*  : add Sigma-Delta configuation at here*/

			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:

			chal_audio_hspath_set_bits_per_sample(handle, pcfg->sample_size);		// set the sample size
			chal_audio_hspath_set_pack(handle, pcfg->sample_pack);				// set FIFO pack/unpack
			chal_audio_hspath_set_mono_stereo(handle, ((pcfg->sample_mode == AUDIO_CHANNEL_STEREO) ? FALSE : TRUE));
			chal_audio_hspath_int_enable(handle, TRUE, FALSE);		


			/*  : add threshold at here  */

			chal_audio_hspath_set_fifo_thres(handle, 0x2, 0x0);

			/*  : add Sigma-Delta configuation at here*/

			break;
		case AUDDRV_PATH_IHF_OUTPUT:

			chal_audio_ihfpath_clr_fifo(handle);	

			chal_audio_ihfpath_set_bits_per_sample(handle, pcfg->sample_size);	// set the sample size
			chal_audio_ihfpath_set_pack(handle, pcfg->sample_pack);				// set FIFO pack/unpack
			// by default, ihf is set to stereo
			chal_audio_ihfpath_set_mono_stereo(handle, ((pcfg->sample_mode == AUDIO_CHANNEL_STEREO) ? FALSE : TRUE));			
			chal_audio_ihfpath_int_enable(handle, TRUE, FALSE);						// enable IHF path interrupt

			/*  : add threshold at here  */

			chal_audio_ihfpath_set_fifo_thres(handle, 0x2, 0x0);

			/*  : add Sigma-Delta configuation at here*/


			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_clr_fifo(handle);	

			chal_audio_earpath_set_bits_per_sample(handle, pcfg->sample_size);	// set the sample size
			chal_audio_earpath_set_pack(handle, pcfg->sample_pack);				// set FIFO pack/unpack
            
			chal_audio_earpath_int_enable(handle, TRUE, FALSE);					    // enable Earpiece path interrupt

			/*  : add threshold at here  */

			chal_audio_earpath_set_fifo_thres(handle, 0x2, 0x0);

			/*  : add Sigma-Delta configuation at here*/

			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
		case AUDDRV_PATH_VIN_INPUT_L:
		case AUDDRV_PATH_VIN_INPUT_R:
		case AUDDRV_PATH_HEADSET_INPUT:

			chal_audio_vinpath_clr_fifo(handle);	

			chal_audio_vinpath_set_bits_per_sample(handle, pcfg->sample_size); 	// set the sample size
			chal_audio_vinpath_set_pack(handle, pcfg->sample_pack);               // set FIFO pack/unpack
			chal_audio_vinpath_int_enable(handle, TRUE, FALSE);			            // enable vin path interrupt
			chal_audio_vinpath_set_mono_stereo(handle, pcfg->sample_mode);

			/*  : add threshold at here  */

			chal_audio_vinpath_set_fifo_thres(handle, 0x2, 0x0);
			/*  : add Sigma-Delta configuation at here*/

			break;

		case AUDDRV_PATH_NVIN_INPUT:
		case AUDDRV_PATH_NVIN_INPUT_L:
		case AUDDRV_PATH_NVIN_INPUT_R:

			chal_audio_nvinpath_clr_fifo(handle);	

			chal_audio_nvinpath_set_bits_per_sample(handle, pcfg->sample_size); 	// set the sample size
			chal_audio_nvinpath_set_pack(handle, pcfg->sample_pack);              // set FIFO pack/unpack
			chal_audio_nvinpath_int_enable(handle, TRUE, FALSE);			        // enable vin path interrupt
   // enable vin path interrupt
			chal_audio_nvinpath_set_mono_stereo(handle, pcfg->sample_mode);
			/*  : add threshold at here  */

			chal_audio_nvinpath_set_fifo_thres(handle, 0x2, 0x0);

			/*  : add Sigma-Delta configuation at here*/
			break;

		case AUDDRV_PATH_EANC_INPUT:


			/*--- set filter coeff ---*/

            chal_audio_eancpath_set_AIIR_coef(handle, (void *)eancAIIRFilterCoeff);

            chal_audio_eancpath_set_AFIR_coef(handle, (void *)eancAIIRFilterCoeff);
 
            chal_audio_eancpath_set_DIIR_coef(handle, (void *)eanc96kIIRFilterCoeff);

            chal_audio_eancpath_set_UIIR_coef(handle, (void *)eanc48kIIRFilterCoeff);

			/*--- set control and gain ---*/

			chal_audio_eancpath_ctrl_tap(handle, 0x5555);

			/* Need to lower the CIC gain otherwise default gain is saturating the signal on FPGA*/
			chal_audio_eancpath_set_cic_gain(handle, 0x0700, 0x0700);

			/*--- set input and output for EANC ---*/

            chal_audio_eancpath_set_input_mic(handle, pcfg->eanc_input);

            if(pcfg->eanc_output == AUDDRV_PATH_EARPICEC_OUTPUT)
            {
                chal_audio_earpath_eanc_in(handle, CHAL_AUDIO_ENABLE);
            }

            if(pcfg->eanc_output == AUDDRV_PATH_IHF_OUTPUT)
            {
                chal_audio_ihfpath_eanc_in(handle, CHAL_AUDIO_ENABLE);
            }

            if(pcfg->eanc_output == AUDDRV_PATH_HEADSET_OUTPUT)
            {
                chal_audio_hspath_eanc_in(handle, CHAL_AUDIO_ENABLE);
            }

			/*---  set to 96K mode ---*/

			chal_audio_eancpath_set_mode(handle, TRUE);

			/*---  set FIFO control ---*/

			chal_audio_eancpath_clr_fifo(handle);	

			chal_audio_eancpath_set_bits_per_sample(handle, pcfg->sample_size); 	// set the sample size
			chal_audio_eancpath_set_pack(handle, pcfg->sample_pack);              // set FIFO pack/unpack
			chal_audio_eancpath_int_enable(handle, TRUE, FALSE);					        // enable EANC path interrupt
			
			/*  : add threshold at here  */

			break;


		case AUDDRV_PATH_SIDETONE_INPUT:

			/*--- sidetone go to output path ---*/

            if(pcfg->sidetone_output == AUDDRV_PATH_EARPICEC_OUTPUT)
			{
               chal_audio_earpath_sidetone_in(handle, CHAL_AUDIO_ENABLE);
			}
            if(pcfg->sidetone_output == AUDDRV_PATH_IHF_OUTPUT)
			{
               chal_audio_hspath_sidetone_in(handle, CHAL_AUDIO_ENABLE);
			}
            if(pcfg->sidetone_output == AUDDRV_PATH_HEADSET_OUTPUT)
			{
                chal_audio_hspath_sidetone_in(handle, CHAL_AUDIO_ENABLE);
			}
			
			/*--- sidetone go back to input path ---*/

			if((pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT)
			  ||(pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT_L)
			  ||(pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT_R))
			{
                chal_audio_vinpath_select_sidetone(handle, CHAL_AUDIO_ENABLE);
			}
			if((pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT)
			  ||(pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT_L)
			  ||(pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT_R))			
			{
                chal_audio_nvinpath_select_sidetone(handle, CHAL_AUDIO_ENABLE);
			}

             /*--- Update filter coefficients ---*/
#if 0   //In Rhea there is no such controls.
            chal_audio_stpath_load_filter((CHAL_HANDLE)SDT_BASE_ADDR, (void *)stoneFirCoeff, 0);
            chal_audio_stpath_set_filter_taps((CHAL_HANDLE)SDT_BASE_ADDR, 63, 63);

            /* Enable soft slope, set linear gain */
            chal_audio_stpath_set_sofslope((CHAL_HANDLE)SDT_BASE_ADDR, CHAL_AUDIO_ENABLE, CHAL_AUDIO_ENABLE, 0x0800);

            /* disable clipping, enable filtering, do not bypass gain control */
            chal_audio_stpath_config_misc((CHAL_HANDLE)SDT_BASE_ADDR, CHAL_AUDIO_DISABLE, CHAL_AUDIO_DISABLE, CHAL_AUDIO_DISABLE);
#endif
#if	0		// move to csl_set_gain()
            /* : call chal_audio_sidetonepath_set_gain() for this */
            chal_audio_stpath_set_gain((CHAL_HANDLE)SDT_BASE_ADDR, type);
#endif

			break;

		default:
			audio_xassert(0, path_id);
	}
	return;
}


//============================================================================
//
// Function Name: UInt32 csl_caph_audioh_get_fifo_addr(int path_id, UInt32 *dma_addr)
//
// Description:  Get the FIFO address
//
// Parameters:   path_id : audio path
//
// Return:       CSL_CAPH_AUDIOH_BUFADDR_t: buffer address
//
//============================================================================

CSL_CAPH_AUDIOH_BUFADDR_t csl_caph_audioh_get_fifo_addr(int path_id)
{
    CSL_CAPH_AUDIOH_BUFADDR_t dualBuf;
    dualBuf.bufAddr = 0;
    dualBuf.buf2Addr = 0;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:
			chal_audio_vibra_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:
			chal_audio_hspath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_IHF_OUTPUT:
			chal_audio_ihfpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:
			chal_audio_earpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_HEADSET_INPUT:
			chal_audio_vinpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_VIN_INPUT:
			chal_audio_vinpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			chal_audio_vinpath_get_dma_port_addr2(handle, &(dualBuf.buf2Addr));
			break;
		case AUDDRV_PATH_VIN_INPUT_L:
			chal_audio_vinpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_VIN_INPUT_R:
			chal_audio_vinpath_get_dma_port_addr2(handle, &(dualBuf.bufAddr));
			break;			
		case AUDDRV_PATH_NVIN_INPUT:
			chal_audio_nvinpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			chal_audio_nvinpath_get_dma_port_addr2(handle, &(dualBuf.buf2Addr));
			break;
		case AUDDRV_PATH_NVIN_INPUT_L:
			chal_audio_nvinpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		case AUDDRV_PATH_NVIN_INPUT_R:
			chal_audio_nvinpath_get_dma_port_addr2(handle, &(dualBuf.bufAddr));
			break;			
		case AUDDRV_PATH_EANC_INPUT:
			chal_audio_eancpath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
			break;
		default:
            audio_xassert(0, path_id);
	}
    return dualBuf;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_start(int path_id)
//
// Description:  Start playback or recording on path "path_id" on CSL layer, the path is already configured
//
// Parameters:   path_id : audio path
//
// Return:
//
//============================================================================
void csl_caph_audioh_start(int path_id)
{
	UInt16	chnl_enable = 0x0;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chnl_enable = CHAL_AUDIO_ENABLE;
			chal_audio_vibra_set_dac_pwr(handle, chnl_enable);
			chal_audio_vibra_enable(handle, chnl_enable);	

			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:

//			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO_LEFT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_RIGHT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_MONO)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT; 
			}

			chal_audio_hspath_set_dac_pwr(handle,chnl_enable);
			chal_audio_hspath_set_gain(handle, 0);
			chal_audio_hspath_enable(handle, chnl_enable);	

#ifdef PMU_BCM59055

//			PMU_DRV_AUDIO_HS_PowerUp(TRUE,NULL);
	
#endif			
			break;

		case AUDDRV_PATH_IHF_OUTPUT:

//			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO_LEFT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_RIGHT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_MONO)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			}
			
			chal_audio_ihfpath_set_dac_pwr(handle,chnl_enable);			
			chal_audio_ihfpath_set_gain(handle, 0);
			chal_audio_ihfpath_enable(handle, chnl_enable);	
#ifdef PMU_BCM59055
//			PMU_DRV_AUDIO_IHF_PowerUp (NULL);

#endif			
			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:

            /* Isolate Input = 0 */
            chal_audio_earpath_clear_isolation_ctrl(handle, CHAL_AUDIO_AUDIOTX_ISO_IN);
            /* Power up the earpiece DAC */
            chal_audio_earpath_set_dac_pwr(handle, CHAL_AUDIO_ENABLE);

            /* Wait for 40msec */
            OSTASK_Sleep(40);	//	SysTimeDelayMilliSec(AUDIOH_HW_DACPWRUP_SETTLE_TIME);

			chal_audio_earpath_set_gain(handle, 0);
            /* Powerup the Ear Piece Driver */
            chal_audio_earpath_set_drv_pwr(handle, CHAL_AUDIO_ENABLE);

            /* Wait for 40msec */
            OSTASK_Sleep(40);	// SysTimeDelayMilliSec(AUDIOH_HW_SLOWRAMP_RAMP1UP_TIME);

            /* External Pop-Click sequence */
            chal_audio_earpath_set_slowramp_ctrl(handle, CHAL_AUDIO_AUDIOTX_SR_EXT_POPCLICK);

            /* Cause a raising edge on SR_PUP_ED_DRV_TRIG and END_PWRDOWN to 1*/
            chal_audio_earpath_set_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);
            chal_audio_earpath_set_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);

            /* Wait for 150usec */
            OSTASK_Sleep(1);	//	SysTimeDelayMicroSec(AUDIOH_HW_SLOWRAMP_PWRDN_PULSE_TIME);

            /* END_PWRDOWN to 0. This will create a 150usec pulse for this signal */
            chal_audio_earpath_clear_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);
            chal_audio_earpath_set_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);
            chal_audio_earpath_set_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE);


            /* End RAMP1_45M and Start RAMP2_5M */
            chal_audio_earpath_clear_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);
            chal_audio_earpath_set_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);

            /* Wait for 5msec */
            OSTASK_Sleep(5);	// SysTimeDelayMilliSec(AUDIOH_HW_SLOWRAMP_RAMP2UP_TIME);

            /* End RAMP2_5M */
            chal_audio_earpath_clear_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);
            chal_audio_earpath_clear_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE);
            chal_audio_earpath_set_slowramp_ctrl(handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRUP);

			chal_audio_earpath_enable(handle, CHAL_AUDIO_ENABLE);	// Enable the Earpiece path

			break;

		case AUDDRV_PATH_VIN_INPUT:
                	chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
                	chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
			// Enable the digital microphone			
			chal_audio_vinpath_digi_mic_enable(handle, chnl_enable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			chal_audio_dmic1_pwrctrl(handle,TRUE);
			micStatus |= 0x3;
			break;

		case AUDDRV_PATH_VIN_INPUT_L:
        	        chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
			// Enable the digital microphone			
			chal_audio_vinpath_digi_mic_enable(handle, chnl_enable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			chal_audio_dmic1_pwrctrl(handle,TRUE);
			micStatus |= 0x1;
			break;

		case AUDDRV_PATH_VIN_INPUT_R:
	                chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
			// Enable the digital microphone			
			chal_audio_vinpath_digi_mic_enable(handle, chnl_enable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			chal_audio_dmic1_pwrctrl(handle,TRUE);
			micStatus |= 0x2;
			break;

		case AUDDRV_PATH_NVIN_INPUT:
	                chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
               		chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
			// Enable the digital microphone			
			chal_audio_nvinpath_digi_mic_enable(handle, chnl_enable);
			//DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.
			chal_audio_dmic2_pwrctrl(handle,TRUE);
			micStatus |= 0xC;
			break;

		case AUDDRV_PATH_NVIN_INPUT_L:
        	        chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
			// Enable the digital microphone			
			chal_audio_nvinpath_digi_mic_enable(handle, chnl_enable);
			//DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.
			chal_audio_dmic2_pwrctrl(handle,TRUE);
			micStatus |= 0x4;
			break;

		case AUDDRV_PATH_NVIN_INPUT_R:
	                chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
			// Enable the digital microphone			
			chal_audio_nvinpath_digi_mic_enable(handle, chnl_enable);
			//DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.
			chal_audio_dmic2_pwrctrl(handle,TRUE);
			micStatus |= 0x8;
			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:

            chal_audio_vinpath_select_primary_mic(handle, CHAL_AUDIO_ENABLE);
			chal_audio_mic_pwrctrl(handle, TRUE);

			break;

		case AUDDRV_PATH_HEADSET_INPUT:

            chal_audio_vinpath_select_primary_mic(handle, CHAL_AUDIO_ENABLE);
			chal_audio_hs_mic_pwrctrl(handle, TRUE);

			break;

		case AUDDRV_PATH_EANC_INPUT:
            chal_audio_vinpath_select_primary_mic(handle, CHAL_AUDIO_ENABLE);
			chal_audio_mic_pwrctrl(handle, TRUE);

			chal_audio_eancpath_enable(handle, TRUE);

			break;

		case AUDDRV_PATH_SIDETONE_INPUT:

			chal_audio_stpath_enable(handle, CHAL_AUDIO_ENABLE);

			break;

		default:
            audio_xassert(0, path_id);
	}
	return;
}


//============================================================================
//
// Function Name: void csl_caph_audioh_stop_keep_config(int path_id)
//
// Description:  Stop playback or recording on path "path_id" on CSL layer, but keep the configuration. 
// So can re-start without configuring again. Pause/resume case.
//
// Parameters:   path_id : audio path
//
// Return:     
//
//============================================================================

void csl_caph_audioh_stop_keep_config(int path_id)
{
	UInt16	chnl_disable = 0x0;
	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chal_audio_vibra_set_dac_pwr(handle, 0);
			chal_audio_vibra_int_enable(handle, FALSE, FALSE);			
			chal_audio_vibra_enable(handle, 0);

			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:
#ifdef PMU_BCM59055
		
//			PMU_DRV_AUDIO_HS_PowerUp(FALSE,NULL);

#endif
			chal_audio_hspath_int_enable(handle, FALSE, FALSE);
			chal_audio_hspath_enable(handle, 0);
			chal_audio_hspath_set_dac_pwr(handle,0);
			break;

		case AUDDRV_PATH_IHF_OUTPUT:
#ifdef PMU_BCM59055
//			PMU_DRV_AUDIO_IHF_PowerDown(NULL);

#endif				
			chal_audio_ihfpath_int_enable(handle, FALSE, FALSE);
			chal_audio_ihfpath_enable(handle, 0);
			chal_audio_ihfpath_set_dac_pwr(handle, 0);				
			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:
			chal_audio_earpath_int_enable(handle, FALSE, FALSE);
			chal_audio_earpath_enable(handle, 0);	// Disable the Earpiece path
			chal_audio_earpath_set_dac_pwr(handle, 0);

			break;

		case AUDDRV_PATH_VIN_INPUT:
                	chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;
                	chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;			
			chal_audio_vinpath_digi_mic_disable(handle, chnl_disable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			//need to check whether both DIGI mic are turned off.
			//Then to disable the clock.
			chal_audio_dmic1_pwrctrl(handle,FALSE);
			micStatus &= 0xFC;
			break;
		case AUDDRV_PATH_VIN_INPUT_L:
         	        chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;			
			chal_audio_vinpath_digi_mic_disable(handle, chnl_disable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			//need to check whether both DIGI mic are turned off.
			//Then to disable the clock.
			if (!(micStatus & 0x2))
			    chal_audio_dmic1_pwrctrl(handle,FALSE);
			micStatus &= 0xFE;
			break;
		case AUDDRV_PATH_VIN_INPUT_R:
	                chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;
			chal_audio_vinpath_digi_mic_disable(handle, chnl_disable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			//need to check whether both DIGI mic are turned off.
			//Then to disable the clock.
			if (!(micStatus & 0x1))
			    chal_audio_dmic1_pwrctrl(handle,FALSE);
			micStatus &= 0xFD;
			break;

		case AUDDRV_PATH_NVIN_INPUT:
                 	chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;
                	chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;				
			chal_audio_nvinpath_digi_mic_disable(handle, chnl_disable);	
			//DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.
			//need to check whether both DIGI mic are turned off.
			//Then to disable the clock.
                        chal_audio_dmic2_pwrctrl(handle,FALSE);
			micStatus &= 0xF3;			
			break;
		case AUDDRV_PATH_NVIN_INPUT_L:
         	        chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;			
			chal_audio_nvinpath_digi_mic_disable(handle, chnl_disable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			//need to check whether both DIGI mic are turned off.
			//Then to disable the clock.
			if (!(micStatus & 0x8))
			    chal_audio_dmic2_pwrctrl(handle,FALSE);
			micStatus &= 0xFB;
			break;
		case AUDDRV_PATH_NVIN_INPUT_R:
         	        chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;				
			chal_audio_nvinpath_digi_mic_disable(handle, chnl_disable);
			//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
			//need to check whether both DIGI mic are turned off.
			//Then to disable the clock.
			if (!(micStatus & 0x4))
			    chal_audio_dmic2_pwrctrl(handle,FALSE);
			micStatus &= 0xF7;
			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:
            chal_audio_vinpath_select_primary_mic(handle, 0);
			chal_audio_mic_pwrctrl(handle, 0);
			break;

		case AUDDRV_PATH_HEADSET_INPUT:
            chal_audio_vinpath_select_primary_mic(handle, 0);
			chal_audio_hs_mic_pwrctrl(handle, 0);
			break;
            
		default:
			audio_xassert(0, path_id);
	}
	return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_stop(int path_id)
//
// Description:  Stop playback or recording on path "path_id" on CSL layer. Also wipe out the configuration.
//
// Parameters:   path_id : audio path
//
// Return:     
//
//============================================================================

void csl_caph_audioh_stop(int path_id)
{
	csl_caph_audioh_stop_keep_config (path_id);
	csl_caph_audioh_unconfig(path_id);
	return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_mute(int path_id)
//
// Description:  mute playback path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:    
//
//============================================================================

void csl_caph_audioh_mute(int path_id, Boolean mute_ctrl)
{
	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chal_audio_vibra_mute(handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:

			chal_audio_hspath_mute(handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_IHF_OUTPUT:

			chal_audio_ihfpath_mute(handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_mute(handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_HEADSET_INPUT:

			chal_audio_mic_mute(handle,  mute_ctrl);

			break;
		case AUDDRV_PATH_VIN_INPUT:
		case AUDDRV_PATH_VIN_INPUT_L:
		case AUDDRV_PATH_VIN_INPUT_R:
			if(mute_ctrl == TRUE)
			    chal_audio_dmic1_pwrctrl(handle, FALSE);
                        else
			    chal_audio_dmic1_pwrctrl(handle, TRUE);
			break;

		case AUDDRV_PATH_NVIN_INPUT:
		case AUDDRV_PATH_NVIN_INPUT_L:
		case AUDDRV_PATH_NVIN_INPUT_R:
			if(mute_ctrl == TRUE)
			    chal_audio_dmic2_pwrctrl(handle, FALSE);
                        else
			    chal_audio_dmic2_pwrctrl(handle, TRUE);
	
			break;

		default:
			audio_xassert(0, path_id);
	}
	return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_setgain(int path_id, UInt32 gain, UInt32 gain1)
//
// Description:  set gain on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//              gain and gain1
//
// Return:   
//
//============================================================================

void csl_caph_audioh_setgain(int path_id ,UInt32 gain_or_bitselect, UInt32 fine_gain)
{

	switch(path_id)
	{

		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_set_gain(handle, gain_or_bitselect);
			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:
			break;

		case AUDDRV_PATH_IHF_OUTPUT:
			break;


		case AUDDRV_PATH_EANC_INPUT:

			chal_audio_eancpath_set_cic_gain(handle, gain_or_bitselect, fine_gain);

			break;
		
		case AUDDRV_PATH_SIDETONE_INPUT:

		 /* : call chal_audio_sidetonepath_set_gain() for this */
#if 0 // There is no such control in Rhea.
			chal_audio_stpath_set_gain((CHAL_HANDLE)SDT_BASE_ADDR, gain);
#endif            
			break;

        case AUDDRV_PATH_ANALOGMIC_INPUT:
        case AUDDRV_PATH_HEADSET_INPUT:

			chal_audio_mic_pga(handle, gain_or_bitselect);

			break;
        case AUDDRV_PATH_VIN_INPUT:
        case AUDDRV_PATH_VIN_INPUT_L:
        case AUDDRV_PATH_VIN_INPUT_R:
#if ( CHIP_REVISION == 10 )		//# A0
			chal_audio_vinpath_set_cic_scale(handle, gain_or_bitselect, fine_gain,
				       	gain_or_bitselect, fine_gain);
#endif
#if ( CHIP_REVISION == 20 )		//# B0
			chal_audio_vinpath_set_cic_scale(handle, gain_or_bitselect | fine_gain,
				       	gain_or_bitselect | fine_gain);
#endif
			break;		
        case AUDDRV_PATH_NVIN_INPUT:
        case AUDDRV_PATH_NVIN_INPUT_L:
        case AUDDRV_PATH_NVIN_INPUT_R:
#if ( CHIP_REVISION == 10 )		//# A0
			chal_audio_nvinpath_set_cic_scale(handle, gain_or_bitselect, fine_gain,
				       	gain_or_bitselect, fine_gain);
#endif
#if ( CHIP_REVISION == 20 )		//# B0
			chal_audio_nvinpath_set_cic_scale(handle, gain_or_bitselect | fine_gain,
				       	gain_or_bitselect | fine_gain);
#endif
			break;		
			
		default:
			audio_xassert(0, path_id);	
	}
	return;
}



//============================================================================
//
// Function Name: void csl_caph_audioh_sidetone_control(int path_id, Boolean ctrl)
//
// Description:  add sidetone into path, or remove it from path
//
// Parameters:   path_id : audio path to accept sidetone
//               ctrl    : TRUE  --- add sidetone into path 
//						 : FALSE --- remove sidetone from path
// Return:  
//
//============================================================================

void csl_caph_audioh_sidetone_control(int path_id, Boolean ctrl)
{

	switch(path_id)
	{

		case AUDDRV_PATH_VIBRA_OUTPUT:
			break;
		
		case AUDDRV_PATH_HEADSET_OUTPUT:
            chal_audio_hspath_sidetone_in(handle, ctrl);
			break;
		
		case AUDDRV_PATH_IHF_OUTPUT:
            chal_audio_hspath_sidetone_in(handle, ctrl);
			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:
            chal_audio_earpath_sidetone_in(handle, ctrl);
			break;
		
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
		case AUDDRV_PATH_VIN_INPUT_L:
		case AUDDRV_PATH_VIN_INPUT_R:
		case AUDDRV_PATH_HEADSET_INPUT:
            chal_audio_vinpath_select_sidetone(handle, ctrl);
			break;
		
		case AUDDRV_PATH_NVIN_INPUT:
		case AUDDRV_PATH_NVIN_INPUT_L:
		case AUDDRV_PATH_NVIN_INPUT_R:
            chal_audio_nvinpath_select_sidetone(handle, ctrl);
			break;

		default:
	        audio_xassert(0, path_id);	
	}
	return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_eanc_output_control(int path_id, Boolean ctrl)
//
// Description:  add enac into path, or remove it from path
//
// Parameters:   path_id   : the output path to accept EANC output
//               ctrl      : TRUE  --- add eanc output into path 
//						   : FALSE --- remove eanc output from path
// Return: 
//
//============================================================================

void csl_caph_audioh_eanc_output_control(int path_id, Boolean ctrl)
{

	switch(path_id)
	{

		case AUDDRV_PATH_VIBRA_OUTPUT:
			break;
		
		case AUDDRV_PATH_HEADSET_OUTPUT:
            chal_audio_hspath_sidetone_in(handle, ctrl);
			break;
		
		case AUDDRV_PATH_IHF_OUTPUT:
            chal_audio_hspath_sidetone_in(handle, ctrl);
			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:
            chal_audio_earpath_sidetone_in(handle, ctrl);
			break;
		
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
		case AUDDRV_PATH_VIN_INPUT_L:
		case AUDDRV_PATH_VIN_INPUT_R:
		case AUDDRV_PATH_HEADSET_INPUT:
            chal_audio_vinpath_select_sidetone(handle, ctrl);
			break;
		
		case AUDDRV_PATH_NVIN_INPUT:
		case AUDDRV_PATH_NVIN_INPUT_L:
		case AUDDRV_PATH_NVIN_INPUT_R:
            chal_audio_nvinpath_select_sidetone(handle, ctrl);
			break;

		default:
		    audio_xassert(0, path_id);
	}
	return;
}


//============================================================================
//
// Function Name: void csl_caph_audioh_loopback_control(int lbpath, Boolean ctrl)
//
// Description:  control the Analog microphone loop back to output path
//
// Parameters:  lbpath  : DAC path to accept loop back
//				ctrl    : the control to loop back 
//						TRUE - enable loop back in path,
//						FALSE - disbale loop back in path
// Return:   
//
//============================================================================

void csl_caph_audioh_loopback_control(int lbpath, Boolean ctrl)
{

	UInt32 dacmask = 0;

	if(lbpath == AUDDRV_PATH_HEADSET_OUTPUT)
	{
		dacmask = CHAL_AUDIO_PATH_HEADSET_LEFT | CHAL_AUDIO_PATH_HEADSET_RIGHT;
	}
	if(lbpath == AUDDRV_PATH_IHF_OUTPUT)
	{
		dacmask = CHAL_AUDIO_PATH_IHF_LEFT;
	}

	if(lbpath == AUDDRV_PATH_EARPICEC_OUTPUT)
	{
		dacmask = CHAL_AUDIO_PATH_EARPIECE;
	}

	chal_audio_loopback_set_out_paths(handle, dacmask, ctrl);
	chal_audio_loopback_enable(handle, ctrl);

	return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_eanc_input_control(int dmic)
//
// Description:  select/remove digital microhone input path to enac
//
// Parameters:   path_id : audio path
//               dmic            : digital micrphone ID 
//								 (dmic  -   0 : mic1, 1 : mic2, 2 : mic3, 3 :  mic4)  
// Return:  
//
//============================================================================

void csl_caph_audioh_eanc_input_control(int dmic)
{
	chal_audio_eancpath_set_input_mic(handle, dmic);
	return;
}

//============================================================================
//
// Function Name: void csl_audio_audiotx_set_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t *writedata)
//
// Description:  AudioTX HS DAC Power Control Register set
//
// Parameters:   CSL_CAPH_AUDIOH_DACCTRL_t : control structure
//				 
// Return:       none
//
//============================================================================

void csl_audio_audiotx_set_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t *writedata)
{
	UInt32 ctrl = 0;

	ctrl |= (writedata->AUDIOTX_TEST_EN) & 3;
	ctrl |= ((writedata->AUDIOTX_BB_STI) & 3) << 2;  

	chal_audio_audiotx_set_dac_ctrl(handle, ctrl);
    return;
}

//============================================================================
//
// Function Name: void csl_audio_audiotx_get_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t *readdata)
//
// Description:  Reading AudioTX HS DAC Power Control Register 
//
// Parameters:   CSL_CAPH_AUDIOH_DACCTRL_t : structure to return control 
//				 
// Return:       none
//
//============================================================================

void csl_audio_audiotx_get_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t *readdata)
{
	UInt32 ctrl;

	ctrl = chal_audio_audiotx_get_dac_ctrl(handle);
	readdata->AUDIOTX_TEST_EN		= ctrl &3;
	readdata->AUDIOTX_BB_STI		= (ctrl >> 2) &3;
	readdata->AUDIOTX_EP_DRV_STO	= (ctrl >> 4) &3;
    return;
}

//============================================================================
//
// Function Name: void csl_caph_audioh_sidetone_config(UInt32 *coeff) 
//
// Description:  configure sidetone filter
//
// Parameters:   coeff   - the coefficient buffer			
// Return:  
//
//============================================================================

void csl_caph_audioh_sidetone_load_filter(UInt32 *coeff)
{
#if ( CHIP_REVISION == 10 )		//# A0
	chal_audio_stpath_load_filter(handle, coeff );
#endif
#if ( CHIP_REVISION == 20 )		//# B0
	chal_audio_stpath_load_filter(handle, coeff, 0 );
#endif
	return;
}	


//============================================================================
//
// Function Name: void csl_caph_audioh_sidetone_config(UInt32 gain)
//
// Description:  Set sidetone gain
//
// Parameters:  gain   - gain valaue.			
// Return:  
//
//============================================================================

void csl_caph_audioh_sidetone_set_gain(UInt32 gain)
{
	chal_audio_stpath_set_gain(handle, gain);
	return;
}	


//============================================================================
//
// Function Name: void csl_caph_audioh_set_hwgain(CSL_CAPH_HW_GAIN_e hw, 
// 						UInt32 gain)
//
// Description:  Set HW gain
//
// Parameters:  hw     - which HW gain
// 		gain   - gain value in Q13.2.			
// Return:  
//
//============================================================================

void csl_caph_audioh_set_hwgain(CSL_CAPH_HW_GAIN_e hw, UInt32 gain)
{
	CAPH_AUDIOH_MIC_GAIN_e micGainSelect = CAPH_AUDIOH_NONE_MIC_GAIN;
    csl_caph_Mic_GainMapping_t outGain;
    memset(&outGain, 0, sizeof(csl_caph_Mic_GainMapping_t));

    outGain = csl_caph_gain_GetMicMappingGain((Int16)gain);
	switch(hw)
	{
		case CSL_CAPH_AMIC_PGA_GAIN:
			chal_audio_mic_pga(handle, (int)(outGain.micPGA));
			break;
		case CSL_CAPH_AMIC_DGA_COARSE_GAIN:
		case CSL_CAPH_DMIC1_DGA_COARSE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC1_COARSE_GAIN;
			chal_audio_vinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICBitSelect));
			break;
		case CSL_CAPH_AMIC_DGA_FINE_GAIN:
		case CSL_CAPH_DMIC1_DGA_FINE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC1_FINE_GAIN;
			chal_audio_vinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICFineScale));
			break;
		case CSL_CAPH_DMIC2_DGA_COARSE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC2_COARSE_GAIN;
			chal_audio_vinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICBitSelect));
			break;
		case CSL_CAPH_DMIC2_DGA_FINE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC2_FINE_GAIN;
			chal_audio_vinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICFineScale));
			break;

		case CSL_CAPH_DMIC3_DGA_COARSE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC3_COARSE_GAIN;
			chal_audio_nvinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICBitSelect));
			break;
			
		case CSL_CAPH_DMIC3_DGA_FINE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC3_FINE_GAIN;
			chal_audio_nvinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICFineScale));
			break;
			
		case CSL_CAPH_DMIC4_DGA_COARSE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC4_COARSE_GAIN;
			chal_audio_nvinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICBitSelect));
			break;
			
		case CSL_CAPH_DMIC4_DGA_FINE_GAIN:
			micGainSelect = CAPH_AUDIOH_MIC4_FINE_GAIN;
			chal_audio_nvinpath_set_each_cic_scale(handle,
					micGainSelect,
					(UInt32)(outGain.micCICFineScale));
			break;

		default:
			audio_xassert(0, hw);
			
	}
	return;
}	



//============================================================================
//
// Function Name: void csl_caph_audioh_ihfpath_set_dac_pwr(UInt16 enable_chnl)
//
// Description:  Set DAC power on IHF path. For testing purpose
//
// Parameters:  enable_chnl - DAC channle to enable.			
// Return:  
//
//============================================================================

void csl_caph_audioh_ihfpath_set_dac_pwr(UInt16 enable_chnl)
{
	chal_audio_ihfpath_set_dac_pwr(handle, enable_chnl);	
	return;
}	



//============================================================================
//
// Function Name: CSL_CAPH_AUDIOH_IHF_DAC_PWR_e csl_caph_audioh_ihfpath_get_dac_pwr(void)
//
// Description:  Get DAC power on IHF path. For testing purpose
//
// Parameters:  void			
// Return:  CSL_CAPH_AUDIOH_IHF_DAC_PWR_e dac power status.
//
//============================================================================

CSL_CAPH_AUDIOH_IHF_DAC_PWR_e csl_caph_audioh_ihfpath_get_dac_pwr(void)
{
	return (CSL_CAPH_AUDIOH_IHF_DAC_PWR_e)chal_audio_ihfpath_get_dac_pwr(handle);	
}	




//============================================================================
//
// Function Name: void csl_caph_audioh_vinpath_digi_mic_enable(UInt16 ctrl)
//
// Description:  Enable the mic. For testing purpose
//
// Parameters:  ctrl. Left and/or right channel to enable			
// Return: 
//
//============================================================================

void csl_caph_audioh_vinpath_digi_mic_enable(UInt16 ctrl)
{
	chal_audio_vinpath_digi_mic_enable(handle, ctrl);
	return;
}	


//============================================================================
//
// Function Name: CSL_CAPH_AUDIOH_VINPATH_DMIC_ENABLE_e csl_caph_audioh_vinpath_digi_mic_enable_read()
//
// Description:  Get the enabling status of the mic1 and mic2. For testing purpose
//
// Return:  CSL_CAPH_AUDIOH_VINPATH_DMIC_ENABLE_e mic status.
//
//============================================================================

CSL_CAPH_AUDIOH_VINPATH_DMIC_ENABLE_e csl_caph_audioh_vinpath_digi_mic_enable_read(void)
{
	return (CSL_CAPH_AUDIOH_VINPATH_DMIC_ENABLE_e)chal_audio_vinpath_digi_mic_enable_read(handle);
}	

//============================================================================
//
// Function Name: void csl_caph_audioh_nvinpath_digi_mic_enable(UInt16 ctrl)
//
// Description:  Enable the Eanc mic. For testing purpose
//
// Parameters:  ctrl. Left and/or right channel to enable			
// Return: 
//
//============================================================================

void csl_caph_audioh_nvinpath_digi_mic_enable(UInt16 ctrl)
{
	chal_audio_nvinpath_digi_mic_enable(handle, ctrl);
	return;
}	


//============================================================================
//
// Function Name: CSL_CAPH_AUDIOH_NVINPATH_DMIC_ENABLE_e csl_caph_audioh_nvinpath_digi_mic_enable_read()
//
// Description:  Get the enabling status of the mic3 and mic4. For testing purpose
//
// Return:  CSL_CAPH_AUDIOH_NVINPATH_DMIC_ENABLE_e mic status.
//
//============================================================================

CSL_CAPH_AUDIOH_NVINPATH_DMIC_ENABLE_e csl_caph_audioh_nvinpath_digi_mic_enable_read(void)
{
	return (CSL_CAPH_AUDIOH_NVINPATH_DMIC_ENABLE_e)chal_audio_nvinpath_digi_mic_enable_read(handle);
}	

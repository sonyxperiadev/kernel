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
*  @file   csl_audio.c
*
*  @brief  cls layer driver for audio device driver
*
****************************************************************************/
#include "mobcom_types.h"
#include "auddrv_def.h"
#include "csl_audio.h"
#include "chal_audio.h"
#include "sysmap_types.h"
#include "csl_sysmap.h"
#include "audio_driver.h"
#include "pmu.h"


//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************

CSL_Path_t path[AUDDRV_PATH_TOTAL];

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

static CHAL_HANDLE chal_handle;
static int chal_handle_init = 0;
static int chal_handle_init_count = 0;

//****************************************************************************
// local function declarations
//****************************************************************************



//******************************************************************************
// local function definitions
//******************************************************************************


//============================================================================
//
// Function Name: Boolean csl_audio_init(int path_id)
//
// Description:  Initialize audio path on CSL layer
//
// Parameters:   path_id : audio path
//				 
// Return:       the call result
//
//============================================================================

Boolean csl_audio_init ( int path_id )
{

	path[path_id].path_status = 1;
	path[path_id].handle = chal_audio_init( csl_sysmap_get_base_addr(AUDIOH), csl_sysmap_get_base_addr(SDT) );
	chal_handle_init_count++;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:
			chal_audio_vibra_clr_fifo(path[path_id].handle);	
			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:
			chal_audio_hspath_clr_fifo(path[path_id].handle);	
			break;
		case AUDDRV_PATH_IHF_OUTPUT:
			chal_audio_ihfpath_clr_fifo(path[path_id].handle);	
			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:
			chal_audio_earpath_clr_fifo(path[path_id].handle);	
			break;
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
			chal_audio_vinpath_clr_fifo(path[path_id].handle);	
			break;
		case AUDDRV_PATH_NVIN_INPUT:
			chal_audio_nvinpath_clr_fifo(path[path_id].handle);	
			break;
//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		case AUDDRV_PATH_EANC_INPUT:
			chal_audio_eancpath_clr_fifo(path[path_id].handle);	
			break;
		default:
			break;
	}
	
	
	return	0;
}

//============================================================================
//
// Function Name: Boolean csl_audio_deinit(int path_id)
//
// Description:  Deinitialize audio path on CSL layer
//
// Parameters:   path_id : audio path
//				 
// Return:       the call result
//
//============================================================================

Boolean csl_audio_deinit(int path_id)
{
	chal_handle_init_count--;
	if(chal_handle_init_count <= 0)
	{
		chal_handle_init_count = 0;
		chal_audio_deinit(path[path_id].handle);
	}
	return	0;
}


//============================================================================
//
// Function Name: Boolean csl_audio_config(int path_id)
//
// Description:  Configure the audio path on CSL layer
//
// Parameters:   path_id : audio path
//				 
// Return:       the call result
//
//============================================================================

Boolean csl_audio_config(int path_id, void *p)
{
	audio_config_t *pcfg = (void *)p;
	Boolean ret = 0;

	path[path_id].sample_rate = pcfg->sample_rate;
	path[path_id].sample_size = pcfg->sample_size;
	path[path_id].sample_pack = pcfg->sample_pack;
	path[path_id].sample_mode = pcfg->sample_mode;
	path[path_id].mic_id = pcfg->mic_id;
	path[path_id].eanc_input  = pcfg->eanc_input;
	path[path_id].eanc_output = pcfg->eanc_output;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chal_audio_vibra_set_bits_per_sample(path[path_id].handle, pcfg->sample_size); 		// set the sample size
			chal_audio_vibra_set_pack(path[path_id].handle, pcfg->sample_pack);					// set FIFO pack/unpack
			chal_audio_vibra_int_enable(path[path_id].handle, TRUE, TRUE);			            // enable vin path interrupt

			/* : add threshold at here  */


			/*  : add Sigma-Delta configuation at here*/

			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:

			chal_audio_hspath_set_bits_per_sample(path[path_id].handle, pcfg->sample_size);		// set the sample size
			chal_audio_hspath_set_pack(path[path_id].handle, pcfg->sample_pack);				// set FIFO pack/unpack
			chal_audio_hspath_set_mono_stereo(path[path_id].handle, ((pcfg->sample_mode == AUDIO_CHANNEL_STEREO) ? FALSE : TRUE));
			chal_audio_hspath_int_enable(path[path_id].handle, TRUE, TRUE);					    // enable Headset path interrupt

			/*  : add threshold at here  */

			chal_audio_hspath_set_fifo_thres(path[path_id].handle, 0x60, 0x02);

			/*  : add Sigma-Delta configuation at here*/

			break;
		case AUDDRV_PATH_IHF_OUTPUT:

			chal_audio_ihfpath_set_bits_per_sample(path[path_id].handle, pcfg->sample_size);	// set the sample size
			chal_audio_ihfpath_set_pack(path[path_id].handle, pcfg->sample_pack);				// set FIFO pack/unpack
			chal_audio_ihfpath_int_enable(path[path_id].handle, TRUE, TRUE);						// enable IHF path interrupt

			/*  : add threshold at here  */

			chal_audio_ihfpath_set_fifo_thres(path[path_id].handle, 0x60, 0x02);

			/*  : add Sigma-Delta configuation at here*/


			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_set_bits_per_sample(path[path_id].handle, pcfg->sample_size);	// set the sample size
			chal_audio_earpath_set_pack(path[path_id].handle, pcfg->sample_pack);				// set FIFO pack/unpack
			chal_audio_earpath_int_enable(path[path_id].handle, TRUE, TRUE);					    // enable Earpiece path interrupt

			/*  : add threshold at here  */

			chal_audio_earpath_set_fifo_thres(path[path_id].handle, 0x60, 0x02);

			/*  : add Sigma-Delta configuation at here*/

			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:

			chal_audio_vinpath_set_bits_per_sample(path[path_id].handle, pcfg->sample_size); 	// set the sample size
			chal_audio_vinpath_set_pack(path[path_id].handle, pcfg->sample_pack);               // set FIFO pack/unpack
			chal_audio_vinpath_int_enable(path[path_id].handle, TRUE, TRUE);			        // enable vin path interrupt
			
			if(pcfg->sample_mode == AUDIO_CHANNEL_STEREO)
				chal_audio_vinpath_set_mono_stereo(path[path_id].handle, !CHAL_AUDIO_ENABLE);
			else
				chal_audio_vinpath_set_mono_stereo(path[path_id].handle, CHAL_AUDIO_ENABLE);

			/*  : add threshold at here  */


			/*  : add Sigma-Delta configuation at here*/

			break;

		case AUDDRV_PATH_NVIN_INPUT:

			chal_audio_nvinpath_set_bits_per_sample(path[path_id].handle, pcfg->sample_size); 	// set the sample size
			chal_audio_nvinpath_set_pack(path[path_id].handle, pcfg->sample_pack);              // set FIFO pack/unpack
			chal_audio_nvinpath_int_enable(path[path_id].handle, TRUE, TRUE);			        // enable vin path interrupt

			if(pcfg->sample_mode == AUDIO_CHANNEL_STEREO)
				chal_audio_nvinpath_set_mono_stereo(path[path_id].handle, !CHAL_AUDIO_ENABLE);
			else
				chal_audio_nvinpath_set_mono_stereo(path[path_id].handle, CHAL_AUDIO_ENABLE);
			/*  : add threshold at here  */


			/*  : add Sigma-Delta configuation at here*/
			break;

//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;

		case AUDDRV_PATH_EANC_INPUT:


			/*--- set filter coeff ---*/

            chal_audio_eancpath_set_AIIR_coef(path[path_id].handle, (void *)eancAIIRFilterCoeff);

            chal_audio_eancpath_set_AFIR_coef(path[path_id].handle, (void *)eancAIIRFilterCoeff);
 
            chal_audio_eancpath_set_DIIR_coef(path[path_id].handle, (void *)eanc96kIIRFilterCoeff);

            chal_audio_eancpath_set_UIIR_coef(path[path_id].handle, (void *)eanc48kIIRFilterCoeff);

			/*--- set control and gain ---*/

			chal_audio_eancpath_ctrl_tap(path[path_id].handle, 0x5555);

			/* Need to lower the CIC gain otherwise default gain is saturating the signal on FPGA*/
			chal_audio_eancpath_set_cic_gain(path[path_id].handle, 0x0700, 0x0700);

			/*--- set input and output for EANC ---*/

            chal_audio_eancpath_set_input_mic(path[path_id].handle, pcfg->eanc_input);

            if(pcfg->eanc_output == AUDDRV_PATH_EARPICEC_OUTPUT)
            {
                chal_audio_earpath_eanc_in(path[path_id].handle, CHAL_AUDIO_ENABLE);
            }

            if(pcfg->eanc_output == AUDDRV_PATH_IHF_OUTPUT)
            {
                chal_audio_ihfpath_eanc_in(path[path_id].handle, CHAL_AUDIO_ENABLE);
            }

            if(pcfg->eanc_output == AUDDRV_PATH_HEADSET_OUTPUT)
            {
                chal_audio_hspath_eanc_in(path[path_id].handle, CHAL_AUDIO_ENABLE);
            }

			/*---  set to 96K mode ---*/

			chal_audio_eancpath_set_mode(path[path_id].handle, TRUE);

			/*---  set FIFO control ---*/

			chal_audio_eancpath_set_bits_per_sample(path[path_id].handle, pcfg->sample_size); 	// set the sample size
			chal_audio_eancpath_set_pack(path[path_id].handle, pcfg->sample_pack);              // set FIFO pack/unpack
			chal_audio_eancpath_int_enable(path[path_id].handle, TRUE, TRUE);					        // enable EANC path interrupt
			
			/*  : add threshold at here  */

			break;


		case AUDDRV_PATH_SIDETONE_INPUT:

			/*--- sidetone go to output path ---*/

            if(pcfg->sidetone_output == AUDDRV_PATH_EARPICEC_OUTPUT)
			{
               chal_audio_earpath_sidetone_in(path[path_id].handle, CHAL_AUDIO_ENABLE);
			}
            if(pcfg->sidetone_output == AUDDRV_PATH_IHF_OUTPUT)
			{
               chal_audio_hspath_sidetone_in(path[path_id].handle, CHAL_AUDIO_ENABLE);
			}
            if(pcfg->sidetone_output == AUDDRV_PATH_HEADSET_OUTPUT)
			{
                chal_audio_hspath_sidetone_in(path[path_id].handle, CHAL_AUDIO_ENABLE);
			}
			
			/*--- sidetone go back to input path ---*/

			if(pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT)
			{
                chal_audio_vinpath_select_sidetone(path[path_id].handle, CHAL_AUDIO_ENABLE);
			}
			if(pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT)
			{
                chal_audio_nvinpath_select_sidetone(path[path_id].handle, CHAL_AUDIO_ENABLE);
			}

             /*--- Update filter coefficients ---*/

            chal_audio_stpath_load_filter((CHAL_HANDLE) csl_sysmap_get_base_addr(SDT), (void *)stoneFirCoeff, 0);
            chal_audio_stpath_set_filter_taps((CHAL_HANDLE) csl_sysmap_get_base_addr(SDT), 63, 63);

            /* Enable soft slope, set linear gain */
            chal_audio_stpath_set_sofslope((CHAL_HANDLE) csl_sysmap_get_base_addr(SDT), CHAL_AUDIO_ENABLE, CHAL_AUDIO_ENABLE, 0x0800);

            /* disable clipping, enable filtering, do not bypass gain control */
            chal_audio_stpath_config_misc((CHAL_HANDLE) csl_sysmap_get_base_addr(SDT), CHAL_AUDIO_DISABLE, CHAL_AUDIO_DISABLE, CHAL_AUDIO_DISABLE);

#if	0		// move to csl_set_gain()
            /* : call chal_audio_sidetonepath_set_gain() for this */
            chal_audio_stpath_set_gain((CHAL_HANDLE) csl_sysmap_get_base_addr(SDT), type);
#endif

			break;

		default:
			ret = 1;
			break;
	}

	return	ret;
}


//============================================================================
//
// Function Name: UInt32 csl_audio_get_dma_port_addr(int path_id, UInt32 *dma_addr)
//
// Description:  Get the DMA physical destination or sourcr address
//
// Parameters:   path_id : audio path
//				 *dma_addr  : the dma port address 
//
// Return:       call result
//
//============================================================================

Boolean csl_audio_get_dma_port_addr(int path_id, UInt32 *dma_addr)
{

	UInt32 size = 0;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:
			chal_audio_vibra_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:
			chal_audio_hspath_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
		case AUDDRV_PATH_IHF_OUTPUT:
			chal_audio_ihfpath_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:
			chal_audio_earpath_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
			chal_audio_vinpath_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
		case AUDDRV_PATH_NVIN_INPUT:
			chal_audio_nvinpath_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		case AUDDRV_PATH_EANC_INPUT:
			chal_audio_eancpath_get_dma_port_addr( path[path_id].handle, dma_addr);
			break;
		default:
			break;
	}

	return size;
	
}

//============================================================================
//
// Function Name: UInt32 csl_audio_enable_dma_channel(int path_id, Boolean enable)
//
// Description:  Enable/disable the DMA on path
//
// Parameters:   path_id : audio path
//               enable  : enable/disable control
// Return:       call result
//
//============================================================================

Boolean csl_audio_enable_dma_channel(int path_id, Boolean enable)
{

	UInt32 size = 0;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:
			chal_audio_vibra_dma_enable( path[path_id].handle, enable);
			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:
			chal_audio_hspath_dma_enable( path[path_id].handle, enable);
			break;
		case AUDDRV_PATH_IHF_OUTPUT:
			chal_audio_ihfpath_dma_enable( path[path_id].handle, enable);
			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:
			chal_audio_earpath_dma_enable( path[path_id].handle, enable);
			break;
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
			chal_audio_vinpath_dma_enable( path[path_id].handle, enable);
			break;
		case AUDDRV_PATH_NVIN_INPUT:
			chal_audio_nvinpath_dma_enable( path[path_id].handle, enable);
			break;
//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		case AUDDRV_PATH_EANC_INPUT:
			chal_audio_eancpath_dma_enable( path[path_id].handle, enable);
			break;
		default:
			break;
	}

	return size;
	
}




//============================================================================
//
// Function Name: UInt32 csl_audio_copy_data_to_fifo(int path_id, 
//							UInt32 *src, UInt32 length)
//
// Description:  Copy data to FIFO on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//				 *src    : data buffer pointer
//               length  : data buffer size
//
// Return:       the size written to FIFO
//
//============================================================================

UInt32 csl_audio_copy_data_to_fifo(int path_id, UInt32 *src, UInt32 length)
{

	UInt32 size = 0;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:
			size = chal_audio_vibra_write_fifo( path[path_id].handle, (void *)src, length, CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW);				
			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:
			size = chal_audio_hspath_write_fifo( path[path_id].handle, (void *)src, length, CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW);				
			break;
		case AUDDRV_PATH_IHF_OUTPUT:
			size = chal_audio_ihfpath_write_fifo( path[path_id].handle, (void *)src, length, CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW);				
			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:
			size = chal_audio_earpath_write_fifo( path[path_id].handle, (void *)src, length, CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW);				
			break;
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
			break;
		case AUDDRV_PATH_NVIN_INPUT:
			break;
//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		default:
			break;
	}

	return size;
	
}

//============================================================================
//
// Function Name: UInt32 csl_audio_read_data_from_fifo(int path_id, 
//							UInt32 *buf, UInt32 length)
//
// Description:  Read data from FIFO on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//				 *buf    : buffer pointer to save data from FIFO
//               length  : data buffer size
//
// Return:       the size of read data from FIFO
//
//============================================================================

UInt32 csl_audio_read_data_from_fifo(int path_id, UInt32 *buf, UInt32 length)
{
	UInt32 size = 0;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:
			break;
		case AUDDRV_PATH_HEADSET_OUTPUT:
			break;
		case AUDDRV_PATH_IHF_OUTPUT:
			break;
		case AUDDRV_PATH_EARPICEC_OUTPUT:
			break;
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
			size = chal_audio_vinpath_read_fifo( path[path_id].handle, (void *)buf, length, CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW);				
			break;
		case AUDDRV_PATH_NVIN_INPUT:
			size = chal_audio_nvinpath_read_fifo( path[path_id].handle, (void *)buf, length, CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW);				
			break;
//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		default:
			break;
	}

	return size;
}


//============================================================================
//
// Function Name: Boolean csl_audio_start(int path_id)
//
// Description:  Start playback or recording on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:       the call result
//
//============================================================================
Boolean csl_audio_start(int path_id)
{
	Boolean ret = 0;
	UInt16	chnl_enable;

	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chnl_enable = CHAL_AUDIO_ENABLE;
			chal_audio_vibra_set_dac_pwr(path[path_id].handle, chnl_enable);
			chal_audio_vibra_enable(path[path_id].handle, chnl_enable);	

			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:

			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO_LEFT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			}
			else if(path[path_id].sample_mode == CHAL_AUDIO_CHANNEL_RIGHT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_RIGHT;
			}

			chal_audio_hspath_enable(path[path_id].handle, chnl_enable);	
			chal_audio_hspath_set_dac_pwr(path[path_id].handle,chnl_enable);

#ifdef PMU_BCM59055

			PMU_DRV_AUDIO_HS_PowerUp(TRUE,NULL);
	
#endif			
			break;

		case AUDDRV_PATH_IHF_OUTPUT:

			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
			}
			else if(path[path_id].sample_mode == AUDIO_CHANNEL_STEREO_LEFT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
			}
			else if(path[path_id].sample_mode == CHAL_AUDIO_CHANNEL_RIGHT)
			{
				chnl_enable = CHAL_AUDIO_CHANNEL_RIGHT;
			}
			
			chal_audio_ihfpath_enable(path[path_id].handle, chnl_enable);	
			chal_audio_ihfpath_set_dac_pwr(path[path_id].handle,chnl_enable);

#ifdef PMU_BCM59055

			PMU_DRV_AUDIO_IHF_PowerUp(NULL);
	
#endif			

			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_enable(path[path_id].handle, CHAL_AUDIO_ENABLE);	// Enable the Earpiece path
//			chal_audio_earpath_set_dac_pwr(path[path_id].handle, CHAL_AUDIO_ENABLE);

            /* External Pop-Click sequence */
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle, CHAL_AUDIO_AUDIOTX_SR_EXT_POPCLICK);

            /* Isolate Input = 0 */
            chal_audio_earpath_clear_isolation_ctrl(path[path_id].handle, CHAL_AUDIO_AUDIOTX_ISO_IN);

            /* Power up the earpiece DAC */
            chal_audio_earpath_set_dac_pwr(path[path_id].handle, CHAL_AUDIO_ENABLE);

            /* Wait for 40msec */
            OSTASK_Sleep(40);	//	SysTimeDelayMilliSec(AUDIOH_HW_DACPWRUP_SETTLE_TIME);

            /* Cause a raising edge on SR_PUP_ED_DRV_TRIG and END_PWRDOWN to 1*/
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);

            /* Wait for 150usec */
            OSTASK_Sleep(1);	//	SysTimeDelayMicroSec(AUDIOH_HW_SLOWRAMP_PWRDN_PULSE_TIME);

            /* END_PWRDOWN to 0. This will create a 150usec pulse for this signal */
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE);

            /* Powerup the Ear Piece Driver */
            chal_audio_earpath_set_drv_pwr(path[path_id].handle, CHAL_AUDIO_ENABLE);

            /* Wait for 40msec */
            OSTASK_Sleep(40);	// SysTimeDelayMilliSec(AUDIOH_HW_SLOWRAMP_RAMP1UP_TIME);

            /* End RAMP1_45M and Start RAMP2_5M */
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);

            /* Wait for 5msec */
            OSTASK_Sleep(5);	// SysTimeDelayMilliSec(AUDIOH_HW_SLOWRAMP_RAMP2UP_TIME);

            /* End RAMP2_5M */
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRUP);


			break;

		case AUDDRV_PATH_VIN_INPUT:

			chnl_enable = 0;	
			if(path[path_id].mic_id & AUDDRV_MIC_DMIC1)	chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
			if(path[path_id].mic_id & AUDDRV_MIC_DMIC2)	chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;

			chal_audio_vinpath_digi_mic_enable(path[path_id].handle, chnl_enable);	// Enable the digital microphone
			chal_audio_dmic1_pwrctrl(path[path_id].handle,1);

			break;

		case AUDDRV_PATH_NVIN_INPUT:

			chnl_enable = 0;	
			if(path[path_id].mic_id & AUDDRV_MIC_DMIC3)	chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
			if(path[path_id].mic_id & AUDDRV_MIC_DMIC4)	chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;

			chal_audio_nvinpath_digi_mic_enable(path[path_id].handle, chnl_enable);	
			chal_audio_dmic2_pwrctrl(path[path_id].handle,1);

			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:

			if(path[path_id].mic_id == AUDDRV_MIC_AMIC)
			{
				chal_audio_mic_input_select(path[path_id].handle, path[path_id].mic_id);
				chal_audio_mic_pwrctrl(path[path_id].handle, TRUE);
			}
			else
			{
				chal_audio_auxmic_input_select(path[path_id].handle, path[path_id].mic_id);
				chal_audio_auxmic_pwrctrl(path[path_id].handle, TRUE);
			}

			break;

		case AUDDRV_PATH_EANC_INPUT:

			chal_audio_eancpath_enable(path[path_id].handle, TRUE);

			break;

		case AUDDRV_PATH_SIDETONE_INPUT:

			chal_audio_stpath_enable(path[path_id].handle, CHAL_AUDIO_ENABLE);

			break;

		default:
			ret = 1;
			break;
	}

	return	ret;
}


//============================================================================
//
// Function Name: Boolean csl_audio_pause(int path_id)
//
// Description:  Pause playback or recording on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:       the call result
//
//============================================================================
Boolean csl_audio_pause(int path_id)
{

	return	0;
}

//============================================================================
//
// Function Name: Boolean csl_audio_resume(int path_id)
//
// Description:  Resume playback or recording on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:       the call result
//
//============================================================================

Boolean csl_audio_resume(int path_id)
{

	return	0;
}


//============================================================================
//
// Function Name: Boolean csl_audio_stop(int path_id)
//
// Description:  Stop playback or recording on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:       the call result
//
//============================================================================

Boolean csl_audio_stop(int path_id)
{
	Boolean ret = 0;
	
	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chal_audio_vibra_set_dac_pwr(path[path_id].handle, 0);
			chal_audio_vibra_enable(path[path_id].handle, 0);

			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:

#ifdef PMU_BCM59055
		
			PMU_DRV_AUDIO_HS_PowerUp(FALSE,NULL);

#endif
			chal_audio_hspath_enable(path[path_id].handle, 0);
			chal_audio_hspath_set_dac_pwr(path[path_id].handle,0);

			break;

		case AUDDRV_PATH_IHF_OUTPUT:

#ifdef PMU_BCM59055
		
			PMU_DRV_AUDIO_IHF_PowerDown(NULL);

#endif
			chal_audio_ihfpath_enable(path[path_id].handle, 0);
			chal_audio_ihfpath_set_dac_pwr(path[path_id].handle,0);

			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_enable(path[path_id].handle, 0);	// Disable the Earpiece path

			chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRUP);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_PD_ENABLE);

            /* Wait for 35msec */
            OSTASK_Sleep(35);	//	SysTimeDelayMilliSec(AUDIOH_HW_SLOWRAMP_RAMP2DOWN_TIME);
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);

            /* Wait for 10msec */
            OSTASK_Sleep(10);	// SysTimeDelayMilliSec(AUDIOH_HW_SLOWRAMP_RAMP1DOWN_TIME);
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);
            chal_audio_earpath_set_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_PD_ENABLE);

            /* Wait for 150usec */
            OSTASK_Sleep(1);	//	SysTimeDelayMicroSec(AUDIOH_HW_SLOWRAMP_PWRDN_PULSE_TIME);
            chal_audio_earpath_clear_slowramp_ctrl(path[path_id].handle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);

            /* power down the earpiece Driver  */
            chal_audio_earpath_set_drv_pwr(path[path_id].handle, 0);

            /* Isolate Input = 1 */
            chal_audio_earpath_set_isolation_ctrl(path[path_id].handle, CHAL_AUDIO_AUDIOTX_ISO_IN);

            /* Wait for 1 msec */
            OSTASK_Sleep(1);	//	SysTimeDelayMilliSec(AUDIOH_HW_PATHENDIS_SETTLING_TIME);

            /* Power Down the DAC */
            chal_audio_earpath_set_dac_pwr(path[path_id].handle, 0);

			break;


		case AUDDRV_PATH_ANALOGMIC_INPUT:

			chal_audio_vinpath_digi_mic_enable(path[path_id].handle, 0);

			if(path[path_id].mic_id == AUDDRV_MIC_AMIC)	
			{
				chal_audio_mic_pwrctrl(path[path_id].handle, 0);
			}
			else	
			{
				chal_audio_auxmic_pwrctrl(path[path_id].handle, 0);	
			}
	
			break;

		case AUDDRV_PATH_VIN_INPUT:

			chal_audio_vinpath_digi_mic_enable(path[path_id].handle, 0);	
			chal_audio_dmic1_pwrctrl(path[path_id].handle,0);

			break;

		case AUDDRV_PATH_NVIN_INPUT:

			chal_audio_nvinpath_digi_mic_enable(path[path_id].handle, 0);	
			chal_audio_dmic2_pwrctrl(path[path_id].handle,0);
			
			break;

//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		default:
			ret = 1;
			break;
	}

	return	ret;

}

//============================================================================
//
// Function Name: Boolean csl_audio_mute(int path_id)
//
// Description:  mute playback path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:       the call result
//
//============================================================================

Boolean csl_audio_mute(int path_id, Boolean mute_ctrl)
{

	Boolean ret = 0;
	
	switch(path_id)
	{
		case AUDDRV_PATH_VIBRA_OUTPUT:

			chal_audio_vibra_mute(path[path_id].handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:

			chal_audio_hspath_mute(path[path_id].handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_IHF_OUTPUT:

			chal_audio_ihfpath_mute(path[path_id].handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_mute(path[path_id].handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:

			chal_audio_vinpath_mute(path[path_id].handle,  mute_ctrl);

			break;

		case AUDDRV_PATH_NVIN_INPUT:

//			chal_audio_nvinpath_mute(path[path_id].handle,  mute_ctrl);
			
			break;

//		case AUDDRV_PATH_ANALOGMIC_INPUT:
//			break;
		default:
			ret = 1;
			break;
	}

	return	ret;

}

//============================================================================
//
// Function Name: Boolean csl_audio_setgain(int path_id)
//
// Description:  set gain on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:       the call result
//
//============================================================================

Boolean csl_audio_setgain(int path_id ,UInt32 gain, UInt32 gain1)
{

	int pmu_gain;

	switch(path_id)
	{

		case AUDDRV_PATH_EARPICEC_OUTPUT:

			chal_audio_earpath_set_gain(path[path_id].handle, gain);
			break;

		case AUDDRV_PATH_HEADSET_OUTPUT:

#ifdef PMU_BCM59055

			if(	gain <= HS_MIN_APP_GAIN_SCALE) gain = HS_MIN_APP_GAIN_SCALE;
			if(	gain >= HS_MAX_APP_GAIN_SCALE) gain = HS_MAX_APP_GAIN_SCALE;

			pmu_gain = (gain * HS_MAX_PUM_GAIN_SCALE) / HS_MAX_APP_GAIN_SCALE;

			PMU_DRV_AUDIO_HS_SetGain(PMU_AUDIO_HS_BOTH,pmu_gain, NULL);

#endif	
			break;

		case AUDDRV_PATH_IHF_OUTPUT:

#ifdef PMU_BCM59055

			if(	gain <= IHF_MIN_APP_GAIN_SCALE) gain = IHF_MIN_APP_GAIN_SCALE;
			if(	gain >= IHF_MAX_APP_GAIN_SCALE) gain = IHF_MAX_APP_GAIN_SCALE;

			pmu_gain = (gain * IHF_MAX_PUM_GAIN_SCALE) / IHF_MAX_APP_GAIN_SCALE;

			PMU_DRV_AUDIO_IHF_SetGain(pmu_gain, NULL);

#endif			
			break;

		case AUDDRV_PATH_EANC_INPUT:

			chal_audio_eancpath_set_cic_gain(path[path_id].handle, gain, gain1);

			break;
		
		case AUDDRV_PATH_SIDETONE_INPUT:

		 /* : call chal_audio_sidetonepath_set_gain() for this */
			chal_audio_stpath_set_gain((CHAL_HANDLE) csl_sysmap_get_base_addr(SDT), gain);
			break;
		
		case AUDDRV_PATH_ANALOGMIC_INPUT:

			if(	gain < AMIC_MIN_APP_GAIN_SCALE) gain = AMIC_MIN_APP_GAIN_SCALE;
			if(	gain > AMIC_MAX_APP_GAIN_SCALE) gain = AMIC_MAX_APP_GAIN_SCALE;

			pmu_gain = (gain * AMIC_MAX_PUM_GAIN_SCALE) / AMIC_MAX_APP_GAIN_SCALE;

			chal_audio_mic_pga(path[path_id].handle, pmu_gain);

			break;

		default:
		
			break;
	}
	return	0;
}



//============================================================================
//
// Function Name: Boolean csl_audio_sidetone_control(int path_id, Boolean ctrl)
//
// Description:  add sidetone into path, or remove it from path
//
// Parameters:   path_id : audio path to accept sidetone
//               ctrl    : TRUE  --- add sidetone into path 
//						 : FALSE --- remove sidetone from path
// Return:       the call result
//
//============================================================================

Boolean csl_audio_sidetone_control(int path_id, Boolean ctrl)
{

	switch(path_id)
	{

		case AUDDRV_PATH_VIBRA_OUTPUT:
			break;
		
		case AUDDRV_PATH_HEADSET_OUTPUT:
            chal_audio_hspath_sidetone_in(path[AUDDRV_PATH_SIDETONE_INPUT].handle, ctrl);
			break;
		
		case AUDDRV_PATH_IHF_OUTPUT:
            chal_audio_hspath_sidetone_in(path[AUDDRV_PATH_SIDETONE_INPUT].handle, ctrl);
			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:
            chal_audio_earpath_sidetone_in(path[AUDDRV_PATH_SIDETONE_INPUT].handle, ctrl);
			break;
		
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
            chal_audio_vinpath_select_sidetone(path[AUDDRV_PATH_SIDETONE_INPUT].handle, ctrl);
			break;
		
		case AUDDRV_PATH_NVIN_INPUT:
            chal_audio_nvinpath_select_sidetone(path[AUDDRV_PATH_SIDETONE_INPUT].handle, ctrl);
			break;

		default:
		
			break;
	}
	return	0;
}

//============================================================================
//
// Function Name: Boolean csl_audio_eanc_output_control(int path_id, Boolean ctrl)
//
// Description:  add enac into path, or remove it from path
//
// Parameters:   path_id   : the output path to accept EANC output
//               ctrl      : TRUE  --- add eanc output into path 
//						   : FALSE --- remove eanc output from path
// Return:       the call result
//
//============================================================================

Boolean csl_audio_enan_output_control(int path_id, Boolean ctrl)
{

	switch(path_id)
	{

		case AUDDRV_PATH_VIBRA_OUTPUT:
			break;
		
		case AUDDRV_PATH_HEADSET_OUTPUT:
            chal_audio_hspath_sidetone_in(path[AUDDRV_PATH_EANC_INPUT].handle, ctrl);
			break;
		
		case AUDDRV_PATH_IHF_OUTPUT:
            chal_audio_hspath_sidetone_in(path[AUDDRV_PATH_EANC_INPUT].handle, ctrl);
			break;

		case AUDDRV_PATH_EARPICEC_OUTPUT:
            chal_audio_earpath_sidetone_in(path[AUDDRV_PATH_EANC_INPUT].handle, ctrl);
			break;
		
		case AUDDRV_PATH_ANALOGMIC_INPUT:
		case AUDDRV_PATH_VIN_INPUT:
            chal_audio_vinpath_select_sidetone(path[AUDDRV_PATH_EANC_INPUT].handle, ctrl);
			break;
		
		case AUDDRV_PATH_NVIN_INPUT:
            chal_audio_nvinpath_select_sidetone(path[AUDDRV_PATH_EANC_INPUT].handle, ctrl);
			break;

		default:
		
			break;
	}

	return	0;
}


//============================================================================
//
// Function Name: Boolean csl_audio_loopback_control(int lbpath, Boolean ctrl)
//
// Description:  control the Analog microphone loop back to output path
//
// Parameters:  lbpath  : DAC path to accept loop back
//				ctrl    : the control to loop back 
//						TRUE - enable loop back in path,
//						FALSE - disbale loop back in path
// Return:       the call result
//
//============================================================================

Result_t csl_audio_loopback_control(int lbpath, Boolean ctrl)
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

	if(path[AUDDRV_PATH_ANALOGMIC_INPUT].handle)
	{
		chal_audio_loopback_set_out_paths(path[AUDDRV_PATH_ANALOGMIC_INPUT].handle, dacmask, ctrl);
		chal_audio_loopback_enable(path[AUDDRV_PATH_ANALOGMIC_INPUT].handle, ctrl);
	}

	return	0;
}

//============================================================================
//
// Function Name: Boolean csl_audio_eanc_input_control(int dmic)
//
// Description:  select/remove digital microhone input path to enac
//
// Parameters:   path_id : audio path
//               dmic            : digital micrphone ID 
//								 (dmic  -   0 : mic1, 1 : mic2, 2 : mic3, 3 :  mic4)  
// Return:       the call result
//
//============================================================================

Boolean csl_audio_enan_input_control(int dmic)
{

	chal_audio_eancpath_set_input_mic(path[AUDDRV_PATH_EANC_INPUT].handle, dmic);

	return	0;

}

//============================================================================
//
// Function Name: void csl_audio_audiotx_set_dac_ctrl(dac_ctrl_t *writedata)
//
// Description:  AudioTX HS DAC Power Control Register set
//
// Parameters:   dac_ctrl_t : control structure
//				 
// Return:       none
//
//============================================================================

void csl_audio_audiotx_set_dac_ctrl(dac_ctrl_t *writedata)
{

	UInt32 ctrl = 0;

	if(chal_handle_init == 0)
	{
		chal_handle_init = 1;
		chal_handle = chal_audio_init( csl_sysmap_get_base_addr(AUDIOH), csl_sysmap_get_base_addr(SDT) );
	}

	ctrl |= (writedata->AUDIOTX_TEST_EN) & 3;
	ctrl |= ((writedata->AUDIOTX_BB_STI) & 3) << 2;  

	chal_audio_audiotx_set_dac_ctrl(chal_handle, ctrl);

}

//============================================================================
//
// Function Name: void csl_audio_audiotx_get_dac_ctrl(dac_ctrl_t *readdata)
//
// Description:  Reading AudioTX HS DAC Power Control Register 
//
// Parameters:   dac_ctrl_t : structure to return control 
//				 
// Return:       none
//
//============================================================================

void csl_audio_audiotx_get_dac_ctrl(dac_ctrl_t *readdata)
{

	UInt32 ctrl;

	if(chal_handle_init == 0)
	{
		chal_handle_init = 1;
		chal_handle = chal_audio_init( csl_sysmap_get_base_addr(AUDIOH), csl_sysmap_get_base_addr(SDT) );
	}

	ctrl = chal_audio_audiotx_get_dac_ctrl(chal_handle);
	readdata->AUDIOTX_TEST_EN		= ctrl &3;
	readdata->AUDIOTX_BB_STI		= (ctrl >> 2) &3;
	readdata->AUDIOTX_EP_DRV_STO	= (ctrl >> 4) &3;

}



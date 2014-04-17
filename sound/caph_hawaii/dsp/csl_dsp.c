/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*   @file   csl_dsp.c
*
*   @brief  This file contains AP interface functions to DSP
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_arm2sp.h"
#include "csl_vpu.h"
#include "audio_trace.h"
#include <linux/memory.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_cph_cfifo.h>
#include <mach/rdb/brcm_rdb_cph_aadmac.h>
#include <mach/rdb/brcm_rdb_ahintc.h>
#include <mach/rdb/brcm_rdb_cph_ssasw.h>
#include <mach/rdb/brcm_rdb_sspil.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>

AP_SharedMem_t *vp_shared_mem;

static VPUCaptureStatusCB_t VPUCaptureStatusHandler = NULL;
static spinlock_t VPUCaptureStatusLock;
static VPURenderStatusCB_t VPURenderStatusHandler = NULL;
static spinlock_t VPURenderStatusLock;
static USBStatusCB_t USBStatusHandler = NULL;
static spinlock_t USBStatusLock;
static VOIFStatusCB_t VOIFStatusHandler = NULL;
static spinlock_t VOIFStatusLock;
static ARM2SPRenderStatusCB_t ARM2SPRenderStatusHandler = NULL;
static spinlock_t ARM2SPRenderStatusLock;
static ARM2SP2RenderStatusCB_t ARM2SP2RenderStatusHandler = NULL;
static spinlock_t ARM2SP2RenderStatusLock;
static MainAMRStatusCB_t MainAMRStatusHandler = NULL;
static spinlock_t MainAMRStatusLock;
static VoIPStatusCB_t VoIPStatusHandler = NULL;
static spinlock_t VoIPStatusLock;
static AudioLogStatusCB_t AudioLogStatusHandler = NULL;
static spinlock_t AudioLogStatusLock;
static AudioEnableDoneStatusCB_t AudioEnableDoneHandler = NULL;
static spinlock_t AudioEnableDoneLock;
static ARM2SP_HQ_DL_InitDoneStatusCB_t ARM2SP_HQ_DL_InitDoneHandler = NULL;
static spinlock_t ARM2SP_HQ_DL_InitDoneLock;
static PTTStatusCB_t PTTStatusHandler = NULL;
static spinlock_t PTTStatusLock;
static ExtModemCallDoneStatusCB_t ExtModemCallDoneHandler = NULL;
static spinlock_t ExtModemCallDoneLock;
static int dsp_error_already_reported = 0;

/*********************************************************************/
/**
*
*   VPSHAREDMEM_Init initializes AP interface to DSP.
*
*   @param    dsp_shared_mem (in)	AP shared memory address
*
**********************************************************************/
void VPSHAREDMEM_Init(UInt32 *dsp_shared_mem)
{
	vp_shared_mem = (AP_SharedMem_t *) dsp_shared_mem;

	aTrace(LOG_AUDIO_DSP, " VPSHAREDMEM_Init: dsp_shared_mem=0x%lx\n",
	       (UInt32) dsp_shared_mem);

	/* Clear out shared memory */
	memset(vp_shared_mem, 0, sizeof(AP_SharedMem_t));

	vp_shared_mem->vp_shared_cmdq_in = 0;
	vp_shared_mem->vp_shared_cmdq_out = 0;
	vp_shared_mem->vp_shared_statusq_in = 0;
	vp_shared_mem->vp_shared_statusq_out = 0;

	vp_shared_mem->shared_ap_supported_features =
		(SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BITF);

	/* Setting various mixer gains to unity gain */
	CSL_SetARM2SpeechDLGain(0);
	CSL_SetARM2Speech2DLGain(0);
	CSL_SetARM2SpeechHQDLGain(0);
	CSL_SetInpSpeechToARM2SpeechMixerDLGain(0);

	CSL_SetARM2SpeechULGain(0);
	CSL_SetARM2Speech2ULGain(0);
	CSL_SetInpSpeechToARM2SpeechMixerULGain(0);

	CSL_SetARM2SpeechCallRecordGain(0);
	CSL_SetARM2Speech2CallRecordGain(0);

	/* set DSP DL speech record gain */
	CSL_SetDlSpeechRecGain(0);

	/* Initialize spin locks */
	spin_lock_init(&VPUCaptureStatusLock);
	spin_lock_init(&VPURenderStatusLock);
	spin_lock_init(&USBStatusLock);
	spin_lock_init(&VOIFStatusLock);
	spin_lock_init(&ARM2SPRenderStatusLock);
	spin_lock_init(&ARM2SP2RenderStatusLock);
	spin_lock_init(&MainAMRStatusLock);
	spin_lock_init(&VoIPStatusLock);
	spin_lock_init(&AudioLogStatusLock);
	spin_lock_init(&AudioEnableDoneLock);
	spin_lock_init(&ARM2SP_HQ_DL_InitDoneLock);
	spin_lock_init(&PTTStatusLock);
	spin_lock_init(&ExtModemCallDoneLock);
}

/*********************************************************************/
/**
*
*   VPSHAREDMEM_PostCmdQ writes an entry to the VPU command queue.
*
*   @param    status_msg	(in)	input status message pointer
*
**********************************************************************/
void VPSHAREDMEM_PostCmdQ(VPCmdQ_t *cmd_msg)
{
	VPCmdQ_t *p;
	UInt8 next_cmd_in;

	aTrace(LOG_AUDIO_DSP, " VPSHAREDMEM_PostCmdQ: cmdq_in=0x%x,"
	       "cmdq_out=0x%x\n",
	       vp_shared_mem->vp_shared_cmdq_in,
	       vp_shared_mem->vp_shared_cmdq_out);
	next_cmd_in =
	    (UInt8) ((vp_shared_mem->vp_shared_cmdq_in + 1) % VP_CMDQ_SIZE);

	assert(next_cmd_in != vp_shared_mem->vp_shared_cmdq_out);

	p = &vp_shared_mem->vp_shared_cmdq[vp_shared_mem->vp_shared_cmdq_in];
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;

	vp_shared_mem->vp_shared_cmdq_in = next_cmd_in;
	aTrace(LOG_AUDIO_DSP, " VPSHAREDMEM_PostCmdQ: cmd=0x%x, arg0=0x%x,"
	       "arg1=%d, arg2=%d\n",
	       cmd_msg->cmd, cmd_msg->arg0, cmd_msg->arg1, cmd_msg->arg2);

	VPSHAREDMEM_TriggerRIPInt();

}

/*********************************************************************/
/**
*
*   VPSHAREDMEM_ReadStatusQ reads an entry from the VPU status  queue.
*
*   @param    status_msg	(in)	status message destination pointer
*
**********************************************************************/
static Boolean VPSHAREDMEM_ReadStatusQ(VPStatQ_t *status_msg)
{
	VPStatQ_t *p;
	UInt8 status_out = vp_shared_mem->vp_shared_statusq_out;
	UInt8 status_in = vp_shared_mem->vp_shared_statusq_in;

/*	aTrace(LOG_AUDIO_DSP, " VPSHAREDMEM_ReadStatusQ: status_in=0x%x,
 *	status_out=0x%x \n", vp_shared_mem->vp_shared_statusq_in,
 *	vp_shared_mem->vp_shared_statusq_out);
 */
	if (status_out == status_in) {
		return FALSE;
	} else {
		p = &vp_shared_mem->vp_shared_statusq[status_out];
		status_msg->status = p->status;
		status_msg->arg0 = (UInt16) p->arg0;
		status_msg->arg1 = (UInt16) p->arg1;
		status_msg->arg2 = (UInt16) p->arg2;
		status_msg->arg3 = (UInt16) p->arg3;
#ifdef BRCM_RTOS
#else
		aTrace(LOG_AUDIO_DSP,
		       " VPSHAREDMEM_ReadStatusQ: status=%d,"
		       "arg0=%d, arg1=%d, arg2=%d, arg3=%d\n",
		       p->status, p->arg0, p->arg1, p->arg2, p->arg3);
#endif
		vp_shared_mem->vp_shared_statusq_out =
		    (status_out + 1) % VP_STATUSQ_SIZE;

		return TRUE;
	}

}

/*********************************************************************/
/**
*
*   CSL_RegisterVPUCaptureStatusHandler registers VPU capture status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterVPUCaptureStatusHandler(VPUCaptureStatusCB_t callbackFunction)
{
	spin_lock_bh(&VPUCaptureStatusLock);
	VPUCaptureStatusHandler = callbackFunction;
	spin_unlock_bh(&VPUCaptureStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterVPURenderStatusHandler registers VPU render status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterVPURenderStatusHandler(VPURenderStatusCB_t callbackFunction)
{
	spin_lock_bh(&VPURenderStatusLock);
	VPURenderStatusHandler = callbackFunction;
	spin_unlock_bh(&VPURenderStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterUSBStatusHandler registers USB status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterUSBStatusHandler(USBStatusCB_t callbackFunction)
{
	spin_lock_bh(&USBStatusLock);
	USBStatusHandler = callbackFunction;
	spin_unlock_bh(&USBStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterVOIFStatusHandler registers VOIF status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterVOIFStatusHandler(VOIFStatusCB_t callbackFunction)
{
	spin_lock_bh(&VOIFStatusLock);
	VOIFStatusHandler = callbackFunction;
	spin_unlock_bh(&VOIFStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterARM2SPRenderStatusHandler registers main ARM2SP render
*	status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterARM2SPRenderStatusHandler(ARM2SPRenderStatusCB_t
					   callbackFunction)
{
	spin_lock_bh(&ARM2SPRenderStatusLock);
	ARM2SPRenderStatusHandler = callbackFunction;
	spin_unlock_bh(&ARM2SPRenderStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterARM2SP2RenderStatusHandler registers main ARM2SP2 render
*	status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterARM2SP2RenderStatusHandler(ARM2SP2RenderStatusCB_t
					    callbackFunction)
{
	spin_lock_bh(&ARM2SP2RenderStatusLock);
	ARM2SP2RenderStatusHandler = callbackFunction;
	spin_unlock_bh(&ARM2SP2RenderStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterMainAMRStatusHandler registers main AMR status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterMainAMRStatusHandler(MainAMRStatusCB_t callbackFunction)
{
	spin_lock_bh(&MainAMRStatusLock);
	MainAMRStatusHandler = callbackFunction;
	spin_unlock_bh(&MainAMRStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterAudioLogHandler registers VoIP status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterVoIPStatusHandler(VoIPStatusCB_t callbackFunction)
{
	spin_lock_bh(&VoIPStatusLock);
	VoIPStatusHandler = callbackFunction;
	spin_unlock_bh(&VoIPStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterAudioLogHandler registers audio logging status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterAudioLogHandler(AudioLogStatusCB_t callbackFunction)
{
	spin_lock_bh(&AudioLogStatusLock);
	AudioLogStatusHandler = callbackFunction;
	spin_unlock_bh(&AudioLogStatusLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterAudioEnableDoneHandler registers audio enable done
*   status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterAudioEnableDoneHandler(AudioEnableDoneStatusCB_t
					callbackFunction)
{
	spin_lock_bh(&AudioEnableDoneLock);
	AudioEnableDoneHandler = callbackFunction;
	spin_unlock_bh(&AudioEnableDoneLock);

}

/*********************************************************************/
/**
*
*   CSL_RegisterARM2SP_HQ_DL_InitDoneHandler registers audio enable done
*   status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterARM2SP_HQ_DL_InitDoneHandler(ARM2SP_HQ_DL_InitDoneStatusCB_t
					callbackFunction)
{
	spin_lock_bh(&ARM2SP_HQ_DL_InitDoneLock);
	ARM2SP_HQ_DL_InitDoneHandler = callbackFunction;
	spin_unlock_bh(&ARM2SP_HQ_DL_InitDoneLock);

}

/*****************************************************************************/
/**
*
* Function Name: CSL_RegisterExtModemCallDoneHandler
*
*   @note     CSL_RegisterExtModemCallDoneHandler registers the status handler
*             routine to service the DSPs response to the
*             csl_dsp_ext_modem_call. This function registered is called when
*             AP receives VP_STATUS_EXT_MODEM_CALL_DONE reply from the DSP.
*
*   @param    callbackFunction  (in) callback function to register
*
*   @return   None
*
*****************************************************************************/
void CSL_RegisterExtModemCallDoneHandler(
		ExtModemCallDoneStatusCB_t callbackFunction)
{
	spin_lock_bh(&ExtModemCallDoneLock);
	ExtModemCallDoneHandler = callbackFunction;
	spin_unlock_bh(&ExtModemCallDoneLock);
}

/*********************************************************************/
/**
*
*   CSL_RegisterPTTStatusHandler registers PTT status handler.
*
*   @param    callbackFunction	(in)	callback function to register
*
**********************************************************************/
void CSL_RegisterPTTStatusHandler(PTTStatusCB_t callbackFunction)
{
	spin_lock_bh(&PTTStatusLock);
	PTTStatusHandler = callbackFunction;
	spin_unlock_bh(&PTTStatusLock);

}

/*********************************************************************/
/**
*
*   AP_ProcessStatus processes VPU status message from DSP on AP.
*
*
**********************************************************************/
void AP_ProcessStatus(void)
{
	VPStatQ_t status_msg;
	static UInt32 ec26_err_count, ec27_err_count, ec28_err_count;

	while (VPSHAREDMEM_ReadStatusQ(&status_msg)) {
		switch (status_msg.status) {
		case VP_STATUS_RECORDING_DATA_READY:
			{
				spin_lock_bh(&VPUCaptureStatusLock);
				if (VPUCaptureStatusHandler != NULL) {
					VPUCaptureStatusHandler(status_msg.
								arg0);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "VPUCaptureStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&VPUCaptureStatusLock);
				break;

			}

		case VP_STATUS_PLAYBACK_DATA_EMPTY:
			{
				spin_lock_bh(&VPURenderStatusLock);
				if (VPURenderStatusHandler != NULL) {
					VPURenderStatusHandler(status_msg.arg0);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "VPURenderStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&VPURenderStatusLock);
				break;

			}

		case VP_STATUS_USB_HEADSET_BUFFER:
			{
				spin_lock_bh(&USBStatusLock);
				if (USBStatusHandler != NULL) {
					USBStatusHandler(status_msg.arg0,
							 status_msg.arg1,
							 status_msg.arg2);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "USBStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&USBStatusLock);
				break;
			}

		case VP_STATUS_VOIF_BUFFER_READY:
			{
				spin_lock_bh(&VOIFStatusLock);
				if (VOIFStatusHandler != NULL) {
					VOIFStatusHandler(status_msg.arg0,
							  status_msg.arg1);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "VOIFStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&VOIFStatusLock);
				break;
			}

		case VP_STATUS_ARM2SP_EMPTY:
			{
				spin_lock_bh(&ARM2SPRenderStatusLock);
				if (ARM2SPRenderStatusHandler != NULL) {
					ARM2SPRenderStatusHandler(status_msg.
								  arg1);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "ARM2SPRenderStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&ARM2SPRenderStatusLock);
				break;
			}

		case VP_STATUS_ARM2SP2_EMPTY:
			{
				spin_lock_bh(&ARM2SP2RenderStatusLock);
				if (ARM2SP2RenderStatusHandler != NULL) {
					ARM2SP2RenderStatusHandler(status_msg.
								   arg1);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "ARM2SP2RenderStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&ARM2SP2RenderStatusLock);
				break;
			}

		case VP_STATUS_MAIN_AMR_DONE:
			{
				spin_lock_bh(&MainAMRStatusLock);
				if (MainAMRStatusHandler != NULL) {
					MainAMRStatusHandler(status_msg.arg0);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "MainAMRStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&MainAMRStatusLock);
				break;
			}

		case VP_STATUS_VOIP_DL_DONE:
			{
				spin_lock_bh(&VoIPStatusLock);
				if (VoIPStatusHandler != NULL) {
					VoIPStatusHandler();
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "VoIPStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&VoIPStatusLock);
				break;
			}

		case VP_STATUS_SP:
			{
				sp_StatusUpdate();

				break;

			}

		case VP_STATUS_AUDIO_STREAM_DATA_READY:
			{
				spin_lock_bh(&AudioLogStatusLock);
				if (AudioLogStatusHandler != NULL) {
					AudioLogStatusHandler(status_msg.arg2);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "AudioLogStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&AudioLogStatusLock);
				break;
			}

		case VP_STATUS_AUDIO_ENABLE_DONE:
			{
				ec26_err_count = 0;
				ec27_err_count = 0;
				ec28_err_count = 0;
				spin_lock_bh(&AudioEnableDoneLock);
				if (AudioEnableDoneHandler != NULL) {
					AudioEnableDoneHandler(status_msg.arg0);
				} else {
					aTrace(LOG_AUDIO_DSP,
						"AP DSP Interrupt:"
						"AudioEnableDoneHandler"
						"is not registered");
				}
				spin_unlock_bh(&AudioEnableDoneLock);
				break;
			}

		case VP_STATUS_ARM2SP_HQ_DL_INIT_DONE:
			{
				spin_lock_bh(&ARM2SP_HQ_DL_InitDoneLock);
				if (ARM2SP_HQ_DL_InitDoneHandler != NULL) {
					ARM2SP_HQ_DL_InitDoneHandler();
				} else {
					aTrace(LOG_AUDIO_DSP,
						"AP DSP Interrupt:"
						"ARM2SP_HQ_DL_InitDoneHandler"
						"is not registered");
				}
				spin_unlock_bh(&ARM2SP_HQ_DL_InitDoneLock);
				break;
			}

		case VP_STATUS_PTT_STATUS:
			{
				{
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "PTT_STATUS = 0x%x, 0x%x, 0x%x",
						    status_msg.arg0,
						    status_msg.arg1,
						    status_msg.arg2);
				}
				break;
			}

		case VP_STATUS_PTT_UL_FRAME_DONE:
			{
				spin_lock_bh(&PTTStatusLock);
				if (PTTStatusHandler != NULL) {
					PTTStatusHandler(status_msg.arg0,
							  status_msg.arg1,
							  status_msg.arg2);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "PTTStatusHandler"
					       "is not registered");
				}
				spin_unlock_bh(&PTTStatusLock);
				break;
			}

		case VP_STATUS_EXT_MODEM_CALL_DONE:
			{
				spin_lock_bh(&ExtModemCallDoneLock);
				if (ExtModemCallDoneHandler != NULL) {
					ExtModemCallDoneHandler(
						status_msg.arg0);
				} else {
					aTrace(LOG_AUDIO_DSP,
					       "AP DSP Interrupt:"
					       "ExtModemCallDoneHandler"
					       "is not registered");
				}
				spin_unlock_bh(&ExtModemCallDoneLock);
				break;
			}

		case 0xec26:
			{
				if (dsp_error_already_reported == 0) {
					aError("ERROR: Pri Mic AADMAC's "
						"HW_RDY bit not set for "
						"right half when Pri Mic "
						"AADMAC int comes.\n");
					aError("Pri_Mic_AADMAC_SR_1 = "
						"0x%04x%04x\n",
						status_msg.arg0,
						status_msg.arg1);
					aError("Pri_Mic Expected SR1 = "
						"0x%04x%04x\n",
						status_msg.arg2,
						status_msg.arg3);
					aError("shared_aadmac_aud_enable"
						"	= 0x%04x\n",
						vp_shared_mem->
						shared_aadmac_aud_enable);
					ec26_err_count++;
					if (ec26_err_count > 0) {
						ec26_err_count = 0;
						dsp_error_already_reported = 1;
					}
				}
				break;
			}

		case 0xec27:
			{
				if (dsp_error_already_reported == 0) {
					aError("ERROR: Sec Mic AADMAC's HW_RDY"
						" bit not set for right half "
						"when Sec Mic AADMAC int "
						"comes.\n");
					aError("Sec_Mic_AADMAC_SR_1 = "
						"0x%04x%04x\n",
						status_msg.arg0,
						status_msg.arg1);
					aError("Sec_Mic Expected SR1 = "
						"0x%04x%04x\n",
						status_msg.arg2,
						status_msg.arg3);
					aError("shared_aadmac_aud_enable ="
						" 0x%04x\n",
						vp_shared_mem->
						shared_aadmac_aud_enable);
					ec27_err_count++;
					if (ec27_err_count > 0) {
						ec27_err_count = 0;
						dsp_error_already_reported = 1;
					}
				}
				break;
			}

		case 0xec28:
			{
				if (dsp_error_already_reported == 0) {
					aError("ERROR: Spkr AADMAC's HW_RDY "
						"bit not set when Mic AADMAC"
						" int comes.\n");
					aError("Spkr_AADMAC_SR_1 = "
						"0x%04x%04x\n",
						status_msg.arg0,
						status_msg.arg1);
					aError("Spkr_AADMAC_CR_2 = "
						"0x%04x%04x\n",
						status_msg.arg2,
						status_msg.arg3);
					aError("shared_aadmac_aud_enable = "
						"0x%04x\n",
						vp_shared_mem->
						shared_aadmac_aud_enable);
					ec28_err_count++;
					if (ec28_err_count > 4) {
						ec28_err_count = 0;
						dsp_error_already_reported = 1;
					}
				}
				break;
			}
		case 0xec2f:
			{
			if (dsp_error_already_reported == 0) {
				aError("ERROR: External Modem Interface DL "
					"AADMAC's HW_RDY bit not set for "
					"right half when Ext Modem DL AADMAC "
					"int comes.\n");
				aError("Ext_Mdm_DL_AADMAC_SR_1 = 0x%04x%04x\n",
					status_msg.arg0, status_msg.arg1);
				aError("Ext_Mdm_DL Expected SR1 = "
					"0x%04x%04x\n",
					status_msg.arg2, status_msg.arg3);
				dsp_error_already_reported = 1;
			}
			break;
			}
		case 0xec30:
			{
			if (dsp_error_already_reported == 0) {
				aError("ERROR: External Modem Interface UL"
					" AADMAC's HW_RDY bit not set for "
					"right half when Ext Modem DL AADMAC "
					"int comes.\n");
				aError("Ext_Mdm_UL_AADMAC_SR_1 = 0x%04x%04x\n",
					status_msg.arg0, status_msg.arg1);
				aError("Ext_Mdm_UL Expected SR1 = "
					"0x%04x%04x\n",
					status_msg.arg2, status_msg.arg3);
				dsp_error_already_reported = 1;
			}
			break;
			}
		case 0xec31:
			{
			if (dsp_error_already_reported == 0) {
				aError("ERROR: Audio driver has not turned ON"
					" CAPH clk before sending "
					"COMMAND_AUDIO_ENABLE to the DSP.\n");
				aError("AUDIOH_CLKGATE_REG (DSP side) = "
					"0x%04x%04x\n",
					status_msg.arg0, status_msg.arg1);
				aError("COMMAND_AUDIO_ENABLE arg0 = 0x%04x\n",
					status_msg.arg2);
				aError("shared_aadmac_aud_enable (DSP side) ="
					"0x%04x\n",
					status_msg.arg3);
				dsp_error_already_reported = 1;
			}
			break;
			}


		default:
			aTrace(LOG_AUDIO_DSP,
				"AP DSP Interrupt:" "Unknown Status received");

			break;

		}

	}

}

/****************************************************************************/
/**
*
* Function Name: AUDIO_Return_IHF_48kHz_buffer_base_address
*
*   @note   This function returns the base address to the shared memory
*			buffer where the ping-pong 48kHz data would be stored
*			for AADMAC to pick them up for IHF case.
*			This base address needs to be programmed to the
*			AADMAC_CTRL1 register.
*
*   @return ptr (UInt32 *) Pointer to the base address of shared memory
*			ping-pong buffer for transferring 48kHz speech data
*			from DSP to AADMAC for IHF.
*
****************************************************************************/
UInt32 *AUDIO_Return_IHF_48kHz_buffer_base_address(void)
{
	AP_SharedMem_t *ap_shared_mem_ptr;
	struct Dsp_AP_ScratchMem_t *dsp_scratch_mem_ptr;

	if (((vp_shared_mem->shared_dsp_supported_features
			& SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BITF)
			== SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BITF) &&
		((vp_shared_mem->shared_ap_supported_features
			& SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BITF)
			== SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BITF)) {
		dsp_scratch_mem_ptr = DSPDRV_GetPhysicalScratchMemoryAddress();

		return &(dsp_scratch_mem_ptr->scr_ram_aadmac_spkr_low[0]);
		/*return (UInt32 *)(0x34051a80);*/
	} else {
		ap_shared_mem_ptr = DSPDRV_GetPhysicalSharedMemoryAddress();
		return &ap_shared_mem_ptr->shared_aadmac_spkr_low[0];
	}
}


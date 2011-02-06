/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
*   @file   vpu.c
*
*   @brief  This file contains the voice processing unit (VPU) functions
*
****************************************************************************/

//#define ENABLE_LOGGING
#include "mobcom_types.h"
#include "consts.h"
#include "hal_audio.h"
#include <linux/broadcom/chip_version.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include "brcm_alsa.h"
#include "hal_audio_core.h"
#include "hal_audio_config.h"
#include "hal_audio_access.h"
#include <plat/timer.h>

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>

//--#include "mti_trace.h"
//--#include "types.h"
//--#include "assert.h"
//--#include "irqctrl.h"
#include "vpu.h"

//--#include "assert.h"
//--#include "ossemaphore.h"
//--#include "osinterrupt.h"
//--#include "ostask.h"
//--#include "ostimer.h"
#include "vpripcmdq.h"
//--#include "stdlib.h"
//--#include "sdltrace.h"
//--#include "ripcmdq.h"
//--#include "ripproc.h"
#include "memmap.h"
//--#include "audioapi.h"
//--#include "dspcmd.h"
//--#include "log.h"
#include "shared.h"
//--#include "phyframe.h"
//--#include "audvoc_if.h"
//--#include "sysparm.h" // 12192008 michael
//--#include "i2s.h"
#include "sharedmem.h"

//audvoc_if.h
/// VPU interrupt status
typedef enum {

	VPU_INTERRUPT	= 0x1000,		///< reserved
	VPU_STATUS_RECORDING_DATA_READY	///< VPU recording data is ready for read

}Interrpt_Status; // amr nb pcm read back.

extern void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2);

//vpripcmdq.c
void  VPRIPCMDQ_SetCPVTStartStopFlag(UInt16 arg0);
void VPRIPCMDQ_VocoderInit(UInt16 val);
void VPRIPCMDQ_SetVoiceDtx(Boolean enable);
void  VPRIPCMDQ_DSPAudioAlign(UInt16 arg0);
void  VPRIPCMDQ_DSP_AMR_RUN(UInt16 type, Boolean amr_if2_enable, UInt16 mst_flag);



#if defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_)
void Audio_ISR_Handler(StatQ_t msg)
{
}
#endif

extern void TRACE_Printf_Sio( const char *ctrl, ... );

#ifdef  ENABLE_LOGGING
#define _DBG_(a)		a
#else
#define _DBG_(a)
#endif

#if CHIPVERSION >= CHIP_VERSION(BCM2133,0) && defined(POLY_INCLUDED) /* BCM2133 and later */
#include "poly_ringer.h"
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2133,0) /* BCM2133 and later */

#if ((defined(_BCM2153_) && CHIP_REVISION >= 40) || defined(_BCM21551_)||defined(_ATHENA_))
#include <linux/broadcom/gpt_if.h>
#endif

#define PLAYBACK_BLOCK		4
#define RECORDING_BLOCK		2


#if ((defined(_BCM2153_) && CHIP_REVISION >= 40) || defined(_BCM21551_) || defined(_ATHENA_) )

#ifdef FUSE_APPS_PROCESSOR
#define USE_GP_TIMER
#endif

#if defined(_BCM21551_) || defined(_ATHENA_)
#define  VT_GPTIMER_ID          3
#else
#define  VT_GPTIMER_ID          5
#endif

#endif

#define VT_USE_GPT_INDEX 4
#define GPTIMER_BASE_ADDR		 (0x8830100)

typedef struct {
    char *buffer;
    unsigned long iobaseaddr;
    unsigned int iosize;
    volatile u8 *hwregs;
    unsigned int irq;
    struct semaphore vt_irq_sem;
    unsigned int gpt_irq_count;
    struct work_struct intr_work;
    struct workqueue_struct *intr_workqueue;	
} hxgptreg_t;

hxgptreg_t gpthwreg;

typedef enum
{
	VPU_MODE_IDLE,
	VPU_MODE_RECOGNITION,
	VPU_MODE_TRAINING,
	VPU_MODE_RECOG_PLAYBACK,
	VPU_MODE_MEMO_RECORDING,
	VPU_MODE_CALL_RECORDING,
	VPU_MODE_RECORDING_PLAYBACK,
    VPU_MODE_RECORD_AND_PLAYBACK,

	VPU_MODE_VM_PLAYMEMO,
	VPU_MODE_VM_PLAYCALL,
	VPU_MODE_VM_RECORDMEMO,
	VPU_MODE_VM_RECORDCALL
#if CHIPVERSION >= CHIP_VERSION(BCM2133,0) /* BCM2133 and later */
	,VPU_MODE_POLYRINGER_PLAY
#endif
}VPU_Mode_t;

#ifdef	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
//#define AACENC_TEST
#ifdef	AACENC_TEST

static UInt16	pcmSamplesBuf[160];
UInt16 aacEnc_pcmSource;
extern UInt32 Total_fill_size;
extern UInt16 aacenc_infile_done;
extern UInt16 aacenc_enabled;

#endif	//
#endif	//	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE

//******************************************************************************
// Global Variables
//******************************************************************************
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
Boolean		AMR_memo_mode;			// TRUE: in AMR memo recording/playback mode
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))
extern UInt16 fuse_ap_vt_active;
#endif
//******************************************************************************
// Local Macros
//******************************************************************************

#define MAX_VAL_FOR_TESTING     160   //RON 11/16/01 for testing voice apps

//******************************************************************************
// Local Variables
//******************************************************************************

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132C3 and later */
static UInt8 dl_buffer[32*2]; //DL ping-pong double buffer in VT mode only.
static UInt8 len[2];
static UInt8 dec_mode[2];
static UInt8 enc_mode[2];
static Boolean VT_flag = FALSE;
static Boolean prv_VT_flag = FALSE;
static Boolean ping_pong_rd_ind = 0;
static Boolean ping_pong_wr_ind = 1;
static Boolean VT_active_flag = FALSE;
#endif
static UInt8				idle_mode_f;
static VP_Speech_Mode_t	speech_mode;
static UInt8				audioapi_id;
static VPU_Mode_t			vpu_mode;

//RON 11/16/01 added to allow recording /playback for MAX_VAL_FOR_TESTING
//full speech buffers
static UInt8				frame_count;
static UInt16				rec_buffer_count;
static UInt16				play_buffer_count;
static UInt16				temp_speech_store[320*MAX_VAL_FOR_TESTING];
static Boolean				telephony_dtx_mode;
static Boolean				telephony_amr_if2;
static VP_Mode_AMR_t  telephony_encode_amr_mode;
//--static Interrupt_t vt_intr_t=NULL;
//--static Semaphore_t vt_sema_id = NULL;
//--static Task_t vt_task_id = NULL;
static UInt8 *vt_fill_ptr=NULL;
//--static Timer_t VPU_20ms_timer=0;

// cb function pointers for voice memo
static VPUFillFramesCB_t	FillVPFramesCB;
static VPUDumpFramesCB_t	DumpVPFramesCB;

// cb function pointers for telephony application
static VPUDumpFramesCB_t	Telephony_DumpVPFramesCB;
static VPUFillFramesCB_t	Telephony_FillVPFramesCB;

static GP_CB_FUNC the_gpt_cb=NULL;
static UInt32 gVT_Tid=0;
//static TimerEntry_t gAppGPTimerCb=NULL;

extern void VPRIPCMDQ_DSP_CLEAR_VOIPMODE(void);
/////////////////////////////////////////////////////////////////////////////////
// VoIP related
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
static void VoIP_ProcessStatusMainAMRDone(StatQ_t status_msg);
#endif
// parameter that holds codec type, amr mode, dtx for dsp cmd from upper layer
static UInt16 				gVoIP_CodecType;
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) && (!defined(BCM2153_FAMILY_BB_CHIP_BOND)))
__align(0x10000) UInt8 voip_downloadable_image[] = {
 	#include "..\DSP\2153\E1\Downloadable\voip.txt"
};
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) && (!defined(BCM2153_FAMILY_BB_CHIP_BOND)))
#endif //#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

//***************************************************************************************
// TEST ONLY !!!
// Ken 12/18/05 - When EVENTLIST_TEST is defined, a list of pre-recorded events produced via
// off line simulation is played out to the DSP.
//#define	EVENTLIST_TEST

#ifdef EVENTLIST_TEST
Shared_poly_events_t *SHAREDMEM_Get_poly_events_MemPtr(void);
extern			Shared_poly_events_t	*event_page;
static UInt32	next_test_event_in;
static UInt16	next_event,
				eventlist_test[] = {
				#include "new2_dls_Traffic_events.txt"	// Pre-Recorded list of events
				};

void Load_Events(Side eventbuffer)		// Load pre-recorded events for testing.
{
UInt16	num_words,
		next_test_event_out,
		i;

	event_page = SHAREDMEM_Get_poly_events_MemPtr();		// point to the event buffer in shared memory
	next_test_event_out = EVENT_BUFFER_SIZE * eventbuffer;	// output event list array index
	num_words = eventlist_test[next_test_event_in];

	if (num_words == 0)					// end of input
	{
		next_test_event_in = 0;			// Go back to the start and loop through the events forever
		num_words = eventlist_test[next_test_event_in];
	}
TP4= 0xFFF0;
TP4= num_words;
TP4= next_test_event_in;
TP4= next_test_event_out;

	// Write the length and the events to shared memory for the DSP
	for (i=0; i<num_words+1; i++)
		event_page->sharedPR_g_eventBuffer[next_test_event_out++] = eventlist_test[next_test_event_in++];

	event_page->sharedPR_events_ready[eventbuffer]=1;		// Mark events ready for DSP to process
}
#endif	// EVENTLIST_TEST
// TEST ONLY !!!
//***************************************************************************************
//***************************************************************************************

//******************************************************************************
//
// Function Name:  VPU_NS_Set
//
// Description:  This function turn on/off vpu ns status
//
// Notes:
//
//******************************************************************************
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51)) //for 2153 e1 and later, VPU NS is enabled by default
static Boolean VPU_NS_Status = TRUE;
void VPU_NS_Set(Boolean vpu_ns_status)
{
	VPU_NS_Status = vpu_ns_status;
}
#endif

static Boolean VPU_NS_Tuning = FALSE; // 12192008 michael
void VPU_NS_ParmTuning(Boolean vpu_ns_tuning) // 12192008 michael
{
	VPU_NS_Tuning = vpu_ns_tuning;
}


//******************************************************************************
//*************************  Local Functions  **********************************
//******************************************************************************

//******************************************************************************
//
// Function Name:  VPU_AudioTurnOn
//
// Description:  This function turn on  audio unit
//
// Notes:
//
//******************************************************************************

void VPU_AudioTurnOn(							// Initialize the voice processing unit
	Boolean	Uplink,
	Boolean Downlink
)
{
}

//******************************************************************************
//
// Function Name:  VPU_AudioTurnOff
//
// Description:  This function turn on  audio unit
//
// Notes:
//
//******************************************************************************

void VPU_AudioTurnOff(void)					// Initialize the voice processing unit
{
}


//******************************************************************************
//*************************  Global Functions  **********************************
//******************************************************************************

//******************************************************************************
//
// Function Name:  VPU_Init
//
// Description:  This function initializes the voice processing unit
//
// Notes:
//
//******************************************************************************

void VPU_Init()					// Initialize the voice processing unit
{
	CmdQ_t msg;

	//audioapi_id = AUDIO_GetAudioID();
	audioapi_id = AUDIO_ID_VPU;
	vpu_mode = VPU_MODE_IDLE;
	FillVPFramesCB = NULL;
	DumpVPFramesCB = NULL;

#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	AMR_memo_mode = FALSE;	
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */

   	msg.cmd = COMMAND_VPU_ENABLE;
   	//SHAREDMEM_PostCmdQ( &msg );
	post_msg(COMMAND_VPU_ENABLE, 0, 0, 0);
}

//******************************************************************************
//
// Function Name:  VPU_NS_Init
//
// Description:  This function initializes NS for the voice processing unit
//
// Notes:
//
//******************************************************************************

const Int16 ns_max_tbl[2][16] = { // 12192008 michael
	{-800, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320},
	{-320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320, -320}
};

void VPU_NS_Init(VP_Speech_Mode_t  vp_speech_mode)					// Initialize NS for the voice processing unit
{
	UInt16 i, smode;
	SharedMem_t			*mem = SHAREDMEM_GetSharedMemPtr();
	PAGE5_SharedMem_t *pg5_mem = SHAREDMEM_GetPage5SharedMemPtr();

	if (VPU_NS_Tuning) // 12192008 michael
	{
#if 0	//needed for EU phone
		for ( i = 0; i < 16; i++ )
		{
			mem->shared_noise_max_supp_dB[i] = SYSPARM_GetAudioParmAccessPtr()->audio_parm[AUDIO_GetAudioMode()].noise_supp_max;
		}
#endif	
	}
	else
	{
		smode = (vp_speech_mode == VP_SPEECH_MODE_LINEAR_PCM) ? 0 : 1;
		pg5_mem->shared_vp_compander_flag = (smode == 0) ? 2 : 0;
		for ( i = 0; i < 16; i++ )
		{
			mem->shared_noise_max_supp_dB[i] = ns_max_tbl[smode][i];
		}
	}
}

//******************************************************************************
//
// Function Name:  VPU_StartCallRecording()
//
// Description: THis function starts call recording
//
// Notes:
//
//******************************************************************************

void VPU_StartCallRecording(	// starts call recording
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
	UInt8	vp_speech_amr_mode,		// [7|6..4|3..0] = [AMR2_dtx|vp_speech_mode|vp_amr_mode]
	UInt8 	downlink_audio_off		// 1: downlink audio off; 0: downlink audio on
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	)
{
	UInt8 tmp_vp_speech_amr_mode;

    // global used to limit recording/playback to fixed number of buffers
    rec_buffer_count = 0;
    frame_count = 0;

#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
    // Val5 = vp_speech_amr_mode = [(7:7) AMR2_dtx | (6:4) vocoder | (3:0) amr_mode]
    //      = [0x30 0x50...0x57] = [48 80...87] = [PCM AMR475...AMR122] without dtx
    //   or = [     0xd0...0xd7] = [   208...215] = [  AMR475...AMR122] with dtx
    // note: for PCM, dtx is N/A
	// protection for unsupported mode, set to default PCM mode
	if(	( (vp_speech_amr_mode >= 0x50) && (vp_speech_amr_mode <= 0x57) ) ||
		( (vp_speech_amr_mode >= 0xd0) && (vp_speech_amr_mode <= 0xd7) ) )
	{
		tmp_vp_speech_amr_mode = vp_speech_amr_mode;
	}
	else
	{
		tmp_vp_speech_amr_mode = 0x30;
	}
	speech_mode = (VP_Speech_Mode_t) ((tmp_vp_speech_amr_mode>>4) & 0x07);     // VP_SPEECH_MODE_LINEAR_PCM or VP_SPEECH_MODE_AMR_2
#else // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
    speech_mode = VP_SPEECH_MODE_LINEAR_PCM;
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,0) /* BCM2132 and later */

    memset(&temp_speech_store[0],0,sizeof(temp_speech_store));

#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	if(downlink_audio_off)
		idle_mode_f = TRUE;
	else
		idle_mode_f = FALSE;
#else // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	idle_mode_f = FALSE;
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */

	vpu_mode = VPU_MODE_CALL_RECORDING;

#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	if(downlink_audio_off)
	    	VPU_AudioTurnOn(TRUE,FALSE);
	else
	    	VPU_AudioTurnOn(TRUE,TRUE);
#else // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
    VPU_AudioTurnOn(TRUE,TRUE);
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */

	VPRIPCMDQ_StartCallRecording(vp_record_mode, buffer_length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
		, tmp_vp_speech_amr_mode
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
		);
}

//******************************************************************************
//
// Function Name:  VPU_StartCallRecordAndPlayback()
//
// Description: This function starts continuous call recording and playback
//
// Notes:
//
//******************************************************************************

void VPU_StartCallRecordAndPlayback(// starts continuous call recording and playback
	UInt8 	vp_playback_mode,       // playback to speaker/ear-piece, channel or both
	UInt8 	vp_record_mode,			// record from mic, channel or both
	UInt8	buffer_length			// Buffer length
	)
{
    UInt8 i;

    // global used to limit recording/playback to fixed number of buffers
    rec_buffer_count = 0;
    play_buffer_count = 0;
    frame_count = 0;
	speech_mode = VP_SPEECH_MODE_LINEAR_PCM;
	idle_mode_f = FALSE;
	vpu_mode = VPU_MODE_RECORD_AND_PLAYBACK;

	for ( i = 0; i < 4; i++)
	{
        memcpy(
        (UInt16 *) &vp_shared_mem->shared_voice_buf.vp_buf[0].vp_frame.vectors_lin_PCM[i].param,
        &(temp_speech_store[play_buffer_count]),
        (160*sizeof(UInt16)));
        play_buffer_count += 160 ;
	}
	vp_shared_mem->shared_voice_buf.vp_buf[0].vp_speech_mode = speech_mode;
	vp_shared_mem->shared_voice_buf.vp_buf[0].nb_frame = 4;

	for ( i = 0; i < 4; i++)
	{
        memcpy(
        (UInt16 *) &vp_shared_mem->shared_voice_buf.vp_buf[1].vp_frame.vectors_lin_PCM[i].param,
        &(temp_speech_store[play_buffer_count]),
        (160*sizeof(UInt16)));
        play_buffer_count += 160 ;
	}	    
	vp_shared_mem->shared_voice_buf.vp_buf[1].vp_speech_mode = speech_mode;
	vp_shared_mem->shared_voice_buf.vp_buf[1].nb_frame = 4;
	frame_count += 8;

#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	// if memo_record_and_playback, use idle_mode_f = TRUE to turn off audio when finish
	if( ((buffer_length>>4)&0x01) == 1) idle_mode_f = TRUE;	
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */

    VPU_AudioTurnOn(TRUE,TRUE);
	VPRIPCMDQ_StartCallRecordAndPlayback(vp_playback_mode, vp_record_mode, buffer_length);
}

//******************************************************************************
//
// Function Name: VPU_StartMemolRecording
//
// Description: THis function starts memo recording
//
// Notes:
//
//******************************************************************************

void VPU_StartMemolRecording(			// Start memo recording
	VP_Speech_Mode_t 	vp_speech_mode,
  	UInt8				buffer_length
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8				dtx_vp_amr_mode
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
	VP_Mode_AMR_t		vp_mode_amr
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	)
{
	VP_Speech_Mode_t 	tmp_vp_speech_mode;

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	PR_MST_CS1_status |= 1;
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	// for MemoRecording, only use VP_SPEECH_MODE_AMR
	if( vp_speech_mode == VP_SPEECH_MODE_AMR_2 ) tmp_vp_speech_mode = VP_SPEECH_MODE_AMR;
	else										 tmp_vp_speech_mode = vp_speech_mode;
#else // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	tmp_vp_speech_mode = vp_speech_mode;
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */

    speech_mode = tmp_vp_speech_mode;
	idle_mode_f = TRUE;
    frame_count = 0;
    memset(&temp_speech_store[0],0,sizeof(temp_speech_store));
    vpu_mode = VPU_MODE_MEMO_RECORDING;
    rec_buffer_count = 0;

	VPU_AudioTurnOn(TRUE,FALSE);

	VPRIPCMDQ_StartMemolRecording(tmp_vp_speech_mode,buffer_length
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,dtx_vp_amr_mode
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
		,vp_mode_amr
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
		);
}

//******************************************************************************
//
// Function Name: VPU_StopRecording()
//
// Description: This function stops call/memo recording
//
// Notes:
//
//******************************************************************************

void VPU_StopRecording()		// stops call/memo recording
{
	VPRIPCMDQ_StopRecording();

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	if (PR_MST_CS1_status & 1)
		Resume_MSTCS1_flag();
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
}


//******************************************************************************
//
// Function Name: VPU_CancelRecording()
//
// Description: This function cancel call/memo recording. Since this fuction
//				dose not wait last buffer from RIP, it is faster to relase
//				OAK resouce
//
// Notes:
//
//******************************************************************************

void VPU_CancelRecording()		// cancel call/memo recording
{
	
	VPRIPCMDQ_CancelRecording();

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	if (PR_MST_CS1_status & 1)
		Resume_MSTCS1_flag();
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

	if(	vpu_mode == VPU_MODE_MEMO_RECORDING)
	{
		VPU_AudioTurnOff();	
	}
}

//******************************************************************************
//
// Function Name:  VPU_StartRecordingPlayback()
//
// Description:		This function starts recording playback
//
// Notes:
//
//******************************************************************************

void VPU_StartRecordingPlayback(   	//  starts recording playback
	UInt8 	vp_playback_mode	   	// Audio path
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8	disable_tch_vocoder		// TURE: disable TCH voice functions (for AMR memo playback in idle mode)
	,UInt8 	downlink_audio_off		// 1: downlink audio off; 0: downlink audio on
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,VP_Mode_AMR_t	vp_mode_amr		// amr codec mode
	,UInt8 	downlink_audio_off		// 1: downlink audio off; 0: downlink audio on
#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	)
{
	VPlayBack_Buffer_t *pBuf;
	UInt8 i;

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	PR_MST_CS1_status |= 1;
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

	vpu_mode = VPU_MODE_RECORDING_PLAYBACK;

    // global used to limit recording/playback to fixed number of buffers
    play_buffer_count = 0;
    frame_count = 0;

	for (i = 0; i < 2; i++)
	{
		pBuf = &vp_shared_mem->shared_voice_buf.vp_buf[i];
		pBuf->nb_frame = 4;
		pBuf->vp_speech_mode = speech_mode;
		memset(&pBuf->vp_frame, 0, sizeof(pBuf->vp_frame));
	}

#if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */
	if(downlink_audio_off)
		idle_mode_f = TRUE;			// to turn off audio when finishing playback (in idle/data mode)
	else
		idle_mode_f = FALSE;
#else // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */
	idle_mode_f = FALSE;
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */

    VPU_AudioTurnOn(TRUE,TRUE);
	VPRIPCMDQ_StartRecordingPlayback(vp_playback_mode
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	, disable_tch_vocoder		// TURE: disable TCH voice functions (for AMR memo playback in idle mode)
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	, vp_playbackmix_mode
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	);
}


//******************************************************************************
//
// Function Name:  VPU_StopPlayback()
//
// Description:		This function stops playback
//
// Notes:
//
//******************************************************************************

void VPU_StopPlayback(		// stops  playback
	UInt8	flag
)
{

	memset((void*)vp_shared_mem->shared_voice_buf.vp_buf, 0, sizeof(vp_shared_mem->shared_voice_buf.vp_buf));
	VPRIPCMDQ_StopPlayback(flag);

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	if (PR_MST_CS1_status & 1)
		Resume_MSTCS1_flag();
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
}


//******************************************************************************
//
// Function Name:  VPU_CancelPlayback()
//
// Description:		This function stops playback
//
// Notes:
//
//******************************************************************************

void VPU_CancelPlayback()		// cancel  playback
{

	VPRIPCMDQ_CancelPlayback();

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	if (PR_MST_CS1_status & 1)
		Resume_MSTCS1_flag();
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

	VPU_AudioTurnOff();
}



#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO) )
//read the status and deliver it to AP.
void CP_Process_VPU_Status(void)
{
	DSPCMD_t	dspCmd;

	dspCmd.type = DSPCMD_TYPE_SEND_IPC_AUDIO_CTRL;

	DSPCore_PostCmd(&dspCmd);
}

void CP_Process_VPU_Status_In_Task(void)
{
	VPStatQ_t vpu_status_msg;
    UInt8 nFrames;
    UInt8 i;

	while ( VPSHAREDMEM_ReadStatusQ( &vpu_status_msg ) )
	{
		MSG_LOGV("VPU_VPSHAREDMEM_ReadStatusQ= %d \r\n", vpu_status_msg.status);

		Audio_VPU_ISR_Handler(vpu_status_msg);
	}
}
#endif


//VPU function calls go from AP to CP
//VPU status go from CP to AP.
//VPU data are transfered between DSP and AP.

// In FUSE software system, AP process the vpu status msg coming from CP.

void AP_Process_VPU_Status( VPStatQ_t vp_status_msg)
{
	VPlayBack_Buffer_t *pPlayBuf;
	VRecording_Buffer_t	*pRecordBuf;
    UInt8 nFrames;
    UInt8 i;

	switch ( vp_status_msg.status )
	{
		case VP_STATUS_RECORDING_DATA_READY:


#if  defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_)
			{
				StatQ_t msg;
				if ( vpu_mode == VPU_MODE_VM_PLAYCALL)
				{
					msg.status	= VPU_STATUS_RECORDING_DATA_READY;
					msg.arg0	= vp_status_msg.arg0;
					msg.arg1	= vp_status_msg.arg1;
					msg.arg2	= vp_status_msg.arg2;
					Audio_ISR_Handler(msg);
					break;
				}
			}
#endif
		  //point to ping-pong buf of recording data
			pRecordBuf = &vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0];
			nFrames = pRecordBuf->nb_frame;
			if(nFrames > 4) nFrames = 0;

			switch(vpu_mode)
			{
			case VPU_MODE_VM_RECORDMEMO:
				//SIO_PutString( PortB, (UInt8 *) "VPU_RECMEMO\r\n");
			case VPU_MODE_VM_RECORDCALL:
				//SIO_PutString( PortB, (UInt8 *) "VPU_RECCALL\r\n");
				if (DumpVPFramesCB)
					DumpVPFramesCB((UInt8*)&pRecordBuf->vr_frame, nFrames);
				break;

			default: // for any recording-test mode
					if ( (frame_count < (MAX_VAL_FOR_TESTING-4))

#ifdef	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
#if	0		
					|| (aacenc_enabled == 1)
#endif	//
#endif	//	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE	
					)
				{
					frame_count += nFrames;
					for ( i = 0; i < nFrames; i++)
					{
						if ( vpu_mode == VPU_MODE_MEMO_RECORDING )
						{		
							if ( speech_mode == VP_SPEECH_MODE_EFR )
							  {
							    memcpy(&(temp_speech_store[rec_buffer_count]), 		(UInt16 *) &vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0].vr_frame.vectors_efr[i], sizeof(VR_Frame_EFR_t));							 
							    rec_buffer_count += sizeof(VR_Frame_EFR_t)>>1;	

							  }
							else if ( speech_mode == VP_SPEECH_MODE_FR )
							  {
							    memcpy(&(temp_speech_store[rec_buffer_count]), (UInt16 *) &vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0].vr_frame.vectors_fr[i], sizeof(VR_Frame_FR_t));							 		rec_buffer_count += sizeof(VR_Frame_FR_t)>>1;
							  }
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */
							else if ( speech_mode == VP_SPEECH_MODE_AMR )
							{
								memcpy(
									(void*) &(temp_speech_store[rec_buffer_count]),
									(void*) &(vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0].vr_frame.vectors_amr[i]),
									sizeof(VR_Frame_AMR_t)
									);
								rec_buffer_count += sizeof(VR_Frame_AMR_t)>>1;
							}
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */
							else
							{
			
								memcpy(&(temp_speech_store[rec_buffer_count]),
								(UInt16 *) &vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0].vr_frame.vectors_lin_PCM[i].param,
								(160*sizeof(UInt16)));	
								rec_buffer_count += 160;
		
#ifdef	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
#endif		//	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE			
							}
						}
						else
						{
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
							if ( speech_mode == VP_SPEECH_MODE_AMR_2 )
							{
								memcpy(
									(void*) &(temp_speech_store[rec_buffer_count]),
									(void*) &(vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0].vr_frame.vectors_amr[i]),
									sizeof(VR_Frame_AMR_t)
									);
								rec_buffer_count += sizeof(VR_Frame_AMR_t)>>1;
							}
							else
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
							{
								memcpy(&(temp_speech_store[rec_buffer_count]),
								(UInt16 *) &vp_shared_mem->shared_voice_buf.vr_buf[vp_status_msg.arg0].vr_frame.vectors_lin_PCM[i].param,
								(160*sizeof(UInt16)));
								rec_buffer_count += 160;
							}
		
						}
					}
				}
				else
				{
					VPU_CancelRecording();
				}
			}

			break;

		case VP_STATUS_PLAYBACK_DATA_EMPTY:
			switch(vpu_mode)
			{
			case VPU_MODE_VM_PLAYMEMO:
			case VPU_MODE_VM_PLAYCALL:
				pPlayBuf = &vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0];
				pPlayBuf->vp_speech_mode = speech_mode;
				pPlayBuf->nb_frame = PLAYBACK_BLOCK;
				if (FillVPFramesCB)
					FillVPFramesCB( (UInt8*)&pPlayBuf->vp_frame, PLAYBACK_BLOCK);
				break;

			case VPU_MODE_RECORDING_PLAYBACK:
			case VPU_MODE_RECORD_AND_PLAYBACK:
				if ( frame_count < (MAX_VAL_FOR_TESTING-4) )
				{
					frame_count += 4;		
					for ( i = 0; i < 4; i++)
					{
						if ( speech_mode == VP_SPEECH_MODE_EFR )
						{
							memcpy(
							(UInt16 *) &vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_frame.vectors_efr[i],
							&(temp_speech_store[play_buffer_count]),
							sizeof(VR_Frame_EFR_t));	
							play_buffer_count += sizeof(VR_Frame_EFR_t)>>1 ;		
						}
						else if ( speech_mode == VP_SPEECH_MODE_FR )
						{
							memcpy(
							(UInt16 *) &vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_frame.vectors_fr[i],
							&(temp_speech_store[play_buffer_count]),
							sizeof(VR_Frame_FR_t));	
							play_buffer_count += sizeof(VR_Frame_FR_t)>>1 ;		
						}
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
						else if ( speech_mode == VP_SPEECH_MODE_AMR )
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
						else if ( speech_mode == VP_SPEECH_MODE_AMR || speech_mode == VP_SPEECH_MODE_AMR_2 )
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */
						{
							memcpy(
								(void*) &(vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_frame.vectors_amr[i]),
								(void*) &(temp_speech_store[play_buffer_count]),
								sizeof(VR_Frame_AMR_t)
								);
							play_buffer_count += sizeof(VR_Frame_AMR_t)>>1;

							if( speech_mode == VP_SPEECH_MODE_AMR )
								vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_speech_mode = VP_SPEECH_MODE_AMR;
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
							if( speech_mode == VP_SPEECH_MODE_AMR_2 )
								vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_speech_mode = VP_SPEECH_MODE_AMR_2;
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
						}
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) /* BCM2121 and later */
						else
						{
							memcpy(
							(UInt16 *) &vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_frame.vectors_lin_PCM[i].param,
							&(temp_speech_store[play_buffer_count]),
							(160*sizeof(UInt16)));	
							play_buffer_count += 160 ;		
						}
					}
					vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].vp_speech_mode = speech_mode;
					vp_shared_mem->shared_voice_buf.vp_buf[vp_status_msg.arg0].nb_frame = 4;
				}
				else
				{
					// VPRIPCMDQ_StopPlayback(idle_mode_f); 	 // 				   VPU_AudioTurnOff();
					VPRIPCMDQ_CancelPlayback();
					if(idle_mode_f) VPU_AudioTurnOff();
				}			
				break;
			}
			break;

		default :
			break;
	}
}


//******************************************************************************
//
// Function Name:	VPU_ProcessStatus
//
// Description:		This function is called by  high level RIP interrupt service routine
//					to  handles VP status queue
//
// Notes:
//
//******************************************************************************

void VPU_ProcessStatus( void )	// Process  VP status queue
{
	VPStatQ_t status_msg;

	while ( VPSHAREDMEM_ReadStatusQ( &status_msg ) )
	{
		pr_info("VPU_VPSHAREDMEM_ReadStatusQ= %d \r\n", status_msg.status);

		AP_Process_VPU_Status( status_msg );
	}
}

//******************************************************************************
//
// Function Name:  VPU_StartPlaybackMemo()
//
// Description:		This function starts memo recording
//
// Notes:
//
//******************************************************************************
Boolean VPU_StartPlaybackMemo(					// starts memo playback
	VPUFillFramesCB_t	FillFramesCB,		// callback function to fill playback frame
	VP_Speech_Mode_t 	vp_speech_mode,		// speech mode
	UInt8				audioPath			// audio path
	)
{
	VPlayBack_Buffer_t *pBuf;
	UInt8 i;

#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	PR_MST_CS1_status |= 1;			//bit 0: PR status	//bit 1: MST CS1 flag;
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

	FillVPFramesCB = FillFramesCB;
    speech_mode = vp_speech_mode;

#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	// force to use AMR_2 to enable playback during the call
	if (speech_mode == VP_SPEECH_MODE_AMR) speech_mode = VP_SPEECH_MODE_AMR_2;
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */

	for (i = 0; i < 2; i++)
	{
		pBuf = &vp_shared_mem->shared_voice_buf.vp_buf[i];
		pBuf->nb_frame = PLAYBACK_BLOCK;
		pBuf->vp_speech_mode = speech_mode;
		memset(&pBuf->vp_frame, 0, sizeof(pBuf->vp_frame));
	}

	VPU_AudioTurnOn(TRUE,TRUE);
	vpu_mode = (audioPath == 1) ? VPU_MODE_VM_PLAYMEMO : VPU_MODE_VM_PLAYCALL;

	VPRIPCMDQ_StartRecordingPlayback(audioPath
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	, 1		// TURE: disable TCH voice functions (for AMR memo playback in idle mode)
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	, MIX_NONE		//VP_PlaybackMix_Mode_t defined in shared.h
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	);

	return TRUE;
}


//******************************************************************************
//
// Function Name:  VPU_StartRecordingMemo()
//
// Description:		This function starts memo recording
//
// Notes:
//
//******************************************************************************
Boolean VPU_StartRecordingMemo(				// start memo recording
	VPUDumpFramesCB_t	DumpFramesCB,		// callback function to dump recorded frame
	VP_Speech_Mode_t 	vp_speech_mode,		// speech mode
	UInt8				audioPath,			// audio path
	UInt32				nParam				// speech mode-specific parameters
											// for AMR it is VP_Mode_AMR_t
	)
{
#if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	PR_MST_CS1_status |= 1;
#endif // #if !( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

	DumpVPFramesCB = DumpFramesCB;
    speech_mode = vp_speech_mode;

#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	VPU_AudioTurnOn(TRUE, (audioPath == 1) ? FALSE : TRUE);
	vpu_mode = (audioPath == 1) ? VPU_MODE_VM_RECORDMEMO : VPU_MODE_VM_RECORDCALL;

	//first init VPU NS
	VPU_NS_Init(speech_mode);

	{
		// AMR can only be recorded during speech mode
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
		if (VPU_NS_Status)
			VPRIPCMDQ_StartCallRecording(
				audioPath,
				RECORDING_BLOCK,
				(vp_speech_mode == VP_SPEECH_MODE_LINEAR_PCM) ? (0x30|BIT08_MASK):((0x50 + nParam)|BIT08_MASK)
				);
		else
			VPRIPCMDQ_StartCallRecording(
				audioPath,
				RECORDING_BLOCK,
				(vp_speech_mode == VP_SPEECH_MODE_LINEAR_PCM) ? 0x30 : 0x50 + nParam
				);
#else	
		VPRIPCMDQ_StartCallRecording(
			audioPath,
			RECORDING_BLOCK,
			(vp_speech_mode == VP_SPEECH_MODE_LINEAR_PCM) ? 0x30 : 0x50 + nParam
			);
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))
	}
#else // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	if(audioPath == 1)
	{
		// turn on up link
		VPU_AudioTurnOn(TRUE,FALSE);
		vpu_mode = VPU_MODE_VM_RECORDMEMO;
		VPRIPCMDQ_StartMemolRecording(speech_mode,RECORDING_BLOCK
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	, 1		// TURE: disable TCH voice functions (for AMR memo playback in idle mode)
#endif // #if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
		);
	}
	else
	{
		// both link must have been on during the call
		vpu_mode = VPU_MODE_VM_RECORDCALL;
		VPRIPCMDQ_StartCallRecording(speech_mode, RECORDING_BLOCK);
	}
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */


	return TRUE;
}


//******************************************************************************
//
// Function Name:  VPU_StopMemo()
//
// Description:	   This function stops voice memo related activity
//
// Notes:
//
//******************************************************************************
Boolean VPU_StopMemo()
{
	
	switch(vpu_mode)
	{
	case VPU_MODE_VM_PLAYMEMO:
		VPU_CancelPlayback();
		VPU_AudioTurnOff();
		break;

	case VPU_MODE_VM_PLAYCALL:
		VPU_CancelPlayback();
		VPU_AudioTurnOff();
		break;

	case VPU_MODE_VM_RECORDMEMO:
		VPU_CancelRecording();			// use CancelRecording for faster stop
		VPU_AudioTurnOff();
		break;

	case VPU_MODE_VM_RECORDCALL:
		VPU_CancelRecording();			// use CancelRecording for faster stop
		VPU_AudioTurnOff();
		break;

	default:
		return FALSE;
	}

	FillVPFramesCB = NULL;
	DumpVPFramesCB = NULL;
	vpu_mode = VPU_MODE_IDLE;

	return TRUE;
}

#if 1 //def	WANT_VT
void GPTimerRegister(GP_CB_FUNC gpt_cb)
{
  //the_gpt_cb=gpt_cb;
}


void GPTimerUnRegister()
{
  //the_gpt_cb=NULL;
}

static void VTTIMER_Cbk(void)
{

	//if(gAppGPTimerCb)
		//gAppGPTimerCb(gVT_Tid);

}
void VTTIMER_Start()
{
    GPT_REG_T *gptreg;
    gptreg = (GPT_REG_T *)gpthwreg.hwregs;
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_rr = 0xffffffff;
  
    //power up the timer
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_TIMER_PWRON;

    //26Mhz
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_CLKSEL;
  
    // Use output1
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_INT2_ASSIGN;

    //enable interrupt
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_TIMER_ENABLE;

    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_rr = (0x6590)*20;

	 gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_INT_ENABLE;
  
    //power up the timer
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_TIMER_PWRON;

}

void VTTIMER_Stop (void)
{
#ifdef USE_GP_TIMER
    GPT_REG_T *gptreg;
    gptreg = (GPT_REG_T *)gpthwreg.hwregs;
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr &= ~(GPT_CSR_INT_ENABLE|GPT_CSR_TIMER_ENABLE);
	 //power down the timer
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr &= ~(GPT_CSR_TIMER_PWRON);
#endif
}

static void VT_Task_func()
{
#if 0
   const UInt8 telephonyLenAMR[16] = { 13, 14, 16, 18, 19, 21, 26, 31,
				      6,  6,  6,  6,  0,  0,  0,  1 };
  UInt8 amr_mode;

  if (Telephony_FillVPFramesCB)
    Telephony_FillVPFramesCB((UInt8 *)&vt_fill_ptr, 1);

  if (vt_fill_ptr==NULL) return;

  amr_mode = (vt_fill_ptr[0]) & 0x0f;

  assert(amr_mode <= 8 || amr_mode == 15);

  VPU_StartMainAMRDecodeEncode(amr_mode, vt_fill_ptr,
			       telephonyLenAMR[amr_mode],
			       telephony_encode_amr_mode, FALSE);
  VPU_VT_Clear();
#endif
}

#ifdef USE_GP_TIMER

static void VT_Task_Entry()
{
  //while (1) {
    //OSSEMAPHORE_Obtain(vt_sema_id, TICKS_FOREVER);
    //VT_Task_func();
  //}
}

irqreturn_t vt_gptimer_hisr(int irq, void *dev_id)
{
    hxgptreg_t *dev = (hxgptreg_t *) dev_id;
    GPT_REG_T *gptreg;
	dev->gpt_irq_count++;
	//pr_warning("  vt_hisr %d\n",dev->gpt_irq_count);
	queue_work(dev->intr_workqueue, &(dev->intr_work));
	/* clear dec IRQ & enable bit in HW reg */

    gptreg = (GPT_REG_T *)dev->hwregs;
    gptreg->gpt_reg[VT_USE_GPT_INDEX].gpt_csr |= GPT_CSR_INT_FLG;

	return IRQ_HANDLED;
}

static void vt_gptimer_lisr()
{
  //if (vt_intr_t) {
    //OSINTERRUPT_Trigger(vt_intr_t);
  //}
}

static void vt_gpt_20mstimer_init()
{
    int result;
    gpthwreg.iobaseaddr = GPTIMER_BASE_ADDR;
    gpthwreg.iosize = sizeof(GPT_REG_T);
    if (!request_mem_region(gpthwreg.iobaseaddr, gpthwreg.iosize, "gptreg")) 
    {
		_DBG_(TRACE_Printf_Sio( "request_mem_region fai \r\n"));
        return;
    }
    gpthwreg.hwregs =(volatile u8 *)ioremap_nocache(gpthwreg.iobaseaddr,gpthwreg.iosize);
    gpthwreg.gpt_irq_count=0;
    gpthwreg.intr_workqueue = create_workqueue("vt-gpt");
    if (!gpthwreg.intr_workqueue)
    {
		_DBG_(TRACE_Printf_Sio( "create_workqueue vt-gpt Error \r\n"));
    } 

    result = request_irq(IRQ_GPT2, vt_gptimer_hisr, IRQF_NO_SUSPEND ,  "gptvt20ms", (void *)&gpthwreg);
    if (result == -EINVAL) 
    {
		_DBG_(TRACE_Printf_Sio( "result == -EINVAL \r\n"));
		return;
    }
    else if (result == -EBUSY) 
    {
		_DBG_(TRACE_Printf_Sio( " result == -EBUSY \r\n"));
		return;
    }

    VTTIMER_Start();
}

static void vt_gpt_20mstimer_deinit()
{
    VTTIMER_Stop();
}

#else

static void ProcessVPU20msTimer(Timer_t	id)
{
  VT_Task_func();
}
#endif

#endif //WANT_VT

#if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C2 (with patch) and later */
//******************************************************************************
//
// Function Name:  VPU_StartTelephony()
//
// Description:		This function starts full duplex telephony session
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
Boolean VPU_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUDumpFramesCB_t telephony_fill_cb,
	VP_Mode_AMR_t	 encode_amr_mode,  // AMR mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE)
	Boolean	     amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format
	)
{
	Telephony_DumpVPFramesCB = telephony_dump_cb;
	Telephony_FillVPFramesCB = telephony_fill_cb;
	telephony_dtx_mode = dtx_mode;
	telephony_amr_if2 = amr_if2_enable;
	telephony_encode_amr_mode = encode_amr_mode;

#if CHIPVERSION == CHIP_VERSION(BCM2132,32) /* with BCM2132C2 patch */
	DSPCMD_RIPWrite(0xE012,0x3CD6);
	DSPCMD_RIPWritePRAM(0xBC0C,0x3CD8);
	DSPCMD_RIPWritePRAM(0xBC04,0xBE5C);
#endif


#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
	//AP sends a message to CP. Upon receiving the message,
	// CP will set the fuse_ap_vt_active flag. DSP interrupts to CP will be forwarded
	// to the AP through IPC (see VPU_ProcessStatusMainAMRDone()).

	VPRIPCMDQ_SetCPVTStartStopFlag(1);
#endif
	// initialize AMR codec

	VPRIPCMDQ_VocoderInit(SPEECH_MODE_AFS);
	VPRIPCMDQ_SetVoiceDtx(dtx_mode);
	SHAREDMEM_DLAMR_Speech_Init();
	SHAREDMEM_ULAMR_Speech_Init();

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
	VPRIPCMDQ_DSPAudioAlign(1);
#endif

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132C3 and later */
 	//Clear DL double buffer in VT;
 	{
 	    UInt8 i;
 
 	    for (i=0; i<64; i++)
 	    {
 	        dl_buffer[i] = 0;
 	    }
 	    for (i=0; i<2; i++)
 	    {
 		    len[i] = 0;
 		    dec_mode[i] = 0;
 		    enc_mode[i] = encode_amr_mode;
 	    }
 	    VT_flag = FALSE;
 	    prv_VT_flag = FALSE;
 		ping_pong_rd_ind = 0;
 		ping_pong_wr_ind = 1;
        VT_active_flag = TRUE;
 	}
#else
	VPRIPCMDQ_SetDSP_AMR_RUN(encode_amr_mode, amr_if2_enable);
#endif

#if 0 // def WANT_VT
#ifdef USE_GP_TIMER

	vt_sema_id = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	vt_task_id = OSTASK_Create(VT_Task_Entry, "VT Task",
				    NORMAL-2, 1024);

	vt_intr_t = OSINTERRUPT_Create(vt_gptimer_hisr,
				       "VT timer HISR",
				       IPRIORITY_HIGH,
				       1024*4);
	vt_gpt_20mstimer_init();

#else

	if( !VPU_20ms_timer )
	  VPU_20ms_timer = OSTIMER_Create( ProcessVPU20msTimer, 0,
					    TICKS_ONE_SECOND/50, TICKS_ONE_SECOND/50 );
	OSTIMER_Reset(VPU_20ms_timer);
	OSTIMER_Start(VPU_20ms_timer);
#endif
#endif //WANT_VT
	return TRUE;
}

//******************************************************************************
//
// Function Name:  VPU_IsAmrIf2()
//
// Description:		This function give full duplex telephony session AMR format
//
// Notes:
//
//******************************************************************************
Boolean VPU_IsAmrIf2(void)
{
	return telephony_amr_if2;
}

//******************************************************************************
//
// Function Name:  VPU_StartMainAMRDecodeEncode()
//
// Description:		This function passes the AMR frame to be decoded
//					from application to DSP and starts its decoding
//					as well as encoding of the next frame.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
void VPU_StartMainAMRDecodeEncode(
	VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
	UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
	UInt8				length,		// number of bytes of the AMR speech data to be decoded
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
	)
{

	// update dtx mode if necessary
	if (telephony_dtx_mode != dtx_mode)
	{
		telephony_dtx_mode = dtx_mode;
		VPRIPCMDQ_SetVoiceDtx(dtx_mode);
	}

 	//Add double buffer in DL VT;
     {
         UInt8 i;

         len[ping_pong_wr_ind] = (length+3)/4;
         dec_mode[ping_pong_wr_ind] = decode_amr_mode;
         enc_mode[ping_pong_wr_ind] = encode_amr_mode;
         for (i=0; i<32; i++)
         {
             dl_buffer[(ping_pong_wr_ind<<5)+i] = pBuf[i];
         }
     }
     prv_VT_flag = VT_flag;
     VT_flag = TRUE;
     ping_pong_wr_ind=!ping_pong_wr_ind;

     // decode the next downlink AMR speech data from application
     SHAREDMEM_WriteDL_AMR_Speech(1, VPU_VT_dec_mode(), VPU_VT_ptr(),
				  VPU_VT_len(), telephony_amr_if2, NULL,NULL);
     // signal DSP to start AMR decoding and encoding
     VPRIPCMDQ_DSP_AMR_RUN(VPU_VT_enc_mode(), telephony_amr_if2, FALSE);

}

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132C3 and later */
//******************************************************************************
//
// Function Name:  VPU_VT_Activ()
//
// Description:		This function check if full duplex telephony session is active
//
// Notes:
//
//******************************************************************************
Boolean VPU_VT_Activ(void)
{
	return VT_active_flag;
}

//******************************************************************************
//
// Function Name:  VPU_VT_Clear()
//
// Description:		This function clear full duplex telephony session flag
//
// Notes:
//
//******************************************************************************
void VPU_VT_Clear(void)
{
	prv_VT_flag = FALSE;
	ping_pong_rd_ind=!ping_pong_rd_ind;
}

//******************************************************************************
//
// Function Name:  VPU_VT_enc_mode()
//
// Description:		This function get full duplex telephony session	AMR uplink mode
//
// Notes:
//
//******************************************************************************
UInt8 VPU_VT_enc_mode(void)
{
	return enc_mode[ping_pong_rd_ind];
}

//******************************************************************************
//
// Function Name:  VPU_VT_dec_mode()
//
// Description:		This function get full duplex telephony session	AMR downlink mode
//
// Notes:
//
//******************************************************************************
UInt8 VPU_VT_dec_mode(void)
{
	return dec_mode[ping_pong_rd_ind];
}

//******************************************************************************
//
// Function Name:  VPU_VT_len()
//
// Description:		This function get full duplex telephony session	AMR downlink length
//
// Notes:
//
//******************************************************************************
UInt8 VPU_VT_len(void)
{
	return len[ping_pong_rd_ind];
}

//******************************************************************************
//
// Function Name:  VPU_VT_ptr()
//
// Description:		This function get full duplex telephony session	AMR downlink buffered data
//
// Notes:
//
//******************************************************************************
UInt32 *VPU_VT_ptr(void)
{
	return (UInt32 *)(&dl_buffer[ping_pong_rd_ind<<5]);
}
#endif

//******************************************************************************
//
// Function Name:  VPU_ProcessStatusMainAMRDone()
//
// Description:		This function handles the processing needed after ARM receives
//					STATUS_MAIN_AMR_DONE from DSP.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************

void VPU_ProcessStatusMainAMRDone(StatQ_t status_msg)
{
    StatQ_t msg;
    UInt32 *pBuf;
    msg=status_msg;

#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
    if (((status_msg.arg0 & 0xf000) == VOIP_PCM)            // VoIP PCM
		|| ((status_msg.arg0 & 0xf000) == VOIP_FR)     // VoIP FR
		|| ((status_msg.arg0 & 0xf000) == VOIP_AMR475))   // VoIP AMRNB
	{
		VoIP_ProcessStatusMainAMRDone(status_msg);
		return;
	}
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))


    if(fuse_ap_vt_active)
    {
	    msg.status	= STATUS_MAIN_AMR_DONE;
		  msg.arg0	= 0;
		  msg.arg1	= 0;
		  msg.arg2	= 0;
	    Audio_ISR_Handler(msg);
	  }
	  else
	  {
	    // encoded uplink AMR speech data now ready in DSP shared memory, copy it to application
	    // pBuf is to point the start of the encoded speech data buffer
	    SHAREDMEM_ReadUL_AMR_Speech(&pBuf, telephony_amr_if2);
	    if (Telephony_DumpVPFramesCB)
		    Telephony_DumpVPFramesCB((UInt8*)pBuf, SHAREDMEM_ReadUL_AMR_Mode());
		}
	
#else



	// encoded uplink AMR speech data now ready in DSP shared memory, copy it to application
	// pBuf is to point the start of the encoded speech data buffer
	SHAREDMEM_ReadUL_AMR_Speech(&pBuf, telephony_amr_if2);
	if (Telephony_DumpVPFramesCB)
		Telephony_DumpVPFramesCB((UInt8*)pBuf, SHAREDMEM_ReadUL_AMR_Mode());
#endif
}

//******************************************************************************
//
// Function Name:  VPU_StopTelephony()
//
// Description:		This function stops full duplex telephony session
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
Boolean VPU_StopTelelphony(void)
{
	Telephony_DumpVPFramesCB = NULL;

	the_gpt_cb=NULL;

#if CHIPVERSION == CHIP_VERSION(BCM2132,32) /* with BCM2132C2 patch */
	DSPCMD_RIPWrite(0xE012,0x42E9);
	DSPCMD_RIPWritePRAM(0xBC0C,0x42EA);
	DSPCMD_RIPWritePRAM(0xBC04,0xBCE8);
#endif

	 // clear DSP shared memory
	SHAREDMEM_DLAMR_Speech_Init();
	SHAREDMEM_ULAMR_Speech_Init();

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132C3 and later */
	//Clear DL double buffer in VT;
	{
	    UInt8 i;

		for (i=0; i<64; i++)
		{
		    dl_buffer[i] = 0;
		}
		for (i=0; i<2; i++)
		{
		    len[i] = 0;
		    dec_mode[i] = 0;
		    enc_mode[i] = 0;
		}
		VT_flag = FALSE;
		prv_VT_flag = FALSE;
		ping_pong_rd_ind = 0;
		ping_pong_wr_ind = 1;
		VT_active_flag = FALSE;
	}
#endif

#if 0 //def WANT_VT
#ifdef USE_GP_TIMER
	vt_gpt_20mstimer_deinit();
	if (vt_task_id) {
	  OSTASK_Destroy(vt_task_id);
	  vt_task_id=NULL;
	}
	if (vt_sema_id) {
	  OSSEMAPHORE_Destroy(vt_sema_id);
	  vt_sema_id=NULL;
	}

	if (vt_intr_t) {
	  OSINTERRUPT_Destroy(vt_intr_t);
	  vt_intr_t=NULL;
	}
#else
	OSTIMER_Stop(VPU_20ms_timer);
#endif
#endif //WANT_VT
	Telephony_DumpVPFramesCB=NULL;
	Telephony_FillVPFramesCB=NULL;
	vt_fill_ptr=NULL;

	return TRUE;
}
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C2 (with patch) and later */



#ifdef VPU_INCLUDED
#if !defined(FUSE_COMMS_PROCESSOR)

#define NUM_I2S_BLOCKS 4
#define I2S_DMA_BLOCK 8192 //8192 // Check OMX if 4K smaples is ok, make it large to reduce the DMA interrupt frequency

#define AMR_DECODED_FRAME_SIZE 320
#define VPU_NFRAMES 4
#define UPSAMPLE_RATIO 12

#pragma arm section zidata="uncacheable"
static UInt16 i2s_buffers[NUM_I2S_BLOCKS*I2S_DMA_BLOCK];
#pragma arm section

static int buf_wr_idx=0, buf_rd_idx=0;
static I2S_HANDLE i2s_handle;

static Boolean i2s_tx_started=FALSE;

static Boolean
IIS_From_ADC_tx_callback(Boolean in)
{
  _DBG_(TRACE_Printf_Sio("%s called\r\n", __FUNCTION__ ));

  i2s_queue_tx_buf(i2s_handle, i2s_buffers+buf_rd_idx*I2S_DMA_BLOCK,
		   VPU_NFRAMES*AMR_DECODED_FRAME_SIZE/2);
  buf_rd_idx = (buf_rd_idx+1)%NUM_I2S_BLOCKS;

  return TRUE;
}

static Boolean
IIS_From_ADC_Dump(UInt8*			pSrc,
		  UInt32			nFrames)
{

  _DBG_(TRACE_Printf_Sio("%s called %04x %04x %04x %04x\r\n", __FUNCTION__, pSrc[0], pSrc[1], pSrc[2], pSrc[3]));

  memcpy(i2s_buffers+buf_wr_idx*I2S_DMA_BLOCK, pSrc,nFrames * AMR_DECODED_FRAME_SIZE);

  buf_wr_idx = (buf_wr_idx+1)%NUM_I2S_BLOCKS;

  if (!i2s_tx_started) i2s_start_tx(i2s_handle);
  i2s_tx_started=TRUE;

  return TRUE;
}


Boolean
VPU_EnableI2S_From_ADC(VPU_Link_t vpu_link)
{
  Boolean ret;

  memset(i2s_buffers, 0, sizeof(i2s_buffers));
  i2s_handle=i2s_init(0);
  if (i2s_handle==NULL) return FALSE;

  i2s_register_tx_Cb(i2s_handle, IIS_From_ADC_tx_callback);

  i2s_queue_tx_buf(i2s_handle, i2s_buffers, VPU_NFRAMES*AMR_DECODED_FRAME_SIZE/2);
  i2s_queue_tx_buf(i2s_handle, i2s_buffers+I2S_DMA_BLOCK, VPU_NFRAMES*AMR_DECODED_FRAME_SIZE/2);
  i2s_queue_tx_buf(i2s_handle, i2s_buffers+2*I2S_DMA_BLOCK, VPU_NFRAMES*AMR_DECODED_FRAME_SIZE/2);

  buf_wr_idx=0;
  buf_rd_idx=3;

  i2s_set_mode(i2s_handle, FALSE);

  i2s_set_tx_format(i2s_handle, I2S_SAMPLERATE_8000HZ, FALSE);

  ret = VPU_StartRecordingMemo(IIS_From_ADC_Dump, VP_SPEECH_MODE_LINEAR_PCM, vpu_link, 0);

  return ret;
}


Boolean
VPU_DisableI2S_From_ADC()
{
  Boolean ret;

  i2s_stop_tx(i2s_handle, FALSE);
  i2s_cleanup(i2s_handle);


  ret = VPU_StopMemo();

  i2s_tx_started=FALSE;

  return ret;
}
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////
// VoIP related APIs
//
//
//******************************************************************************
//
// Function Name:  VoIP_Task_func()
//
// Description:		This function handle the VoIP DL data in every 20ms
//
// Notes:			
//******************************************************************************

static void VoIP_Task_func()
{
 	// fill DL data, 1 frame (20ms per dsp interface).
	if (Telephony_FillVPFramesCB)
	{ 
		if ( (gVoIP_CodecType & 0xf000) == 0x1000 )
			Telephony_FillVPFramesCB((UInt8 *)&vt_fill_ptr, 160); 
		if ( (gVoIP_CodecType & 0xf000) == 0x2000 )
			Telephony_FillVPFramesCB((UInt8 *)&vt_fill_ptr, 79);
		if ( (gVoIP_CodecType & 0xf000) == 0x3000 )
			Telephony_FillVPFramesCB((UInt8 *)&vt_fill_ptr, 16);
	}
	if (vt_fill_ptr==NULL) return;
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND))) 
	SHAREDMEM_WriteDL_VoIP_Data(gVoIP_CodecType, (UInt16 *)vt_fill_ptr);
#endif	
	// Send dsp cmd to start decoder: reuse previous API
	VPRIPCMDQ_DSP_AMR_RUN(gVoIP_CodecType, FALSE, FALSE);
}


//******************************************************************************
//
// Function Name:  VoIP_Task_Entry()
//
// Description:		VoIP Task Entry
//
// Notes:			
//******************************************************************************
void VoIP_Task_Entry(struct work_struct *work)
{
	static int lastcnt;
	
	set_user_nice(current, -15);

   if(	lastcnt != gpthwreg.gpt_irq_count)
   	{
		VoIP_Task_func();
		//_DBG_(TRACE_Printf_Sio(" Done gpt_irq_count =%d \d\n",gpthwreg.gpt_irq_count));
   	}
    else
    {
		_DBG_(TRACE_Printf_Sio( "It seems two IST in 20ms, in another word, no IST in previous\r\n"));
    }
    lastcnt = gpthwreg.gpt_irq_count;

}

//******************************************************************************
//
// Function Name:  VoIP_StartTelephony()
//
// Description:		This function starts full duplex VoIP telephony session
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
// this api is called in virtual driver. once called, it will do
// 1. prepare and init
// 2. create the voip task to handle 20ms timer
// 3. create the 20ms timer
// 4. set some flags
// 5. save the voip codec type (passed by upper layer)
Boolean VoIP_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUDumpFramesCB_t telephony_fill_cb,
	UInt16	 voip_codec_type,  // AMR mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
	Boolean	     amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
	)
{
	Telephony_DumpVPFramesCB = telephony_dump_cb;
	Telephony_FillVPFramesCB = telephony_fill_cb;
	telephony_dtx_mode = dtx_mode;
	telephony_amr_if2 = FALSE;
	gVoIP_CodecType = voip_codec_type; // this is just for test, need to finalize with omx
	_DBG_(TRACE_Printf_Sio( "=====VoIP_StartTelephony voip_codec_type=0x%x\r\n", gVoIP_CodecType));

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))  
	// download the voip image for 2153E2 and later
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) && (!defined(BCM2153_FAMILY_BB_CHIP_BOND)))
   VPRIPCMDQ_DownloadVoipImage();
#endif
	//AP sends a message to CP. Upon receiving the message, 
	// CP will set the fuse_ap_vt_active flag. DSP interrupts to CP will be forwarded
	// to the AP through IPC (see VPU_ProcessStatusMainAMRDone()). 
	VPRIPCMDQ_SetCPVTStartStopFlag(1);

	VPRIPCMDQ_DSPAudioAlign(1); 
#endif

#ifdef USE_GP_TIMER
    sema_init(&gpthwreg.vt_irq_sem, 0);
    INIT_WORK(&gpthwreg.intr_work, VoIP_Task_Entry);
    vt_gpt_20mstimer_init(); 
#endif

   return TRUE;
}


//******************************************************************************
//
// Function Name:  VoIP_ProcessStatusMainAMRDone()
//
// Description:		This function handles the processing needed after ARM receives
//					STATUS_MAIN_AMR_DONE from DSP.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
static void VoIP_ProcessStatusMainAMRDone(StatQ_t status_msg)
{
	static UInt16 Buf[160]; // buffer to hold UL data 
	UInt8 data_len;

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))
	if(fuse_ap_vt_active)
    	{
    		Audio_ISR_Handler(status_msg); 
	}
	else
	{			
	    if (Telephony_DumpVPFramesCB) {
		    data_len = SHAREDMEM_ReadUL_VoIP_Data(status_msg.arg0, Buf);
		    Telephony_DumpVPFramesCB((UInt8*)Buf, data_len);
			
		
	    }
	}
#else
	_DBG_(TRACE_Printf_Sio( "=====VoIP_ProcessStatusMainAMRDone status=0x%x 0x%x 0x%x 0x%x", 
		status_msg.status, status_msg.arg0, status_msg.arg1, status_msg.arg2));
    
   // encoded uplink AMR speech data now ready in DSP shared memory, copy it to application
	// pBuf is to point the start of the encoded speech data buffer
	if (Telephony_DumpVPFramesCB) {
		data_len = SHAREDMEM_ReadUL_VoIP_Data(status_msg.arg0, Buf);
		Telephony_DumpVPFramesCB((UInt8*)Buf, data_len);
	}
#endif
}
#endif // #if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))

//******************************************************************************
//
// Function Name:  VoIP_StopTelephony()
//
// Description:		This function stops full duplex telephony session
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
Boolean VoIP_StopTelephony(void)
{    
#ifdef USE_GP_TIMER
    vt_gpt_20mstimer_deinit();
    free_irq(IRQ_GPT2,&gpthwreg);
    flush_workqueue(gpthwreg.intr_workqueue);
    destroy_workqueue(gpthwreg.intr_workqueue);
    release_mem_region(gpthwreg.iobaseaddr, gpthwreg.iosize);
#endif 

    Telephony_DumpVPFramesCB=NULL; 
	 Telephony_FillVPFramesCB=NULL; 
	 vt_fill_ptr=NULL; 

#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND))) 
	VPRIPCMDQ_DSP_CLEAR_VOIPMODE();
#endif
    
	return TRUE;
}

//******************************************************************************
//
// Function Name:  VoIP_SetTelephony_Params()
//
// Description:		This function sets the voip telephony params, such as mode, dtx, codec
//
// Notes:			
//******************************************************************************
Boolean VoIP_SetTelephony_Params(UInt16 param)
{
	// need hookup with virtual driver
	gVoIP_CodecType = param;
	// return ture for now. return false if illegal codec param is passed down later.
	return TRUE;
}

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) && (!defined(BCM2153_FAMILY_BB_CHIP_BOND))) //for 2153 e2 and later
//******************************************************************************
//
// Function Name:  VoIP_DownloadImage()
//
// Description:	This function downloads the voip image to dsp
//
// Notes:		
//******************************************************************************
void VoIP_DownloadImage(void)
{
	static UInt16 voip_image_dl = 0;
	UInt8 *voip_src_addr;
	UInt8 *voip_dst_addr;
	UInt32 voip_length, voip_dma_addr, i;
	UInt16 data, *ptr;
	UInt16 addrH, addrL, len;
	CmdQ_t msg;

	//sprintf( Buf, "509 (voip dma): soft_load_flag = %d",Val2);
	//AT_OutputStr( chan, (UInt8 *)Buf );

	// per dsp test harness
	RIPCMD_DSP_PATCH_OVERLAY( 3, 0, 0 );
	
	voip_src_addr =  (UInt8 *)voip_downloadable_image;
	voip_dst_addr =  (UInt8 *)voip_downloadable_image;
	voip_length = sizeof(voip_downloadable_image);

	//sprintf( Buf, "\n[voip_src_addr, voip_dst_addr, voip_length]: 0x%x 0x%x\n", voip_src_addr, voip_dst_addr, voip_length);	
	//AT_OutputStr( chan, (UInt8 *)Buf );

	voip_dma_addr = (UInt32)(voip_dst_addr - SDRAM_BASE + 0x10000);
	addrH = (voip_dma_addr >> 17) & 0xFFFF;
	addrL = (voip_dma_addr & 0x1FFFF) >> 1;
	len = 0x200;
	//if( Val2 == 1 ) len = 0x8000 | 0x200;
	//else			len = 0x200;
	if (voip_image_dl == 0)
	{
		ptr = (UInt16 *)voip_downloadable_image;
		for(i=0; i <(voip_length)>>1; i++)
		{
			data = *ptr;
			*ptr++ = ((data >> 8) & 0xff) | ((data << 8) & 0xff00);
		}
		voip_image_dl = 1;
	}
	
	//ptr = (UInt16 *)voip_downloadable_image;
	//sprintf( Buf, "\n %x %x %x %x \n", ptr[0], ptr[1], ptr[2], ptr[3]);	
	//AT_OutputStr( chan, (UInt8 *)Buf );

	//sprintf( Buf, "\nDMA address: 0x%x 0x%x 0x%x 0x%x \n", voip_dma_addr, addrH, addrL, len);
	//AT_OutputStr( chan, (UInt8 *)Buf );

	//	Load voip to DSP
	msg.cmd = COMMAND_START_PRAM_FUNCT;
	msg.arg0 = addrH;
	msg.arg1 = addrL;
	msg.arg2 = len;

	SHAREDMEM_PostCmdQ( &msg );
}
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 51) && (!defined(BCM2153_FAMILY_BB_CHIP_BOND)))
#endif //#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated,
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
*
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/
/**
*
*   @file   ripisr.c
*
*   @brief  This file contains all ISR function for ARM(MSP) and DSP(RIP) interface
*
****************************************************************************/

#include <assert.h>
#include "mti_trace.h"
#include "stdio.h"
#include "stdlib.h"
#include "mobcom_types.h"
#include "chip_version.h"
#include "ossemaphore.h"
#include "dsp_feature_def.h"
#include "consts.h"
#include "msconsts.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "ripisr.h"
#include "osinterrupt.h"
#include "memmap.h"
#include "adcmgr_api.h"
#include "sysparm.h"
#include "xassert.h"
#include "log.h"
#include "sio.h"
#include "msdebug_internal.h"
#include "ripproc.h"

//#define ENABLE_LOGGING
#include "mti_trace.h"

#ifdef VPU_INCLUDED
#include "vpu.h"
#ifdef DSP_VPU_TEST_HARNESS
extern UInt8 dspvputest_harness_flag;
#endif
#endif

#include "ripcmdq.h"

#include "jtag_assert.h"

#ifdef DSP_COMMS_INCLUDED
#include "dspcomms.h"
#else
#define JTAG_FRIENDLY_L1_ASSERT(x, y)
#endif //DSP_COMMS_INCLUDED

static Interrupt_t rip_hisr;


//#define WB_AMR_TEST

#if ( defined(WB_AMR_TEST) || defined(AACENC_TEST) ||	defined(QSW_PLAY_TEST))
extern UInt16 dl_codecs_dsp_testHarness_Flag;
#endif

//#define DSP_DIRECT_PLAY_TEST
//#define JUNO_TEST
//#define USB_HEADSET_TEST

#ifdef DSP_DIRECT_PLAY_TEST
UInt16 mp3_aac_file_done=0, mp3_aac_fill_size=2048;
UInt32 mp3_aac_file_length=4096, mp3_aac_file_start=0x80940000;
UInt16 *p_mp3_aac;
UInt16 mp3_aac=0;	//0:mp3;1:aac
Int16 curr_code=-1;	//-1:no;0:mp3;1:aac
#endif	//#ifdef DSP_DIRECT_PLAY_TEST

#ifdef QSW_PLAY_TEST
#define	QSW_OUTBUF_LIMIT 0x200000
UInt16 qsw_file_done=0, qsw_fill_size=512;
UInt32 qsw_file_length, qsw_test_file_start;
UInt16 *p_qsw_file;
UInt16 qsw_play_mode;
Int16 curr_load_mode=1;
extern UInt16 Input_Buffer_Select_Flag;
extern UInt16 Output_Buffer_Select_Flag;
extern UInt16 Which_DSP_Select_Flag;
extern UInt32 QswBytesWritten;
extern UInt16 Qsw_OutBuf_Tst[];
extern UInt16 *Qsw_OutBuf_Tst_Ptr;
#endif	//#ifdef QSW_PLAY_TEST

#ifdef	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
#ifdef	AACENC_TEST

extern UInt16 *aacenc_inBufPtr;
extern UInt16 *aacenc_inFilePtr;
extern UInt32 aacenc_infilesize;
extern UInt16 aacenc_infile_done;
extern UInt16 aacenc_outbuf_in, aacenc_outbuf_out;
extern UInt32 aacenc_codec_ready_timeout;
extern UInt16 aacenc_codec_ready;

extern UInt32 Total_fill_size;

extern UInt16 aacenc_outBuf[];

extern UInt32 BytesWritten2OutBuf;
extern UInt16 *aacenc_outBufPtr;

extern UInt16 aacenc_outBuf_capture;

//extern UInt16 dl_codecs_dsp_testHarness_Flag;

extern UInt16 aacEncStreamingMode;

extern UInt16 sw_fifo_index;

extern UInt16 AACEnc_pcmSource;

extern UInt16 *aacenc_inFileBasePtr;
extern UInt16 playback_Loopcount;

#define MAX_PLAYBACK_LOOPCOUNT	5
#define ENCODEROUTBUF_SIZE	0x100000

extern UInt16 bt_mixer_buf_log_on;
extern UInt16 bt_log_buf[0x80000];
extern UInt16 *bt_buf_ptr;
extern UInt32 BytesWritten2btBuf;

#endif	//	AACENC_TEST
#endif	//	DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE


#ifdef WB_AMR_TEST

#define WB_PRAM_CODEC_INPUT_SIZE 0X1000

extern UInt16 wb_amr_outBuf[];
extern UInt32 wordwritten;
extern UInt16 *wb_amr_outBufPtr, wb_amr_outbuf_in, wb_amr_outbuf_out;
extern UInt16 *wb_amr_inFilePtr;
extern UInt16 wb_amr_file_done;
extern UInt32 wb_amr_file_length;
extern UInt32 wb_amr_test_file_start;
extern UInt16 wb_amr_fill_size;
extern UInt16 wb_amr_codec;
extern UInt16 OutBufSelect;




#endif	//#ifdef WB_AMR_TEST

UInt16 rip_read_address, rip_read_tag, rip_read_data;



//******************************************************************************
// Local Function Prototypes
//******************************************************************************
static void RIPISR_HISR( void );		// Process the RIP interrupt

static void RIPISR_LISR( void );


static void SMCISR_LISR( void );


#ifdef HISTORY_LOGGING
#include "msdebug.h"
#endif

typedef struct
{
	RIPISR_HISR_CB hisr_cb;
	RIPISR_PROCESSSTATUS_CB msg_cb;
} RIPISR_CLIENT_INFO;

static Audio_ISR_Handler_t	client_Audio_ISR_Handler = NULL;
static VPU_ProcessStatus_t	client_VPU_ProcessStatus = NULL;

#define RIPISR_MAX_CLIENTS	4
static RIPISR_CLIENT_INFO ripisr_clients[RIPISR_MAX_CLIENTS];
static UInt32 ripisr_client_cnt = 0;

static void RIPISR_HISR_Common(void);
static Boolean RIPISR_ProcessMsg_Common(StatQ_t* 	status_msg);

//******************************************************************************
//
// Function Name:	RIPISR_Init
//
// Description:		This function creates a HISR to process the RIP interrupt
//					installs the vector for the interrupt handler, and enables
//					the interrupt..
//
// Notes:
//
//******************************************************************************

void RIPISR_Init()						// Initialize the RIP ISR
{
#ifndef WIN32
	rip_hisr = OSINTERRUPT_Create( RIPISR_HISR, HISRNAME_RIP, IPRIORITY_HIGH,
				HISRSTACKSIZE_RIP );

	IRQ_Register( RIP_IRQ, RIPISR_LISR );
	IRQ_Enable( RIP_IRQ );
#endif
	RIPISR_Register((RIPISR_HISR_CB)RIPISR_HISR_Common,	(RIPISR_PROCESSSTATUS_CB) RIPISR_ProcessMsg_Common);
#ifdef DSP_COMMS_INCLUDED
	RIPISR_COMMS_Init();
#endif// DSP_COMMS_INCLUDED


	IRQ_Register(SMC_INT_IRQ, SMCISR_LISR);
	IRQ_Enable(SMC_INT_IRQ);



}


void SMCISR_LISR()
{
	IRQ_Clear( SMC_INT_IRQ );
}



void RIPISR_Register(	RIPISR_HISR_CB hisr_cb,	RIPISR_PROCESSSTATUS_CB msg_cb)
{
	ripisr_clients[ripisr_client_cnt].hisr_cb = hisr_cb;
	ripisr_clients[ripisr_client_cnt].msg_cb = msg_cb;
	ripisr_client_cnt++;
	xassert(ripisr_client_cnt <= 4, ripisr_client_cnt);
}

void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb )
{
	client_VPU_ProcessStatus = hisr_cb;
	//VPU_ProcessStatusMainAMRDone at CP for 3G, at AP for VT
	//VPU_ProcessStatus from ripisr_audio.c at CP
	//VPU_ProcessStatus from vpu.c at AP
}

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb )
{
	client_Audio_ISR_Handler = isr_cb;
	//Audio_ISR_Handler from ripisr_audio.c at CP
	//in edge_CP_standalone build, Audio_ISR_Handler is at CP.
}

//will figure out how to avoid vpu.c call this function.
/*static*/ void Audio_ISR_Handler(StatQ_t msg)
{
	if( client_Audio_ISR_Handler != NULL )
	{
		client_Audio_ISR_Handler( msg );
	}
}

static void VPU_ProcessStatus( void ) //(VPStatQ_t vp_status_msg)
{
	if( client_VPU_ProcessStatus != NULL )
	{
		client_VPU_ProcessStatus( );
	}
}

//******************************************************************************
//
// Function Name:	RIPISR_HISR
//
// Description:		This function is the RIP interrupt service routine.
//
// Notes:
//
//******************************************************************************

static void RIPISR_HISR()				// Process the RIP interrupt
{
	StatQ_t 	status_msg;
	UInt32 count_i;
	Boolean msg_processed;

	//	Initializations made to avoid warning by COVERITY Checker: UNINIT
	status_msg.status = 0x0;
	status_msg.arg0 = 0;
	status_msg.arg1 = 0;
	status_msg.arg2 = 0;

	for (count_i = 0; count_i < ripisr_client_cnt; count_i++)
	{
		if(ripisr_clients[count_i].hisr_cb != NULL)
			(*ripisr_clients[count_i].hisr_cb)();
	}

	while ( SHAREDMEM_ReadStatusQ( &status_msg ) )
	{
		msg_processed = FALSE;
		for (count_i = 0; count_i < ripisr_client_cnt; count_i++)
		{
			if(ripisr_clients[count_i].msg_cb != NULL)
			{
				msg_processed = (*ripisr_clients[count_i].msg_cb)(&status_msg);
				if(msg_processed == TRUE) break;
			}
		}
		if(msg_processed == FALSE)
		{
			char string[80];

			sprintf(string, "\r\n ripisr, %02x %04x %04x %04x", status_msg.status, status_msg.arg0,
			status_msg.arg1, status_msg.arg2);
			SIO_PutString(PortB, string);
			JTAG_FRIENDLY_L1_ASSERT( FALSE, (status_msg.arg0 << 16) | status_msg.status );

		}
	}
}

static void RIPISR_HISR_Common(void)
{
	#ifdef VPU_INCLUDED
#ifdef DSP_VPU_TEST_HARNESS
	if( dspvputest_harness_flag == 1 )
		DSP_VPU_ProcessStatus();
	else
#endif //#ifdef DSP_VPU_TEST_HARNESS
		  VPU_ProcessStatus();
	#endif  //#ifdef VPU_INCLUDED
}

static Boolean RIPISR_ProcessMsg_Common(StatQ_t* 	p_status_msg)
{
	Boolean msg_processed = TRUE;
	StatQ_t status_msg = *p_status_msg;

	switch ( status_msg.status )
	{
		case STATUS_READ_STATUS :
			rip_read_address = status_msg.arg0;
			rip_read_tag = status_msg.arg1;
			rip_read_data = status_msg.arg2;
			break;

			case STATUS_VERSION_ID :
#if ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
				//RIPPROC.c is not built in AP image.
#else
				//on CP, RIPPROC.c is built with modem in CP image.
				RIPPROC_SetVersion( status_msg.arg0, status_msg.arg1, status_msg.arg2 );
				PHYFRAME_DSPSuspendAck( STATUS_VERSION_ID, 0 );
#endif
				L1_LOGV4("Status_version_ID received",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);

				break;
#if	defined(ATHENA_FPGA_VERSION)
			case STATUS_SHARED_MEM_SIZE:
				{
					UInt32 shared_mem_size_dsp;
					UInt32 shared_mem_size_arm;

					if(status_msg.arg0 == 0xABAB)	//	keyword
					{
						shared_mem_size_dsp = (status_msg.arg1 << 16) | status_msg.arg2;
						shared_mem_size_arm = sizeof(Dsp_SharedMem_t);

						L1_LOGV4("\n STATUS_SHARED_MEM_SIZE \n",shared_mem_size_dsp*2,shared_mem_size_arm,status_msg.arg2,0);

						if((shared_mem_size_dsp*2) != shared_mem_size_arm)	//	DSP 1 word = 2bytes
						{
							char string[0x50];
							sprintf(string, "\r\n ripisr, shared memory mismatch %02x %04x %04x %04x", status_msg.status, status_msg.arg0,
							status_msg.arg1, status_msg.arg2);
							SIO_PutString(PortB, (UInt8 *)string);
							JTAG_FRIENDLY_L1_ASSERT( FALSE, 0 );
						}
						else
						{
							L1_LOGV4("\n STATUS_SHARED_MEM_SIZE \n",shared_mem_size_dsp*2, shared_mem_size_arm, 0, 0);
						}
					}

			  }
				break;
#endif


		case STATUS_ASK_START_DDMA:
			// L1_LOGV4("STATUS_ASK_START_DDMA",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);
			{
				CmdQ_t msg;

				msg.cmd = COMMAND_START_DDMA;
				msg.arg0 = status_msg.arg0;
				msg.arg1 = status_msg.arg1;
				msg.arg2 = 0;

				SHAREDMEM_PostCmdQ( &msg );
			}
			break;


		case STATUS_TEST_DDMA:
			// L1_LOGV4("STATUS_TEST_DDMA",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);
			break;

		case STATUS_AAC_PLAIN_ERROR:
			L1_LOGV4("STATUS_AAC_PLAIN_ERROR",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);
			break;

		case STATUS_CTM_ABORT_BY_PR:
			L1_LOGV4("STATUS_CTM_ABORT_BY_PR",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);
			break;

		case STATUS_PR_ABORT_BY_CTM:
			L1_LOGV4("STATUS_PR_ABORT_BY_CTM",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);
			break;


		case STATUS_MAIN_AMR_DONE:
//		L1_LOG("DSP AMR Done");
#ifdef VPU_INCLUDED
			VPU_ProcessStatusMainAMRDone(status_msg);
#endif // #ifdef VPU_INCLUDED
		break;



			case STATUS_AUDIO_STREAM_DATA_READY:

			L1_LOGV4("RIPISR: STATUS_AUDIO_STREAM_DATA_READY: ",status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);
			Audio_ISR_Handler(status_msg);
			break;


		case STATUS_ASRB_LOOP:
				L1_LOGV4("ASRB loop status: ",status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);
				break;

		case STATUS_DSP_SYNC:
				L1_LOGV4("DSP sync status: ",status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);
				break;



		default :
			msg_processed = FALSE;
			break;
	}
	return msg_processed;
}

//******************************************************************************
//
// Function Name:	RIPISR_LISR
//
// Description:		This function is the Low Level ISR for the RIP interrupt.
//					It simply triggers the HISR.
//
// Notes:
//
//******************************************************************************
#define REFRESH_IRQ RIP_IRQ
static void RIPISR_LISR()
{
#ifdef NO_DSP_TEST
	//Ignore any DSP interrupt for NO_DSP_TEST mode
	IRQ_Clear( RIP_IRQ );
	return;
#else //NO_DSP_TEST
	SharedMem_t *mem;
	UInt16 rfic_flag;
	Boolean NeedHisr;


#ifdef DSP_COMMS_INCLUDED
#ifdef HISTORY_LOGGING
	UInt16 i=1;
#endif
#endif	// DSP_COMMS_INCLUDED

	mem = (SharedMem_t *)(SHAREDMEM_GetSharedMemPtr());
	NeedHisr = FALSE;
	IRQ_SoftInt_Clear(REFRESH_IRQ);

	rfic_flag = mem->shared_rfic_request;
	do
	{
		IRQ_Clear( RIP_IRQ );
		if( rfic_flag)
		{
			mem->shared_rfic_request = 0;
#ifdef DSP_COMMS_INCLUDED
			NeedHisr |= RFICISR_LISR();
#endif

		}
		if( mem->shared_rip_request)
		{
			mem->shared_rip_request = 0;
			NeedHisr = TRUE;
		}
#ifdef DSP_COMMS_INCLUDED
		if( RIPPROC_Get_RFIC_Type()!=0)
			 NeedHisr = TRUE;
#ifdef HISTORY_LOGGING
		if (his_log_switch & HIS_LOG_INT_TASK)
		{
			MSDEBUG_LogRipSpecials(rfic_flag, rfic_phase1, rfic_phase2, (UInt16)NeedHisr, i);
		}
		i = i<<1;
#endif
#endif	// DSP_COMMS_INCLUDED

	}
	while( (rfic_flag = mem->shared_rfic_request)!= 0);
	if(NeedHisr)
		OSINTERRUPT_Trigger( rip_hisr );

#endif //NO_DSP_TEST
}


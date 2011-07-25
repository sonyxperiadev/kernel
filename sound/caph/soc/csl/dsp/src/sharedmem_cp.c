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

#include <string.h>
#include <assert.h>
#include "mobcom_types.h"
#include "chip_version.h"
//#include "dsp_feature_def.h"
#include "consts.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "memmap.h"
#include "ostypes.h"
#include "timer.h"
#include "irqctrl.h"
#include "xassert.h"
//#include "sysparm.h"
#include "ram_memmap.h"
#include "l1misc.h"
#include "sysparm.h"
//#include "ms_config.h" //HFA, JPG

#if ((!defined(FUSE_APPS_PROCESSOR) ) && (defined (UMTS)))
#define AMR_LOGGING
#endif
#ifdef AMR_LOGGING
#include "log_sig_code.h" //required for logging UMACDL_AMR_DL.
#endif

#include "jtag_assert.h"

#ifdef DSP_COMMS_INCLUDED
#define ENABLE_LOGGING
#include "log.h"
#endif //DSP_COMMS_INCLUDED

#if !defined(_ATHENA_)
#if !defined(_SAMOA_)		// from SAMOA FPGA branch, will need to change to use chal before chip arrives
#define USE_TL3R_CHAL
#endif
#endif

#if defined(USE_TL3R_CHAL)
#include "chal_types.h"
#include "chal_tl3r.h"
#include "sysmap_types.h"
#include "csl_sysmap.h"

CHAL_HANDLE chal_tl3r_handle = NULL;
#endif
#if defined(DEVELOPMENT_ONLY)
UInt16 at_mdsptst_audio_logging = FALSE;
UInt16 at_mtst_track_logging =FALSE;
UInt16 at_mtst_ncell_logging = FALSE;
UInt16 at_mtst_fer_logging = FALSE;
Boolean at_mtst_frameentry_logging = TRUE;
Boolean at_mtst_fsc_logging = FALSE;
Boolean at_mtst_event_logging = FALSE;

#endif
#ifdef WIN32
#undef SHARED_RAM_BASE
UInt8 SHARED_RAM_BASE[0x50000];
#endif


#define BMODEM_BASE_ADDR 0x3A050000
#define CAPH_BASE_ADDR   0x35020000
#define DSPCCU_BASE_ADDR 0x3a056000
#define CSR_BASE_ADDR	 0x35008000


#pragma arm section zidata = "shared_rip_mem_sect"
static Dsp_SharedMem_t			CP_shared_memory;	// Define global structure variable and put them at the dsp-arm shared memory
	  
#pragma arm section

static Dsp_SharedMem_t 			*dsp_shared_mem = 	    &CP_shared_memory;		// Define pointers to global strcuture of the shared memory


static Dsp_SharedMem_t			*unpaged_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t			*shared_mem = &CP_shared_memory; 
static Dsp_SharedMem_t 			*pg1_shared_mem = &CP_shared_memory;							
static Dsp_SharedMem_t 			*event_page = &CP_shared_memory;								
#ifdef DSP_FEATURE_RFAPI
static Dsp_SharedMem_t 			*rftest_page = &CP_shared_memory;
#endif
static Dsp_SharedMem_t			*pg4_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg5_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg6_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg7_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg10_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg11_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg14_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg15_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg18_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg21_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg24_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg27_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg28_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg29_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t			*pg30_shared_mem = &CP_shared_memory;
#ifdef DSP_FEATURE_HIGH_QUALITY_ADC			 											
static Dsp_SharedMem_t			*pg31_shared_mem = &CP_shared_memory;					
static Dsp_SharedMem_t 			*pg32_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg33_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg34_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg35_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg36_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg37_shared_mem = &CP_shared_memory;
static Dsp_SharedMem_t 			*pg38_shared_mem = &CP_shared_memory;
																		
#endif

static UInt8 AMR_Codec_Mode_Good = 7;
#if defined(UMTS) && defined(DEVELOPMENT_ONLY)
Boolean is_amr_voice_test = FALSE;
static UInt32 amr_voice_counter =0;
#endif


//******************************************************************************
//
// Function Name:	SHAREDMEM_Init
//
// Description:		Initialize CP Shared Memory
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_Init()
{
	// The size of shared memory is defined at link
	// time.  Currently 8K bytes are reserved.

	// Clear out shared memory
	// chage until unowned cdb  changes propagate to cib, changes in apps wrapper
//	memset(&CP_shared_memory, 0, sizeof(Dsp_SharedMem_t));

#if (defined(_ATHENA_))
	memset(&CP_shared_memory, 0, 0x30000);		// solution for 0xef00 assert on some t-birds and athenarays until vpu_init is moved to ap side
	*((UInt32 *)(SMICONF0)) = SHARED_RAM_BASE | 0x0A;
	*((UInt32 *)(SMICONF1)) = SHARED_RAM_BASE;
	//*((UInt32 *)(SMICONF1)) = ( (UInt32)DlsFile ) | 0x0A | 0x00;
	//*((UInt32 *)(SMICONF2)) = SHARED_RAM_BASE | 0x0A | 0x01| 0x20;
	*((UInt32 *)(SMICONF2)) = SHARED_RAM_BASE | 0x0A | 0x00;
	//*((UInt32 *)(SMICONF3)) = SHARED_RAM_BASE | 0x1C | 0x01 | 0x20;
	*((UInt32 *)(SMICONF3)) = SHARED_RAM_BASE | 0x10 | 0x00;
	*((UInt32 *)(SMICONF4)) = SHARED_RAM_BASE | 0x0A | 0x60; 	

#else // For Rhea, Samoa, etc:
	memset(&CP_shared_memory, 0, 0x40000);		// solution for 0xef00 assert on some t-birds and athenarays until vpu_init is moved to ap side
	#if !defined(USE_TL3R_CHAL)

	*((UInt32 *)(SMICONF0)) = SHARED_RAM_BASE | 0x100 | 0x08; // Non-buffered read and write to shared memory
	*((UInt32 *)(SMICONF1)) = BMODEM_BASE_ADDR| 0x14; // To clear BMODEM (CP interrupt)
	*((UInt32 *)(SMICONF2)) = CAPH_BASE_ADDR  | 0xe;  // To access Audio block
	//*((UInt32 *)(SMICONF3)) = SHARED_RAM_BASE | 0x10 | 0x00; 
	*((UInt32 *)(SMICONF4)) = SHARED_RAM_BASE | 0x08 | 0x60; // Buffered read and write to shared memory

	#else
	chal_tl3r_handle = chal_tl3r_init(csl_sysmap_get_base_addr(AHB_DSP_TL3R));


	//*((UInt32 *)(SMICONF0)) = SHARED_RAM_BASE | 0x100 | 0x08; // Non-buffered read and write to shared memory
	chal_tl3r_set_config_reg(chal_tl3r_handle, 
						/* index */ CHAL_TL3R_CONFIG_REGISTER_0, 
						/* sm_base_addr */ SHARED_RAM_BASE,
						/* addr_mode */ CHALTL3R_TL3RCONFIG_ADDR_MODE_NARROW,
						/* rbuf_adaptive_en */ CHALTL3R_TL3RCONFIG_RBUF_ADAPTIVE_DISABLE,
						/* wbuf_en */ CHALTL3R_TL3RCONFIG_WBUF_DISABLE,
						/* page_size3to0 */ CHALTL3R_TL3RCONFIG_PAGE_SIZE_256KW,
						/* rbuf_enable */ CHALTL3R_TL3RCONFIG_RBUF_DISABLE
						);
						
	// *((UInt32 *)(SMICONF1)) = BMODEM_BASE_ADDR| 0x14; // To clear BMODEM (CP interrupt)
	chal_tl3r_set_config_reg(chal_tl3r_handle, 
						/* index */ CHAL_TL3R_CONFIG_REGISTER_1, 
						/* sm_base_addr */ BMODEM_BASE_ADDR,
						/* addr_mode */ CHALTL3R_TL3RCONFIG_ADDR_MODE_LEGACY,
						/* rbuf_adaptive_en */ CHALTL3R_TL3RCONFIG_RBUF_ADAPTIVE_DISABLE,
						/* wbuf_en */ CHALTL3R_TL3RCONFIG_WBUF_DISABLE,
						/* page_size3to0 */ CHALTL3R_TL3RCONFIG_PAGE_SIZE_4KW,
						/* rbuf_enable */ CHALTL3R_TL3RCONFIG_RBUF_DISABLE
						);

	// *((UInt32 *)(SMICONF2)) = CAPH_BASE_ADDR  | 0xe;  // To access Audio block
	chal_tl3r_set_config_reg(chal_tl3r_handle, 
					/* index */ CHAL_TL3R_CONFIG_REGISTER_2, 
					/* sm_base_addr */ CAPH_BASE_ADDR,
					/* addr_mode */ CHALTL3R_TL3RCONFIG_ADDR_MODE_LEGACY,
					/* rbuf_adaptive_en */ CHALTL3R_TL3RCONFIG_RBUF_ADAPTIVE_DISABLE,
					/* wbuf_en */ CHALTL3R_TL3RCONFIG_WBUF_DISABLE,
					/* page_size3to0 */ CHALTL3R_TL3RCONFIG_PAGE_SIZE_32KW,
					/* rbuf_enable */ CHALTL3R_TL3RCONFIG_RBUF_DISABLE
					);
					
	

	// *((UInt32 *)(SMICONF3)) = DSPCCU_BASE_ADDR  | 0xe;  // To access DSP CCU block
	chal_tl3r_set_config_reg(chal_tl3r_handle, 
					/* index */ CHAL_TL3R_CONFIG_REGISTER_3, 
					/* sm_base_addr */ DSPCCU_BASE_ADDR,
					/* addr_mode */ CHALTL3R_TL3RCONFIG_ADDR_MODE_LEGACY,
					/* rbuf_adaptive_en */ CHALTL3R_TL3RCONFIG_RBUF_ADAPTIVE_DISABLE,
					/* wbuf_en */ CHALTL3R_TL3RCONFIG_WBUF_DISABLE,
					/* page_size3to0 */ CHALTL3R_TL3RCONFIG_PAGE_SIZE_4KW,
					/* rbuf_enable */ CHALTL3R_TL3RCONFIG_RBUF_DISABLE
					);

	
					
	// *((UInt32 *)(SMICONF4)) = SHARED_RAM_BASE | 0x08 | 0x60; // Buffered read and write to shared memory
	chal_tl3r_set_config_reg(chal_tl3r_handle, 
					/* index */ CHAL_TL3R_CONFIG_REGISTER_4, 
					/* sm_base_addr */ SHARED_RAM_BASE,
					/* addr_mode */ CHALTL3R_TL3RCONFIG_ADDR_MODE_LEGACY,
					/* rbuf_adaptive_en */ CHALTL3R_TL3RCONFIG_RBUF_ADAPTIVE_ENABLE,
					/* wbuf_en */ CHALTL3R_TL3RCONFIG_WBUF_ENABLE,
					/* page_size3to0 */ CHALTL3R_TL3RCONFIG_PAGE_SIZE_256KW,
					/* rbuf_enable */ CHALTL3R_TL3RCONFIG_RBUF_DISABLE
					);



	// *((UInt32 *)(SMICONF5)) = CSR_BASE_ADDR  | 0xe;  // To access CSR block
	chal_tl3r_set_config_reg(chal_tl3r_handle, 
					/* index */ CHAL_TL3R_CONFIG_REGISTER_5, 
					/* sm_base_addr */ CSR_BASE_ADDR,
					/* addr_mode */ CHALTL3R_TL3RCONFIG_ADDR_MODE_LEGACY,
					/* rbuf_adaptive_en */ CHALTL3R_TL3RCONFIG_RBUF_ADAPTIVE_DISABLE,
					/* wbuf_en */ CHALTL3R_TL3RCONFIG_WBUF_DISABLE,
					/* page_size3to0 */ CHALTL3R_TL3RCONFIG_PAGE_SIZE_4KW,
					/* rbuf_enable */ CHALTL3R_TL3RCONFIG_RBUF_DISABLE
					);





	
	#endif
					
/* Route the audio interrupts over to the DSP */
//*((UInt32 *)(0x3a050260)) = 0x00000080;  // To route normal audio interrupt to the DSP
//*((UInt32 *)(0x3a050280)) = 0x00000100;  // To route error audio interrupt to the DSP

#endif// ATHENA


}

//******************************************************************************
//
// Function Name:	SHAREDMEM_PostCmdQ
//
// Description:		Post an entry to the command queue
//
// Notes:
//
//******************************************************************************

#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)

#else // defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)

#ifndef _NODEBUG

   
CmdQHist_t	cmd_q_hist[CMD_Q_HIST_SZ];
CmdQHist_t	fcmd_q_hist[CMD_Q_HIST_SZ];
StatQHist_t	stat_q_hist[STAT_Q_HIST_SZ];
static UInt8	cmd_hist_index = 0;
static UInt8    fast_cmd_hist_index = 0;
static UInt8	hist_index = 0;

extern Ticks_t TIMER_GetValue(void);
#endif


//******************************************************************************
//
// Function Name:	SHAREDMEM_get_audio_logging
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
Boolean  SHAREDMEM_is_audio_logging_set(UInt16 mask)
{
	Boolean bit_set = FALSE;
	if (at_mdsptst_audio_logging & mask)
		bit_set = TRUE;

	return (bit_set);
}

#define TIMEOUT_WAIT_FOR_CMDQ_AVAIABLE ( TICKS_ONE_SECOND/100 )

void SHAREDMEM_PostCmdQ(			// Post an entry to the command queue
	CmdQ_t *cmd_msg					// Entry to post
	)
{
	CmdQ_t	*p;
	UInt8	next_cmd_in;
	UInt32	mask;

#ifndef NO_DSP_TEST
	//Skip the real command queue operation if it is NO_DSP_TEST mode
	UInt32 Enter_Time = TIMER_GetValue();

	// wait until we have room.

	while( (UInt8)(( pg5_shared_mem->shared_cmdq_in + 1 ) % CMDQ_SIZE) == pg5_shared_mem->shared_cmdq_out )

	{
		if( TIMER_GetValue() - Enter_Time > TIMEOUT_WAIT_FOR_CMDQ_AVAIABLE )
		{
			// timeout. We will know something wrong here by assertion below.
			break;
		}
	}

	mask = ARM_DisableIRQFIQ();
	next_cmd_in = (UInt8)(( pg5_shared_mem->shared_cmdq_in + 1 ) % CMDQ_SIZE);
	JTAG_FRIENDLY_L1_ASSERT( next_cmd_in != pg5_shared_mem->shared_cmdq_out, next_cmd_in );
	p = &pg5_shared_mem->shared_cmdq[ pg5_shared_mem->shared_cmdq_in ];
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;

/*	if (p->arg0 == 0xE5C2)
	{
		p->arg1 = 0x48;
	}
*/
	pg5_shared_mem->shared_cmdq_in = next_cmd_in;
#ifndef _NODEBUG
	cmd_q_hist[cmd_hist_index].cmd_q = *cmd_msg;
	cmd_q_hist[cmd_hist_index].time_stamp = TIMER_GetValue();
	cmd_q_hist[cmd_hist_index].shared_cmdq_in = next_cmd_in;
	cmd_q_hist[cmd_hist_index].shared_cmdq_out = (UInt8) pg5_shared_mem->shared_cmdq_out;
	cmd_hist_index++;
	cmd_hist_index = (cmd_hist_index & (CMD_Q_HIST_SZ - 1));
#endif


//	L1_LOGV4("cmd, arg0, arg1,arg2 ", p->cmd, p->arg0, p->arg1, p->arg2);


	ARM_RecoverIRQFIQ(mask);
	IRQ_TriggerRIPInt();
#endif// NO_DSP_TEST

#if defined(DEVELOPMENT_ONLY) && defined (DSP_COMMS_INCLUDED)
   if ( SHAREDMEM_is_audio_logging_set (0x8000) )
	   CAL_LOGV4("DSP CMD",(pg5_shared_mem->shared_cmdq_in<<16 | pg5_shared_mem->shared_cmdq_out<<8 | p->cmd),p->arg0,p->arg1,p->arg2);
#endif

}

//******************************************************************************
//
// Function Name:	SHAREDMEM_PostFastCmdQ
//
// Description:		Post an entry to the fast command queue
//
// Notes:
//
//******************************************************************************
void SHAREDMEM_PostFastCmdQ(		// Post an entry to the fast command queue
	CmdQ_t *cmd_msg					// Entry to post
	)
{
	CmdQ_t	*p;
	UInt8	next_cmd_in;
	UInt32	mask;

	UInt32 Enter_Time = TIMER_GetValue();

	// wait until we have room.

	while( (UInt8)(( pg5_shared_mem->shared_fast_cmdq_in + 1 ) % FAST_CMDQ_SIZE) == pg5_shared_mem->shared_fast_cmdq_out )
	{
		if( TIMER_GetValue() - Enter_Time > TIMEOUT_WAIT_FOR_CMDQ_AVAIABLE )
		{
			// timeout. We will know something wrong here by assertion below.
			break;
		}
	}

	mask = ARM_DisableIRQFIQ();
	next_cmd_in = (UInt8)(( pg5_shared_mem->shared_fast_cmdq_in + 1 ) % FAST_CMDQ_SIZE);
	JTAG_FRIENDLY_L1_ASSERT( next_cmd_in != pg5_shared_mem->shared_fast_cmdq_out, next_cmd_in);

	p = &pg5_shared_mem->shared_fast_cmdq[ pg5_shared_mem->shared_fast_cmdq_in ];
	
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;
	pg5_shared_mem->shared_fast_cmdq_in = next_cmd_in;

#ifndef _NODEBUG
	fcmd_q_hist[fast_cmd_hist_index].cmd_q = *cmd_msg;
	fcmd_q_hist[fast_cmd_hist_index].time_stamp = TIMER_GetValue();
	fcmd_q_hist[fast_cmd_hist_index].shared_cmdq_in = next_cmd_in;
	fcmd_q_hist[fast_cmd_hist_index].shared_cmdq_out = (UInt8) pg5_shared_mem->shared_fast_cmdq_out;
	fast_cmd_hist_index++;
	fast_cmd_hist_index = (fast_cmd_hist_index & (CMD_Q_HIST_SZ - 1));
#endif

	ARM_RecoverIRQFIQ(mask);
	IRQ_TriggerRIPInt();

#if defined(DEVELOPMENT_ONLY) && defined (DSP_COMMS_INCLUDED)
   if ( SHAREDMEM_is_audio_logging_set (0x8000) )
	CAL_LOGV4("Fast CMD",(pg5_shared_mem->shared_fast_cmdq_in<<16 | pg5_shared_mem->shared_fast_cmdq_out<<8 | p->cmd),p->arg0,p->arg1,p->arg2);
#endif


}


void SHAREDMEM_PostCmdQ_LISR(			// Post an entry to the command queue
	CmdQ_t *cmd_msg					// Entry to post
	)
{
	CmdQ_t	*p;
	UInt8	next_cmd_in;
	UInt32 Enter_Time = TIMER_GetValue();

	// wait until we have room.

	while( TRUE )
	{
		if( (UInt8)(( pg5_shared_mem->shared_cmdq_in + 1 ) % CMDQ_SIZE) == pg5_shared_mem->shared_cmdq_out )
		{
			if( TIMER_GetValue() - Enter_Time > TIMEOUT_WAIT_FOR_CMDQ_AVAIABLE )
			{
				// timeout. We will know something wrong here by assertion below.
				xassert(FALSE,Enter_Time);
			}
		}
		else
		{
			break;
		}
	}

//	mask = ARM_DisableIRQFIQ();
	next_cmd_in = (UInt8)(( pg5_shared_mem->shared_cmdq_in + 1 ) % CMDQ_SIZE);
	assert( next_cmd_in != pg5_shared_mem->shared_cmdq_out );

	p = &pg5_shared_mem->shared_cmdq[ pg5_shared_mem->shared_cmdq_in ];
	
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;
	pg5_shared_mem->shared_cmdq_in = next_cmd_in;

//	ARM_RecoverIRQFIQ(mask);
	IRQ_TriggerRIPInt();

#ifndef _NODEBUG
	cmd_q_hist[cmd_hist_index].cmd_q = *cmd_msg;
	cmd_q_hist[cmd_hist_index].time_stamp = TIMER_GetValue();
	cmd_q_hist[cmd_hist_index].shared_cmdq_in = next_cmd_in;
	cmd_q_hist[cmd_hist_index].shared_cmdq_out = (UInt8) pg5_shared_mem->shared_cmdq_out;
	cmd_hist_index++;
	cmd_hist_index = (cmd_hist_index & (CMD_Q_HIST_SZ - 1));

#endif
#if defined(DEVELOPMENT_ONLY) && defined (DSP_COMMS_INCLUDED)
   if ( SHAREDMEM_is_audio_logging_set (0x8000) )
	 CAL_LOGV4("DSP CMD",(pg5_shared_mem->shared_cmdq_in<<16 | pg5_shared_mem->shared_cmdq_out<<8 | p->cmd),p->arg0,p->arg1,p->arg2);
#endif
  


}

//******************************************************************************
//
// Function Name:	SHAREDMEM_PostFastCmdQ
//
// Description:		Post an entry to the fast command queue
//
// Notes:
//
//******************************************************************************
void SHAREDMEM_PostFastCmdQ_LISR(		// Post an entry to the fast command queue
	CmdQ_t *cmd_msg					// Entry to post
	)
{
	CmdQ_t	*p;
	UInt8	next_cmd_in;
	UInt32 Enter_Time = TIMER_GetValue();

	// wait until we have room.

	while( TRUE )
	{
		if( (UInt8)(( pg5_shared_mem->shared_fast_cmdq_in + 1 ) % FAST_CMDQ_SIZE) == pg5_shared_mem->shared_fast_cmdq_out )
		{
			if( TIMER_GetValue() - Enter_Time > TIMEOUT_WAIT_FOR_CMDQ_AVAIABLE )
			{
				// timeout. We will know something wrong here by assertion below.
				xassert(FALSE,Enter_Time);
			}
		}
		else
		{
			break;
		}
	}

//	ARM_DisableIRQFIQ(mask);
	next_cmd_in = (UInt8)(( pg5_shared_mem->shared_fast_cmdq_in + 1 ) % FAST_CMDQ_SIZE);
	assert( next_cmd_in != pg5_shared_mem->shared_fast_cmdq_out );

	p = &pg5_shared_mem->shared_fast_cmdq[ pg5_shared_mem->shared_fast_cmdq_in ];
	
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;
	pg5_shared_mem->shared_fast_cmdq_in = next_cmd_in;

//	IRQ_RecoverIRQFIQ(mask);
	IRQ_TriggerRIPInt();

#ifndef _NODEBUG
	fcmd_q_hist[fast_cmd_hist_index].cmd_q = *cmd_msg;
	fcmd_q_hist[fast_cmd_hist_index].time_stamp = TIMER_GetValue();
	fcmd_q_hist[fast_cmd_hist_index].shared_cmdq_in = next_cmd_in;
	fcmd_q_hist[fast_cmd_hist_index].shared_cmdq_out = (UInt8) pg5_shared_mem->shared_fast_cmdq_out;
	fast_cmd_hist_index++;
	fast_cmd_hist_index = (fast_cmd_hist_index & (CMD_Q_HIST_SZ - 1));
#endif

#if defined(DEVELOPMENT_ONLY) && defined (DSP_COMMS_INCLUDED)
   if ( SHAREDMEM_is_audio_logging_set (0x8000) )
	 CAL_LOGV4("Fast CMD",(pg5_shared_mem->shared_fast_cmdq_in<<16 | pg5_shared_mem->shared_fast_cmdq_out<<8 | p->cmd),p->arg0,p->arg1,p->arg2);
#endif
	
}
//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadStatusQ
//
// Description:		Read an entry from the command queue
//
// Notes:
//
//******************************************************************************

Boolean SHAREDMEM_ReadStatusQ(			// Read an entry from the command queue
	StatQ_t *status_msg					// Entry from queue
	)
{
	StatQ_t *p;
	UInt16	status_out;
	UInt16	status_in;
	Boolean	result;
	
	status_out = pg5_shared_mem->shared_statusq_out;
	status_in = pg5_shared_mem->shared_statusq_in;
	if ( status_out == status_in )
	{
		result = FALSE;
	}
	else
	{
		p = &pg5_shared_mem->shared_statusq[ status_out ];
		status_msg->status = p->status;
		status_msg->arg0 = (UInt16)p->arg0;
		status_msg->arg1 = (UInt16)p->arg1;
		status_msg->arg2 = (UInt16)p->arg2;

		pg5_shared_mem->shared_statusq_out = ( status_out + 1 ) % STATUSQ_SIZE;

		result = TRUE;


#ifndef _NODEBUG
		stat_q_hist[hist_index].stat_q = *status_msg;
		stat_q_hist[hist_index].time_stamp = TIMER_GetValue();
		stat_q_hist[hist_index].shared_statusq_in = (UInt8) pg5_shared_mem->shared_statusq_in;
		stat_q_hist[hist_index].shared_statusq_out = (UInt8) pg5_shared_mem->shared_statusq_out;
		hist_index++;
		hist_index = (hist_index & (STAT_Q_HIST_SZ - 1));
#endif

	}
	return(result);
}
#endif //defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) && defined(BSP_IPC_AP)




//******************************************************************************
//
// Function Name:	SHAREDMEM_GetDspModemAudioSharedMemPtr
//
// Description:		Return pointer to 32-bit aligned shared memory
//
// Notes:
//
//******************************************************************************

Dsp_SharedMem_t *SHAREDMEM_GetDsp_SharedMemPtr()		// Return pointer to 32-bit aligned shared memory
{
	return dsp_shared_mem;
}	





//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteTestPoint
//
// Description:		Write to a test point in shared memory
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_WriteTestPoint(	// Write to a test point
	UInt8	tp,					// Test Point
	UInt16	value				// Test Point Value
	)
{
	UInt16 *ptr;

	ptr = (UInt16 *)&unpaged_shared_mem->shared_debug[tp];
	*ptr = value;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetSharedMemPtr
//
// Description:		Return pointer to shared memory
//
// Notes:
//
//******************************************************************************

Unpaged_SharedMem_t *SHAREDMEM_GetUnpagedSharedMemPtr()// Return pointer to unpaged shared memory
{
	return unpaged_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetSharedMemPtr
//
// Description:		Return pointer to shared memory
//
// Notes:
//
//******************************************************************************

SharedMem_t *SHAREDMEM_GetSharedMemPtr()// Return pointer to shared memory
{

	return shared_mem;


}	


//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage1SharedMemPtr
//
// Description:		Return pointer to Page1 shared memory
//
// Notes:
//
//******************************************************************************

PAGE1_SharedMem_t *SHAREDMEM_GetPage1SharedMemPtr()// Return pointer to shared memory
{


	return pg1_shared_mem;



}	


//******************************************************************************
//
// Function Name:	SHAREDMEM_GetSharedMemRfSpiPtr
//
// Description:		Return pointer to shared memory RFSPI structure
//
// Notes:
//
//******************************************************************************

SPI_Records_t *SHAREDMEM_GetSharedMemRfSpiPtr() 
{
unpaged_shared_mem = SHAREDMEM_GetUnpagedSharedMemPtr();

	return  &(unpaged_shared_mem->shared_spi_cmd);



}	
//******************************************************************************
//
// Function Name:	SHAREDMEM_Get_poly_events_MemPtr
//
// Description:		Return pointer to polyringer event buffer
//
// Notes:
//
//******************************************************************************

Shared_poly_events_t *SHAREDMEM_Get_poly_events_MemPtr()// Return pointer to shared memory
{

	return event_page;
}	

#ifdef DSP_FEATURE_RFAPI
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetRFTest_MemPtr
//
// Description:		Return pointer to rf_rx_test_data and rf_tx_test_data structures
//
// Notes:				Overlay with polyrigner in Pg3
//
//******************************************************************************

Shared_RF_rxtx_test_Page_t *SHAREDMEM_GetRFTest_MemPtr()// Return pointer to shared memory
{


	return rftest_page;


}	
#endif

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage5SharedMemPtr
//
// Description:		Return pointer to Page5 shared memory
//
// Notes:
//
//******************************************************************************

Shared_poly_audio_t *SHAREDMEM_GetPage4SharedMemPtr()// Return pointer to page 5 shared memory
{


	return pg4_shared_mem;



}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage5SharedMemPtr
//
// Description:		Return pointer to Page5 shared memory
//
// Notes:
//
//******************************************************************************

PAGE5_SharedMem_t *SHAREDMEM_GetPage5SharedMemPtr()// Return pointer to page 5 shared memory
{


	return pg5_shared_mem;




}	
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage6SharedMemPtr
//
// Description:		Return pointer to Page6 shared memory
//
// Notes:
//
//******************************************************************************

PAGE6_SharedMem_t *SHAREDMEM_GetPage6SharedMemPtr()// Return pointer to page 5 shared memory
{
	return pg6_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage7SharedMemPtr
//
// Description:		Return pointer to Page7 shared memory
//
// Notes:
//
//******************************************************************************

PAGE7_SharedMem_t *SHAREDMEM_GetPage7SharedMemPtr()// Return pointer to page 7 shared memory
{
	return pg7_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage10SharedMemPtr
//
// Description:		Return pointer to Page10 shared memory
//
// Notes:
//
//******************************************************************************

PAGE10_SharedMem_t *SHAREDMEM_GetPage10SharedMemPtr()// Return pointer to page 10 shared memory
{
	return pg10_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage11SharedMemPtr
//
// Description:		Return pointer to Page11 shared memory
//
// Notes:
//
//******************************************************************************

PAGE11_SharedMem_t *SHAREDMEM_GetPage11SharedMemPtr()// Return pointer to page 11 shared memory
{
	return pg11_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage14SharedMemPtr
//
// Description:		Return pointer to Page14 shared memory
//
// Notes:
//
//******************************************************************************

PAGE14_SharedMem_t *SHAREDMEM_GetPage14SharedMemPtr()// Return pointer to page 14 shared memory
{
	return pg14_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage15SharedMemPtr
//
// Description:		Return pointer to Page15 shared memory
//
// Notes:
//
//******************************************************************************

PAGE15_SharedMem_t *SHAREDMEM_GetPage15SharedMemPtr()// Return pointer to page 15 shared memory
{
	return pg15_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage18SharedMemPtr
//
// Description:		Return pointer to Page18 shared memory
//
// Notes:
//
//******************************************************************************

PAGE18_SharedMem_t *SHAREDMEM_GetPage18SharedMemPtr()// Return pointer to page 18 shared memory
{
	return pg18_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage21SharedMemPtr
//
// Description:		Return pointer to Page21 shared memory
//
// Notes:
//
//******************************************************************************

PAGE21_SharedMem_t *SHAREDMEM_GetPage21SharedMemPtr()// Return pointer to page 21 shared memory
{
	return pg21_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage24SharedMemPtr
//
// Description:		Return pointer to Page24 shared memory
//
// Notes:
//
//******************************************************************************

PAGE24_SharedMem_t *SHAREDMEM_GetPage24SharedMemPtr()// Return pointer to page 24 shared memory
{
	return pg24_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage27SharedMemPtr
//
// Description:		Return pointer to Page27 shared memory
//
// Notes:
//
//******************************************************************************

PAGE27_SharedMem_t *SHAREDMEM_GetPage27SharedMemPtr()// Return pointer to page 27 shared memory
{
	return pg27_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage28SharedMemPtr
//
// Description:		Return pointer to Page28 shared memory
//
// Notes:
//
//******************************************************************************

PAGE28_SharedMem_t *SHAREDMEM_GetPage28SharedMemPtr()// Return pointer to page 28 shared memory
{
	return pg28_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage29SharedMemPtr
//
// Description:		Return pointer to Page29 shared memory
//
// Notes:
//
//******************************************************************************

PAGE29_SharedMem_t *SHAREDMEM_GetPage29SharedMemPtr()// Return pointer to page 29 shared memory
{
	return pg29_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage30SharedMemPtr
//
// Description:		Return pointer to Page30 shared memory
//
// Notes:
//
//******************************************************************************

PAGE30_SharedMem_t *SHAREDMEM_GetPage30SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg30_shared_mem;
}	

#ifdef DSP_FEATURE_HIGH_QUALITY_ADC
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage31SharedMemPtr
//
// Description:		Return pointer to Page31 shared memory
//
// Notes:
//
//******************************************************************************

PAGE31_SharedMem_t *SHAREDMEM_GetPage31SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg31_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage33SharedMemPtr
//
// Description:		Return pointer to Page32 shared memory
//
// Notes:
//
//******************************************************************************

PAGE32_SharedMem_t *SHAREDMEM_GetPage32SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg32_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage33SharedMemPtr
//
// Description:		Return pointer to Page33 shared memory
//
// Notes:
//
//******************************************************************************

PAGE33_SharedMem_t *SHAREDMEM_GetPage33SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg33_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage34SharedMemPtr
//
// Description:		Return pointer to Page34 shared memory
//
// Notes:
//
//******************************************************************************

PAGE34_SharedMem_t *SHAREDMEM_GetPage34SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg34_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage35SharedMemPtr
//
// Description:		Return pointer to Page35 shared memory
//
// Notes:
//
//******************************************************************************

PAGE35_SharedMem_t *SHAREDMEM_GetPage35SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg35_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage36SharedMemPtr
//
// Description:		Return pointer to Page36 shared memory
//
// Notes:
//
//******************************************************************************

PAGE36_SharedMem_t *SHAREDMEM_GetPage36SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg36_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage37SharedMemPtr
//
// Description:		Return pointer to Page37 shared memory
//
// Notes:
//
//******************************************************************************

PAGE37_SharedMem_t *SHAREDMEM_GetPage37SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg37_shared_mem;
}	

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage31SharedMemPtr
//
// Description:		Return pointer to Page31 shared memory
//
// Notes:
//
//******************************************************************************

PAGE38_SharedMem_t *SHAREDMEM_GetPage38SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg38_shared_mem;
}	
#endif// DSP_FEATURE_HIGH_QUALITY_ADC

//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteDL_AMR_Speech
//
// Description:		Write Downlink AMR Speech Data into Share Memory
//
// Notes:
//
//******************************************************************************
static UInt8 good_speech_cnt =0;
static UInt8 bad_speech_cnt = 0;
static Boolean replace_bad_sid = FALSE;

void SHAREDMEM_WriteDL_AMR_Speech(UInt8 rx_type,UInt8 frame_type, UInt32 *data, UInt8 length, Boolean amr_if2_enable, UInt16 *softA, UInt32 *deciphbit)
{
	int i;
#ifndef L1TEST

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	

	
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->DL_MainAMR_buf);
#else
	UInt16 dummy;
	UInt16 *ptr = &dummy;
#endif
#ifndef __BIG_ENDIAN
 	UInt8 *data8 = (UInt8*)data;
#endif

	if (amr_if2_enable)
	{
		UInt16 *pSrc = (UInt16*)data;
		
		for ( i = 0; i < length*2; i++)
		{
		  //			ptr[i+2] = ((pSrc[i] & 0xff00)) >> 8 | ((pSrc[i] & 0x00ff) << 8);
		  ptr[i+2]=pSrc[i]; 
		}
	}
	else
	{
#ifndef L1TEST
#if defined(UMTS)
	if( SYSPARM_GetMSTWCDMA() /*&& ( !IS_FRAME_TYPE_AMR_WB( frame_type ) )*/ ) //MobC00093269-MST processing only if NB-AMR frame enabled -- No longer valid comment, since WB-AMR also supports MST
	{
		ptr = (UInt16*)&(vp_shared_mem->DL_wcdma_mst_amr_buf);

	}
#endif	
#endif
		for( i = 0 ; i < length; i++)
		{
#ifdef __BIG_ENDIAN
			ptr[2*i+2] = (data[i] >> 16 )&0xffff;
			ptr[2*i+3] = data[i]&0xffff;
#else
			ptr[2*i+2] = data8[i * 4]<<8 | data8[i * 4 +1];
			ptr[2*i+3] = data8[i * 4 + 2]<<8 | data8[i * 4 + 3];
#endif
		}

	}
	if( rx_type == 0 ) //CRC BAD
	{
#if defined(UMTS)
		if( SYSPARM_GetMSTWCDMA() && (!amr_if2_enable) /*&& ( !IS_FRAME_TYPE_AMR_WB( frame_type ) ) */) //MobC00093269-MST processing only for NB-AMR		
		{
		    if( !IS_FRAME_TYPE_AMR_WB( frame_type ) )
		{
			for( i=0; i< 38; i++)
			{
#ifdef __BIG_ENDIAN
				ptr[32 +2*i]= softA[2*i+1];
				ptr[32 + 2*i+1] = softA[2*i];
#else
				ptr[32 +2*i] = softA[2*i];
				ptr[32 + 2*i+1] = softA[2*i+1];
#endif
			}
		
			 if( deciphbit !=NULL)
			 {
#ifndef __BIG_ENDIAN
				data8 = (UInt8*)deciphbit;
#endif
				for( i=0; i <3; i++)
				{
#ifdef __BIG_ENDIAN
					ptr[124+2*i]=  (deciphbit[i]>>16)&0xffff;
					ptr[124+2*i+1]=deciphbit[i]&0xffff; 
#else
				ptr[124+2*i] = data8[i * 4]<<8 | data8[i * 4 +1];
				ptr[124+2*i+1] = data8[i * 4 + 2]<<8 | data8[i * 4 + 3];
#endif
				}
				   ptr[129] = 0x1;
			 }
			 else
			 {
				for( i=0; i< 6; i++)
					ptr[124+i]=0;
			}
	//			L1_LOG_ARRAY("cipher", &ptr[94],12);
		     }
		     else
             {
               //copy the soft bits
               for( i = 0; i < 46; i++ )
               	{
#ifdef __BIG_ENDIAN
				   ptr[32 +2*i]= softA[2*i+1];
				   ptr[32 + 2*i+1] = softA[2*i];
#else
				   ptr[32 +2*i] = softA[2*i];
				   ptr[32 + 2*i+1] = softA[2*i+1];
#endif
               	}
			   if( deciphbit !=NULL)
			   {
#ifndef __BIG_ENDIAN
				  data8 = (UInt8*)deciphbit;
#endif
				  for( i=0; i <3; i++)
				  {
#ifdef __BIG_ENDIAN
					ptr[124+2*i]=  (deciphbit[i]>>16)&0xffff;
					ptr[124+2*i+1]=deciphbit[i]&0xffff; 
#else
				    ptr[124+2*i] = data8[i * 4]<<8 | data8[i * 4 +1];
				    ptr[124+2*i+1] = data8[i * 4 + 2]<<8 | data8[i * 4 + 3];
#endif
				  }
				
				   ptr[129] = 0x1;
			   }
			   else
	           {
				    for( i=0; i< 6; i++)
					    ptr[124+i]=0;
	            }
		     } //WB AMR
		}
#endif


		bad_speech_cnt ++;
		if(bad_speech_cnt >= 5)
		{
			replace_bad_sid = TRUE;
			good_speech_cnt = 0;
		}

		if( !IS_FRAME_TYPE_AMR_WB( frame_type ) ) //MobC00093269-NB-AMR
		{
			if( frame_type < E_AMR_NB_SID )
			{
				ptr[0] = AMR_RX_SPEECH_BAD;
				ptr[1] = frame_type;

			}
			else if( frame_type == E_AMR_NB_SID )
			{
				 if ( replace_bad_sid)
					ptr[0] = AMR_RX_SPEECH_BAD;
				 else
				    ptr[0] = AMR_RX_SID_BAD;

				ptr[1] = AMR_Codec_Mode_Good;
			}
			else if( (frame_type < E_AMR_NB_NO_DATA )&&(frame_type > E_AMR_NB_SID ) ) //If any value between E_AMR_NB_SID and E_AMR_NB_NO_DATA
			{
				xassert(FALSE, frame_type);
			}
			else if( frame_type == E_AMR_NB_NO_DATA )
			{
				ptr[0] = AMR_RX_NO_DATA;
				ptr[1] = AMR_Codec_Mode_Good;
			}
			else
				xassert( FALSE, frame_type);
		}
		else //MobC00093269-WB-AMR
		{
			if(frame_type < E_AMR_WB_SID )
			{
				ptr[0] = AMR_RX_SPEECH_BAD;
				ptr[1] = frame_type;

			}
			else if( frame_type == E_AMR_WB_SID )
			{
				if ( replace_bad_sid)
					ptr[0] = AMR_RX_SPEECH_BAD;
				else
				    ptr[0] = AMR_RX_SID_BAD;
				ptr[1] = AMR_Codec_Mode_Good;
			}
			//MobC00099688, Coverity dead code prevention, Srirang, 01/25/10
			else if( (frame_type < E_AMR_WB_NO_DATA ) && (frame_type > E_AMR_WB_SID ))
			{
			    xassert(FALSE, frame_type);
			}
			else if( frame_type == E_AMR_WB_NO_DATA )
			{
				ptr[0] = AMR_RX_NO_DATA;
				ptr[1] = AMR_Codec_Mode_Good;
			}
			else
				xassert( FALSE, frame_type);
		}
	}
	else if( rx_type == 1) //CRC OK
	{
		good_speech_cnt ++;
		if(good_speech_cnt >= 2)
		{
			replace_bad_sid = FALSE;
			bad_speech_cnt = 0;
		}
		if( ! ( IS_FRAME_TYPE_AMR_WB( frame_type ) ) ) //MobC00093269-NB-AMR
		{
			if( frame_type < E_AMR_NB_SID )
			{
				ptr[0] = AMR_RX_SPEECH_GOOD;
				ptr[1] = frame_type;
				AMR_Codec_Mode_Good = frame_type;
			}
			else if( frame_type == E_AMR_NB_SID )
			{
				if( ptr[4]&0x1000)
					ptr[0] = AMR_RX_SID_UPDATE;
				else
					ptr[0] = AMR_RX_SID_FIRST; //AMR_RX_SID_UPDATE
				ptr[1] = AMR_Codec_Mode_Good ;
			}
			else if( ( frame_type < E_AMR_NB_NO_DATA )&&( frame_type > E_AMR_NB_SID ) )
			{
				xassert(FALSE, frame_type);
			}
			else if( frame_type == E_AMR_NB_NO_DATA )
			{
				ptr[0] = AMR_RX_NO_DATA;
				ptr[1] = AMR_Codec_Mode_Good;
			}
			else
				xassert( FALSE, frame_type);
		}
		else //MobC00093269-WB-AMR
		{
			if(frame_type < E_AMR_WB_SID )
			{
				ptr[0] = AMR_RX_SPEECH_GOOD;
				ptr[1] = frame_type;
				AMR_Codec_Mode_Good = frame_type;
			}
			else if( frame_type == E_AMR_WB_SID )
			{
				if( ptr[4]&0x1000)
					ptr[0] = AMR_RX_SID_UPDATE;
				else
					ptr[0] = AMR_RX_SID_FIRST; //AMR_RX_SID_UPDATE
				ptr[1] = AMR_Codec_Mode_Good ;
			}
			else if( frame_type == E_AMR_WB_NO_DATA )
			{
				ptr[0] = AMR_RX_NO_DATA;
				ptr[1] = AMR_Codec_Mode_Good;
			}
			else if( (frame_type < E_AMR_WB_NO_DATA ) || (frame_type > E_AMR_WB_SID ) )
			{
			    xassert(FALSE, frame_type);
			}
			else
				xassert( FALSE, frame_type);
		}
	}
	else
		xassert( FALSE, rx_type);

#ifdef AMR_LOGGING
	Log_DebugSignal( UMACDL_AMR_DL, ptr, 36, 0, 0 );
#endif
}
//******************************************************************************
//
// Function Name:	SHAREDMEM_DLAMR_Speech_Init
//
// Description:		Clear Downlink AMR Speech Data 
// Notes:
//
//******************************************************************************
void SHAREDMEM_DLAMR_Speech_Init(void)
{

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	

	UInt16 *ptr = (UInt16*)&(vp_shared_mem->DL_MainAMR_buf);

	ptr[0] = AMR_RX_NO_DATA;
	ptr[1] = AMR_Codec_Mode_Good;
	memset( &ptr[2], 0, 32);
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ULAMR_Speech_Init
//
// Description:		Clear Uplink AMR Speech Data 
// Notes:
//
//******************************************************************************
void SHAREDMEM_ULAMR_Speech_Init(void)
{

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	

	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
	ptr[0] = AMR_TX_NO_DATA;
	ptr[1] = AMR_Codec_Mode_Good;
	memset( &ptr[2], 0, 32);
}

 
//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadUL_AMR_Speech
//
// Description:		Read uplink AMR Speech Data from Share Memory
//
// Notes:
//
//******************************************************************************
static UInt32 ul_speech_data[8];
//AMR-NB version of read buffer
void SHAREDMEM_ReadUL_AMR_Speech(UInt32 **speech_data, Boolean amr_if2_enable)
{
	int i;

	UInt16 UL_AMR_buf[18];
	UInt8  *temp_buf8 = (UInt8  *)UL_AMR_buf;

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	

	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
	//Note: ptr[0] RX_Frame_type
	//      ptr[1] AMR code rate
	//      ptr[2]~ptr[17] speech data   
//	L1_LOG_ARRAY("AMR UL", ptr,36);
#ifdef __BIG_ENDIAN
	memcpy(UL_AMR_buf, ptr,36);
#else
	UL_AMR_buf[0]=ptr[0];
	UL_AMR_buf[1]=ptr[1];
	for ( i = 2; i < 18; i++)
	{
		temp_buf8[i*2] = ptr[i] >> 8;
		temp_buf8[i*2 + 1] = ptr[i] & 0xFF;
	}
#endif

	if(UL_AMR_buf[0] == AMR_TX_SID_FIRST)
	{
		memset(&UL_AMR_buf[2],0,32);
#ifdef __BIG_ENDIAN
		UL_AMR_buf[4] = ((ptr[1]&0x1)<<11)|((ptr[1]&0x2)<<9)|((ptr[1]&0x4)<<7);
#else
		UL_AMR_buf[4] = ((ptr[1]&0x1)<<3)|((ptr[1]&0x2)<<1)|((ptr[1]&0x4)<<15);
#endif
	}
	if( UL_AMR_buf[0] == AMR_TX_SID_UPDATE)
	{
		memset(&UL_AMR_buf[5],0,26);
#ifdef __BIG_ENDIAN
		UL_AMR_buf[4] |= ((ptr[1]&0x1)<<11)|((ptr[1]&0x2)<<9)|((ptr[1]&0x4)<<7)|0x1000;
#else
		UL_AMR_buf[4] |= ((ptr[1]&0x1)<<3)|((ptr[1]&0x2)<<1)|((ptr[1]&0x4)<<15)|0x10;
#endif
	}
	if (amr_if2_enable)
	{
		UInt16 *pDst = (UInt16*)ul_speech_data;
		for ( i = 0; i < 16; i++)
		{
			pDst[i] = ((UL_AMR_buf[i+2] & 0xff00)) >> 8 | ((UL_AMR_buf[i+2] & 0x00ff) << 8);
		}
	}
	else
	{
		UInt32 * temp_buf32 = (UInt32  *)UL_AMR_buf;
		for( i =0; i < 8; i++)
		{
			ul_speech_data[i] = temp_buf32[i+1];
		}
	}
#if defined(UMTS) && defined(DEVELOPMENT_ONLY)
	if(is_amr_voice_test)
	{
		for( i = 1; i < 8; i++)
		{
			ul_speech_data[i] = 99000*(i+31)*(i+1)+99*i+987543; 
		}
		amr_voice_counter ++;
		ul_speech_data[0] = amr_voice_counter;
		L1_LOG_ARRAY("AMR TEST UL", ul_speech_data ,32);
	}
#endif
	*speech_data = ul_speech_data;

//	L1_LOGV4("UL AMR speech", ul_speech_data[0],ul_speech_data[1],ul_speech_data[2],ul_speech_data[3]);
}

//Max WB-AMR frame size is 60 bytes (477 bits rounded to nearest byte) and there is 4 bytes header on the buffer received from DSP. So we allocate 16 DWORDS size buffer
static UInt32 ul_amr_wb_speech_data[16];
//MobC00093269 - AMR-WB version of read buffer
void SHAREDMEM_ReadUL_WB_AMR_Speech(UInt32 **speech_data, Boolean amr_if2_enable)
{
	int i;
	UInt16 UL_AMR_buf[32]; //Max WB-AMR frame size is 60 bytes (477 bits rounded to nearest byte) and there is 4 bytes header on the buffer received from DSP
	UInt8  *temp_buf8 = (UInt8 *)UL_AMR_buf;

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	

	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);

	//Note: ptr[0] RX_Frame_type
	//      ptr[1] AMR code rate
	//      ptr[2]~ptr[31] speech data
//	L1_LOG_ARRAY("AMR UL", ptr, 62);
#ifdef __BIG_ENDIAN
	memcpy(UL_AMR_buf, ptr, 33);
#else
    //Copy out the header. Header is 4 bytes
	UL_AMR_buf[0]=ptr[0];
	UL_AMR_buf[1]=ptr[1];
	//Copy the Speech data which is max 60 bytes for highest WB-AMR rate
	for ( i = 2; i < 32; i++)
	{
		temp_buf8[i*2] = ptr[i] >> 8;
		temp_buf8[i*2 + 1] = ptr[i] & 0xFF;
	}
#endif

	if(UL_AMR_buf[0] == AMR_TX_SID_FIRST)
	{
		memset(&UL_AMR_buf[2], 0, 60);
#ifdef __BIG_ENDIAN
		UL_AMR_buf[4] = ((ptr[1]&0x1)<<11)|((ptr[1]&0x2)<<9)|((ptr[1]&0x4)<<7);
#else
		UL_AMR_buf[4] = ((ptr[1]&0x1)<<3)|((ptr[1]&0x2)<<1)|((ptr[1]&0x4)<<15);
#endif
	}
	if( UL_AMR_buf[0] == AMR_TX_SID_UPDATE)
	{
		memset(&UL_AMR_buf[5], 0, 54);
#ifdef __BIG_ENDIAN
		UL_AMR_buf[4] |= ((ptr[1]&0x1)<<11)|((ptr[1]&0x2)<<9)|((ptr[1]&0x4)<<7)|0x1000;
#else
		UL_AMR_buf[4] |= ((ptr[1]&0x1)<<3)|((ptr[1]&0x2)<<1)|((ptr[1]&0x4)<<15)|0x10;
#endif
	}
	if (amr_if2_enable)
	{
		UInt16 *pDst = (UInt16*)ul_amr_wb_speech_data;
		for ( i = 0; i < 30; i++)
		{
			pDst[i] = ((UL_AMR_buf[i+2] & 0xff00)) >> 8 | ((UL_AMR_buf[i+2] & 0x00ff) << 8);
		}
	}
	else
	{
		UInt32 * temp_buf32 = (UInt32  *)UL_AMR_buf;
		//Now copy the actual data which is made of 15 DWORDs or 60 bytes by skipping first 4 bytes of header
		for( i = 0; i < 15; i++)
		{
			ul_amr_wb_speech_data[i] = temp_buf32[i+1];
		}
	}
#if defined(UMTS) && defined(DEVELOPMENT_ONLY)
	if(is_amr_voice_test)
	{
		for( i = 1; i < 15; i++)
		{
			ul_amr_wb_speech_data[i] = 99000*(i+31)*(i+1)+99*i+987543;
		}
		amr_voice_counter ++;
		ul_amr_wb_speech_data[0] = amr_voice_counter;
	}
#endif
	*speech_data = ul_amr_wb_speech_data;
    //L1_LOGV4("UL AMR speech", ul_speech_data[0],ul_speech_data[1],ul_speech_data[2],ul_speech_data[3]);
} //End of SHAREDMEM_ReadUL_WB_AMR_Speech()


//******************************************************************************
//
// Function Name:	VPSHAREDMEM_ReadUL_AMR_TxType
//
// Description:		Read uplink AMR Speech Data from Share Memory
//
// Notes:
//
//******************************************************************************
UInt16 SHAREDMEM_ReadUL_AMR_TxType(void)
{



	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	


	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
	return( ptr[0]);
}

//******************************************************************************
//
// Function Name:	VPSHAREDMEM_ReadUL_AMR_Mode
//
// Description:		Read uplink AMR Speech Codec Mode from Share Memory
//
// Notes:
//
//******************************************************************************
UInt16 SHAREDMEM_ReadUL_AMR_Mode(void)
{

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	


	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
	UInt16 temp;

	if( ptr[0] == AMR_TX_SPEECH_GOOD)
	{
		temp = ptr[1];
	}
	else if( (ptr[0] == AMR_TX_SID_FIRST)|| (ptr[0]==AMR_TX_SID_UPDATE))
	{
		temp = 8;
	}
	else if( ptr[0] == AMR_TX_NO_DATA)
	{
		temp = 15;
	}
	else
	{
		xassert( FALSE, ptr[0]);
		temp = 0;
	}
	//L1_LOGV4("UL type,mode, mac_frame_type", ptr[0],ptr[1],temp,0);
	return(temp);
}

//MobC00093269 - Get BO or the frame rate, called by UMACUL before Speech TFC selection
UInt16 SHAREDMEM_ReadUL_UMTS_AMR_Mode( T_AMR_CODEC_TYPE codec_type )
{

	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());	


	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
	UInt16 temp = E_AMR_NB_NO_DATA; //Initialize to no data

	if( ptr[0] == AMR_TX_SPEECH_GOOD)
	{
		temp = ptr[1];
	}
	else if( (ptr[0] == AMR_TX_SID_FIRST)|| (ptr[0]==AMR_TX_SID_UPDATE))
	{
		temp = ( codec_type == E_AMR_NB ) ? E_AMR_NB_SID : E_AMR_WB_SID;
	}
	else if( ptr[0] == AMR_TX_NO_DATA)
	{
		temp = ( codec_type == E_AMR_NB ) ? E_AMR_NB_NO_DATA : E_AMR_WB_NO_DATA;
	}
	else
	{
		xassert( FALSE, ptr[0]);
	}
//	L1_LOGV4("UL type,mode, mac_frame_type", ptr[0],ptr[1],temp,0);
	return(temp);
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_set_audio_logging
//
// Description:		Initialize LOgging Flag 
//
// Notes:
//
//******************************************************************************
void  SHAREDMEM_set_audio_logging_flag(UInt16 input)
{
	at_mdsptst_audio_logging = input;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_get_audio_logging
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
UInt16  SHAREDMEM_get_audio_logging_flag(void)
{
	return (at_mdsptst_audio_logging);
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_set_track_logging_flag
//
// Description:		Initialize LOgging Flag 
//
// Notes:
//
//******************************************************************************
void  SHAREDMEM_set_track_logging_flag(UInt16 input)
{
	at_mtst_track_logging  = input;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_get_track_logging_flag
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
UInt16  SHAREDMEM_get_track_logging_flag(void)
{
	return (at_mtst_track_logging );
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_set_ncell_logging_flag
//
// Description:		Initialize LOgging Flag 
//
// Notes:
//
//******************************************************************************
void  SHAREDMEM_set_ncell_logging_flag(UInt16 input)
{
	at_mtst_ncell_logging  = input;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_get_ncell_logging_flag
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
UInt16  SHAREDMEM_get_ncell_logging_flag(void)
{
	return (at_mtst_ncell_logging );
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_set_fer_logging_flag
//
// Description:		Initialize LOgging Flag 
//
// Notes:
//
//******************************************************************************
void  SHAREDMEM_set_fer_logging_flag(UInt16 input)
{
	at_mtst_fer_logging  = input;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_get_fer_logging_flag
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
UInt16  SHAREDMEM_get_fer_logging_flag(void)
{
	return (at_mtst_fer_logging );
}



//******************************************************************************
//
// Function Name:	SHAREDMEM_set_frameentry_logging_flag
//
// Description:		Initialize LOgging Flag 
//
// Notes:
//
//******************************************************************************
void  SHAREDMEM_set_frameentry_logging_flag(Boolean input)
{
	at_mtst_frameentry_logging  = input;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_get_frameentry_logging_flag
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
Boolean  SHAREDMEM_get_frameentry_logging_flag(void)
{
	return (at_mtst_frameentry_logging );
}
//******************************************************************************
//
// Function Name:	SHAREDMEM_get_fsc_logging_flag
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
Boolean  SHAREDMEM_get_fsc_logging_flag(void)
{
	return (at_mtst_fsc_logging );
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_get_event_logging_flag
//
// Description:		Return logging flag
//
// Notes:
//
//******************************************************************************
Boolean  SHAREDMEM_get_event_logging_flag(void)
{
	return (at_mtst_event_logging );
}


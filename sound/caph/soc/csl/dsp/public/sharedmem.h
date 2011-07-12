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

#ifndef	_INC_SHAREDMEM_H_
#define	_INC_SHAREDMEM_H_



#include "shared.h"
#include "sharedmem_comms.h"    
#include "chip_version.h"
//#include "dsp_feature_def.h"

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

#ifndef _NODEBUG
   
#define CMD_Q_HIST_SZ		(2 << 7)
#define STAT_Q_HIST_SZ		(2 << 7)

typedef struct
{
	UInt32		time_stamp;
	CmdQ_t		cmd_q;
 	UInt8		shared_cmdq_in;
	UInt8		shared_cmdq_out;
} CmdQHist_t;

typedef struct
{
	UInt32		time_stamp;
 	StatQ_t		stat_q;
	UInt8		shared_statusq_in;
	UInt8		shared_statusq_out;
} StatQHist_t;

extern CmdQHist_t	cmd_q_hist[CMD_Q_HIST_SZ];
extern CmdQHist_t	fcmd_q_hist[CMD_Q_HIST_SZ];
extern StatQHist_t	stat_q_hist[STAT_Q_HIST_SZ];
#endif //_NODEBUG

typedef enum
{
	E_AMR_NB = 0x06,
	E_AMR_WB = 0x0A,
	E_AMR_INVALID
}T_AMR_CODEC_TYPE;

/* AMR frame rate for NB and WB */
typedef enum
{
	E_AMR_NB_4_DOT_75 = 0x00,
	E_AMR_NB_5_DOT_15 = 0x01,
	E_AMR_NB_5_DOT_90 = 0x02,
	E_AMR_NB_6_DOT_70 = 0x03,
	E_AMR_NB_7_DOT_40 = 0x04,
	E_AMR_NB_7_DOT_95 = 0x05,
	E_AMR_NB_10_DOT_2 = 0x06,
	E_AMR_NB_12_DOT_2 = 0x07,
	E_AMR_NB_SID = 0x08,
	E_AMR_NB_NO_DATA = 0x0F,

	E_AMR_WB_6_DOT_6 = 0x10,
	E_AMR_WB_8_DOT_85 = 0x11,
	E_AMR_WB_12_DOT_65 = 0x12,
	E_AMR_WB_14_DOT_25 = 0x13,
	E_AMR_WB_15_DOT_85 = 0x14,
	E_AMR_WB_18_DOT_25 = 0x15,
	E_AMR_WB_19_DOT_85 = 0x16,
	E_AMR_WB_23_DOT_05 = 0x17,
	E_AMR_WB_23_DOT_85 = 0x18,
	E_AMR_WB_SID = 0x19,
	E_AMR_WB_NO_DATA = 0x30
}T_AMR_FRAME_TYPE;

#define IS_FRAME_TYPE_AMR_WB(frame_type) ( (  frame_type ) >> 4 )


void SHAREDMEM_Init( void );			// Initialize Shared Memory

void SHAREDMEM_PostCmdQ(				// Post an entry to the command queue
	CmdQ_t *cmd_msg						// Entry to post
	);
void SHAREDMEM_PostCmdQ_LISR(				// Post an entry to the command queue
	CmdQ_t *cmd_msg						// Entry to post
	);

void SHAREDMEM_PostFastCmdQ(			// Post an entry to the fast command queue
	CmdQ_t *cmd_msg						// Entry to post
	);
void SHAREDMEM_PostFastCmdQ_LISR(			// Post an entry to the fast command queue
	CmdQ_t *cmd_msg						// Entry to post
	);

Boolean SHAREDMEM_ReadStatusQ(			// Read an entry from the command queue
	StatQ_t *status_msg					// Entry from queue
	);									// TRUE, if entry is available

void SHAREDMEM_WriteTestPoint(			// Write to a test point
	UInt8	tp,							// Test Point
	UInt16	value						// Test Point Value
	);

//SharedMem_t *SHAREDMEM_GetSharedMemPtr( void );

#if defined(DEVELOPMENT_ONLY)

void  SHAREDMEM_set_audio_logging_flag(UInt16 input);
UInt16  SHAREDMEM_get_audio_logging_flag(void);
Boolean  SHAREDMEM_is_audio_logging_set(UInt16 mask);
void  SHAREDMEM_set_track_logging_flag(UInt16 input);
UInt16  SHAREDMEM_get_track_logging_flag(void);
void  SHAREDMEM_set_ncell_logging_flag(UInt16 input);
UInt16  SHAREDMEM_get_ncell_logging_flag(void);
void  SHAREDMEM_set_fer_logging_flag(UInt16 input);
UInt16  SHAREDMEM_get_fer_logging_flag(void);
void  SHAREDMEM_set_frameentry_logging_flag(Boolean input);
Boolean  SHAREDMEM_get_frameentry_logging_flag(void);
Boolean  SHAREDMEM_get_fsc_logging_flag(void);
Boolean  SHAREDMEM_get_event_logging_flag(void);

#endif	//  DEVELOPMENT_ONLY

void SHAREDMEM_ReadUL_WB_AMR_Speech(UInt32 **speech_data, Boolean amr_if2_enable); //MobC00093269 - AMR-WB version of read buffer
//MobC00093269- Maintained this function for VPU interface -
//Remove it if VPU also uses the SHAREDMEM_ReadUL_UMTS_AMR_Mode version in which case the below function has to be deleted or updated
UInt16 SHAREDMEM_ReadUL_UMTS_AMR_Mode( T_AMR_CODEC_TYPE codec_type ); //MobC00093269 - AMR-WB version of BO query

#endif
										// Return pointer to shared memory
#endif	// _INC_SHAREDMEM_H_




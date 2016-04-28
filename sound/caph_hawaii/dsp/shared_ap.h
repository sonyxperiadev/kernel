/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#ifndef    _INC_SHARED_AP_H_
#define    _INC_SHARED_AP_H_

#ifdef MSP
#include "mobcom_types.h"
#else
#include "types.h"
#endif
#include "consts.h"

/**
 * \defgroup AP_Shared_Memory AP_Shared_Memory
 * @{
 */

//******************************************************************************
// The RIP definition of the shared memory simply places each variable in a
//        special data section.
//***********************************************************************************************************
// THE DEFINES ARE JUST FOR TAGS ONLY. THERE IS NO PAGING IN TLIII MEMORY ARCH.
// PLEASE DO NOT USE THEM AS PAGING CONCEPT
//           DO NOT FORCE DEFINES TO HARD CODED ADDRESSES 
// GENERAL
//        SEC_CMD_STATUS
//    AUDIO
//        SEC_GEN_AUDIO 
// 
//    DEBUG 
//        SEC_DIAGNOSIS     
//
//
//***********************************************************************************************************
#ifdef RIP
// General
#define AP_SHARED_SEC_CMD_STATUS       __attribute__( ( section( ".DSECT ap_shared_sec_cmd_status") ) )          
// Audio                                    
#define AP_SHARED_SEC_GEN_AUDIO        __attribute__( ( section( ".DSECT ap_shared_sec_gen_audio") ) )          
// Debug
#define AP_SHARED_SEC_DIAGNOS          __attribute__( ( section( ".DSECT ap_shared_sec_diagnos") ) )          
#define DSP_AP_SCRATCHMEM_SEC	\
	__attribute__((section(".DSECT dsp_ap_scratchmem_sec")))
#    undef EXTERN
#    ifdef RIP_EXTERN_DECLARE
#        define EXTERN
#    else
#        define    EXTERN    extern
#    endif
#endif	// RIP
 
#ifdef MSP
#define EXTERN
#define    AP_SHARED_SEC_CMD_STATUS        
#define    AP_SHARED_SEC_GEN_AUDIO
#define    AP_SHARED_SEC_DIAGNOS         
#define	DSP_AP_SCRATCHMEM_SEC
#endif	// MSP



#define	VP_CMDQ_SIZE					16		// Number of command queue entries
#define	VP_STATUSQ_SIZE					8		// Number of status queue entries
#define G711_FRAME_SIZE                 40    	// packed 80 samples (10ms) frame 	

#define NUM_OF_8K_SAMP_PER_INT0_INT     8       // Number of 8kHz samples per INT0 interrupt (for 16kHz * 2)
                                                // Should be same as in hwregs.inc
#define NUM_OF_48K_SAMP_PER_INT0_INT    (3*2*8) // Number of 48kHz samples per INT0 interrupt (for 16kHz * 2)
                                                // Should be same as in hwregs.inc
#define NUM_OF_48K_SAMP_PER_LEG_INT0_INT (3)    // Number of 48kHz samples per INT0 interrupt (for 16kHz * 2)
                                                // Should be same as in hwregs.inc

/**
 * \defgroup External_Modem_Interface External_Modem_Interface
 * @{
 */
/**
 * Define for number of samples to be transferred per AADMAC interrupt for
 * external modem
 */
#define NUM_OF_EXT_MODEM_INTERF_SAMP_PER_INT  160
/**
 * @}
 */

#define DJB_BUFFER_SIZE     			600		// 600 words ~ 300ms

/*---------- Defining supported features --------*/

/* AADMAC buffers are in scratch memory instead of shared memory */
#define SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BIT 0
#define SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BITF \
	(1<<SUPP_FEAT_AADMAC_BUF_SCRATCH_MEM_BIT)


/*---------- Typedefs --------*/
typedef struct
{
	UInt32 array2[35];
} dummy2_config_t; 

typedef struct
{
 UInt32 array3[10];
} dummy3_queries_t;

typedef struct
{
	UInt32 array4[2];											   
} dummy4_input_t;										

typedef struct
{
    UInt16 cmd;
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
} VPCmdQ_t;

typedef struct
{
    UInt16 status;
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
    UInt16 arg3;
} VPStatQ_t;


// VOIP data struct and defines
typedef enum
{
    GOOD_FRAME,
    BAD_FRAME
} G711_FrameType_t;

typedef struct       // G.711 speech data for a 10ms frame
{
	UInt16				frame_type; // 0 - good frame, 1 - bad frame
	UInt16 				payload[G711_FRAME_SIZE];    
} VR_Frame_G711_t;

typedef struct 		 				//  Data block of voice recording
{    
	UInt16 voip_vocoder;
	                
	union
	{
		UInt16			frame_pcm[320];		//voip mode 1/5: PCM, 8KHz use 160, 16KHz use 320
		UInt16			frame_fr[79];		//voip mode 2: [VAD, SP, dtx_enable|BFI, SID, TAF], fr special format (76w)
		UInt16			frame_amr[18];		//voip mode 3: frametype, mode, coded_bit(MSB first)
		VR_Frame_G711_t	frame_g711[2];		//voip mode 4: G.711 encoded samples
		VR_Frame_AMR_WB_t frame_amr_wb;		//voip mode 6: AMR-WB
	} voip_frame;
} VOIP_Buffer_t;        

//******************************************************************************
//
//        Data structure for voice recording
//
//******************************************************************************

typedef struct 		 				//  Data block of voice recording
{                    
	UInt16	vp_speech_mode;			///< FR/EFR/AMR/Linear PCM speech mode
	UInt16	nb_frame;				///< Number of vectors
	union
	{
		VR_Frame_FR_t	vectors_fr[RECORDING_FRAME_PER_BLOCK];
		VR_Frame_EFR_t	vectors_efr[RECORDING_FRAME_PER_BLOCK_EFR];	
		VR_Lin_PCM_t	vectors_lin_PCM[RECORDING_FRAME_PER_BLOCK_LPCM];
		VR_Frame_AMR_t	vectors_amr[RECORDING_FRAME_PER_BLOCK_AMR];	
	}vr_frame;
} VRecording_Buffer_t;        

//******************************************************************************
//
//        Data structure for voice playback
//
//******************************************************************************

typedef struct 		 				//  Data block of voice playback
{                    
	UInt16	vp_speech_mode;			// FR/EFR/AMR/Linear PCM speech mode
	UInt16	nb_frame;				// Number of vectors
	union
	{
		VR_Frame_FR_t	vectors_fr[PLAYBACK_FRAME_PER_BLOCK];
		VR_Frame_EFR_t	vectors_efr[PLAYBACK_FRAME_PER_BLOCK_EFR];	
		VR_Lin_PCM_t	vectors_lin_PCM[PLAYBACK_FRAME_PER_BLOCK_LPCM];
		VR_Frame_AMR_t	vectors_amr[RECORDING_FRAME_PER_BLOCK_AMR];	
	}vp_frame;
} VPlayBack_Buffer_t;

//******************************************************************************
//
//		Data structures for voice recording and playback
//
//******************************************************************************

typedef struct
{
	VRecording_Buffer_t vr_buf[2];	// Voice recording (ping-pong) buffer
	VPlayBack_Buffer_t vp_buf[2];   // Voice playback (ping-pong) buffer
} shared_voice_buf_t;				// buffer for voice recording and playback

typedef struct
{
   UInt16 *freeListp;
   UInt16 *plStartp;
   UInt16 *plEndp;
   UInt16 freeListSize;
   UInt16 numEntry;
   UInt16 entrySizeInWords;
   UInt16 numAlloc;
} DJB_PAYLOADQ;

/* Adaptive Jitter Control Module (decoder) Statistics */
typedef struct
{
   /* Not in state structure because we want to clear it often with stats reset flag */
   UInt16    peakHoldingTime;    /* Peak Holding Time since last statistic */

   /* Total number of superpackets arriving on the egress path */
   UInt16    packetCount;        /* # of packets received */

   /* addTail and reorder are successful additions to the JB */
   /* addTail is typically 1..N times the packetCount if superpackets are enabled */
   UInt16    addTailCount;       /* # of frames added to tail of JB - normal case */
   UInt16    reorderCount;       /* # of frames reordered */

   /* overrun, duplicate, outOfRange, tooLate, jitterMax excess are packets that have been discarded */
   UInt16    overrunCount;       /* Decoder overrun count */
   UInt16    duplicateCount;     /* # of duplicate frames deleted */
   UInt16    outOfRangeCount;    /* # of frames with timestamps too far from current TS to be handled */
   UInt16    tooLateCount;       /* Packet arrived too late (it's redundant mate already played) */

   /* cantDecode are packets that can't be played out due to algorithm not available */
   UInt16    cantDecodeCount;    /* Can't decode packet - decoder not in load or pkt hdr bad */

   /* The following are internal to the AJC module - they do not represent physical packets */
   UInt16    ajcUnderrunCount;   /* Adaptive Jitter Control: jitter buffer underruns */
   UInt16    ajcDeleteCount;     /* Adaptive Jitter Control: # of packet deletes done to reduce holding time */
   UInt16    ajcRepeatCount;     /* Adaptive Jitter Control: # of packet repeats done to either increase holding time
                                    or due to late frame or lost frames. */
   UInt16    ajcResyncCount;     /* Number of times ajb resynced (went through buildout state) */
   UInt16    ajcPhaseJitterCount;     /* Number of times ajb inserted a phase discontinuity
                                    (possibly in silence or during CNG or due to a repeat/delete). */
   UInt16    ajcHdrPayloadDiff; /* difference between header and payload buffer pool allocation (should be always 0) */
   UInt16    ajcPayloadOverflowE; /* number of overruns in the payload buffer when the header pool is not full (should be always 0) */
   UInt16    ajcStatsReserve1;
   UInt16    ajcStatsReserve2;
   UInt16    ajcStatsReserve3;
   UInt16    ajcStatsReserve4;
} DJB_STATS;

//******************************************************************************
// Shared memory enumerations
//******************************************************************************

typedef enum
{
    RECORD_NONE = 0,
    MICROPHONE_ONLY = 1,
    FROM_CHANNEL_ONLY = 2,
    MICROPHONE_AND_CHANNEL = 3
} VP_Record_Mode_t;

typedef enum
{
    PLAYBACK_NONE = 0,
    SPEAKER_EAR_PIECE_ONLY = 1,
    OVER_THE_CHANNEL_ONLY = 2,
    SPEAKER_EAR_PIECE_AND_CHANNEL = 3,
    AMR_TONE_DECODE = 4,
    AMR_TONE_DECODE_2_MICROPHONE_ONLY = 5
} VP_Playback_Mode_t;

    
typedef enum
{
    MIX_NONE = 0,                                // mix none
    MIX_SPEAKER_EAR_PIECE_ONLY = 1,              // mix vpu decodering with dnlink, send to speaker
    MIX_OVER_THE_CHANNEL_ONLY = 2,               // mix vpu decodering with microphone, send to uplink
    MIX_SPEAKER_EAR_PIECE_AND_CHANNEL = 3        // mix vpu decodering with both, send respectively
} VP_PlaybackMix_Mode_t;

typedef enum
{
    VP_MODE_IDLE,
    VP_MODE_RECOGNITION,                         // **NOT USED ANYMORE**
    VP_MODE_TRAINING,                            // **NOT USED ANYMORE**
    VP_MODE_RECOG_PLAYBACK,                      // **NOT USED ANYMORE**
    VP_MODE_MEMO_RECORDING,
    VP_MODE_CALL_RECORDING,
    VP_MODE_RECORDING_PLAYBACK,
    VP_MODE_RECORD_AND_PLAYBACK_NOT_USED,
    VP_MODE_NULL_FRAME_INT
}VPMode_t;

typedef enum
{
    PR_MODE_IDLE,
    PR_MODE_PLAY
}PRMode_t;

typedef enum
{
    PR_OUTMODE_FIFO = 0,
    PR_OUTMODE_8KHZ = 1
}PROUTMode_t;

typedef enum
{
    PR_OUTDIR_STEREO_PR_HW = 0,                        //default, backward compatiable
    PR_OUTDIR_SPEAKER_EAR_PIECE_ONLY = 1,            //only work with PR_OUTMODE_8KHZ
    PR_OUTDIR_OVER_THE_CHANNEL_ONLY = 2,            //only work with PR_OUTMODE_8KHZ
    PR_OUTDIR_SPEAKER_EAR_PIECE_AND_CHANNEL = 3        //only work with PR_OUTMODE_8KHZ
}PROUTDir_t;

typedef enum
{
    SRC_0 = 0,                                        //no SRC
    SRC_48K8K = 1,                                    //SRC 48KHz to 8KHz
    SRC_48K16K = 2                                    //SRC 48KHz to 16KHz
}SampleRateC_t;

// ---- Typedef Declarations -----------------------------------------
/**
 * @addtogroup AP_Queues
 * @{
 */
/**
 * @addtogroup VP_Queues
 * @{
 */
/**
 * @addtogroup VP_Command_Queue
 * @{ 
 */
 typedef enum
{// Command						       		Code		Arguments (** = unused command)
 // ========================	       		====		====================================
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x00  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    */
	VP_COMMAND_START_TRAINING,				// 0x0 	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x01  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    */
	VP_COMMAND_CANCEL_TRAINING,	 			// 0x1	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x02  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_START_RECOGNITION,			// 0x2	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x03  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_CANCEL_RECOGNITION,			// 0x3	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x04  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_START_RECOGNITION_PLAYBACK,	// 0x4	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x05  
    *  \par Description 
    *              Initiates Speech Call Recording (working in both idle and speech call mode).
    *              Call Recording is a legacy command name. It can be used in both idle mode 
    *              (as Memo Recording) and speech call mode. In idle mode, the only source is 
    *              the microphone. While during the speech call, it is flexible to record from 
    *              a variety of sources:
    *
    *              \htmlonly <ol> 
    *              <li> Recording Near-End speech exclusively, i.e., from the microphone e.g. for voice recognition. </li>
    *              <li> Recording Far-End speech exclusively, i.e., from the channel and lastly </li>
    *              <li> Recording both Near and Far-End speech from the microphone and the channel default. </li>
    *              </ol>
    *              <br>
    *              \endhtmlonly
    *
    *              For Call Recording, the recorded format can be either PCM or AMR format. \BR
    *
    *              This is the main Speech recording command from the ARM. This command is to 
    *              be sent at the start of the memo recording. Before getting this command
    *              ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
    *              the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
    *              
    *              After getting this
    *              command, DSP keeps getting 20ms worth of data from the microphone path and keeps
    *              encoding the data in the mode required by this command.  After every speech buffer 
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
    *              has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
    *              generates an interrupt.  This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
    *              
    *              @param  VP_Record_Mode_t vp_record_mode; Indicates the source of recording
    *              @param  UInt16 nr_frame;  Number of 20ms frames to record per speech buffer.
    *              @param  UInt16 {bit8:      enables Noise Suppressor for Recording
    *                              bit7:      vp_dtx_enable; DTX enable for AMR encoder
    *                              bit6-bit4: VP_Speech_Mode_t vp_speech_mode; Only support LINEAR_PCM and AMR_2.
    *                              bit3:      N/A
    *                              bit2-bit0: AMR_Mode vp_amr_mode;} 
    *  \see  VP_Record_Mode_t, VP_Speech_Mode_t, AMR_Mode, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *        VP_STATUS_RECORDING_DATA_READY, VP_COMMAND_CANCEL_RECORDING, VP_COMMAND_STOP_RECORDING,
    *        shared_voice_buf
    *  
    */
	VP_COMMAND_START_CALL_RECORDING,		// 0x5 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x06  
    *  \par Description 
    *              Terminates recording modes at the nr_frame boundary of ping-pong buffer, i.e. after it completes 
    *              emptying the current ping-pong buffer.
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_STOP_RECORDING,				// 0x6 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x07  
    *  \par Description 
    *              Initiates Memo Recording. This is used for recording in idle mode. This is a 
    *              legacy command and can be potentially be removed. The functionality of this 
    *              command can be easily be provided by the VP_COMMAND_START_CALL_RECORDING command.
    *              This command is to be sent at the start of the memo recording. Before getting this 
    *              command ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, 
    *              and the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. After getting this
    *              command, DSP keeps getting 20ms worth of data from the microphone path and keeps
    *              collecting the data as required by this command. After every speech buffer 
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
    *              has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
    *              generates an interrupt.  This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
    *              This command only supports recording of data in PCM format
    *              
    *              @param  VP_Record_Mode_t vp_record_mode; Indicates the source of recording; Only VP_SPEECH_MODE_LINEAR_PCM_8K or 
    *                                                        VP_SPEECH_MODE_LINEAR_PCM_16K supported. Default (VP_SPEECH_MODE_LINEAR_PCM_8K)
    *              @param  UInt16 nr_frame;  Number of 20ms frames to record per speech buffer.
    *              @param  {bit8: enables Noise Suppressor for Recording, bit7: DTX enable}
    *
    *  \see VP_COMMAND_START_CALL_RECORDING, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *       VP_STATUS_RECORDING_DATA_READY, VP_COMMAND_CANCEL_RECORDING, VP_COMMAND_STOP_RECORDING,
    *       shared_voice_buf
    *  
    */
    VP_COMMAND_START_MEMO_RECORDING,		// 0x7 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x08  
    *  \par Description 
    *              This command Initiates Playback. The speech data can be in either PCM format or 
    *              AMR format and is read by the DSP from Shared memory. The playback modes of 
    *              operation are activated and de-activated via this same command.
    *
    *              \htmlonly 
    *              <pre>
    *     +-----------------------------+---------------------------------------------------------------------------------------------------------+
    *     |vp_playback_mode             |                         vp_playbackmix_mode                                                             |
    *     |                             +-------------+----------------------------+---------------------------+----------------------------------+
    *     |                             |  MIX_NONE   | MIX_SPEAKER_EAR_PIECE_ONLY | MIX_OVER_THE_CHANNEL_ONLY | MIX_SPEAKER_EAR_PIECE_AND_CHANNEL|
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |PLAYBACK_NONE                | UL=UL_SP    | UL=UL_SP                   | UL=UL_SP                  | UL=UL_SP                         | 
    *     |                             | DL=DL_SP    | DL=DL_SP                   | DL=DL_SP                  | DL=DL_SP                         |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |SPEAKER_EAR_PIECE_ONLY       | UL=UL_SP    | UL=UL_SP                   | UL=UL_SP                  | UL=UL_SP                         |
    *     |                             | DL=Playback | DL=DL_SP+Playback          | DL=Playback               | DL=DL_SP+Playback                |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |OVER_THE_CHANNEL_ONLY        | UL=Playback | UL=Playback                | UL=UL_SP+Playback         | UL=UL_SP+Playback                |
    *     |                             | DL=DL_SP    | DL=DL_SP                   | DL=DL_SP                  | DL=DL_SP                         |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |SPEAKER_EAR_PIECE_AND_CHANNEL| UL=Playback | UL=Playback                | UL=UL_SP+Playback         | UL=UL_SP+Playback                | 
    *     |                             | DL=Playback | DL=DL_SP+Playback          | DL=Playback               | DL=DL_SP+Playback                |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *              </pre>
    *              <br>
    *              \endhtmlonly
    *
    *              Upon activation, this operating mode continues under full control of the ARM.  
    *              After every speech buffer has been read and processed, the DSP issues a 
    *              VP_STATUS_PLAYBACK_DATA_EMPTY command to the ARM and generates an interrupt. This prompts the 
    *              ARM to write new speech playback data into the Shared memory ping-pong buffer 
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) while the DSP 
    *              continues to read data from the other buffer.  This sequence continues until the DSP is instructed 
    *              to stop through receipt of either the VP_COMMAND_CANCEL_PLAYBACK or the VP_COMMAND_STOP_PLAYBACK command.
    *
    *              \note  To prevent the 20ms speech fragments left in the audio output buffer from repeatedly being played 
    *              out to the speaker/ear-piece in IDLE mode, it is necessary for the ARM to instruct the DSP to disable the
    *              audio path via the vp_disable_audio_path flag sent in the VP_COMMAND_STOP_PLAYBACK command's argument.
    *              
    *              @param  VP_Playback_Mode_t vp_playback_mode;
    *              @param  VP_PlaybackMix_Mode_t vp_playbackmix_mode;
    *
    *  \see  VP_Playback_Mode_t, VP_PlaybackMix_Mode_t, VP_STATUS_PLAYBACK_DATA_EMPTY, VP_COMMAND_CANCEL_PLAYBACK, 
    *        VP_COMMAND_STOP_PLAYBACK, shared_voice_buf
    */
	VP_COMMAND_START_RECORDING_PLAYBACK,	// 0x8 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x09  
    *  \par Description 
    *              Terminates playback modes at the nb_frame boundary of ping-pong buffer.
    *              
    *              @param  Boolean vp_disable_audio_path - if TRUE: DSP disables UL/DL audio connection.
    *  
    *              \note  To prevent the 20ms speech fragments left in the audio output buffer from repeatedly being played 
    *              out to the speaker/ear-piece in IDLE mode, it is necessary for the ARM to instruct the DSP to disable the
    *              audio path via the “vp_disable_audio_path” flag.
    */
	VP_COMMAND_STOP_PLAYBACK,				// 0x9 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0a  
    *  \par Description 
    *              Immediate interruption of recording modes.
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_CANCEL_RECORDING,			// 0xA 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0b  
    *  \par Description 
    *              Immediate interruption of playback modes.
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_CANCEL_PLAYBACK,				// 0xB 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0c  
    *  \par Description 
    *              This command initiates continuous Recording and Playback. This reuses the separate voice recording 
    *              and playback modes/commands supports continuous call recording and playback.  The only exception 
    *              is that the recording and playback speech formats are constrained to be linear PCM. \BR
    *
    *              After getting this
    *              command, DSP keeps getting 20ms worth of data from the microphone path and keeps
    *              encoding the data in the mode required by this command.  After every speech buffer 
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
    *              has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
    *              generates an interrupt.  This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command. \BR
    *
    *              Upon activation, this operating mode continues under full control of the ARM.  
    *              After every speech buffer has been read and processed, the DSP issues a 
    *              VP_STATUS_PLAYBACK_DATA_EMPTY command to the ARM and generates an interrupt. This prompts the 
    *              ARM to write new speech playback data into the Shared memory ping-pong buffer 
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) while the DSP 
    *              continues to read data from the other buffer.  This sequence continues until the DSP is instructed 
    *              to stop through receipt of either the VP_COMMAND_CANCEL_PLAYBACK or the VP_COMMAND_STOP_PLAYBACK command.
    *
    *              \note PCM format only.
    *              
    *              @param  VP_Playback_Mode_t vp_playback_mode;
    *              @param  VP_Record_Mode_t vp_record_mode; Indicates the source of recording
    *              @param  UInt16 {bit4: flag_memo_record_playback (TRUE: in idle mode), 
    *                              bit3-bit0: nr_frame Number of 20ms frames to record per speech buffer.}
    *
    *  \see  VP_Record_Mode_t, VP_Speech_Mode_t, AMR_Mode, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *        VP_STATUS_RECORDING_DATA_READY, VP_COMMAND_CANCEL_RECORDING, VP_COMMAND_STOP_RECORDING,
    *        VP_Playback_Mode_t, VP_PlaybackMix_Mode_t, VP_STATUS_PLAYBACK_DATA_EMPTY, VP_COMMAND_CANCEL_PLAYBACK, 
    *        VP_COMMAND_STOP_PLAYBACK, shared_voice_buf
    */
	VP_COMMAND_START_RECORD_AND_PLAYBACK,	// 0xC	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0d  
    *  \par Description 
    *              This command Starts generation of 20ms frame interrupts if TRUE and stops it if FALSE.
    *
    *              \note This command is used for debugging purpose only.
    *              
    *              @param  Boolean Enable: If set to TRUE, enabled interrupts to ARM every 20ms with 
    *                                      VP_STATUS_NULL_FRAME_INT reply
    *  
    *  \see VP_STATUS_NULL_FRAME_INT
    */
	VP_COMMAND_NULL_FRAME_INT,				// 0xD 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0e  
    *  \par Description 
    *              Used to Pause Call/Memo Recording and/or Playback.
    *              
    *              @param  UInt16 {bit1: PAUSE_REC, bit0: PAUSE_PLAY}
    *  
    */
	VP_COMMAND_PAUSE,						// 0xE 	() arg0.[b1=1 b0=1]=[PAUSE_REC PAUSE_PLAY], 0:no change
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0f  
    *  \par Description 
    *              Used to Resume Call/Memo Recording and/or Playback.
    *              
    *              @param  UInt16 {bit1: RESUME_REC, bit0: RESUME_PLAY}
    *  
    */
	VP_COMMAND_RESUME,						// 0xF 	() arg0.[b1=1 b0=1]=[RESUME_REC RESUME_PLAY], 0:no change
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x10
    *  \par Description 
    *       This command enables/disables Digital Sound. \BR
    *
    *       This command is used for connecting to mono-bluetooth chip using HCI interface.\BR
    *
    *       The Bluetooth chip would be connected to the base-band using the PCM interface.
    *
    *              
    *              @param  UInt16 	ds_enable_flag
    *              					{
    *              						bit0: 	= 1 Enable Digital Sound
    *              								= 0 Disable Digital Sound
    *              						bit1: 	= 1 Packed 16-bit AADMAC interface
    *              								= 0 Unpacked 16-bit - one sample per 32-bit long-word in the lower 16-bits
    *              					}
    */    	
	VP_COMMAND_DIGITAL_SOUND,				// 0x10		( enable )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x11  
    *  \par Description 
    *       This command is used for configuring the ARM2SP interface for mixing/playing 8kHz or 16kHz PCM data to either 
    *       Uplink or Downlink. \BR
    *
    *       This command is sent by the ARM to the DSP for starting or stopping of data transfer from the 
    *       ARM to the DSP. \BR
    *
    *  \note Before getting this command ARM should have sent COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
    *              
    *       To start ARM2SP: ARM enables ARM2SP by VP_COMMAND_SET_ARM2SP. 
    *       To resume ARM2SP (JunoC0): ARM enables ARM2SP by VP_COMMAND_SET_ARM2SP  with arg1=1. 
    *       
    *       To control shared_Arm2SP_InBuf[]: DSP interrupts ARM with
    *       VP_STATUS_ARM2SP_EMPTY( shared_Arm2SP_done_flag, shared_Arm2SP_InBuf_out, Arm2SP_flag ) 
    *       whenever finishing every half PCM data. shared_Arm2SP_InBuf_out indicates the next index DSP will read.
    *
    *       To stop ARM2SP:\BR
    *       Quick stop: ARM disables ARM2SP using VP_COMMAND_SET_ARM2SP (arg0=0).\BR
    *       \note The ARM2SP will be stopped right away. There may be un-finished PCM data in shared_Arm2SP_InBuf[].\BR
    *
    *       Finishing stop: After finishing filling the last 4-speech frames for 8kHz or last 2-speech frames for 16kHz PCM data, ARM set shared_Arm2SP_done_flag=1 
    *       to indicate the end of PCM data. DSP ISR finishes the last 2 or 4-speech frames and disables ARM2SP. DSP sends 
    *       VP_STATUS_ARM2SP_EMPTY and arg0=0. \BR
    *
    *       To pause ARM2SP (JunoC0): ARM disables ARM2SP using VP_COMMAND_SET_ARM2SP (arg0=0, arg1=1).
    *
    *              
    *              @param  UInt16 Arm2SP_flag \BR
    *                      \htmlonly 
    *                      <pre>
    *                             {
    *                               bit15     - Sampling Rate of ARM2SP stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12 - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP_EMPTY reply
    *                                           (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                           2 for 16kHz)
    *                               bit11     - reserved
    *                               bit10     - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9      - DL_OVERWRITE_bit   1= overwrite Arm2SP buffer to DL (only when DL_MIX_bit=0)
    *                               bit8      - DL_MIX_bit         1= mixing Arm2SP buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7      - MONO_ST_bit        1= Stereo (not used if not 48k)
    *                                                              0= Mono
    *                               bit6      - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5      - UL_OVERWRITE_bit   1= overwrite Arm2SP buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4      - UL_MIX_bit         1= mixing Arm2SP buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3      - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2      - 48K input          1= Sampling rate of ARM2SP stream = 48k (irrespective of bit15 setting),
    *                                                              0= sampling rate determined by bit 15
    *                               bit1      - UL_enable          [1/0] = [enable/disable]
    *                               bit0      - DL_enable          [1/0] = [enable/disable]
    *                             }
    *                      </pre>
    *                      \endhtmlonly
    *              @param  UInt16 Reset_out_ptr_flag \BR
    *                                        =0, reset output pointer - shared_Arm2SP_InBuf_out - of buffer
    *                                            shared_Arm2SP_InBuf[] to 0. Used for new arm2sp session.\BR
    *                                        =1, keep output pointer - shared_Arm2SP_InBuf_out - unchange. 
    *                                            Used for PAUSE/RESUME the same arm2sp session.
    *
    *  \see ARM2SP_interface, shared_Arm2SP_InBuf, shared_Arm2SP_InBuf_out, shared_Arm2SP_done_flag, 
    *       VP_STATUS_ARM2SP_EMPTY 
    *
    */    	
	VP_COMMAND_SET_ARM2SP,					// 0x11		
	VP_COMMAND_SET_BT_NB,					// 0x12		( not used for Rhea )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x13  
    *  \par Description 
    *      This command enables USB headset interface. 
    *      Use USB headset sharedmem buffers as audio input and/or output ping-pong buffer.
    *
    *      @param  UInt16 pg_aud_usb_enable \BR
    *              \htmlonly 
    *              <pre>
    *                     { 
    *                         bit0     = [1/0] = [enable/disable] USB headset audio IN buffer.
    *                         bit1     = [1/0] = [enable/disable] USB headset audio OUT buffer.
    *                         bits7-4  = Input Sampling Rate Conversion index (SampleRateC_t).
    *                         bits11-8 = Output Sampling Rate Conversion index (SampleRateC_t).
    *                     }
    *              </pre>
    *              \endhtmlonly
    *
    * \note SRC should be done on arm side in general. Currently in this interface,
    *       only SRC_48K8K (0) and SRC_48K16K (1) with simple DOWN-SAMPLE are supported in DSP code.
    *              
    *  \see USB_Headset_Interface, VP_STATUS_USB_HEADSET_BUFFER, SampleRateC_t
    */	
	VP_COMMAND_USB_HEADSET,					// 0x13		( set pg_aud_usb_enable = arg0, [b15|b14|b13:b10|b9|b8|b7:b4|b3:b2|b1|b0]=[DL_GAIN|0|DL_SRC|0|UL_GAIN|UL_SRC|xx|DL|UL] )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x14  
    *  \par Description 
    *      This command starts the MultiMedia WB_AMR record OR playback. Prior to this command
    *      shared_WB_AMR_Ctrl_state must be configured.
    *
    *      \note Prior to sending this command, APE should enable the Audio interrupts to
    *      come to the DSP at 16kHz using the COMMAND_AUDIO_ENABLE command, and the audio
    *      input or output should be enabled using the COMMAND_AUDIO_CONNECT command.
    *              
    *              @param  UInt16  wb_amr_play_rec_mode = 1 for decoder (Play) \BR
    *                                                   = 2 for encoder (Record) \BR
    *                              
    *              \see shared_WB_AMR_Ctrl_state, MM_VPU_Interface, COMMAND_AUDIO_ENABLE, 
    *              COMMAND_AUDIO_CONNECT, MM_VPU_RECORD, MM_VPU_PLAYBACK, VP_COMMAND_MM_VPU_DISABLE
    */
    VP_COMMAND_MM_VPU_ENABLE,				// 0x14
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x15 
    *  \par Description 
    *      This command stops the MultiMedia WB_AMR record OR playback.
    *
    *              @param  None
    *                              
    *              \see shared_WB_AMR_Ctrl_state, MM_VPU_Interface, COMMAND_AUDIO_ENABLE, 
    *              COMMAND_AUDIO_CONNECT, MM_VPU_RECORD, MM_VPU_PLAYBACK
    */
	VP_COMMAND_MM_VPU_DISABLE,             // 0x15
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x16 
    *  \par Description 
    *      Initializes the vocoder. This is not always needed as when the new vocoder is different than 
    *      the old one an initialization of that vocoder is anyways done in GenDelayedNextEvents in 
    *      int1_smc.c, by looking at the frame entries.
    *              
    *              
    *              @param SpeechMode_t Speech_mode  - Speech Codec to initialize
    *
    *              \see SpeechMode_t 
    */	
	VP_COMMAND_VOCODER_INIT,		       // 0x16		( efr_flag )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code           
    *                    0x17
    *  \par Description 
    *       This command is used for configuring the ARM2SP2 interface for mixing/playing 8kHz or 16kHz PCM data to either 
    *       Uplink or Downlink. \BR
    *
    *       This command is sent by the ARM to the DSP for starting or stopping of data transfer from the 
    *       ARM to the DSP. \BR
    *
    *  \note Before getting this command ARM should have sent COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
    *              
    *       To start ARM2SP2: ARM enables ARM2SP2 by VP_COMMAND_SET_ARM2SP2. 
    *       To resume ARM2SP2 (JunoC0): ARM enables ARM2SP2 by VP_COMMAND_SET_ARM2SP2  with arg1=1. 
    *       
    *       To control shared_Arm2SP2_InBuf[1280]: DSP interrupts ARM with 
    *       VP_STATUS_ARM2SP2_EMPTY( shared_Arm2SP2_done_flag, shared_Arm2SP2_InBuf_out, Arm2SP2_flag ) 
    *       whenever finishing every 640 PCM data. shared_Arm2SP2_InBuf_out indicates the next index DSP will read. 
    *
    *       To stop ARM2SP2:\BR
    *       Quick stop: ARM disables ARM2SP2 using VP_COMMAND_SET_ARM2SP2 (arg0=0).\BR
    *       \note The ARM2SP2 will be stopped right away. There may be un-finished PCM data in shared_Arm2SP2_InBuf[1280].\BR
    *
    *       Finishing stop: After finishing filling the last 2 or 4-speech frame PCM data, ARM set shared_Arm2SP2_done_flag=1 
    *       to indicate the end of PCM data. DSP ISR finishes the last 2 or 4-speech frames and disables ARM2SP2. DSP sends 
    *       VP_STATUS_ARM2SP2_EMPTY and arg0=0. \BR
    *
    *       To pause ARM2SP2 (JunoC0): ARM disables ARM2SP2 using VP_COMMAND_SET_ARM2SP2 (arg0=0, arg1=1).
    *
    *              
    *              @param  UInt16 Arm2SP2_flag \BR
    *                      \htmlonly 
    *                      <pre>
    *                             {
    *                               bit15       - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12   - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP2_EMPTY reply
    *                                             (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                             2 for 16kHz)
    *                               bit11       - reserved
    *                               bit10       - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9        - DL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to DL (only when DL_MIX_bit=0)
    *                               bit8        - DL_MIX_bit         1= mixing Arm2SP2 buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7        - MONO_ST_bit        1= Stereo (not used if not 48k)
    *                                                                0= Mono
    *                               bit6        - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5        - UL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4        - UL_MIX_bit         1= mixing Arm2SP2 buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3        - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2        - reserved
    *                               bit1        - UL_enable          [1/0] = [enable/disable]
    *                               bit0        - DL_enable          [1/0] = [enable/disable]
    *                             }
    *                      </pre>
    *                      \endhtmlonly
    *              @param  UInt16 Reset_out_ptr_flag \BR
    *                                        =0, reset output pointer - shared_Arm2SP2_InBuf_out - of buffer
    *                                            shared_Arm2SP2_InBuf[] to 0. Used for new arm2sp2 session.\BR
    *                                        =1, keep output pointer - shared_Arm2SP2_InBuf_out - unchange. 
    *                                            Used for PAUSE/RESUME the same arm2sp2 session.
    *
    *  \see ARM2SP_interface, shared_Arm2SP2_InBuf, shared_Arm2SP2_InBuf_out, shared_Arm2SP2_done_flag, 
    *       VP_STATUS_ARM2SP2_EMPTY 
    *
    */    	
	VP_COMMAND_SET_ARM2SP2,      		   // 0x17	
	VP_COMMAND_VOIF_CONTROL,      		   // 0x18		(enable/disable)
	VP_COMMAND_SP,						   // 0x19	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x1A
    *  \par Description 
    *       This command runs the main AMR vocoder during GSM idle to support WCDMA/VOIP voice call. This command has to be 
    *       sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. The input to the speech 
    *       decoder comes in DL_MainAMR_buf.param, and the output of the encoder is stored in UL_MainAMR_buf.param. 
    *
    *  \note Before getting this command ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT.
    *              
    *              @param  UInt16 For VOIP: {bit15-bit8: VOIP mode, bit1 - VoLTECallEnable, bit0 - Voip_DTX_Enable} \BR
    *                             For Non-VOIP: {bit15-bit8:0, bit4: 1/0=WB_AMR/NB_AFS, bit3-0: AMR UL Mode set}
    *                             
    *              @param  UInt16 AMR_IF2_flag   - AMR Iterface Format 2 flag
    *              @param  UInt16 WCDMA_MST_flag - WCDMA MST SD decoding flag \BR 
    *                                            = 2 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = TRUE; \BR
    *                                            = 1 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = FALSE; \BR
    *                                            = 0 - MainAMR_WCDMA_MST_flag = FALSE, MainAMR_WCDMA_MST_init = FALSE;
    *   \par Associated Replies
    *        For every VP_COMMAND_MAIN_AMR_RUN command, an associated VP_STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, VP_STATUS_MAIN_AMR_DONE
    */    
    VP_COMMAND_MAIN_AMR_RUN,				// 0x1A		( run main AMR vocoder during GSM idle to support WCDMA voice call, arg0=UL AMR codec mode request )	
	VP_COMMAND_CLEAR_VOIPMODE,				// 0x1B (arg0 = 0 to clear VOIPmode)
	VP_COMMAND_CNC_ENABLE,					// 0x1C (arg0=1 to enable click removal)
   /** \HR */
   /** \par Module
	* 				   Audio 
	*  \par Command Code		   
	* 				   0x1D
	*  \par Description 
	* 	  This command is obsolete
	*/
	VP_COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE,	// 0x1D	  ( obsolete )
   /** \HR */
   /** \par Module
	* 				   Audio 
	*  \par Command Code		   
	* 				   0x1E
	*  \par Description 
	* 	  This command enables EANC
	*
	*  \note Currently this command is only supporting audio coming in from the AADMAC channel 2 and looping it back to channel 1 
	*           which is being used by IHF speaker. 
	* 			 
	* 			 @param  UInt16  	{
	*								Bit 0: Enable_EANC: 0 = Disable, 1 = Enable
	*							}
	*
	* 			 \see   
	*/
	VP_COMMAND_ENABLE_EANC,	// 0x1E	  ( enable )	
   /** \HR */
   /** \par Module
	* 				   Audio 
	*  \par Command Code		   
	* 				   0x1F
	*  \par Description 
	* 	  This command enables VPU command queue processing, called from AP
	*
	* 			 
	* 			 @param  none  	
	*
	* 			 \see   
	*/
	VP_COMMAND_VPU_ENABLE,	// 0x1F	  ( enable )	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x20
    *  \par Description 
    *       This command initializes Adaptive Jitter Buffer instance for VoLTE.
    */    
    VP_COMMAND_VOLTE_INIT,	// 0x20
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x21
    *  \par Description 
    *       This command flushes Adaptive Jitter Buffer for VoLTE.
    */    
    VP_COMMAND_VOLTE_START_STREAM,	// 0x21
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x22
    *  \par Description 
    *       This command delivers VoLTE DL speech frame to the DSP.
    *
    *  \note Before getting this command ARM should have enabled AMR-NB or AMR-WB VoIP encoder with VP_COMMAND_MAIN_AMR_RUN
    *              
    *              @param  UInt16 RTP timestamp (least 16-bit) \BR
    *                             
    *              @param  UInt16 {bit15-bit8: codec type, bit4: frame quality, bit3-0: frame type } \BR
    *              @param  UInt16 {bit15-bit8: buffer index, bit7-0: frame index } \BR
    */    
    VP_COMMAND_VOLTE_PUT_FRAME,	// 0x22
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x23
    *  \par Description 
    *       This command enable/disable PTT session.
    *
    *              @param  UInt16 {bit0: 1: Enable; 0: disable;}
    */    
    VP_COMMAND_PTT_ENABLE,	// 0x23
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x24
    *  \par Description 
    *       This command enable/disable a call using an external modem.\BR
    *
    *       This should be sent before the enabling of any audio at the start
    *       of a call and after the disabling of any audio at the end of a
    *       call.\BR
    *
    *       For this command, the DSP enables the external modem interface, it
    *       configures the AADMAC channels for speaker and mic external modem
    *       paths and then sends a reply VP_STATUS_EXT_MODEM_CALL_DONE to the
    *       AP with the same argument as the one passed in the command.\BR
    *
    *       The Audio clocks should not be shut-off before this command is
    *       sent, and the response to it has been received from the DSP by
    *       the AP.
    *
    *              @param  UInt16 arg0 = 1: Enable\BR
    *                                  = 0: Disable
    */    
    VP_COMMAND_EXT_MODEM_CALL,	// 0x24
    /** \HR */
    /** \par Module
     *                    Audio
     *  \par Command Code
     *                    0x25
     *  \par Description
     *       This command is used for configuring the ARM2SP_HQ_DL interface
     *       for mixing/playing 48kHz PCM data to Downlink. \BR
     *
     *       This command is sent by the ARM to the DSP for starting or
     *       stopping of 48kHz data transfer from the ARM to the DSP. \BR
     *
     *  \note Before getting this command ARM should have sent
     *       COMMAND_AUDIO_ENABLE, and the audio inputs and outputs enabled
     *       by COMMAND_AUDIO_CONNECT, along with enabling of the 48kHz
     *       AADMAC interface in shared_aadmac_aud_enable. \BR
     *
     *       To start ARM2SP_HQ_DL: ARM enables ARM2SP_HQ_DL by
     *                              VP_COMMAND_SET_ARM2SP_HQ_DL.
     *       To resume ARM2SP_HQ_DL: ARM enables ARM2SP_HQ_DL by
     *                              VP_COMMAND_SET_ARM2SP_HQ_DL with arg1=1.
     *
     *       To control shared_Arm2SP_InBuf[]: DSP interrupts ARM
     *       with VP_STATUS_ARM2SP_HQ_DL_EMPTY( shared_ARM2SP_HQ_DL_done_flag,
     *       shared_ARM2SP_HQ_DL_InBuf_out, ARM2SP_HQ_DL_flag )
     *       whenever finishing every 2/4 20ms data.
     *       shared_ARM2SP_HQ_DL_InBuf_out indicates the next index DSP will
     *       read.\BR
     * 
     *       To stop ARM2SP_HQ_DL:\BR
     *       Quick stop: ARM disables ARM2SP_HQ_DL using
     *       VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0).\BR
     *       The ARM2SP_HQ_DL will be stopped right away. There may be
     *       un-finished PCM data in shared_Arm2SP_InBuf[].\BR
     * 
     *       Finishing stop: After finishing filling the last speech frames of
     *       PCM data, ARM set shared_ARM2SP_HQ_DL_done_flag=1
     *       to indicate the end of PCM data. DSP ISR finishes the last speech
     *       frames and disables ARM2SP_HQ_DL. DSP sends
     *       VP_STATUS_ARM2SP_HQ_DL_EMPTY and arg0=0. \BR
     * 
     *       To pause ARM2SP_HQ_DL : ARM disables ARM2SP_HQ_DL using
     *       VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0, arg1=1).
     * 
     * 
     *              @param  UInt16 ARM2SP_HQ_DL_flag \BR
     *                      \htmlonly
     *                      <pre>
     *                             {
     *                               bit14:10    - reserved
     *                               bit9        - DL_OVERWRITE_bit
     *                                             1= overwrite ARM2SP_HQ_DL
     *                                                buffer to DL
     *                                                (only when DL_MIX_bit=0)
     *                               bit8        - DL_MIX_bit
     *                                             1= mixing ARM2SP_HQ_DL
     *                                                buffer with DL
     *                                                (regardless
     *                                                DL_OVERWRITE_bit
     *                                                setting)
     *                               bit7        - 48kHz Mono/Stereo
     *                                             [1/0] = [Mono/Stereo]
     *                               bit6-1      - reserved
     *                               bit0        - 48kHz DL PCM enable
     *                                             [1/0] = [enable/disable]
     *                             }
     *                      </pre>
     *                      \endhtmlonly
     * 
     *  \see ARM2SP_HQ_DL_interface, shared_Arm2SP_InBuf,
     *       shared_ARM2SP_HQ_DL_InBuf_out, shared_ARM2SP_HQ_DL_done_flag,
     *       VP_STATUS_ARM2SP_HQ_DL_EMPTY
     *
     */
 	VP_COMMAND_SET_ARM2SP_HQ_DL,   		   // 0x25
    /** \HR */
    /** \par Module
     *                    Audio
     *  \par Command Code
     *                    0x26
     *  \par Description
     *       This command is used for configuring the ARM2SP2_HQ_DL interface
     *       for mixing/playing 48kHz PCM data to Downlink. \BR
     *
     *       This command is sent by the ARM to the DSP for starting or
     *       stopping of 48kHz data transfer from the ARM to the DSP. \BR
     *
     *  \note Before getting this command ARM should have sent
     *       COMMAND_AUDIO_ENABLE, and the audio inputs and outputs enabled
     *       by COMMAND_AUDIO_CONNECT, along with enabling of the 48kHz
     *       AADMAC interface in shared_aadmac_aud_enable. \BR
     *
     *       To start ARM2SP2_HQ_DL: ARM enables ARM2SP2_HQ_DL by
     *                              VP_COMMAND_SET_ARM2SP2_HQ_DL.
     *       To resume ARM2SP2_HQ_DL: ARM enables ARM2SP2_HQ_DL by
     *                              VP_COMMAND_SET_ARM2SP2_HQ_DL with arg1=1.
     *
     *       To control shared_Arm2SP2_InBuf[]: DSP interrupts ARM
     *       with VP_STATUS_ARM2SP2_HQ_DL_EMPTY( shared_ARM2SP2_HQ_DL_done_flag,
     *       shared_ARM2SP2_HQ_DL_InBuf_out, ARM2SP2_HQ_DL_flag )
     *       whenever finishing every 2/4 20ms data.
     *       shared_ARM2SP2_HQ_DL_InBuf_out indicates the next index DSP will
     *       read.\BR
     * 
     *       To stop ARM2SP2_HQ_DL:\BR
     *       Quick stop: ARM disables ARM2SP2_HQ_DL using
     *       VP_COMMAND_SET_ARM2SP2_HQ_DL (arg0=0).\BR
     *       The ARM2SP2_HQ_DL will be stopped right away. There may be
     *       un-finished PCM data in shared_Arm2SP2_InBuf[].\BR
     * 
     *       Finishing stop: After finishing filling the last speech frames of
     *       PCM data, ARM set shared_ARM2SP2_HQ_DL_done_flag=1
     *       to indicate the end of PCM data. DSP ISR finishes the last speech
     *       frames and disables ARM2SP2_HQ_DL. DSP sends
     *       VP_STATUS_ARM2SP2_HQ_DL_EMPTY and arg0=0. \BR
     * 
     *       To pause ARM2SP2_HQ_DL : ARM disables ARM2SP2_HQ_DL using
     *       VP_COMMAND_SET_ARM2SP2_HQ_DL (arg0=0, arg1=1).
     * 
     * 
     *              @param  UInt16 ARM2SP2_HQ_DL_flag \BR
     *                      \htmlonly
     *                      <pre>
     *                             {
     *                               bit14:10    - reserved
     *                               bit9        - DL_OVERWRITE_bit
     *                                             1= overwrite ARM2SP2_HQ_DL
     *                                                buffer to DL
     *                                                (only when DL_MIX_bit=0)
     *                               bit8        - DL_MIX_bit
     *                                             1= mixing ARM2SP2_HQ_DL
     *                                                buffer with DL
     *                                                (regardless
     *                                                DL_OVERWRITE_bit
     *                                                setting)
     *                               bit7        - 48kHz Mono/Stereo
     *                                             [0/1] = [Mono/Stereo]
     *                               bit6-1      - reserved
     *                               bit0        - 48kHz DL PCM enable
     *                                             [1/0] = [enable/disable]
     *                             }
     *                      </pre>
     *                      \endhtmlonly
     * 
     *  \see ARM2SP2_HQ_DL_interface, shared_Arm2SP2_InBuf,
     *       shared_ARM2SP2_HQ_DL_InBuf_out, shared_ARM2SP2_HQ_DL_done_flag,
     *       VP_STATUS_ARM2SP2_HQ_DL_EMPTY
     *
     */
 	VP_COMMAND_SET_ARM2SP2_HQ_DL,  		   // 0x26
	VP_COMMAND_ECHO_CANCELLATION,			// 0x27		( cmd_action, mode )
	VP_COMMAND_NOISE_SUPPRESSION,			// 0x28		( cmd_action )
	VP_COMMAND_ENABLE_DUAL_MIC,				// 0x29 (enable) arg0=0 Disable, 1 enable;
	VP_COMMAND_SWITCH_AUDIO_PROFILE,		// 0x2A	arg0 = Is_spkr_mode, arg1 = 0, arg2 = 0;
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2B
    *  \par Description 
    *       This command controls Downlink Noise Suppressor.
    *              
    *              @param  0 - disable, 1 - enable
    *
    */
	VP_COMMAND_DOWNLINK_NOISE_SUPPRESSION,	// 0x2B (arg0: 0 - disable, 1 - enable)
    VP_COMMAND_VOICE_FILTER_COEFS,			// 0x2C
	VP_COMMAND_AUDIO_ENHANCEMENT_ENABLE,	// 0x2D
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2E
    *  \par Description 
    *       This command configures the first 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 0
    *              @param  UInt16 Coefficient 1
    *              @param  UInt16 Coefficient 2
    *
    *  \see Software_Sidetone
    */
	VP_COMMAND_SIDETONE_COEFS_012,			// 0x2E		( SIDETONE_FILTER Biquad filter coefs 0, 1 and 2 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2F 
    *  \par Description 
    *       This command configures the second 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 3
    *              @param  UInt16 Coefficient 4
    *              @param  UInt16 Coefficient 5
    *
    *  \see Software_Sidetone
    */
	VP_COMMAND_SIDETONE_COEFS_345,			// 0x2F		( SIDETONE_FILTER Biquad filter coefs 3, 4 and 5 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x30 
    *  \par Description 
    *       This command configures the third 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 6
    *              @param  UInt16 Coefficient 7
    *              @param  UInt16 Coefficient 8
    *
    *  \see Software_Sidetone
    */
	VP_COMMAND_SIDETONE_COEFS_678,			// 0x30		( SIDETONE_FILTER Biquad filter coefs 6, 7 and 8 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x31
    *  \par Description 
    *       This command configures the last (10th) sidetone Double Biquad filter's parameter.
    *              
    *              @param  UInt16 Coefficient 9
    *
    *  \see Software_Sidetone
    */
	VP_COMMAND_SIDETONE_COEFS_9,			// 0x31		( SIDETONE_FILTER Biquad filter coef 9 )
	VP_COMMAND_INIT_SIDETONE_EXPANDER,		/* 0x32 */
   /** \HR */
   /** \par Module
    *                    Audio
    *  \par Command Code
    *                    0x33
    *  \par Description
    *       This command enables/disables the Audio interrupts and configures
    *       the hardware audio paths.
    *       Currently this command comes before the VP_COMMAND_AUDIO_CONNECT.
    *
    *              @param  UInt16 AUDIO_DAC_ENABLE = 1 - Enable DAC,\BR
    *                             AUDIO_ADC_ENABLE = 2 - Enable ADC,\BR
    *                             0 - Disable both ADC and DAC
    *              @param  Boolean 16k_sampling - 0 = 8k mode, 1 = 16k mode
    *              @param  Boolean WB_DUAL_MIC_ENABLE - Enables Wide-band dual
    *              mics.
    *              When enabled, always uses 16k mode irrespective of what is
    *              set in 16k_sampling argument above
    *
    */
	VP_COMMAND_AUDIO_ENABLE,			/* 0x33	( enable ) */
   /** \HR */
   /** \par Module
    *                    Audio
    *  \par Command Code
    *                    0x34
    *  \par Description
    *       Connect/disconnect the audio input/output from ADC/DAC
    *
    *              @param  Boolean pg_aud_in_enable - Used to enable or disable
    *              the input samples from ADC
    *              @param  Boolean pg_aud_out_enable - Used to enable or
    *              disable the input samples to DAC
    */
	VP_COMMAND_AUDIO_CONNECT,			/* 0x34	(in_enable,
							   out_enable ) */
   /** \HR */
   /** \par Module
    *                    Audio
    *  \par Command Code
    *                    0x35
    *  \par Description
    *       This command is used to Start and Stop the
    *       secondary speaker AADMAC channel (channel 16).
    *
    *       It also defines whether the speaker path is mono or stereo.
    *
    *       The actual enable of the AADMAC channel happens in the ISR of the
    *       primary microphone channel interrupt to the DSP.
    *
    *       It also determines whether to use Echo Reference from this path
    *       instead of from the primary speaker channel path. This is valid
    *       only for mono speaker channel.
    *
    *       @param  UInt16 enable = 0 -> Disable
    *                             = 1 -> Enable
    *       @param  UInt8 stereo_enable = 0 ->Mono
    *                                   = 1 -> Stereo
    *       @param  UInt16 use_ec_ref_from_this_path = 0 -> Use the primary
    *                                          speaker channel for echo
    *                                          reference(valid only if
    *                                          primary speaker is mono).
    *                                   = 1 -> use secondary speaker
    *                                          channel  for echo reference
    *                                          (valid only if secondary
    *                                          speaker is mono).
    *
    */
	VP_COMMAND_SPKR2_EN				/* 0x35	(in_enable,
							   out_enable ) */

} VPCommand_t;                                 
/**
 * @}
 */

/**
 * @addtogroup VP_Status_Queue
 * @{ 
 */
typedef enum
{// Status                                       Code        Arguments (** = unused status)
 // ========================                   ====        ====================================
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x00  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_TRAINING_DONE,                // 0x0  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x01  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_NEW_VECT_READY,                // 0x1  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x02  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_NAME_ID,                        // 0x2  (} **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x03  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_RECOGNITION_DONE,                // 0x3  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x04 
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_RECOGNITION_PLAYBACK_DONE    ,    // 0x4  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x05  
    *  \par Description 
    *              This reply is sent by the DSP to the ARM during both voice recording (working 
    *              in both idle and speech call mode), after every speech buffer has been filled
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame). The voice recording can
    *              be started by either VP_COMMAND_START_CALL_RECORDING or 
    *              VP_COMMAND_START_MEMO_RECORDING.
    *              This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
    *              
    *              @param  rec_buf_index {0 or 1}: Index of the recording buffer in Shared memory.
    *              @param  stop_recording_flag {0 or 1}: Indicates if this is the last buffer of recording data.
    *              @param  AMR_Mode vp_amr_mode;
    *               
    *  \see  VP_Record_Mode_t, VP_Speech_Mode_t, AMR_Mode, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *        VP_COMMAND_START_CALL_RECORDING, VP_COMMAND_START_MEMO_RECORDING, VP_COMMAND_CANCEL_RECORDING, 
    *        VP_COMMAND_STOP_RECORDING, shared_voice_buf
    *  
    */
    VP_STATUS_RECORDING_DATA_READY,            // 0x5  ()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x06  
    *  \par Description 
    *              This reply is sent by the DSP to the ARM during both voice playback (working 
    *              in both idle and speech call mode), after every speech buffer has been emptied
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) by the DSP. The voice playback can
    *              be started by VP_COMMAND_START_RECORDING_PLAYBACK. \BR
    *
    *              This reply prompts the 
    *              ARM to write new speech playback data into the Shared memory ping-pong buffer 
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) while the DSP 
    *              continues to read data from the other buffer.  This sequence continues until the DSP is instructed 
    *              to stop through receipt of either the VP_COMMAND_CANCEL_PLAYBACK or the VP_COMMAND_STOP_PLAYBACK command.
    *                            
    *              @param  play_buf_index {0 or 1]: Index of the playback buffer in Shared memory.
    *              @param  stop_playback_flag {0 or 1}: Indicates if this is the last buffer of playback data.
    *               
    *  \see  VP_COMMAND_START_RECORDING_PLAYBACK, VP_COMMAND_CANCEL_PLAYBACK, 
    *        VP_COMMAND_STOP_PLAYBACK, shared_voice_buf
    *  
    */
    VP_STATUS_PLAYBACK_DATA_EMPTY,             // 0x6  ()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x07 
    *  \par Description 
    *              This status reply is sent every 20ms by the DSP to the ARM after enabling of it by 
    *              VP_COMMAND_NULL_FRAME_INT command
    *              
    *              \note This reply is only used for debugging
    *
    *              @param  None
    * 
    *  \see VP_COMMAND_NULL_FRAME_INT
    *  
    */
    VP_STATUS_NULL_FRAME_INT,                  // 0x7  ()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x08
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_AMR2_ARORT,                      // 0x8  () AMR_2 aborted by DSP because playing polyringer
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x09
    *  \par Description
    *       This reply is part of the ARM2SP interface for mixing/playing 8kHz/16kHz PCM data to either 
    *       Uplink or Downlink. \BR 
    *       This reply is sent by the DSP whenever finishing every 640 PCM data.
    *       shared_Arm2SP_InBuf_out indicates the next index DSP will read.
    *
    *              @param  UInt16 shared_Arm2SP_done_flag - flag indicating the end of transfer of data
    *              @param  UInt16 shared_Arm2SP_InBuf_out - index indicating next index DSP will read from the
    *                                                       shared_Arm2SP_InBuf[] buffer. 
    *              @param  UInt16 Arm2SP_flag             - if shared_Arm2SP_done_flag is set then Arm2SP_flag = 0 \BR
    *                                                       else send back Arm2SP_flag received in the VP_COMMAND_SET_ARM2SP command \BR
    *              \htmlonly 
    *              <pre>
    *                             {
    *                               bit15     - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12 - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP_EMPTY reply
    *                                           (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                           2 for 16kHz)
    *                               bit11     - reserved
    *                               bit10     - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9      - DL_OVERWRITE_bit   1= overwrite Arm2SP buffer to DL (only when DL_MIX_bit=0)
    *                               bit8      - DL_MIX_bit         1= mixing Arm2SP buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7      - MONO_ST_bit        1= Stereo (not used if not 48k)
    *                                                              0= Mono
    *                               bit6      - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5      - UL_OVERWRITE_bit   1= overwrite Arm2SP buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4      - UL_MIX_bit         1= mixing Arm2SP buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3      - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2      - reserved
    *                               bit1      - UL_enable          [1/0] = [enable/disable]
    *                               bit0      - DL_enable          [1/0] = [enable/disable]
    *                             }
    *              </pre>
    *              \endhtmlonly
    *
    *
    *  \see ARM2SP_interface, shared_Arm2SP_InBuf, shared_Arm2SP_InBuf_out, shared_Arm2SP_done_flag, 
    *       VP_COMMAND_SET_ARM2SP
    *
    */    
	VP_STATUS_ARM2SP_EMPTY,					// 0x09
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0A
    *  \par Description
    *       This reply is part of the ARM2SP2 interface for mixing/playing 8kHz/16kHz PCM data to either 
    *       Uplink or Downlink. \BR 
    *       This reply is sent by the DSP whenever finishing every 640 PCM data.
    *       shared_Arm2SP2_InBuf_out indicates the next index DSP will read.
    *
    *              @param  UInt16 shared_Arm2SP2_done_flag - flag indicating the end of transfer of data
    *              @param  UInt16 shared_Arm2SP2_InBuf_out - index indicating next index DSP will read from the
    *                                                        shared_Arm2SP2_InBuf[] buffer. 
    *              @param  UInt16 Arm2SP2_flag             - if shared_Arm2SP2_done_flag is set then Arm2SP2_flag = 0 \BR
    *                                                        else send back Arm2SP2_flag received in the VP_COMMAND_SET_ARM2SP2 command \BR
    *              \htmlonly 
    *              <pre>
    *                             {
    *                               bit15       - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12   - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP2_EMPTY reply
    *                                             (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                             2 for 16kHz)
    *                               bit11       - reserved
    *                               bit10       - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9        - DL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to DL (only when DL_MIX_bit=0)
    *                               bit8        - DL_MIX_bit         1= mixing Arm2SP2 buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7        - MONO_ST_bit        1= Stereo (not used if not 48k)
    *                                                                0= Mono
    *                               bit6        - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5        - UL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4        - UL_MIX_bit         1= mixing Arm2SP2 buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3        - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2        - reserved
    *                               bit1        - UL_enable          [1/0] = [enable/disable]
    *                               bit0        - DL_enable          [1/0] = [enable/disable]
    *                             }
    *              </pre>
    *              \endhtmlonly
    *
    *
    *  \see ARM2SP_interface, shared_Arm2SP2_InBuf, shared_Arm2SP2_InBuf_out, shared_Arm2SP2_done_flag, 
    *       VP_COMMAND_SET_ARM2SP2
    *
    */	
	VP_STATUS_ARM2SP2_EMPTY,				// 0x0A
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0B  
    *  \par Description
    *       This reply is part of the USB_Headset_Interface, which is used for connecting a USB microphone 
    *       and/or speaker to the baseband chip.
    *
    *       This interface is enabled using VP_COMMAND_USB_HEADSET  command. 
    *
    *       This reply is sent by the DSP after every 20ms speech frame. It indicates
    *       to the ARM to go ahead and write the microphone data to the consumed input buffer and 
    *       read the speaker data from filled up output buffer. The pointers to the input buffer and 
    *       the output buffers are contained in this reply.
    *
    *              @param  UInt16 pg_aud_usb_enable (This is the same first parameter received in VP_COMMAND_USB_HEADSET)
    *              @param  UInt16 Input buffer pointer already read/consumed by the DSP.
    *              @param  UInt16 Output buffer pointer already filled by the DSP.
    *
    *
    *  \see USB_Headset_Interface, VP_COMMAND_USB_HEADSET
    *
    */		
	VP_STATUS_USB_HEADSET_BUFFER,			// 0x0B ( arg0=pg_aud_usb_enable, arg1=pg_aud_in_ptr_usb_arm, arg2=pg_aud_out_ptr_usb_arm  )
	VP_STATUS_BT_NB_BUFFER_DONE,			// 0x0C	( arg0=isr ptr, arg1=bk ptr(used by arm or dsp bk) )
	VP_STATUS_VOIF_BUFFER_READY,			// 0x0D
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0E
    *  \par Description 
    *       This reply is sent by the DSP during two cases
    *
    *     
    *       \section WCDMA_encoded_data_interface GSM idle to support WCDMA/VOIP voice call.
    *       This reply is sent by the DSP
    *       every-time it completes processing the COMMAND_MAIN_AMR_RUN command - i.e. it has encoded the speech data and 
    *       stored it in UL_MainAMR_buf buffer for ARM to pick up and when it has completed speech decoding the data 
    *       sent by the ARM in DL_MainAMR_buf
    *
    *              @param  UInt16 For Non-VOIP: amr_tx_type 
    *              @param  UInt16 {bit4: =1/0 - WBAMR/NBAMR, bit3-bit0: active_ulink_mode_set}
    *              @param  Boolean dtx_enable
    *
    *       \note For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent 
    *             in the status queue.
    *       \see  DL_MainAMR_buf, UL_MainAMR_buf, COMMAND_MAIN_AMR_RUN, WCDMA_encoded_data_interface
    *
    *       \section MM_VPU_PLAYBACK
    *       If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer becomes lower 
    *       than one frame's worth of samples, DSP sends an interrupt to the ARM with a status reply 
    *       VP_STATUS_PRAM_CODEC_INPUT_EMPTY, requesting the ARM to send more downlink data immediately, 
    *       and informing that under-flow is happening.  
    *       
    *       \see MM_VPU_PLAYBACK 
    *                     
    *              @param  UInt16 WB_AMR_InBuf_data_available (Amount of input data available)
    *              @param  UInt16 WB_AMR_InBuf_in (Read Index)
    *              @param  UInt16 WB_AMR_InBuf_out  (Write Index)
    *              
    */    
	VP_STATUS_PRAM_CODEC_INPUT_EMPTY,		// 0x0E
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0F
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer, 
    *       becomes lower than the threshold shared_Inbuf_LOW_Sts_TH, the DSP sends a interrupt to 
    *       the ARM with a status reply VP_STATUS_PRAM_CODEC_INPUT_LOW, requesting the ARM to send 
    *       more encoded downlink data. 
    *                     
    *              @param  UInt16 Threshold
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *   
    *  \see MM_VPU_PLAYBACK, shared_Inbuf_LOW_Sts_TH
    */    
	VP_STATUS_PRAM_CODEC_INPUT_LOW,		// 0x0F
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x10
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       If the amount of remaining space in the output PCM buffer 
    *       (as selected by shared_WB_AMR_Ctrl_state variable) becomes 
    *       lower than one frame's worth of samples, DSP sends an interrupt 
    *       to the ARM with a status reply VP_STATUS_PRAM_CODEC_OUTPUT_FULL, 
    *       requesting the ARM to stop more downlink data immediately, and 
    *       informing that over-flow is happening.  
    *                     
    *              @param  UInt16 Threshold = 0
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *   
    *  \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state
    */    
	VP_STATUS_PRAM_CODEC_OUTPUT_FULL,		// 0x10	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x11
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       If the amount of remaining space in the output PCM buffer (as selected by 
    *       shared_WB_AMR_Ctrl_state variable) becomes lower than the threshold 
    *       shared_Outbuf_LOW_Sts_TH, the DSP sends a interrupt to the ARM with a 
    *       status reply STATUS_PRAM_CODEC_OUTPUT_LOW, requesting the ARM to slow down 
    *       in sending more downlink data. 
    *                     
    *              @param  UInt16 Threshold
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *   
    *  \see MM_VPU_PLAYBACK, shared_Inbuf_LOW_Sts_TH, shared_WB_AMR_Ctrl_state
    */    
	VP_STATUS_PRAM_CODEC_OUTPUT_LOW,		// 0x11	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x12
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       When ARM was to complete the decoding process, it would set the 
    *       appropriate Input Data Done Flag (as selected by the 
    *       shared_WB_AMR_Ctrl_state variable). The DSP stops any further decoding 
    *       after it completes decoding the last frame of data and sends back a 
    *       STATUS_PRAM_CODEC_DONEPLAY reply to the ARM. 
    *                     
    *              @param  UInt16 Input done flag as selected by shared_WB_AMR_Ctrl_state
    *              @param  UInt16 Number of Wide-band AMR frames (higher word)
    *              @param  UInt16 Number of Wide-band AMR frames (higher word)
    *   
    *  \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state
    */    
	VP_STATUS_PRAM_CODEC_DONEPLAY,			// 0x12	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x13  
    *  \par Description
    *       This reply is part of the MM VPU Record interface, which records 16kHz microphone samples to 
    *       WB-AMR encoded data. 
    *
    *       This recording is enabled using VP_COMMAND_MM_VPU_ENABLE  command. 
    *
    *       This reply is sent by the DSP after completing encoding of 4 20ms frames of data. It indicates
    *       to the ARM to go ahead and read this data from 
    *       shared_encoder_OutputBuffer[shared_encodedSamples_buffer_out[0]]  
    *
    *              @param  UInt16 wb_amr_encoded_total_size - Size of the encoded data size in 8-bit bytes
    *              @param  UInt16 WB_AMR_OutBuf_in - Write Index - copy of shared_encodedSamples_buffer_in[0]
    *              @param  UInt16 WB_AMR_OutBuf_out - Read Index - copy of shared_encodedSamples_buffer_out[0]
    *
    *
    *  \see MM_VPU_RECORD, MM_VPU_Interface, VP_COMMAND_MM_VPU_ENABLE, shared_encoder_OutputBuffer,
    *       shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out, VP_COMMAND_MM_VPU_DISABLE
    *
    */	
    VP_STATUS_PRAM_CODEC_DONE_RECORD,		// 0x13		
	VP_STATUS_SP,							// 0x14
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x15
    *  \par Description 
    *       This reply is sent by the DSP during GSM idle to support WCDMA/VOIP voice call. This reply is sent by the DSP
    *       every-time it completes processing the VP_COMMAND_MAIN_AMR_RUN command - i.e. it has encoded the speech data and 
    *       stored it in UL_MainAMR_buf buffer for ARM to pick up and when it has completed speech decoding the data 
    *       sent by the ARM in DL_MainAMR_buf
    *                     
    *              @param  UInt16 For Non-VOIP: amr_tx_type
    *              @param  UInt16 {bit4: =1/0 - WBAMR/NBAMR, bit3-bit0: active_ulink_mode_set}
    *              @param  Boolean dtx_enable
    *              
    *   \par Associated Command
    *        For every VP_COMMAND_MAIN_AMR_RUN command, an associated VP_STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, VP_COMMAND_MAIN_AMR_RUN
    */    
    VP_STATUS_MAIN_AMR_DONE,					// 0x15		( report main AMR encoder done, TX frame type, AMR mode, dtx_enable )	
	VP_STATUS_CNC_EMERGENCY_CALL,				// 0x16		(  )
	VP_STATUS_CNC_DETECTED,						// 0x17		(  )
	VP_STATUS_VOIP_DL_DONE,						// 0x18		(  )
	VP_STATUS_AUDIO_STREAM_DATA_READY,			// 0x19 ( arg0 = shared_audio_stream_0_crtl, arg1 = shared_audio_stream_1_crtl, arg2 = audio_stream_buf_idx )
	   /** \HR */
	   /** \par Module
	    *                    Audio
	    *  \par Command Code
	    *                    0x1A
	    *  \par Description
	    *       This reply is sent by the DSP after the initializations to be done by COMMAND_AUDIO_ENABLE are complete.
	    *
	    *              @param  UInt16 Value of shared_aadmac_aud_enable used in the initialization.
	    *              @param  None
	    *              @param  None
	    *              @param  None
	    *
	    *   \par Associated Command
	    *        For every COMMAND_AUDIO_ENABLE command in the CP's command queue, an associated VP_STATUS_AUDIO_ENABLE_DONE reply
	    *        would be sent in the vp_status queue.
	    *   \sa  shared_aadmac_aud_enable, UCOMMAND_AUDIO_ENABLE
	    */
	VP_STATUS_AUDIO_ENABLE_DONE,                 // 0x1A ( )
	   /** \HR */
	   /** \par Module
	    *                    Audio
	    *  \par Command Code
	    *                    0x1B
	    *  \par Description
	    *       This reply is sent by the DSP to echo VP_COMMAND_PTT_ENABLE.
	    *
	    *              @param  UInt16 PTT_flag
	    *              @param  UInt16 curr_int_rate_flag
	    *              @param  UInt16 shared_voif_enable_flag (debugging)
	    *              @param  None
	    *
	    */
	VP_STATUS_PTT_STATUS,                 // 0x1B ( )
	   /** \HR */
	   /** \par Module
	    *                    Audio
	    *  \par Command Code
	    *                    0x1C
	    *  \par Description
	    *       This reply is sent by the DSP to indicate PTT UL frame is read and ask next frame.
	    *
	    *              @param  UInt16 PTT_UL_buffer_index
	    *              @param  UInt16 PTT_flag
	    *              @param  UInt16 curr_int_rate_flag
	    *              @param  None
	    *
	    */
	VP_STATUS_PTT_UL_FRAME_DONE,                 // 0x1C ( )
	   /** \HR */
	   /** \par Module
	    *                    Audio
	    *  \par Command Code
	    *                    0x1D
	    *  \par Description
	    *       This reply from the DSP lets the AP know that the DSP has
	    *       enabled the external modem interface, and configured the
	    *       AADMAC channels for UL and DL external modem paths The
	    *       argument is the same as the one passed in the command.
	    *
        *              @param  UInt16 arg0 = 1: Enable\BR
        *                                  = 0: Disable
 	    *
	    */
	VP_STATUS_EXT_MODEM_CALL_DONE,               // 0x1D (0=disable, 1=Enable)
	   /** \HR */
	   /** \par Module
	    *                    Audio
	    *  \par Command Code
	    *                    0x1E
	    *  \par Description
	    *       This reply is part of the ARM2SP_HQ_DL interface for
	    *       mixing/playing 48kHz to Downlink. \BR
	    *       This reply is sent by the DSP whenever finishing every 2/4 20ms
	    *       PCM frame.
	    *       shared_ARM2SP_HQ_DL_InBuf_out indicates the next index DSP will
	    *       read.
	    *
	    *              @param  UInt16 shared_ARM2SP_HQ_DL_done_flag - flag
	    *                             indicating the end of transfer of data
	    *              @param  UInt16 shared_ARM2SP_HQ_DL_InBuf_out - index
	    *                             indicating next index DSP will read
	    *                             from the shared_Arm2SP_InBuf[]
	    *                             buffer.
	    *              @param  UInt16 ARM2SP_HQ_DL_flag - if
	    *                             shared_ARM2SP_HQ_DL_done_flag is set
	    *                             then ARM2SP_HQ_DL_flag = 0 \BR
	    *                             else send back ARM2SP_HQ_DL_flag
	    *                             received in the
	    *                             VP_COMMAND_SET_ARM2SP_HQ_DL command \BR
	    *              \htmlonly
	    *              <pre>
	    *                             {
	    *                               bits14:12   - Number of frames of data
	    *                                             after which the DSP is
	    *                                             supposed to send the
	    *                                             VP_STATUS_ARM2SP_HQ_DL_EMPTY
	    *                                             reply
	    *                                             (between 1 and 4 or 1 and 2)
	    *                               bit11:10    - reserved
	    *                               bit9        - DL_OVERWRITE_bit
	    *                                             1= overwrite ARM2SP_HQ_DL
	    *                                                buffer to DL
	    *                                                (only when DL_MIX_bit=0)
	    *                               bit8        - DL_MIX_bit
	    *                                             1= mixing ARM2SP_HQ_DL
	    *                                                buffer with DL
	    *                                                (regardless
	    *                                                DL_OVERWRITE_bit
	    *                                                setting)
	    *                               bit7        - 48kHz Mono/Stereo
	    *                                             [1/0] = [Mono/Stereo]
	    *                               bit6-1      - reserved
	    *                               bit0        - 48kHz DL PCM enable
	    *                                             [1/0] = [enable/disable]
	    *                             }
	    *              </pre>
	    *              \endhtmlonly
	    *
	    *
	    *  \see ARM2SP_HQ_DL_interface, shared_Arm2SP_InBuf,
	    *       shared_ARM2SP_HQ_DL_InBuf_out, shared_ARM2SP_HQ_DL_done_flag,
	    *       VP_COMMAND_SET_ARM2SP_HQ_DL
	    *
	    */
		VP_STATUS_ARM2SP_HQ_DL_EMPTY, 				// 0x1E
	   /** \HR */
	   /** \par Module
	    *                    Audio
	    *  \par Command Code
	    *                    0x1F
	    *  \par Description
	    *       This reply is part of the ARM2SP2_HQ_DL interface for
	    *       mixing/playing 48kHz to Downlink. \BR
	    *       This reply is sent by the DSP whenever finishing every 2/4 20ms
	    *       PCM frame.
	    *       shared_ARM2SP2_HQ_DL_InBuf_out indicates the next index DSP will
	    *       read.
	    *
	    *              @param  UInt16 shared_ARM2SP2_HQ_DL_done_flag - flag
	    *                             indicating the end of transfer of data
	    *              @param  UInt16 shared_ARM2SP2_HQ_DL_InBuf_out - index
	    *                             indicating next index DSP will read
	    *                             from the shared_Arm2SP2_InBuf[]
	    *                             buffer.
	    *              @param  UInt16 ARM2SP2_HQ_DL_flag - if
	    *                             shared_ARM2SP2_HQ_DL_done_flag is set
	    *                             then ARM2SP2_HQ_DL_flag = 0 \BR
	    *                             else send back ARM2SP2_HQ_DL_flag
	    *                             received in the
	    *                             VP_COMMAND_SET_ARM2SP2_HQ_DL command \BR
	    *              \htmlonly
	    *              <pre>
	    *                             {
	    *                               bits14:12   - Number of frames of data
	    *                                             after which the DSP is
	    *                                             supposed to send the
	    *                                             VP_STATUS_ARM2SP2_HQ_DL_EMPTY
	    *                                             reply
	    *                                             (between 1 and 4 or 1 and 2)
	    *                               bit11:10    - reserved
	    *                               bit9        - DL_OVERWRITE_bit
	    *                                             1= overwrite ARM2SP2_HQ_DL
	    *                                                buffer to DL
	    *                                                (only when DL_MIX_bit=0)
	    *                               bit8        - DL_MIX_bit
	    *                                             1= mixing ARM2SP2_HQ_DL
	    *                                                buffer with DL
	    *                                                (regardless
	    *                                                DL_OVERWRITE_bit
	    *                                                setting)
	    *                               bit7        - 48kHz Mono/Stereo
	    *                                             [1/0] = [Mono/Stereo]
	    *                               bit6-1      - reserved
	    *                               bit0        - 48kHz DL PCM enable
	    *                                             [1/0] = [enable/disable]
	    *                             }
	    *              </pre>
	    *              \endhtmlonly
	    *
	    *
	    *  \see ARM2SP2_HQ_DL_interface, shared_Arm2SP2_InBuf,
	    *       shared_ARM2SP2_HQ_DL_InBuf_out, shared_ARM2SP2_HQ_DL_done_flag,
	    *       VP_COMMAND_SET_ARM2SP2_HQ_DL
	    *
	    */
		VP_STATUS_ARM2SP2_HQ_DL_EMPTY, 				// 0x1F
		   /** \HR */
		   /** \par Module
		    *                    Audio
		    *  \par Command Code
		    *                    0x20
		    *  \par Description
		    *       This reply is part of the ARM2SP_HQ_DL interface for
		    *       mixing/playing 48kHz to Downlink. \BR
		    *       This reply is sent by the DSP after it finishes initializing
		    *       the ARM2SP HQ DL interface.
		    *
		    *              @param  UInt16 ARM2SP_HQ_DL_flag received in the
		    *                             VP_COMMAND_SET_ARM2SP_HQ_DL command \BR
		    *              \htmlonly
		    *              <pre>
		    *                             {
		    *                               bits14:12   - Number of frames of data
		    *                                             after which the DSP is
		    *                                             supposed to send the
		    *                                             VP_STATUS_ARM2SP_HQ_DL_EMPTY
		    *                                             reply
		    *                                             (between 1 and 4 or 1 and 2)
		    *                               bit11:10    - reserved
		    *                               bit9        - DL_OVERWRITE_bit
		    *                                             1= overwrite ARM2SP_HQ_DL
		    *                                                buffer to DL
		    *                                                (only when DL_MIX_bit=0)
		    *                               bit8        - DL_MIX_bit
		    *                                             1= mixing ARM2SP_HQ_DL
		    *                                                buffer with DL
		    *                                                (regardless
		    *                                                DL_OVERWRITE_bit
		    *                                                setting)
		    *                               bit7        - 48kHz Mono/Stereo
		    *                                             [1/0] = [Mono/Stereo]
		    *                               bit6-1      - reserved
		    *                               bit0        - 48kHz DL PCM enable
		    *                                             [1/0] = [enable/disable]
		    *                             }
		    *              </pre>
		    *              \endhtmlonly
		    *
		    *
		    *  \see ARM2SP_HQ_DL_interface, shared_Arm2SP_InBuf,
		    *       shared_ARM2SP_HQ_DL_InBuf_out, shared_ARM2SP_HQ_DL_done_flag,
		    *       VP_COMMAND_SET_ARM2SP_HQ_DL
		    *
		    */
			VP_STATUS_ARM2SP_HQ_DL_INIT_DONE, 				// 0x20
		   /** \HR */
		   /** \par Module
		    *                    Audio
		    *  \par Command Code
		    *                    0x21
		    *  \par Description
		    *       This reply is part of the ARM2SP2_HQ_DL interface for
		    *       mixing/playing 48kHz to Downlink. \BR
		    *       This reply is sent by the DSP after it finishes initializing
		    *       the ARM2SP2 HQ DL interface.
		    *
		    *              @param  UInt16 ARM2SP2_HQ_DL_flag received in the
		    *                             VP_COMMAND_SET_ARM2SP2_HQ_DL command \BR
		    *              \htmlonly
		    *              <pre>
		    *                             {
		    *                               bits14:12   - Number of frames of data
		    *                                             after which the DSP is
		    *                                             supposed to send the
		    *                                             VP_STATUS_ARM2SP2_HQ_DL_EMPTY
		    *                                             reply
		    *                                             (between 1 and 4 or 1 and 2)
		    *                               bit11:10    - reserved
		    *                               bit9        - DL_OVERWRITE_bit
		    *                                             1= overwrite ARM2SP2_HQ_DL
		    *                                                buffer to DL
		    *                                                (only when DL_MIX_bit=0)
		    *                               bit8        - DL_MIX_bit
		    *                                             1= mixing ARM2SP2_HQ_DL
		    *                                                buffer with DL
		    *                                                (regardless
		    *                                                DL_OVERWRITE_bit
		    *                                                setting)
		    *                               bit7        - 48kHz Mono/Stereo
		    *                                             [1/0] = [Mono/Stereo]
		    *                               bit6-1      - reserved
		    *                               bit0        - 48kHz DL PCM enable
		    *                                             [1/0] = [enable/disable]
		    *                             }
		    *              </pre>
		    *              \endhtmlonly
		    *
		    *
		    *  \see ARM2SP2_HQ_DL_interface, shared_Arm2SP2_InBuf,
		    *       shared_ARM2SP2_HQ_DL_InBuf_out, shared_ARM2SP2_HQ_DL_done_flag,
		    *       VP_COMMAND_SET_ARM2SP2_HQ_DL
		    *
		    */
			VP_STATUS_ARM2SP2_HQ_DL_INIT_DONE 				// 0x21

} VPStatus_t;
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */
  

//******************************************************************************
// Shared Memory Definition 
//******************************************************************************
// SINCE TLIII CORE/MSS, SHARED MEMORY PAGING IS REMOVED.
// THREE BIG CATERGORIES ARE DEFINED:
//
//
//                 1. MODEM
//                    SEC_CMD_STATUS
//                    SEC_MST_SAIC_GEN_MODEM
//                    SEC_MODEM_MET
//                    SEC_RESERVED_PART1                
//                    SEC_SMC_AFC_AGC
//                    SEC_RFIC
//                    SEC_RESERVED_PART2
//
//                2. AUDIO
//                    SEC_GEN_AUDIO
//                    SEC_NS
//                    SEC_NLP
//                    SEC_ECHO
//                    SEC_VPU
//                    SEC_AUX_AUDIO
//
//                3. DEBUG
//                    SEC_DIAGNOSIS    
//                    SEC_DSP_GEN_DEBUG    
//
//******************************************************************************
#ifdef MSP
typedef struct
{
#endif

EXTERN UInt32 NOT_USE_shared_memory_align                               AP_SHARED_SEC_CMD_STATUS;

/*-----------------------------------------------------------------------------------------------*/
/**
 * @addtogroup AP_Queues
 * @{
 */

/**
 * @addtogroup VP_Queues
 * @{
 */
/**
 * @addtogroup VP_Command_Queue
 * @{ 
 */
EXTERN UInt16 vp_shared_cmdq_in                                  AP_SHARED_SEC_CMD_STATUS;
EXTERN UInt16 vp_shared_cmdq_out                                 AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @addtogroup VP_Status_Queue
 * @{ 
 */
EXTERN UInt16 vp_shared_statusq_in                               AP_SHARED_SEC_CMD_STATUS;
EXTERN UInt16 vp_shared_statusq_out                              AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */

/**
 * @addtogroup VP_Command_Queue
 * @{ 
 */
EXTERN VPCmdQ_t vp_shared_cmdq[ VP_CMDQ_SIZE ]                   AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @addtogroup VP_Status_Queue
 * @{ 
 */
EXTERN VPStatQ_t vp_shared_statusq[ VP_STATUSQ_SIZE ]            AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

/* This is an internal DSP buffer and not necessarily a shared buffer */
EXTERN UInt16 ext_48khz_echo_ref[(IHF_48K_BUF_SIZE*2)+48] AP_SHARED_SEC_GEN_AUDIO;

/* Unused in DSP */
EXTERN UInt16 shared_unused_0x1000[AUDIO_SIZE_PER_PAGE-((IHF_48K_BUF_SIZE*2)+48)] AP_SHARED_SEC_GEN_AUDIO;
/* Unused in DSP */
EXTERN UInt16 shared_unused_1[1] AP_SHARED_SEC_GEN_AUDIO; // This buffer is not used and hence change to only 1 word - should be removed later on

/**
 * \ingroup MM_VPU_RECORD
 */
/**
 * This buffer is used to send back MM_VPU recoder's (encoder's) encoded data from
 * the DSP to the AP. The DSP writes the encoded data in shared_encoder_OutputBuffer 
 * with an index from shared_encodedSamples_buffer_in[0], and the ARM reads the encoded
 * data from shared_encoder_OutputBuffer with an index of shared_encodedSamples_buffer_out[0] 
 *
 * \see shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out
 *
 */
#ifdef FPGA_AUDIO_HUB_VERIFICATION
EXTERN UInt32 shared_encoder_OutputBuffer[AUDIO_SIZE_PER_PAGE/2] AP_SHARED_SEC_GEN_AUDIO;
#else
EXTERN UInt16 shared_encoder_OutputBuffer[AUDIO_SIZE_PER_PAGE] AP_SHARED_SEC_GEN_AUDIO;
#endif
/** @} */

/**
 * @addtogroup USB_Headset_Interface
 *
 * This interface is used for connecting a USB microphone and/or speaker to the
 * baseband chip. Instead of routing the data to the regular microphone and the speaker
 * voice paths, the DSP routes the data from and/or to the USB shared memory buffers.
 * The ARM fills in the microphone data from the USB stack on the ARM side and passes 
 * this data to the DSP using the shared memory buffers. The DSP fills in the 
 * speaker data to the shared memory buffers from which the ARM reads out to the 
 * USB stack on the ARM side.
 *
 * To start DSP interface for USB headset:
 * - ARM enables the voice interrupts to the DSP using the COMMAND_AUDIO_ENABLE command,
 *   and enables the input and/or the output to the USB using the COMMAND_AUDIO_CONNECT 
 *   command.
 * - ARM enables USB headset shared memory interface by COMMAND_USB_HEADSET. 
 * - At COMMAND_USB_HEADSET enable command, DSP starts using shared_usb_headset_audio_in_buf_8or16k0[ ]
 *   and shared_usb_headset_audio_out_buf_8or16k0[ ] as audio in sources and audio out sink.
 * - When DSP audio ISR changes 20ms boundary, DSP sends STATUS_USB_HEADSET_BUFFER with the 
 *   in/out buffer pointers for ARM use. ARM then should write audio in buffer and 
 *   read audio out buffer accordingly. 
 *
 *   To stop USB headset:
 * - ARM disables the voice interrupts using the COMMAND_AUDIO_ENABLE command,
 *   and disables the input and/or the output to the USB using the COMMAND_AUDIO_CONNECT 
 *   command.
 * - ARM disables USB headset interface using COMMAND_USB_HEADSET (arg0=0). 
 *
 * \note
 *    - USB headset Uplink (Microphone) and Downlink (Speaker) can be enabled/disabled separately.
 *    - USB headset UL and DL can have different Sampling Rates. 
 *      SRC should be done on arm side in general. Currently in this interface,
 *      only SRC_48K8K (0) and SRC_48K16K (1) with simple DOWN-SAMPLE are supported in DSP code.
 * 
 * \see COMMAND_USB_HEADSET, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, STATUS_USB_HEADSET_BUFFER
 * @{
 */
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 8kHz/16kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_8or16k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of input samples. Once the DSP is done
 * playing the samples from this buffer, it would go ahead and read the samples 
 * from shared_usb_headset_audio_in_buf_8or16k1. Once it has completed reading
 * samples from shared_usb_headset_audio_in_buf_8or16k1, it would go ahead and
 * read samples back from this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_in_buf_8or16k1, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_8or16k0[320]   AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 48kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_48k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of input samples. Once the DSP is done
 * playing the samples from this buffer, it would go ahead and read the samples 
 * from shared_usb_headset_audio_in_buf_48k1. Once it has completed reading
 * samples from shared_usb_headset_audio_in_buf_48k1, it would go ahead and
 * read samples back from this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_in_buf_8or16k0 !!!!!
 *
 * \see shared_usb_headset_audio_in_buf_48k1, COMMAND_USB_HEADSET, 
 *      shared_usb_headset_audio_in_buf_8or16k0
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_48k0[960]	     AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 8kHz/16kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_8or16k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of input samples. First the DSP 
 * plays the samples from shared_usb_headset_audio_in_buf_8or160, then 
 * it would go ahead and read the samples 
 * from this buffer. Once it has completed reading
 * samples from this buffer , it would go ahead and
 * read samples back from shared_usb_headset_audio_in_buf_8or160. 
 * The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_in_buf_8or16k0, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_8or16k1[320]   AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 48kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_48k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of input samples. First the DSP 
 * plays the samples from shared_usb_headset_audio_in_buf_48k0, then 
 * it would go ahead and read the samples 
 * from this buffer. Once it has completed reading
 * samples from this buffer , it would go ahead and
 * read samples back from shared_usb_headset_audio_in_buf_48k0. 
 * The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_in_buf_8or16k1 !!!!!
 *
 * \see shared_usb_headset_audio_in_buf_48k0, COMMAND_USB_HEADSET,
 *      shared_usb_headset_audio_in_buf_8or16k1
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_48k1[960]	     AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 8kHz/16kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_8or16k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to this buffer. The second speech frame would then go to 
 * shared_usb_headset_audio_out_buf_8or16k1 after 20ms. Further 20ms later, 
 * it will again fill this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_out_buf_8or16k1, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_8or16k0[320]  AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 48kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_48k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to this buffer. The second speech frame would then go to 
 * shared_usb_headset_audio_out_buf_48k1 after 20ms. Further 20ms later, 
 * it will again fill this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_out_buf_8or16k0 !!!!!
 *
 * \see shared_usb_headset_audio_out_buf_48k1, COMMAND_USB_HEADSET, 
 *      shared_usb_headset_audio_out_buf_8or16k0
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_48k0[960]     AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 8kHz/16kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_8or16k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to shared_usb_headset_audio_out_buf_8or16k0. The second speech frame would then go to 
 * this buffer after 20ms. Further 20ms later, it will again fill 
 * shared_usb_headset_audio_out_buf_8or16k0. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_out_buf_8or16k0, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_8or16k1[320]  AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 48kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_48k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to shared_usb_headset_audio_out_buf_48k0. The second speech frame would then go to 
 * this buffer after 20ms. Further 20ms later, it will again fill 
 * shared_usb_headset_audio_out_buf_48k0. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_out_buf_8or16k1 !!!!!
 *
 * \see shared_usb_headset_audio_out_buf_48k0, COMMAND_USB_HEADSET, 
 *      shared_usb_headset_audio_out_buf_8or16k1
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_48k1[960]     AP_SHARED_SEC_GEN_AUDIO;
/** 
 * @} 
 */
EXTERN UInt16 shared_BTnbdinLR_unused[1] AP_SHARED_SEC_GEN_AUDIO;

/**
 * @addtogroup ARM2SP_HQ_DL_Interface
 *
 * This interface is used for mixing/playing \Bold{48kHz Mono/Stereo PCM data
 * to Downlink}.\BR
 *
 * \note This interface works in only one way - from ARM to DSP. \BR
 *
 * One use-case of this interface is to support S-barge-in feature where a
 * music might be playing in the downlink using this interface and an
 * uplink record/speech recognition happens at the same time, then this
 * UL would get the benefit of echo cancellation of the music.
 *
 * To start ARM2SP_HQ_DL: ARM enables ARM2SP_HQ_DL by
 * VP_COMMAND_SET_ARM2SP_HQ_DL.\BR
 *
 * Once the DSP completes the processing of this command it sends
 * the VP_STATUS_ARM2SP_HQ_DL_INIT_DONE to the AP to indicate to it
 * that the AADMAC channel is ready for transmission.
 *
 * To stop ARM2SP_HQ_DL:\BR
 * Quick stop: ARM disables ARM2SP_HQ_DL using
 * VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0).\BR
 * \note The ARM2SP_HQ_DL will be stopped right away.
 * There may be un-finished PCM data in shared_Arm2SP_HQ_DL_InBuf[].\BR
 *
 * \see shared_Arm2SP_HQ_DL_InBuf,
 *      VP_COMMAND_SET_ARM2SP_HQ_DL,
 *      VP_STATUS_ARM2SP_HQ_DL_INIT_DONE
 *
 * @{ */
EXTERN UInt32 shared_Arm2SP_HQ_DL_InBuf[NUM_OF_48K_SAMP_PER_INT0_INT*2]
	AP_SHARED_SEC_GEN_AUDIO;
 /* @} */

EXTERN UInt16 shared_BTnbdinLR_1_unused[
	2*640-1-(NUM_OF_48K_SAMP_PER_INT0_INT*2*2)] AP_SHARED_SEC_GEN_AUDIO;
/**
 * @addtogroup VPU_Shared_Memory_Interface NOT_USED 
 *
 * This frame-aware interface is provided for transfer of both data and control 
 * information between the ARM and DSP during NB-AMR or 8k PCM record and 
 * playback. \BR
 *
 * This interface supports individual or simultaneous record and playback either 
 * stand-alone (memo mode) or along with a phone-call, of either uplink or downlink 
 * or both. \BR
 *
 * Dedicated ping-pong buffers will 
 * be used for transferring speech data between the two processors while the 
 * VP_Command_Queue and VP_Status_Queue queues, also residing in Shared memory,
 * will be used for exchanging control information.
 *
 * \BR
 *
 * This interface supports simultaneous record and playback. It has two dedicated 
 * buffers each for record (shared_voice_buf.vr_buf[2]) and for playback 
 * (shared_voice_buf.vp_buf[2]). These buffers are identical in size and structure.
 * The data is exchanged in a ping-pong fashion - i.e. when DSP is reading data from
 * one buffer, ARM is writing into another buffer and vice-versa.
 *
 * \note Two words are placed before each buffer for indicating the format of the 
 * speech data, i.e., AMR/Linear PCM and the number of 20 ms speech frames contained.  
 *
 * \BR
 *
 * Speech data in an uncompressed form will be exchanged between ARM and DSP in a 
 * maximum of 640 word allocation buffer sizes (representing 80 msec worth of speech 
 * data) or in smaller sizes for compressed speech (size is in multiples of 20ms frames). 
 * Since double buffering is required this results in a requirement of 1280 words of 
 * Shared memory per buffer (2560 words total), which will be operated in a cyclic mode.  
 *
 * \note The transfer of each data buffer from the DSP to the ARM will always be 
 * accompanied by an associated command.  However, the converse is not always true 
 * for data sent from the ARM to the DSP.
 *
 * \BR
 *
 * This interface needs to be enabled using COMMAND_VPU_ENABLE command in the main
 * command queue.
 *
 * \note This interface provides for recording/playback in either \Bold{AMR-NB OR 
 * PCM - Not Both}
 *
 *
 * \see VP_Command_Queue, VP_Status_Queue, shared_voice_buf, COMMAND_VPU_ENABLE
 * @{ */
EXTERN shared_voice_buf_t shared_voice_buf                 AP_SHARED_SEC_GEN_AUDIO; //!< This buffer is used
                                                                                 //!< to transfer data 
                                                                                 //!< between the DSP and the
                                                                                 //!< ARM for VPU - recording
                                                                                 //!< and playback
                                                                                 //!< \see VP_Queues
/** @} */

/** 
 * @addtogroup WCDMA_encoded_data_interface 
 * 
 * This interface is provided to transfer data back and forth simultaneously between the ARM and the DSP
 * for WCDMA or VoIP call. The data being transfered is in \Bold{AMR-NB\, AMR-WB\, FR or PCM formats only}. \BR
 *
 * The COMMAND_MAIN_AMR_RUN 
 * has to be sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. 
 * The input to the speech decoder comes in DL_MainAMR_buf.param, and the output of the encoder is 
 * stored in UL_MainAMR_buf.param \BR
 *
 * For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent in the 
 * status queue. 
 * 
 * \note This interface can be only used for simultaneous capture and playback. It cannot be used for only
 *       capture or only playback of AMR/VoIP data.
 *
 * \BR
 *
 * \note This interface has to be able to run independently of the record and playback interface, as there
 *       can be use cases of record and playback during a WCDMA or VoIP call.
 * 
 * \see COMMAND_MAIN_AMR_RUN, DL_MainAMR_buf, UL_MainAMR_buf, AP_DL_MainAMR_buf, AP_UL_MainAMR_buf, STATUS_MAIN_AMR_DONE.
 * @{ */
EXTERN VR_Frame_AMR_WB_t AP_DL_MainAMR_buf    				   AP_SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
                                                                                 //!< to transfer the input to
                                                                                 //!< the speech decoder in the 
                                                                                 //!< DSP from 
                                                                                 //!< WCDMA channel decoder
                                                                                 //!< This buffer is filled by
                                                                                 //!< the ARM (as well as emtying
                                                                                 //!< the UL_MainAMR_buf buffer)
                                                                                 //!< and then it sends
                                                                                 //!< the COMMAND_MAIN_AMR_RUN
                                                                                 //!< command to the DSP to start
                                                                                 //!< decoding the encoded 
                                                                                 //!< AMR data
                                                                                 //!< \sa COMMAND_MAIN_AMR_RUN,
                                                                                 //!< STATUS_MAIN_AMR_DONE,
                                                                                 //!< UL_MainAMR_buf
EXTERN VR_Frame_AMR_WB_t AP_UL_MainAMR_buf    				   AP_SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
                                                                                 //!< to transfer the output of
                                                                                 //!< the speech encoder in 
                                                                                 //!< the DSP to 
                                                                                 //!< WCDMA channel encoder
                                                                                 //!< This buffer is filled by
                                                                                 //!< the DSP (as well as emtying
                                                                                 //!< the DL_MainAMR_buf buffer)
                                                                                 //!< and then it sends
                                                                                 //!< the STATUS_MAIN_AMR_DONE
                                                                                 //!< reply to the ARM to start
                                                                                 //!< channel encoding the 
                                                                                 //!< AMR data
                                                                                 //!< \sa COMMAND_MAIN_AMR_RUN,
                                                                                 //!< STATUS_MAIN_AMR_DONE,
                                                                                 //!< DL_MainAMR_buf
/** @} */

/**
 * @addtogroup ARM2SP_interface 
 *
 * This interface is used for mixing/playing \Bold{8kHz/16kHz PCM data to either Uplink or 
 * Downlink or Both}.\BR
 *
 * There are 2 instances of this interface.\BR
 *
 * This PCM data can be mixed before or after Uplink or Downlink speech processing.
 *
 * \note This interface works in only one way - from ARM to DSP. \BR
 *
 * One use-case of this interface is mixing of call-record warning tones on UL and DL. 
 * The other is mixing of un-equalized music playback (music sharing) to the Uplink. \BR
 *
 * In this interface 4-speech frames worth of data is transferred at a time from
 * ARM to DSP in ping-pong buffers - i.e. ARM fills empty half of shared_Arm2SP_InBuf[].
 * When shared_Arm2SP_InBuf_out=0, ARM fills next half number of words from shared_Arm2SP_InBuf[half].
 * When shared_Arm2SP_InBuf_out=half, ARM fills half number of words from shared_Arm2SP_InBuf[0]. \BR
 *
 * To start ARM2SP: ARM enables ARM2SP by COMMAND_SET_ARM2SP.\BR
 *
 * To resume ARM2SP (JunoC0): ARM enables ARM2SP by COMMAND_SET_ARM2SP  with arg1=1. 
 * 
 * To control shared_Arm2SP_InBuf[]: DSP interrupts ARM with
 * STATUS_ARM2SP_EMPTY( shared_Arm2SP_done_flag, shared_Arm2SP_InBuf_out, Arm2SP_flag ) 
 * whenever finishing every half PCM data. shared_Arm2SP_InBuf_out indicates the next index DSP will read.
 *
 * To stop ARM2SP:\BR
 * Quick stop: ARM disables ARM2SP using COMMAND_SET_ARM2SP (arg0=0).\BR
 * \note The ARM2SP will be stopped right away. There may be un-finished PCM data in shared_Arm2SP_InBuf[].\BR
 *
 * Finishing stop: After finishing filling the last 2/4-speech frame PCM data, ARM set shared_Arm2SP_done_flag=1 
 * to indicate the end of PCM data. DSP ISR finishes the last 2/4-speech frames and disables ARM2SP. DSP sends 
 * STATUS_ARM2SP_EMPTY and arg0=0. \BR
 *
 * To pause ARM2SP (JunoC0): ARM disables ARM2SP using COMMAND_SET_ARM2SP (arg0=0, arg1=1).
 *
 * \note When used for mixing warning tone during speech recording, ARM2SP should be enabled periodic 
 * (every a few seconds) instead of being enabled all the time with tone plus lots 0's PCM. 
 * For example, ARM can fill 2/4-speech frames (640 PCM samples) of tone plus 0's and set 
 * shared_Arm2SP_done_flag=1. DSP will mix or overwrite these 2/4-speech frames and disable ARM2SP, then 
 * send STATUS_ARM2SP_EMPTY( shared_Arm2SP_done_flag, 
 * shared_Arm2SP_InBuf_out, Arm2SP_flag )
 *
 * \note The speech processing occurs at the rate specified by AP. If there is a mis-match 
 * between the rate set by the AP and ARM2SP\’s rate, the Sample Rate
 * Conversion occurs right at the start.
 *
 * \note For the second instance of the ARM2SP, please read ARM2SP2 everywhere ARM2SP is written above.
 * 
 * \see shared_Arm2SP_InBuf, shared_Arm2SP_InBuf_out, shared_Arm2SP_done_flag, 
 *      COMMAND_SET_ARM2SP, STATUS_ARM2SP_EMPTY,
 *      shared_Arm2SP2_InBuf, shared_Arm2SP2_InBuf_out, shared_Arm2SP2_done_flag, 
 *      COMMAND_SET_ARM2SP2, STATUS_ARM2SP2_EMPTY 
 * 
 * @{ */
/**
 * This flag indicates the end of the transfer of data from the ARM to the DSP.
 * The transfer of data by ARM2SP interface can be disabled by setting 
 * shared_Arm2SP_done_flag=1 after filling the shared_Arm2SP_InBuf[] buffer. 
 * DSP will disable the feature when it reaches the next 2/4-speech frame boundary. */
EXTERN UInt16    shared_Arm2SP_done_flag                   					AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is used for mixing PCM data in sharedmem buffer to DL and/or 
 * UL during speech call. One usage is to send warning tone when doing call 
 * recording. The buffer is used as ping-pong buffer, each with 2/4-speech frame 
 * (4*160) or (2*320).
 */ 
EXTERN UInt32    shared_Arm2SP_InBuf[ARM2SP_INPUT_SIZE_48K/2]  				AP_SHARED_SEC_GEN_AUDIO;
/**
 * This index indicates the next index DSP will read from the shared_Arm2SP_InBuf[] 
 * buffer. 
 *
 * \note This index is changed only by the DSP. 
 * 
 * */
EXTERN UInt16    shared_Arm2SP_InBuf_out                   					AP_SHARED_SEC_GEN_AUDIO;
/** @} */

/** 
 * @addtogroup Shared_Audio_Buffers
 *
 * @{ */
/**
 * @addtogroup ARM2SP2_interface 
 * @{ */
/**
 * This flag indicates the end of the transfer of data from the ARM to the DSP.
 * The transfer of data by ARM2SP2 interface can be disabled by setting 
 * shared_Arm2SP2_done_flag=1 after filling the shared_Arm2SP2_InBuf[] buffer. 
 * DSP will disable the feature when it reaches the next 2/4-speech frame boundary. */
EXTERN UInt16 shared_Arm2SP2_done_flag                   					AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is used for mixing PCM data in sharedmem buffer to DL and/or 
 * UL during speech call. One usage is to send warning tone when doing call 
 * recording. The buffer is used as ping-pong buffer, each with 2/4-speech frame 
 * (4*160) or (2*320).
 */ 
EXTERN UInt32 shared_Arm2SP2_InBuf[ARM2SP_INPUT_SIZE_48K/2]    				AP_SHARED_SEC_GEN_AUDIO;
/**
 * This index indicates the next index DSP will read from the shared_Arm2SP2_InBuf[] 
 * buffer. 
 *
 * \note This index is changed only by the DSP. 
 * 
 * */
EXTERN UInt16 shared_Arm2SP2_InBuf_out                   					AP_SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */


/** 
 * @addtogroup MM_VPU_Interface 
 * @{
 */
/**
 * \ingroup MM_VPU_RECORD
 * \par For MM_VPU Encoder
 * The first index is used to send back MM_VPU recoder's (encoder's) encoded data from
 * the DSP to the AP. The DSP writes the encoded data in shared_encoder_OutputBuffer 
 * with an index from shared_encodedSamples_buffer_in[0]. DSP updates the first index.
 *
 * \see shared_encoder_OutputBuffer
 *
 */
EXTERN UInt16 shared_encodedSamples_buffer_in[2]                         	AP_SHARED_SEC_GEN_AUDIO;
/**
 * \ingroup MM_VPU_RECORD
 * \par For MM_VPU Encoder
 * The first index is used to send back MM_VPU recoder's (encoder's) encoded data from
 * the DSP to the AP. The ARM reads the encoded
 * data from shared_encoder_OutputBuffer with an index of shared_encodedSamples_buffer_out[0].
 * ARM updates the first index. 
 *
 * \see shared_encoder_OutputBuffer
 *
 */
EXTERN UInt16 shared_encodedSamples_buffer_out[2]                         	AP_SHARED_SEC_GEN_AUDIO;

/**
 * This variable should be configured by the AP before sending the COMMAND_MM_VPU_ENABLE command to the DSP. 
 * It controls the encoder and decoder of the COMMAND_MM_VPU_ENABLE.
 *
 * The bit map of this variable is as follows: \BR
 *
 * \htmlonly 
 * <pre>
 * { bits 3-0: Bit-rate mode - 
 *                           0: 6.60 kbit/s  
 *                           1: 8.85 kbit/s  
 *                           2: 12.65 kbit/s 
 *                           3: 14.25 kbit/s 
 *                           4: 15.85 kbit/s 
 *                           5: 18.25 kbit/s 
 *                           6: 19.85 kbit/s 
 *                           7: 23.05 kbit/s 
 *                           8: 23.85 kbit/s 
 *   bit  4  : DTX mode - 0=disabled, 1=enabled 
 *   bits 6-5: Bitstream mode - 0=ITU Format 1, 2=MIME
 *   bit  10 : Download mode - 0=DMA, 1=Software (Not used currently)
 *   bit  12-11: Output Select for the decoder (does not affect the encoder. 
 *             Encoder's output is always sent in shared_encoder_OutputBuffer)
 * }
 *
 * MM VPU Decode:
 * =============
 * +--------------------------------------+--------------------------------+--------------------------------+-----------------------------------+
 * | Output Select Value                  |               00               |               01               |  10 (used only for WB-AMR Decode) |
 * +--------------------------------------+--------------------------------+--------------------------------+-----------------------------------+
 * | Encoded Speech Inp Buf               |shared_decoder_InputBuffer      |shared_decoder_InputBuffer      |shared_decoder_InputBuffer         |
 * | ARM's Encoded Speech Write Indx      |shared_NEWAUD_InBuf_in[0]       |shared_NEWAUD_InBuf_in[1]       |shared_encodedSamples_buffer_in[0] |
 * | DSP's Encoded Speech Read Indx       |shared_NEWAUD_InBuf_out[0]      |shared_NEWAUD_InBuf_out[1]      |shared_encodedSamples_buffer_out[0]|
 * | Input Data Done Flag                 |shared_NEWAUD_InBuf_done_flag[0]|shared_NEWAUD_InBuf_done_flag[1]|shared_encodedSamples_done_flag[0] |
 * | PCM Output Data Buffer               |shared_pram_codec_out0          |shared_newpr_codec_out0         |shared_decoder_OutputBuffer        |
 * | DSP's Decoded Speech (PCM) Write Indx|shared_NEWAUD_OutBuf_in[0]      |shared_NEWAUD_OutBuf_in[1]      |shared_decodedSamples_buffer_in[0] |
 * | Decoded (PCM) Speech Read Indx       |shared_NEWAUD_OutBuf_out[0]     |shared_NEWAUD_OutBuf_out[1]     |shared_decodedSamples_buffer_out[0]|
 * +--------------------------------------+--------------------------------+--------------------------------+-----------------------------------+
 *
 * MM VPU Encode:
 * =============
 * +--------------------------------------+-----------------------------------+
 * | PCM Speech (from MIC) Inp Buf        |Internal to the DSP                |
 * | DSP's PCM Speech Write Indx          |N/A                                |
 * | ARM's PCM Speech Read Indx           |N/A                                |
 * | Speech Capture Done Flag(Not useful) |shared_decodedSamples_done_flag[0] |
 * | Encoded Speech Out Buf               |shared_encoder_OutputBuffer        |
 * | DSP's Encoded Speech Write Indx      |shared_encodedSamples_buffer_in[0] |
 * | ARM's Encoded Speech Read Indx       |shared_encodedSamples_buffer_out[0]|
 * +--------------------------------------+-----------------------------------+
 *
 * </pre>
 * \endhtmlonly
 * \note Case of Output Select Value = %10 - does not seem to support support playback to the speaker in the code !!! \BR
 *
 * \note In the case of Output Select Value = %10 - the indexes of the encoded speech are the same as the indexes
 *       used by the MM_VPU_RECORD for encoded speech, except the encoded speech buffer is different
 *                
 * \see shared_decoder_InputBuffer, shared_NEWAUD_InBuf_in, shared_NEWAUD_InBuf_out, shared_NEWAUD_InBuf_done_flag
 *      shared_pram_codec_out0, shared_NEWAUD_OutBuf_in, shared_NEWAUD_OutBuf_out, shared_newpr_codec_out0, shared_decoder_OutputBuffer,
 *      shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out, shared_encodedSamples_done_flag, shared_decodedSamples_buffer_in,
 *      shared_decodedSamples_buffer_out, shared_encoder_OutputBuffer,
 *      MM_VPU_RECORD, MM_VPU_PLAYBACK
 */ 
EXTERN UInt16 shared_WB_AMR_Ctrl_state                                     	AP_SHARED_SEC_GEN_AUDIO;                        // prgrammable states
/**
 * @addtogroup MM_VPU_PLAYBACK
 * @{
 */ 
/**
 * These Read indices are ONLY used for MM_VPU_PLAYBACK interface (even though the name
 * suggests otherwise). \BR
 *
 * They are used to the point till which compressed WB-AMR data is written.\BR
 * 
 * They both point inside shared_decoder_InputBuffer.\BR
 *
 * ARM updates these indices.
 *
 * \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_decoder_InputBuffer
 */
EXTERN UInt16 shared_NEWAUD_InBuf_in[2]                                   	AP_SHARED_SEC_GEN_AUDIO;                        //arm updates after adding the input data
/**
 * These Write indices are ONLY used for MM_VPU_PLAYBACK interface (even though the name
 * suggests otherwise). \BR
 *
 * They are used to the point from which compressed WB-AMR data is to be read.\BR
 * 
 * They both point inside shared_decoder_InputBuffer.\BR
 *
 * DSP updates these indices.
 *
 * \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_decoder_InputBuffer
 */
EXTERN UInt16 shared_NEWAUD_InBuf_out[2]                                  	AP_SHARED_SEC_GEN_AUDIO;                        //dsp updates after using the input data
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup NEWAUDFIFO_Interface
 * @{
 */ 
/**
 * These Done Flags are used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases they are used to indicate that ARM has no more data to send to the DSP.
 * In case of NEWAUDFIFO_Interface interface, it implies end of un-compressed PCM
 * data, whereas in case of MM_VPU_PLAYBACK, it implies end of compressed WB-AMR encoded
 * data. \BR
 * 
 * Their associated buffers are shared_pram_codec_out0 and shared_newpr_codec_out0 respectively.\BR
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_pram_codec_out0, 
 *      shared_newpr_codec_out0
 */
EXTERN Int16 shared_NEWAUD_InBuf_done_flag[2]                             	AP_SHARED_SEC_GEN_AUDIO;                        //arm set it to 1 if InBuf is all loaded (end of file)

/** @} */

/** @addtogroup MM_VPU_Interface 
 * @{ */
/**
 * \ingroup MM_VPU_PLAYBACK
 *
 * This variable specifies the low threshold for the input buffer of MM_VPU_PLAYBACK.
 *
 * If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer, 
 * becomes lower than the threshold shared_Inbuf_LOW_Sts_TH, the DSP sends a interrupt to 
 * the ARM with a status reply STATUS_PRAM_CODEC_INPUT_LOW, requesting the ARM to send 
 * more downlink data. 
 *
 * \note This threshold is currently not used in Athena. Currently these thresholds 
 * are hard-coded in the code to 0x400 
 * 
 */
EXTERN Int16 shared_Inbuf_LOW_Sts_TH                                     	AP_SHARED_SEC_GEN_AUDIO;
/**
 * \ingroup MM_VPU_PLAYBACK
 *
 * This variable specifies the low threshold for the output buffer of MM_VPU_PLAYBACK.
 *
 * If the amount of remaining space in the output PCM buffer (as selected by 
 * shared_WB_AMR_Ctrl_state variable) becomes lower than the threshold shared_Outbuf_LOW_Sts_TH,
 * the DSP sends a interrupt to the ARM with a status reply STATUS_PRAM_CODEC_OUTPUT_LOW, 
 * requesting the ARM to slow down in sending more downlink data. 
 *
 * \note This threshold is currently not used in Athena. Currently these thresholds are 
 * hard-coded in the code to 0x400 
 * 
 */
EXTERN Int16 shared_Outbuf_LOW_Sts_TH                                     	AP_SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
/**
 * @}
 */
EXTERN UInt16 shared_voif_enable_flag										AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voif_DL_buffer_index									AP_SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_voif_DL_buffer[640]										AP_SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_voif_UL_buffer[640]										AP_SHARED_SEC_GEN_AUDIO;
EXTERN dummy4_input_t shared_SP_input										AP_SHARED_SEC_GEN_AUDIO;
EXTERN dummy2_config_t shared_SP_left_config								AP_SHARED_SEC_GEN_AUDIO;
EXTERN dummy2_config_t shared_SP_right_config								AP_SHARED_SEC_GEN_AUDIO;
EXTERN dummy3_queries_t shared_SP_params_left								AP_SHARED_SEC_GEN_AUDIO;
EXTERN dummy3_queries_t shared_SP_params_right								AP_SHARED_SEC_GEN_AUDIO;
/**
 * This variable tells about the audio sampling rate to the DSP, while using AADMAC during a regular
 * phone call.
 */
EXTERN UInt16 shared_aadmac_audio_samp_rate									AP_SHARED_SEC_GEN_AUDIO;

/**
 * This variable tells what audio paths are enabled.
 */
EXTERN UInt16 shared_aadmac_aud_enable            									AP_SHARED_SEC_GEN_AUDIO;


/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/**
 * @addtogroup IHF_48K_OUTPUT
 *
 * This interface is used for sending 48KHz output data to the AADMAC which then passes the data
 * out to the IHF device. This interface shoule be able to support both mono and stereo data - however 
 * currently it supports only mono.
 *
 * \see COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT
 * @{
 */
/**
 * These buffers are used for transfer of mono data between DSP and AADMAC. 
 *
 * \see COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE
 */
EXTERN UInt16 shared_aud_out_buf_48k[2][IHF_48K_BUF_SIZE]                   AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_aud_out_buf_48k_unused[2][IHF_48K_BUF_SIZE]            AP_SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */

/**
 * @addtogroup HANDSET_AADMAC_BUFFERS
 *
 * This interface is used for receiving 8/16 KHz input data from the AADMAC which then passed over to the regular hand-set.
 *
 * \see
 * @{
 */
/**
 * These buffers are used for getting mono data for primary mic from AADMAC.
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low buffer followed by
 *       the high buffer.
 */
EXTERN UInt32 shared_aadmac_pri_mic_low[NUM_OF_8K_SAMP_PER_INT0_INT*2]                   AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt32 shared_aadmac_pri_mic_high[NUM_OF_8K_SAMP_PER_INT0_INT*2]                  AP_SHARED_SEC_GEN_AUDIO;

/**
 * These buffers are used for getting mono data for primary mic from AADMAC.
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low buffer followed by
 *       the high buffer.
 */
EXTERN UInt32 shared_aadmac_sec_mic_low[NUM_OF_8K_SAMP_PER_INT0_INT*2]                   AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt32 shared_aadmac_sec_mic_high[NUM_OF_8K_SAMP_PER_INT0_INT*2]                  AP_SHARED_SEC_GEN_AUDIO;

/**
 * These buffers are used for sending mono data for speaker from AADMAC (all paths other than IHF).
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low buffer followed by
 *       the high buffer.
 */
EXTERN UInt32 shared_aadmac_spkr_low[NUM_OF_48K_SAMP_PER_INT0_INT]                   AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt32 shared_aadmac_spkr_high[NUM_OF_48K_SAMP_PER_INT0_INT]                  AP_SHARED_SEC_GEN_AUDIO;
/**
 * These buffers are used for getting mono data for echo reference from AADMAC.
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low
 * buffer followed by the high buffer.
 */
EXTERN UInt32 shared_aadmac_echo_ref_low[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt32 shared_aadmac_echo_ref_high[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	AP_SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
/**
 * @}
 */
/* 20 - size of JB stats and delays below + 32 echo ref buffers above */
EXTERN UInt32 shared_UNUSED[95-10-32] AP_SHARED_SEC_GEN_AUDIO;
/**
 * @addtogroup Adaptive Jitter Buffer
 * @{
 */

/** 
 * Adaptive Jitter Buffer statistics for reporting to ARM .\BR
 * 
 */
EXTERN DJB_STATS shared_AjcStatistics AP_SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_unused_extra                                            AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_unused_voif                                             AP_SHARED_SEC_GEN_AUDIO;


#ifdef FPGA_AUDIO_HUB_VERIFICATION
EXTERN UInt16 shared_ap_test_flag_for_sspi4                                 AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dsp_test_flag_for_sspi4                                AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 *shared_ap_test_inptr_for_sspi4                               AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 *shared_dsp_test_outptr_for_sspi4                             AP_SHARED_SEC_GEN_AUDIO;
#endif

EXTERN UInt16 shared_cnc_emergency_flag										AP_SHARED_SEC_GEN_AUDIO;	// 1: emergency call flag
EXTERN UInt16 shared_cnc_debug												AP_SHARED_SEC_GEN_AUDIO;	// 
EXTERN UInt16 shared_cnc_init_flag											AP_SHARED_SEC_GEN_AUDIO;	// 1: init cnc then clear this variable

EXTERN VOIP_Buffer_t VOIP_DL_buf										 	AP_SHARED_SEC_GEN_AUDIO;
EXTERN VOIP_Buffer_t VOIP_UL_buf										 	AP_SHARED_SEC_GEN_AUDIO;

/**
 * @addtogroup Audio_Gains
 * @{
 */

/** 
 * The gain factor, shared_inp_sp_gain_to_arm2sp_mixer_dl is applied on the downlink speech right before 
 * either ARM2SP or ARM2SP2 or BTNB signals are "added" to the downlink (does not matter whether the 
 * ARM2SP, ARM2SP2, BTRNB data is added before or after audio processing).\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 1.14 format).\BR
 *
 * @note This gain is only applied when ARM2SP, ARM2SP2 or BTNB are mixed in the downlink
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_inp_sp_gain_to_arm2sp_mixer_dl[5]			 				AP_SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_inp_sp_gain_to_arm2sp_mixer_ul is applied on the uplink speech right before 
 * either ARM2SP or ARM2SP2 or BTNB or TONEUL signals are "added" to the uplink (does not matter whether the 
 * ARM2SP, ARM2SP2, BTNB TONEUL data is added before or after audio processing).\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 1.14 format).\BR
 *
 * @note This gain is only applied when ARM2SP, ARM2SP2 or BTNB are mixed in the uplink
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_inp_sp_gain_to_arm2sp_mixer_ul[5]			 				AP_SHARED_SEC_GEN_AUDIO;

/** 
 * The gain factor, shared_arm2speech2_call_gain_dl is applied on the shared_Arm2SP2_InBuf[1280] PCM data 
 * on the downlink path (does not matter whether the ARM2SP2 data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_dl[5]			 				AP_SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech2_call_gain_ul is applied on the shared_Arm2SP2_InBuf[1280] PCM data 
 * on the uplink path (does not matter whether the ARM2SP2 data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_ul[5]			 				AP_SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech2_call_gain_rec is applied on the ARM2SP2 PCM data 
 * getting recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_rec[5]			 				AP_SHARED_SEC_GEN_AUDIO;

/** 
 * The gain factor, shared_arm2speech_call_gain_dl is applied on the shared_Arm2SP_InBuf[] PCM data
 * on the downlink path (does not matter whether the ARM2SP data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_dl[5]			 AP_SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_ul is applied on the shared_Arm2SP_InBuf[] PCM data
 * on the uplink path (does not matter whether the ARM2SP data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_ul[5]			 AP_SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_rec is applied on the ARM2SP PCM data 
 * getting recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_rec[5]			 AP_SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the downlink path before going into record.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_speech_rec_gain_dl[5]				 AP_SHARED_SEC_GEN_AUDIO;

/**
 * @}
 */

/**
 * @addtogroup Adaptive Jitter Buffer
 * @{
 */

/** 
 * Adaptive Jitter Buffer queue and payload.\BR
 * 
 */
EXTERN DJB_PAYLOADQ ajcPayloadQueue							AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 ajcPayloadBuffer[DJB_BUFFER_SIZE]				AP_SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */

EXTERN UInt16 shared_audio_stream_0_crtl									AP_SHARED_SEC_DIAGNOS;                        // Ctrl info specifying 1 out of N capture points for audio stream_0
EXTERN UInt16 shared_audio_stream_1_crtl									AP_SHARED_SEC_DIAGNOS;                    	  // Ctrl info specifying 1 out of N capture points for audio stream_1
EXTERN UInt16 shared_audio_stream_2_crtl            						AP_SHARED_SEC_DIAGNOS;                        // Ctrl info specifying 1 out of N capture points for audio stream_0
EXTERN UInt16 shared_audio_stream_3_crtl            						AP_SHARED_SEC_DIAGNOS;                    	  // Ctrl info specifying 1 out of N capture points for audio stream_1
EXTERN Audio_Logging_Buf_t shared_audio_stream_0[2]                         AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio
EXTERN Audio_Logging_Buf_t shared_audio_stream_1[2]                         AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio
EXTERN Audio_Logging_Buf_t shared_audio_stream_2[2]           				AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio
EXTERN Audio_Logging_Buf_t shared_audio_stream_3[2]           				AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio                                                                          
EXTERN UInt16 shared_usb_status_option				           				AP_SHARED_SEC_DIAGNOS;                        // STATUS_USB_HEADSET_BUFFER ptr option 0 or 1                                                                          

EXTERN UInt16 shared_rhea_audio_test_select			           				AP_SHARED_SEC_DIAGNOS;                        // STATUS_USB_HEADSET_BUFFER ptr option 0 or 1                                                                          

//#ifdef PTT_SUPPORT	
EXTERN Int16 shared_PTT_UL_buffer[2][320]	           				        AP_SHARED_SEC_DIAGNOS;                                                                                                  
//#endif

/** @addtogroup Shared_Audio_Buffers
 * @{ */

#if defined(EXT_MODEM_INTERFACE)
/**
 * @addtogroup External_Modem_Interface
 * @{
 */
/**
 * DDR ping-pong buffer for UL data transfer between Rhea and External modem.
 * This are the ping-pong buffers from which the AADMAC UL channel would take
 * the data from to be transmitted to the External modem.
 */
EXTERN UInt32 shared_ext_ul_buf[2][ NUM_OF_EXT_MODEM_INTERF_SAMP_PER_INT/2]       AP_SHARED_SEC_DIAGNOS;
/**
 * DDR ping-pong buffer for DL data transfer between Rhea and External modem.
 * This are the ping-pong buffers to which the AADMAC DL channel would put
 * the data received from the External modem.
 */
EXTERN UInt32 shared_ext_dl_buf[2][ NUM_OF_EXT_MODEM_INTERF_SAMP_PER_INT/2]       AP_SHARED_SEC_DIAGNOS;
/**
 * @}
 */

#endif
/**
 * @addtogroup ARM2SP_HQ_DL_interface
 *
 * This interface is used for mixing/playing \Bold{48kHz Mono/Stereo PCM data
 * to Downlink}.\BR
 *
 * This PCM data will be mixed after Downlink speech processing in the 48kHz
 * mode.
 *
 * \note This interface works in only one way - from ARM to DSP. \BR
 *
 * One use-case of this interface is to support S-barge-in feature where a
 * music might be playing in the downlink using this interface and an
 * uplink record/speech recognition happens at the same time, then this
 * UL would get the benefit of echo cancellation of the music.
 *
 * In this interface 2 or 4-speech frames worth of data is transferred at a
 * time from ARM to DSP in ping-pong buffers - i.e. ARM fills empty half of
 * shared_Arm2SP_InBuf[].
 *
 * When shared_ARM2SP_HQ_DL_InBuf_out=0, ARM fills next half number of words from
 * shared_Arm2SP_InBuf[half].
 * When shared_ARM2SP_HQ_DL_InBuf_out=half, ARM fills half number of words from
 * shared_Arm2SP_InBuf[0]. \BR
 *
 * To start ARM2SP_HQ_DL: ARM enables ARM2SP_HQ_DL by
 * VP_COMMAND_SET_ARM2SP_HQ_DL.\BR
 *
 * To resume ARM2SP_HQ_DL : ARM enables ARM2SP_HQ_DL by
 * VP_COMMAND_SET_ARM2SP_HQ_DL  with arg1=1.
 *
 * To control shared_Arm2SP_InBuf[]: DSP interrupts ARM with \BR
 * VP_STATUS_ARM2SP_HQ_DL_EMPTY( shared_ARM2SP_HQ_DL_done_flag, \BR
 *                               shared_ARM2SP_HQ_DL_InBuf_out, \BR
 *                               ARM2SP_HQ_DL_flag )\BR
 * whenever finishing every half PCM data. shared_ARM2SP_HQ_DL_InBuf_out
 * indicates the next index DSP will read.
 *
 * To stop ARM2SP_HQ_DL:\BR
 * Quick stop: ARM disables ARM2SP_HQ_DL using
 * VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0).\BR
 * \note The ARM2SP_HQ_DL will be stopped right away.
 * There may be un-finished PCM data in shared_Arm2SP_InBuf[].\BR
 *
 * Finishing stop: After finishing filling the last 2/4-speech frame PCM data,
 * ARM set shared_ARM2SP_HQ_DL_done_flag=1 to indicate the end of PCM data.
 * DSP ISR finishes the last 2/4-speech frames and disables ARM2SP_HQ_DL.
 * DSP sends VP_STATUS_ARM2SP_HQ_DL_EMPTY and arg0=0. \BR
 *
 * To pause ARM2SP_HQ_DL: ARM disables ARM2SP_HQ_DL using
 * VP_COMMAND_SET_ARM2SP_HQ_DL (arg0=0, arg1=1).
 *
 * \see shared_Arm2SP_InBuf, shared_ARM2SP_HQ_DL_InBuf_out,
 *      shared_ARM2SP_HQ_DL_done_flag, VP_COMMAND_SET_ARM2SP_HQ_DL,
 *      VP_STATUS_ARM2SP_HQ_DL_EMPTY,
 *
 * @{ */
/**
 * This flag indicates the end of the transfer of data from the ARM to the
 * DSP. The transfer of data by ARM2SP_HQ_DL interface can be disabled by
 * setting shared_ARM2SP_HQ_DL_done_flag=1 after filling the
 * shared_Arm2SP_InBuf[] buffer.
 * DSP will disable the feature when it reaches the next 1/2-speech frame
 * boundary. */
EXTERN UInt16    shared_ARM2SP_HQ_DL_done_flag                   					AP_SHARED_SEC_DIAGNOS;
/**
 * This index indicates the next index DSP will read from the shared_Arm2SP_InBuf[]
 * buffer.
 *
 * \note This index is changed only by the DSP.
 *
 * */
EXTERN UInt16    shared_ARM2SP_HQ_DL_InBuf_out                   					AP_SHARED_SEC_DIAGNOS;

/**
 * The gain factor, shared_arm2speech_hq_call_gain_dl is applied on the 48kHz PCM data
 * on the downlink path.\BR
 *
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_hq_call_gain_dl[5]               					AP_SHARED_SEC_DIAGNOS;

/** @} */

/**
 * @addtogroup ARM2SP2_HQ_DL_interface
 *
 * This interface is used for mixing/playing \Bold{48kHz Mono/Stereo PCM data
 * to Downlink}.\BR
 *
 * This PCM data will be mixed after Downlink speech processing in the 48kHz
 * mode.
 *
 * \note This interface works in only one way - from ARM to DSP. \BR
 *
 * One use-case of this interface is to support S-barge-in feature where a
 * music might be playing in the downlink using this interface and an
 * uplink record/speech recognition happens at the same time, then this
 * UL would get the benefit of echo cancellation of the music.
 *
 * In this interface 2 or 4-speech frames worth of data is transferred at a
 * time from ARM to DSP in ping-pong buffers - i.e. ARM fills empty half of
 * shared_Arm2SP2_InBuf[].
 *
 * When shared_ARM2SP2_HQ_DL_InBuf_out=0, ARM fills next half number of words from
 * shared_Arm2SP2_InBuf[half].
 * When shared_ARM2SP2_HQ_DL_InBuf_out=half, ARM fills half number of words from
 * shared_Arm2SP2_InBuf[0]. \BR
 *
 * To start ARM2SP2_HQ_DL: ARM enables ARM2SP2_HQ_DL by
 * VP_COMMAND_SET_ARM2SP2_HQ_DL.\BR
 *
 * To resume ARM2SP2_HQ_DL : ARM enables ARM2SP2_HQ_DL by
 * VP_COMMAND_SET_ARM2SP2_HQ_DL  with arg1=1.
 *
 * To control shared_Arm2SP2_InBuf[]: DSP interrupts ARM with \BR
 * VP_STATUS_ARM2SP2_HQ_DL_EMPTY( shared_ARM2SP2_HQ_DL_done_flag, \BR
 *                               shared_ARM2SP2_HQ_DL_InBuf_out, \BR
 *                               ARM2SP2_HQ_DL_flag )\BR
 * whenever finishing every half PCM data. shared_ARM2SP2_HQ_DL_InBuf_out
 * indicates the next index DSP will read.
 *
 * To stop ARM2SP2_HQ_DL:\BR
 * Quick stop: ARM disables ARM2SP2_HQ_DL using
 * VP_COMMAND_SET_ARM2SP2_HQ_DL (arg0=0).\BR
 * \note The ARM2SP2_HQ_DL will be stopped right away.
 * There may be un-finished PCM data in shared_Arm2SP2_InBuf[].\BR
 *
 * Finishing stop: After finishing filling the last 2/4-speech frame PCM data,
 * ARM set shared_ARM2SP2_HQ_DL_done_flag=1 to indicate the end of PCM data.
 * DSP ISR finishes the last 2/4-speech frames and disables ARM2SP2_HQ_DL.
 * DSP sends VP_STATUS_ARM2SP2_HQ_DL_EMPTY and arg0=0. \BR
 *
 * To pause ARM2SP2_HQ_DL: ARM disables ARM2SP2_HQ_DL using
 * VP_COMMAND_SET_ARM2SP2_HQ_DL (arg0=0, arg1=1).
 *
 * \see shared_Arm2SP2_InBuf, shared_ARM2SP2_HQ_DL_InBuf_out,
 *      shared_ARM2SP2_HQ_DL_done_flag, VP_COMMAND_SET_ARM2SP2_HQ_DL,
 *      VP_STATUS_ARM2SP2_HQ_DL_EMPTY,
 *
 * @{ */
/**
 * This flag indicates the end of the transfer of data from the ARM to the
 * DSP. The transfer of data by ARM2SP2_HQ_DL interface can be disabled by
 * setting shared_ARM2SP2_HQ_DL_done_flag=1 after filling the
 * shared_Arm2SP2_InBuf[] buffer.
 * DSP will disable the feature when it reaches the next 1/2-speech frame
 * boundary. */
EXTERN UInt16    shared_ARM2SP2_HQ_DL_done_flag                   					AP_SHARED_SEC_DIAGNOS;
/**
 * This index indicates the next index DSP will read from the shared_Arm2SP2_InBuf[]
 * buffer.
 *
 * \note This index is changed only by the DSP.
 *
 * */
EXTERN UInt16    shared_ARM2SP2_HQ_DL_InBuf_out                   					AP_SHARED_SEC_DIAGNOS;

/**
 * The gain factor, shared_arm2speech2_hq_call_gain_dl is applied on the 48kHz PCM data
 * on the downlink path.\BR
 *
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_hq_call_gain_dl[5]               					AP_SHARED_SEC_DIAGNOS;

/** @} */
/**
 * @}
 */

EXTERN UInt16 shared_dsp_supported_features AP_SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_ap_supported_features AP_SHARED_SEC_DIAGNOS;

EXTERN UInt32 NOT_USE_shared_memory_end AP_SHARED_SEC_DIAGNOS;

#ifdef MSP
} AP_SharedMem_t;
#endif

#ifdef MSP
struct Dsp_AP_ScratchMem_t {
#endif

EXTERN UInt32 scr_ram_Arm2SP_HQ_DL_InBuf[NUM_OF_48K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;	/* 2ms stereo music frames of 16-bit samples */

/**
 * @addtogroup HANDSET_AADMAC_BUFFERS
 *
 * This interface is used for receiving 8/16 KHz input data from the AADMAC
 * which then passed over to the regular hand-set.
 *
 * \see
 * @{
 */
/**
 * These buffers are used for getting mono data for primary mic from AADMAC.
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low
 * buffer followed by
 * the high buffer.
 */
EXTERN UInt32 scr_ram_aadmac_pri_mic_low[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;
EXTERN UInt32 scr_ram_aadmac_pri_mic_high[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;

/**
 * These buffers are used for getting mono data for primary mic from AADMAC.
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low
 * buffer followed by
 * the high buffer.
 */
EXTERN UInt32 scr_ram_aadmac_sec_mic_low[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;
EXTERN UInt32 scr_ram_aadmac_sec_mic_high[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;

/**
 * These buffers are used for sending mono data for speaker from AADMAC (all
 * paths other than IHF).
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low
 * buffer followed by
 * the high buffer.
 */
EXTERN UInt32 scr_ram_aadmac_spkr_low[NUM_OF_48K_SAMP_PER_INT0_INT]
	DSP_AP_SCRATCHMEM_SEC;
EXTERN UInt32 scr_ram_aadmac_spkr_high[NUM_OF_48K_SAMP_PER_INT0_INT]
	DSP_AP_SCRATCHMEM_SEC;
/**
 * These buffers are used for getting mono data for echo reference from AADMAC.
 *
 * \see
 *
 * \note The order of the buffers below is important. First should be the low
 * buffer followed by
 * the high buffer.
 */
EXTERN UInt32 scr_ram_aadmac_echo_ref_low[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;
EXTERN UInt32 scr_ram_aadmac_echo_ref_high[NUM_OF_8K_SAMP_PER_INT0_INT*2]
	DSP_AP_SCRATCHMEM_SEC;
/**
 * This buffers is used for sending mono/stereo data for secondary speaker
 * from AADMAC.
 *
 * \see
 *
 * \note Only the low half of the AADMAC buffer is used. The size of the low
 * half is doubled as the data in it can be stereo.
 */
EXTERN UInt32 scr_ram_aadmac_sec_spkr_low[2*NUM_OF_48K_SAMP_PER_INT0_INT]
	DSP_AP_SCRATCHMEM_SEC;

/**
 * @}
 */

#ifdef MSP
};
#endif

/******************************************************************************
* Function Prototypes
******************************************************************************/

#ifdef RIP
void VPSHARED_Init( void );              // Initialize the shared memory
#endif    // RIP



/**
 * @}
 */

#endif	// _INC_SHARED_AP_H_


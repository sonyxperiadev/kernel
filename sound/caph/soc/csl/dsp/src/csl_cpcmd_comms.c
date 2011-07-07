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
//#define ENABLE_LOGGING
 
#include "mobcom_types.h"
#include "chip_version.h"
#include "dsp_public_cp.h"
#include "dsp_feature_def.h"
#include "assert.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "ripcmdq.h"
#include "sysparm.h"
#include "memmap.h"
#include "mti_trace.h"
#include "data.h"
#include "log.h"
#include "l1misc.h" //MobC00104879
#include "celldata.h"

// #include "audioapi_asic.h"  // Coverity clean up - unnecessary header
#include "mphproc.h"

#ifdef DM_SINGLE_AFC_CTRL
#include "l1ud_afc.h"
#endif //DM_SINGLE_AFC_CTRL

#define	AMCR_8K_MODE			0
 
Boolean is_TCHLOOP_C_active = FALSE;


static UInt16   dsp_scell_id = SHARED_INVALID_CELL;
static UInt16 	pre_dsp_scell_id = SHARED_INVALID_CELL;
Int16 	dsp_new_otd[CELL_DATA_SIZE];
Int8 	dsp_fn_adj[CELL_DATA_SIZE];
static  Boolean set_invalid_dsp_scell = FALSE;

//******************************************************************************
// Local Function Prototypes
//******************************************************************************
static void SetTracking(				// Set tracking parameters
	TrackParms_t *parms					// Tracking parameters
	);   

//******************************************************************************
//
// Function Name:	RIPCMDQ_StartFrames
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StartFrames()
{
	CmdQ_t msg;

	msg.cmd = COMMAND_START_FRAMES;
	// Default is to use 40 SMC per frame
	msg.arg0 = 0x4001;	
	msg.arg1 = 1;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_StopFrames
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StopFrames()
{
	CmdQ_t msg;

	msg.cmd = COMMAND_STOP_FRAMES;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_Cell4Bit
//
// Description: Enable or disable 4bit cell id.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_Cell4BitID(		// enable/disable 4 bit cell id
	Boolean set4bit						// False = 4 bit id disabled, else 4 bit ID is enabled
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_SET_4BIT_CELLID;
	msg.arg0 = set4bit;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetSCellSlot
//
// Description: Sets the SCELL slot value.
//
// Notes:
//
//******************************************************************************

extern UInt16 update_null_pg_blk_last_blk_ind;
extern void FRAMEUTIL_SetSCellSlot(UInt16	cell_id, UInt16 slot, UInt8 bandSharedMem);

static Int16 short_frame_offset = -937;

Boolean RIPCMDQ_IsSetInvalidSCell (void)
{
	return (set_invalid_dsp_scell == TRUE);
}

void RIPCMDQ_ClearSetInvalidSCellFlag (void)
{
	set_invalid_dsp_scell = FALSE;
}


void RIPCMDQ_SetSCellSlot(
	UInt16	cell_id,
	UInt16 slot,
	Boolean dtx_enable,
	UInt8 bandSharedMem
	)
{
	CmdQ_t msg;
	pre_dsp_scell_id = dsp_scell_id;
// This will skip NULL_PAGE logic for next paging block
	update_null_pg_blk_last_blk_ind = VALID_PAGING_BLOCK;

	if ( cell_id == SHARED_INVALID_CELL )
		set_invalid_dsp_scell = TRUE;
	else if ( set_invalid_dsp_scell == TRUE )	// new scell change before invalid scell set
		set_invalid_dsp_scell = FALSE;
	
//[GNATS TR762] DSP report is not complete, update fn_adj and otd of ncell based on L1 calculation instead of DSP report
	{
		Int16 otd_adj = 0, old_ncell_otd, new_ncell_offset;
		UInt8 id;
		//calculate the qbc adjustment for the new scell-slot
		if( cell_id != SHARED_INVALID_CELL)
		{
			dsp_new_otd[cell_id] = 0;
			dsp_fn_adj[cell_id] = 0;
		}
		if(( cell_id != SHARED_INVALID_CELL) && ( dsp_scell_id != SHARED_INVALID_CELL))
		{
			if(CELLDATA_IsSync(cell_id) && CELLDATA_IsSync(dsp_scell_id))
			{
				otd_adj = CELLDATA_GetOTD(cell_id);
				L1_LOGV3("NcellDbg: SetSCellSlot: scell_otd,cell_id,slot", otd_adj,cell_id,slot); 
				for(id = 0; id<CELL_DATA_SIZE;++id)
				{
					if(id == cell_id)
						continue;
					if(CELLDATA_IsSync(id))
					{
						old_ncell_otd = CELLDATA_GetOTD(id);
						dsp_new_otd[id] = old_ncell_otd - otd_adj;
						dsp_fn_adj[id] = 0;
						new_ncell_offset = dsp_new_otd[id] * 2 - slot * 625;
						while(new_ncell_offset < short_frame_offset)
						{
							new_ncell_offset += 5000;
							dsp_new_otd[id] += 2500;
							dsp_fn_adj[id] ++;
						}
						while(new_ncell_offset > (4999+short_frame_offset))
						{
							new_ncell_offset -= 5000;
							dsp_new_otd[id] -= 2500;
							dsp_fn_adj[id] --;
						}
						L1_LOGV4("NcellDbg: SetSCellSlot: id, otd_ncell: old/new, dsp_fn_adj", 
								id, old_ncell_otd, dsp_new_otd[id], dsp_fn_adj[id]); 
					}
				}
			}
#ifdef AGPS_FTT_SUPPORT //MobC00104879
                L1_Ftt_SetScellSlot(cell_id, slot);
#endif //MobC00104879
		}
	}
	dsp_scell_id = cell_id;	
	CELLDATA_SetDSPServingSIM (dsp_scell_id != SHARED_INVALID_CELL);

#ifdef SET_SCELL_IN_FRAMEENTRY
	if (( cell_id != SHARED_INVALID_CELL )&&( pre_dsp_scell_id !=SHARED_INVALID_CELL))
	{
		FRAMEUTIL_SetSCellSlot(cell_id, slot, bandSharedMem);	
//		return;
	}

	// if cell_id == SHARED_INVALID_CELL, use traditional cmdq by executing below.
	
#endif
	if ( set_invalid_dsp_scell == FALSE )		// delay invalid DSP serving cell setting to next idle frame in rfic
	{
		msg.cmd = COMMAND_SET_IDLE_MODE;
		msg.arg0 = cell_id;
		msg.arg1 = slot;
		msg.arg2 = ( dtx_enable ) | ( bandSharedMem << 4 );

		SHAREDMEM_PostCmdQ( &msg );
	}
	else
		L1_LOG("Set invalid dsp scell - delay to next idle");
}
//******************************************************************************
//
// Function Name:	RIPCMDQ_GetDSP_SCell
//
// Description: 
//
// Notes:
//
//******************************************************************************

UInt16 RIPCMDQ_GetDSP_SCell( void)
{
	return dsp_scell_id;
}

UInt16 RIPCMDQ_GetPreDSP_SCell( void)
{
	return pre_dsp_scell_id;
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetKc
//
// Description: Sets the value of Kc.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetKc(
	Boolean A5_3_flag,
	UInt8 *kc
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_SET_KC_HI;
	msg.arg2 = 0;
	if (A5_3_flag)
	{
	    msg.arg0 = kc[ 0 ] << 8 | kc[ 1 ];
		msg.arg1 = kc[ 2 ] << 8 | kc[ 3 ];
	}
    else
    {
	    msg.arg0 = kc[ 6 ] << 8 | kc[ 7 ];
	    msg.arg1 = kc[ 4 ] << 8 | kc[ 5 ];
    }

	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_SET_KC_LO;
	if (A5_3_flag)
	{
		msg.arg0 = kc[ 4 ] << 8 | kc[ 5 ];
		msg.arg1 = kc[ 6 ] << 8 | kc[ 7 ];
	}
	else
	{
		msg.arg0 = kc[ 2 ] << 8 | kc[ 3 ];
		msg.arg1 = kc[ 0 ] << 8 | kc[ 1 ];
	}
    
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_CloseTCHLoop
//
// Description:		Request the RIP to close its TCH loop.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_CloseTCHLoop(
	UInt8 subchan
	)
{
	CmdQ_t msg;


	msg.cmd = COMMAND_AUDIO_ENABLE;
	msg.arg0 = 1;
	msg.arg1 = AMCR_8K_MODE;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_CLOSE_TCH_LOOP;
	msg.arg0 = subchan;
	msg.arg1 = 0;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ( &msg );

    {
		UInt16 submode;
		
	    submode = subchan >> 3;
		if ( submode == 0 )
		{
	        submode = ( subchan >> 1 ) & 0x03;
			if ( submode > 1 )
			{// TCHLOOP_C disable SAIC;
				SharedMem_t *mem = (SharedMem_t *)(SHAREDMEM_GetSharedMemPtr());
				mem->shared_flag_SAIC = (MPHPROC_GetSAICFlag()) & 0x0101;	//disable SAIC frame process in loop C
                is_TCHLOOP_C_active = TRUE;
			}
		}
	}
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_OpenTCHLoop
//
// Description:		Request the RIP to open its TCH loop.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_OpenTCHLoop()
{
	CmdQ_t msg;

	msg.cmd = COMMAND_OPEN_TCH_LOOP;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
	// Recover the SAIC flag from disable (due to TCHLOOP_C) to sysparm default
	if ( is_TCHLOOP_C_active )
    {
		SharedMem_t *mem = (SharedMem_t *)(SHAREDMEM_GetSharedMemPtr());
        mem->shared_flag_SAIC = MPHPROC_GetSAICFlag();
		is_TCHLOOP_C_active = FALSE;
	}
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_CloseTCHLoop
//
// Description:		Request the RIP to close its TCH loop.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_CloseMSLoop(
	UInt16 loop_mode,
	UInt16 rxtx0,
	UInt16 rxtx1
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_CLOSE_MS_LOOP;
	msg.arg0 = loop_mode;
	msg.arg1 = rxtx0;
	msg.arg2 = rxtx1;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_OpenMSLoop
//
// Description:		Request the RIP to open its TCH loop.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_OpenMSLoop()
{
	CmdQ_t msg;

	msg.cmd = COMMAND_OPEN_MS_LOOP;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}
#ifdef DSP_FAST_COMMAND
//******************************************************************************
//
// Function Name:	RIPCMDQ_FastTxFull
//
// Description:		Notify the RIP that a Tx buffer has been filled.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FastTxFull(
	UInt8 index,
	TxOpt_t option,
	UInt16 delay
	)
{
	CmdQ_t msg;

	msg.cmd = FAST_COMMAND_TX_FULL;
	msg.arg0 = index;
	msg.arg1 = delay;
	msg.arg2 = option;

	SHAREDMEM_PostFastCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_Fast_FlushBufSimple
//
// Description:		Send a command to the RIP to flush its message buffers.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_Fast_FlushBufSimple( )
{
	CmdQ_t msg;

	L1_LOG("FAST_COMMAND_FLUSH_BUFFERS");
	msg.cmd = FAST_COMMAND_FLUSH_BUFFERS;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostFastCmdQ( &msg );
}
#endif// DSP_FAST_COMMAND

//******************************************************************************
//
// Function Name:	RIPCMDQ_SyncCmd
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SyncCmd(
	UInt16 sync_val
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_SYNC;
	msg.arg0 = sync_val;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_TxFull
//
// Description:		Notify the RIP that a Tx buffer has been filled.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_TxFull(
	UInt8 index,
	TxOpt_t option,
	UInt16 delay
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_TX_FULL;
	msg.arg0 = index;
	msg.arg1 = delay;
	msg.arg2 = option;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_RxEmpty
//
// Description:		Notify the RIP that an Rx buffer has been emptied.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_RxEmpty(
	UInt8 index
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_RX_EMPTY;
	msg.arg0 = index;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_FlushBuffers
//
// Description:		Send a command to the RIP to flush its message buffers.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FlushBuffers()
{

	CmdQ_t	msg;

	msg.cmd = COMMAND_FLUSH_BUFFERS;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );

	// Send a command to the RIP to flush FACCH/RATSCCH message buffers.

	msg.cmd = COMMAND_FLUSH_FACCH_RATSCCH;
	msg.arg0 = TXCODE_FACCH;
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_FLUSH_FACCH_RATSCCH;
	msg.arg0 = TXCODE_RATSCCH;
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_FlushBufSimple
//
// Description:		Send a command to the RIP to flush its message buffers.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FlushBufSimple( )
{
	CmdQ_t msg;

	L1_LOG("COMMAND_FLUSH_BUFFERS");
	msg.cmd = COMMAND_FLUSH_BUFFERS;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_FlushFACCH_RATSCCH
//
// Description:		Send a command to the RIP to flush its pending or partially sent FACCH/RATSCCH.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FlushFACCH_RATSCCH()
{
	CmdQ_t	msg;

	// Send a command to the RIP to flush FACCH/RATSCCH.

	msg.cmd = COMMAND_FLUSH_FACCH_RATSCCH;
	msg.arg0 = TXCODE_FACCH;
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_FLUSH_FACCH_RATSCCH;
	msg.arg0 = TXCODE_RATSCCH;
	SHAREDMEM_PostCmdQ( &msg );
}

   
//******************************************************************************
//
// Function Name:	RIPCMDQ_CLearAMRFlag
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_CLearAMRFlag( void	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_AMR_CLEAR_FLAG;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);

}


//******************************************************************************
//
// Function Name:	RIPCMDQ_Set_Was_Speech_Idle
//
// Description:		Set COMMAND_SET_WAS_SPEECH_IDLE
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_Set_Was_Speech_Idle( 
	void
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_SET_WAS_SPEECH_IDLE;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ(&msg);
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_SetTrackingIdle
//
// Description: Set idle mode tracking parameters.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetTrackingIdle()			// Set idle mode tracking parameters
{

	RIPCMDQ_Set_Was_Speech_Idle();		// report to DSP that speech idle mode has been reached
	L1_LOG("Idle track parm set"); 
	SetTracking( SYSPARM_GetIDLE() );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetTrackingDch
//
// Description: Set DCH mode tracking parameters.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetTrackingDch()			// Set DCH mode tracking parameters
{
	 
	SetTracking( SYSPARM_GetSDCCH() );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetTrackingTch
//
// Description: Set TCH mode tracking parameters.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetTrackingTch()			// Set TCH mode tracking parameters
{
	 
	SetTracking( SYSPARM_GetTCH() );
}
//******************************************************************************
//
// Function Name:	RIPCMDQ_SetTrackingTBF
//
// Description: Set TBF mode tracking parameters.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetTrackingTBF()			// Set TCH mode tracking parameters
{
	L1_LOG("TBF track parm set");	 
	SetTracking( SYSPARM_GetTBF() );
}
//******************************************************************************
//
// Function Name:	RIPCMDQ_SetTrackingTBF1
//
// Description: Set TBF mode tracking parameters.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetTrackingTBF1()			// Set TCH mode tracking parameters
{
	CmdQ_t 	msg;
	TrackParms_t *parms;							// Tracking parameters

	// Use TCH parms for TBF1 tracking
	
	parms = SYSPARM_GetTCH();

	msg.cmd = COMMAND_SCELL_TIME_TRACK;
	msg.arg0 = parms->scell_time.num;	// Number of samples to average
	msg.arg1 = parms->scell_time.shift;	// # of bits to shift (for average)
	msg.arg2 = parms->scell_time.mag;	// Update magnitude limit (QBC)
	
//	L1_LOGV3("TBF track parm set 1", msg.arg0, msg.arg1, msg.arg2);
	SHAREDMEM_PostCmdQ(&msg);	
}
//******************************************************************************
//
// Function Name:	SetTracking
//
// Description:		Set tracking parameters
//
// Notes:
//
//******************************************************************************

static void SetTracking(				// Set tracking parameters
	TrackParms_t *parms					// Tracking parameters
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_SCELL_TIME_TRACK;
	msg.arg0 = parms->scell_time.num;	// Number of samples to average
	msg.arg1 = parms->scell_time.shift;	// # of bits to shift (for average)
	msg.arg2 = parms->scell_time.mag;	// Update magnitude limit (QBC)

	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_NCELL_TIME_TRACK;
	msg.arg0 = parms->ncell_time.num;	// Number of samples to average
	msg.arg1 = parms->ncell_time.shift;	// # of bits to shift (for average)
	msg.arg2 = parms->ncell_time.mag;	// Update magnitude limit (QBC)

	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_SCH_TIME_TRACK;
	msg.arg0 = parms->sch_time_mag;		// Update magnitude limit (QBC)
	msg.arg1 = parms->scell_error_mag;
	msg.arg2 = parms->scell_accum2_shift;

	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_FREQ_TRACK1;
	msg.arg0 = parms->scell_freq.m1;	// Gain K1 Multiplier
	msg.arg1 = parms->scell_freq.s1;	// Gain K1 Shift
	msg.arg2 = parms->scell_leak_thresh;

	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_FREQ_TRACK2;
	msg.arg0 = parms->scell_freq.m2;	// Gain K2 Multiplier
	msg.arg1 = parms->scell_freq.s2;	// Gain K2 Shift
	msg.arg2 = parms->scell_freq.avg_flag;	// Enable Averaging

	SHAREDMEM_PostCmdQ(&msg);
}


//******************************************************************************
//
// Function Name:	SetTracking_HR
//
// Description:		Set tracking parameters for HR
//
// Notes:
//
//******************************************************************************
static void SetTracking_HR(				// Set tracking parameters
	TrackParms_t *parms					// Tracking parameters
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_SCELL_TIME_TRACK1_HR;
	msg.arg0 = parms->scell_time.num;	// Number of samples to average
	msg.arg1 = parms->scell_time.shift;	// # of bits to shift (for average)
	msg.arg2 = parms->scell_time.mag;	// Update magnitude limit (QBC)

	SHAREDMEM_PostCmdQ(&msg);


	msg.cmd = COMMAND_SCELL_TIME_TRACK2_HR;
	msg.arg0 = parms->scell_error_mag;
	msg.arg1 = parms->scell_accum2_shift;
	msg.arg2 = parms->scell_leak_thresh;

	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_FREQ_TRACK1_HR;
	msg.arg0 = parms->scell_freq.m1;	// Gain K1 Multiplier
	msg.arg1 = parms->scell_freq.s1;	// Gain K1 Shift
////	msg.arg2 = parms->scell_leak_thresh;

	SHAREDMEM_PostCmdQ(&msg);

	msg.cmd = COMMAND_FREQ_TRACK2_HR;
	msg.arg0 = parms->scell_freq.m2;	// Gain K2 Multiplier
	msg.arg1 = parms->scell_freq.s2;	// Gain K2 Shift
	msg.arg2 = parms->scell_freq.avg_flag;	// Enable Averaging

	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetTrackingTch_h
//
// Description: Set TCH_H mode tracking parameters.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetTrackingTch_h()			// Set TCH mode tracking parameters
{
	 
	SetTracking_HR( SYSPARM_GetTCH_H() );
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_SetGPIOActiveFrame
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetGPIOActiveFrame(		// Enable active frame GPIO
	UInt16 addr,						// GPWR address
	UInt16 value,						// Active frame value
	UInt16 mask							// Active frame mask
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_GPIO_FRAME_ACTIVE;
	msg.arg0 = addr;
	msg.arg1 = value;
	msg.arg2 = mask;

	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_EnableDTX
//
// Description:		Enable DTX mode
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_EnableDTX(					// Enable DTX mode
	Boolean enable						// TRUE, enable DTX
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_DTX_ENABLE;
	msg.arg0 = enable;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);

	DATA_EnableDTX( enable );			// Make DTX state accessible to data.c

	L1_LOGV("DTX enable=",enable);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_ResetAFCDAC
//
// Description:		Reset AFC DAC.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_ResetAFCDAC(UInt16 AFCAdjed) // Reset AFC DAC
{
	CmdQ_t	msg;

	SYSPARM_SetAFCDAC( AFCAdjed );
	L1_LOGV("RIPCMDQ_ResetAFCDAC", AFCAdjed);

	msg.cmd = COMMAND_RESET_AFC_DAC;

	msg.arg0 = 0;
	msg.arg1 = 3;	//update FREQ_DAC_OFFSET as well as FREQ_DONE_FLAG
	msg.arg2 = FALSE;
#ifndef L1TEST
#ifdef DM_SINGLE_AFC_CTRL
	l1ud_afc_set_gsm_dac_value ( AFCAdjed );
	l1ud_afc_apply_gsm_dac ();
#endif //#ifdef DM_SINGLE_AFC_CTRL
#endif //L1TEST
	SHAREDMEM_PostCmdQ(&msg);

}

void RIPCMDQ_AdjustAFCDAC(Int16 freq_off_900)				
{

	CmdQ_t	msg;

	msg.cmd = COMMAND_RESET_AFC_DAC;
	msg.arg0 = (UInt16)freq_off_900;
	msg.arg1 = 1;	//update FREQ_DAC_OFFSET ONLY
	msg.arg2 = 0;
	L1_LOGV("RIPCMDQ_AdjustAFCDAC", freq_off_900);
	SHAREDMEM_PostCmdQ(&msg);
}

void RIPCMDQ_SetFreqDone(void)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_RESET_AFC_DAC;
	msg.arg0 = 0;		// to make coverity happy 

	msg.arg1 = 2;	//update FREQ_DONE_FLAG ONLY
	msg.arg2 = TRUE;

	L1_LOG("RIPCMDQ_SetFreqDone");
	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_AbortTx
//
// Description:		Clear transmit burst buffer
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_AbortTx(					// Clear transmit burst buffer
	UInt16 index						// Transmit buffer to clear
	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_TX_ABORT;
	msg.arg0 = index;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:  RIPCMDQ_SetGPRSPwrCtrlIdle
//
// Description:  This function sets power control mode as idle.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetGPRSPwrCtrlIdle(
	Boolean reset_cn,
	UInt16	forget_factor,		// MIN(n, MAX(5, T_avg_w/Tdrx))
	UInt8	Pb_db
	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_PWRCTRL_IDLE;
	msg.arg0 = reset_cn;
	msg.arg1 = forget_factor;
	msg.arg2 = Pb_db;

	SHAREDMEM_PostCmdQ(&msg);
}	

//******************************************************************************
//
// Function Name:  RIPCMDQ_SetGPRSPwrCtrlTx
//
// Description:  This function posts a command in the command queue to set
// TX power control
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetGPRSPwrCtrlTx(
	Boolean use_bcch,
	UInt16	forget_factor		// SPEC 5.08 10.2.3
	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_PWRCTRL_TXFER;
	msg.arg0 = use_bcch;
	msg.arg1 = forget_factor;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ(&msg);
}	

//******************************************************************************
//
// Function Name:  RIPCMDQ_SetGPRSAlpha
//
// Description:  This function sets the value of alpha for power control.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetGPRSAlpha(
	UInt8	alpha
	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_PWRCTRL_ALPHA;
	msg.arg0 = alpha;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}	

//******************************************************************************
//
// Function Name:  RIPCMDQ_SetGPRSPmax
//
// Description:  This function sets Pmax for power control.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetGPRSPmax(
	UInt8	Pmax
	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_PWRCTRL_PMAX;
	msg.arg0 = Pmax;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}	

//******************************************************************************
//
// Function Name:  RIPCMDQ_SetGPRSGamma
//
// Description:  This function sets the value of Gamma for power control.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetGPRSGamma(
	UInt8	slot_nu,
	UInt8	gamma
	)
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_PWRCTRL_GAMMA;
	msg.arg0 = slot_nu;
	msg.arg1 = gamma;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}	

//******************************************************************************
//
// Function Name:  RIPCMDQ_SetGPRSPwrCtrlStart
//
// Description:  This function asks OAK to start using the new power control 
//	 			parameter sent before.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetGPRSPwrCtrlStart( void )
{
	CmdQ_t	msg;

	msg.cmd = COMMAND_PWRCTRL_LOAD;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}	

//******************************************************************************
//
// Function Name:	ForceRIPUpdate
//
// Description:		Force RIP update of tx parameters
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_ForceRIPUpdate()			// Force RIP update of tx parameters
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_RELOAD_TX_PARMS;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}



//******************************************************************************
//
// Function Name:	SetAMRParms
//
// Description:		Set AMR parameters
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetAMRParms( 
	UInt16 icm,
	UInt16 acs,
	UInt16 direction
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_SET_AMR_PARMS;
	msg.arg0 = icm;
	msg.arg1 = acs;
	msg.arg2 = direction;

	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	Set Noise suppression
//
// Description:		Set COMMAND_NOISE_SUPPRESSION
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetNoiseSuppress( 
	UInt16 noise_sup
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_NOISE_SUPPRESSION;
	msg.arg0 = noise_sup;
	msg.arg1 = 0;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ(&msg);
	// Andy: noise.c/sigproc.s not in build?? add to makefile. Ronish said BCM2121 can support NS
#ifndef L1TEST
	if ( noise_sup )
	{
		// Initialise the adaptive noise suppresser control module in the ARM 
		// whenever the noise suppresser is enabled in the DSP
		//NOISE_AdapInit();
	}
#endif
}




//******************************************************************************
//
// Function Name:	Force the DSP to update its NULL paging block template
//
// Description:		Set COMMAND_UPDATE_NULL_PAGE_TEMPLATE
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_ForceNullPgTemplateUpdate( UInt8 index )
{

	CmdQ_t 	msg;

	msg.cmd = COMMAND_UPDATE_NULL_PAGE_TEMPLATE;
	msg.arg0 = index;
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	void RIPCMDQ_SetNullPage( Boolean enable)
//
// Description:		When NULL paging block logic turned on, we need different track algorithm
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetNullPage( Boolean enable)
{
	if(enable)
		SetTracking( SYSPARM_GetNULLPAGE() );
	else
		SetTracking( SYSPARM_GetIDLE() );
}

#ifdef	ARM_HS_VOCODER
//******************************************************************************
//
// Function Name:	RIPCMDQ_HS_VOCODER_Done
//
// Description:		Notify DSP to get data because arm HS vocoder is done.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_HS_VOCODER_Done( 
	UInt16 parm
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_HS_VOCODER_DONE;
	msg.arg0 = parm;
	msg.arg1 = 0;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ(&msg);
}
#endif





void RIPCMDQ_WakeupDSP(UInt16 threshold, UInt16 fcrnew)
{
	CmdQ_t msg;
   
	msg.cmd = COMMAND_WAKEUP_SMC;
// CQ MobC00108284, Andy Shen, 04/29/10
// Wait 2 slow clk, value based on DSP clk @ 208 MHz
	msg.arg0 = 13000;
// CQ MobC00108284, Andy Shen, 04/29/10
	msg.arg1 = threshold;   
	msg.arg2 = fcrnew;   
	SHAREDMEM_PostCmdQ( &msg );
}
//******************************************************************************
//
// Function Name:	Enable/Disable Enhanced Measurement
//
// Description:		Set COMMAND_ENHANCED_MEAS_ENABLE
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_EnhMeasEnable( 
	UInt16 enable
	)
{

	CmdQ_t 	msg;

	msg.cmd = COMMAND_ENHANCED_MEAS_ENABLE;
	msg.arg0 = enable;
	msg.arg1 = 0;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ(&msg);
}

#ifdef EPC
//******************************************************************************
//
// Function Name:	RIPCMDQ_EPC
//
// Description:		Set EPC Mode
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_EPC( UInt16 mode, UInt16 flag )
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_EPC;
	msg.arg0 = mode;
	msg.arg1 = flag;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);

	L1_LOGV("Command EPC mode:flag",(mode<<16)|flag);
}
#endif


//******************************************************************************
//
// Function Name:	RIPCMDQ_AdjScellTimeOffset
//
// Description:		Adjust the frame timing by QBC passed in offset
//
// Notes:			
//******************************************************************************
void RIPCMDQ_AdjScellTimeOffset( Int16 offset )
{
	CmdQ_t msg;

	msg.cmd = COMMAND_ADJ_SCELL_TIME_OFFSET;
	msg.arg0 = offset;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}



#ifdef DSP_FEATURE_RFAPI
//******************************************************************************
//
// Function Name:	RIPCMDQ_EnableRFRxTest
//
// Description:		Activate RF_RX_TEST
//
// Notes:			
//******************************************************************************
void RIPCMDQ_EnableRFRxTest( UInt16 mode )
{

#if ( CHIPVERSION == CHIP_VERSION(BCM213x1j, 30) )		
	// work with Juno A0 patch 
	Unpaged_SharedMem_t *unpaged_mem = SHAREDMEM_GetUnpagedSharedMemPtr();
	unpaged_mem->shared_unused_unpaged[0] = mode;
#else

	CmdQ_t msg;

	msg.cmd = COMMAND_ENABLE_RF_RX_TEST;
	msg.arg0 = mode;
	msg.arg1 = 0;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ( &msg );
#endif
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_EnableRFTxTest
//
// Description:		Activate RF_TX_TEST
//
// Notes:			
//******************************************************************************
void RIPCMDQ_EnableRFTxTest( UInt16 mode )
{
#if ( CHIPVERSION == CHIP_VERSION(BCM213x1j, 30) )		
	// work with Juno A0 patch 
	Unpaged_SharedMem_t *unpaged_mem = SHAREDMEM_GetUnpagedSharedMemPtr();
	unpaged_mem->shared_unused_unpaged[1] = mode;
#else

	CmdQ_t msg;

	msg.cmd = COMMAND_ENABLE_RF_TX_TEST;
	msg.arg0 = mode;
	msg.arg1 = 0;
	msg.arg2 = 0;
						
	SHAREDMEM_PostCmdQ( &msg );
#endif
}
#endif


void RIPCMDQ_AbortSearch(UInt16 abort_slot,  UInt16 abort_frame)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_SRCH_ABORT; 			
	
	msg.arg0 = abort_slot;
	msg.arg1 = abort_frame;
	msg.arg2 = 0;
						
	SHAREDMEM_PostCmdQ( &msg );
}


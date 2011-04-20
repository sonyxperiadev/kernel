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
#include "mobcom_types.h"
#include "chip_version.h"
#include "ms.h"
#include "dsp_feature_def.h"
#include "consts.h"
#include "msconsts.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "ripisr.h"
#include "ripproc.h"
#include "osinterrupt.h"
#include "entrydata.h"
#include "ripbuf.h"
#include "sysparm.h"
#include "gprsdata.h"
#include "data.h"
#include "xassert.h"
#include "phyframe.h"
#include "log.h"
#include "linkctrl.h"
#include "phproc.h"
#include "phprim.h"
#include "dsp_public_cp.h"

#define ENABLE_LOGGING
#include "mti_trace.h"
#include "sio.h"
// #include "dbg.h"  // Coverity clean up - unnecessary header

#include "amrproc.h"
#if (defined UMTS)
#include "l1c_olpc_interface.h"	     // MST
#endif

// redefined JTAG_FRIENDLY_L1_ASSERT to return zero, compiler warning cleanup
#define JTAG_FRIENDLY_DSP_ASSERT( expr, value ) { jtag_friendly_l1_assert( (expr), #expr,  __FILE__, __LINE__, value  );\
												 if (!(expr)) return 0; } 



#include "ripout.h"
#include "ctm_proc.h"
#include "ripcmdq.h"
#include "dspcmd.h"

// #include "rficisr.h"  // Coverity clean up - unnecessary header

Boolean ignor_RATSCCH = FALSE;
static Boolean disable_link_est = FALSE;

extern Boolean wait_for_prach_sent;
extern UInt8	prach_sent_time;



//#define MST_TEST
//#define MST_LAB_TEST



#ifdef MST_TEST
UInt16 total_fr_cnt =0;
UInt16 err_fr_cnt   =0;
#endif

#if defined(DEVELOPMENT_ONLY)
static UInt16 meas_report_period_cnt=0;
#endif

UInt16 TCHLoop_type;



extern UInt16 Enable_dsp_assertion_4_WARNING_STATUS_MSG;




//******************************************************************************
// Local Function Prototypes
//******************************************************************************


//******************************************************************************
//
// Function Name:	RIPISR_HISR
//
// Description:		This function is the RIP interrupt service routine.
//
// Notes:
//
//******************************************************************************

#ifndef _NODEBUG	// 050802 SYY
extern StatQ_t		stat_q_last_frame_int;
extern FrameInt_t	frame_int_last;
#endif

static void RIPISR_HISR_Comms(void)
{
	RIPPROC_ProcessRFIC_HISR();
}

static Boolean RIPISR_ProcessMsg_Comms(StatQ_t* 	p_status_msg)
{
	FrameInt_t	frame_int;
	Boolean msg_processed = TRUE;
	StatQ_t status_msg = *p_status_msg;
		switch ( status_msg.status )
		{
			case STATUS_VERSION_ID :

				RIPPROC_SetVersion( status_msg.arg0, status_msg.arg1, status_msg.arg2 );
				PHYFRAME_DSPSuspendAck( STATUS_VERSION_ID, 0 );

				break;
			case STATUS_FRAME_INT :
				frame_int = ENTRYDATA_GetInt( (UInt8)status_msg.arg0 );

				#ifndef _NODEBUG	// 050802 SYY
				stat_q_last_frame_int = status_msg;
				frame_int_last = frame_int;
				#endif
#ifdef VMF_INCLUDE
				if(PHYEvent_IsNonIdleMode())
					VMF_switchto_hisr_context((T_VM_ID)PHYEvent_GetCurGl1Sim()+1);
#endif
				if( frame_int & FRAMEINT_NEW_CHAN )
				{
					//Only if DATA_UpdateCfg gets called, DATA_IsDataServiceOn may return TRUE.
					DATA_UpdateCfg();
				}
				if(frame_int & FRAMEINT_EXTEND)
				{
					if( !PHYEvent_IsNonIdleMode())
					{
						// redefined JTAG_FRIENDLY_L1_ASSERT to return zero, compiler warning cleanup
						JTAG_FRIENDLY_DSP_ASSERT( ENTRYDATA_GetLastExtendEntry() == (UInt8)status_msg.arg0, (UInt8)status_msg.arg0|(ENTRYDATA_GetLastExtendEntry()<<8));
						
						PHYEvent_SetSkipRefresh(FALSE);
						PHYEvent_WriteSharedMem(FALSE,INVALID_CELL_ID, 0);
						
						PHYFRAME_SetExtentINT_HISR(TRUE);
								
					}
					else
						PHYFRAME_SetExtentINT_HISR(FALSE);
				}

				if( DATA_IsDataServiceOn())
				{
					// All the data, SACCH and FACCH are processed here in HISR.

					if( frame_int & FRAMEINT_TX_FACCH )
					{
						frame_int ^= FRAMEINT_TX_FACCH;
						ENTRYDATA_SetInt( (UInt8)status_msg.arg0, frame_int );

						DATA_ProcessTxData( (UInt8)status_msg.arg0 );
					}

					if( frame_int & FRAMEINT_TX_SACCH )
					{
						frame_int ^= FRAMEINT_TX_SACCH;
						ENTRYDATA_SetInt( (UInt8)status_msg.arg0, frame_int );

						DATA_ProcessTxSACCH();
					}

					//if this interrupt is associated to other reason. trigger layer1 task.

					if ( frame_int )
					{
						RIPOUT_GetCB()->ripFrameInt( (UInt8)status_msg.arg0 );
					}
				}

				// Normal Non-Data Call

				else
				{
					// Send FACCH or Data after RIP int is received otherwise RIP
					// may be starving for data.

					if( frame_int & FRAMEINT_TX_FACCH )
					{
						RIPPROC_TxRdy( ENTRY_TXBUF_FACCH, (UInt8)status_msg.arg0 );
					}

					RIPOUT_GetCB()->ripFrameInt( (UInt8)status_msg.arg0 );
				}
#ifdef VMF_INCLUDE
				VMF_switchto_hisr_context(E_VM_ID_NONE);
#endif

				break;

			case STATUS_RX_FULL :
				{
					UInt8 rxbuf_index;
					UInt8 rx_slots_mask = 0;
					UInt8 frame_index;
					SIM_INSTANCE_t sim_id;
					PHY_EVENT_ID_t event_id;
					if ( ( status_msg.arg0 & 0xff00 ) == 0 )
					{
						rxbuf_index = (UInt8)status_msg.arg0;
					}
					else if ( ( status_msg.arg0 & 0xe000 ) == 0 )
					{
						rxbuf_index = ENTRY_RXBUF_TCH0;
						rx_slots_mask = status_msg.arg0 >> 8;
					}
					else if ( ( status_msg.arg0 & 0x0f00 ) == 0 )
					{
						rxbuf_index = ENTRY_RXBUF_SACCH0;
					}
					else
					{
						assert( FALSE );
					}

					if ( rxbuf_index != ENTRY_RXBUF_SPEECH )
					{
						frame_index = SHAREDMEM_GetFrameIndexFromBuffer(rxbuf_index,status_msg.arg2);
						ENTRYDATA_GetSimEvent(frame_index,&sim_id,&event_id);

						if (ENTRYDATA_IsDropEvent(frame_index) && (rxbuf_index < 8))
						{
							L1_LOGV3("RX_FULL Event Dropped: sim, event, fidx", sim_id, event_id, frame_index);
							RIPCMDQ_RxEmpty( rxbuf_index );
							break;
						}
					}	
					
#ifdef VMF_INCLUDE
					if(PHYEvent_IsNonIdleMode())
						VMF_switchto_hisr_context((T_VM_ID)PHYEvent_GetCurGl1Sim()+1);
					else						
						VMF_switchto_hisr_context((T_VM_ID)(sim_id+1));
#endif
					switch( rxbuf_index )
					{
						case ENTRY_RXBUF_TCH0:
						case ENTRY_RXBUF_TCH1:
						case ENTRY_RXBUF_TCH2:
						case ENTRY_RXBUF_TCH3:
							if( DATA_IsDataServiceOn())
							{
								DATA_ProcessRxData();
							}
							else
							{
								PHYFRAME_SetDelayedRxData(status_msg.arg2, rx_slots_mask);
							}
							break;

						case ENTRY_RXBUF_SACCH0:
						case ENTRY_RXBUF_SACCH1:
						case ENTRY_RXBUF_SACCH2:
						case ENTRY_RXBUF_SACCH3:
							RIPOUT_GetCB()->ripRxBuffFull( ENTRY_RXBUF_SACCH_M );
							break;

						case ENTRY_RXBUF_FACCH:
							RIPBUF_SaveFACCH();
							RIPOUT_GetCB()->ripRxBuffFull( rxbuf_index );

/*
							{
								Fn_t fn;

								fn = ENTRYDATA_GetFn( status_msg.arg2 );
								L1_LOGV4( "FACCH RX @ FN with CRC ", fn, status_msg.arg1, 0, 0 );
							}
*/

							break;
						case ENTRY_RXBUF_SPEECH:	// 0x80
							L1_LOGV( "RX type = ", status_msg.arg2 );
							if ( status_msg.arg1 ) rxbuf_index |= 0x20;	// BFI info
							rxbuf_index |= ( status_msg.arg2 & 0x1F );	// SID info
							RIPOUT_GetCB()->ripRxBuffFull( rxbuf_index );
							break;

						case ENTRY_RXBUF_RATSCCH:
							if (!ignor_RATSCCH)
							{
								if(!RIPPROC_ProcessRATSCCH())
									RIPOUT_GetCB()->ripRxBuffFull(rxbuf_index);
							}
							else
							{
								L1_LOG( "RATSCCH message ignored during AMR loop" );
							}
							break;

						default:

							// The following logic is required because
							// the EXT BCCH and DAT0 buffer index are
							// shared.  This is OK because BCCH and
							// Circuit Switch Data should never occur
							// at the same time.

							if ( DATA_IsDataServiceOn() )
							{
								if ( rxbuf_index == ENTRY_RXBUF_DATA0 )
								{
									DATA_ProcessRxData();
								}
								else
								{
									RIPOUT_GetCB()->ripRxBuffFull( rxbuf_index );
								}
							}
							else
							{
								RIPOUT_GetCB()->ripRxBuffFull( rxbuf_index );
							}
							break;
					}
#ifdef VMF_INCLUDE
						VMF_switchto_hisr_context(E_VM_ID_NONE);
#endif
						
				}
				break;

			case STATUS_TX_SENT :
				//nick.
				//We didn't use STATUS_TX_SENT interrupt.
				//Borrow ripTxBuffFree/RIP_TXBUFF_FREE/RIPPROC_TxFree implement TX_RESULT
				//comment out ripTxbufferFree here in case any confusion.


				if((UInt8) status_msg.arg0 == ENTRY_TXBUF_RACH)
				{
					Fn_t fn;

					fn = ENTRYDATA_GetFn( status_msg.arg1 );

					if (((PHYFRAME_GetFrameState() == FRAMESTATE_GPRS_IDLE) ||
					(PHYFRAME_GetFrameState() == FRAMESTATE_GPRS_PRACH))&&PHPROC_GetPRACHReq())
					{
						PHPROC_SetPRACHReq(FALSE);
						SendRACon(fn);
						wait_for_prach_sent = FALSE;
						prach_sent_time =0;
						RIPPROC_SetFirstPrach(FALSE);
					}

					L1_LOGV4( "Access burst TX @ fn, entry_ind, seq_id = ", fn, status_msg.arg1, status_msg.arg2, 0 );
				}
				/*else if((UInt8) status_msg.arg0 == ENTRY_TXBUF_SACCH)
				{	//SACCH measurement report debug
					Fn_t fn;

					fn = ENTRYDATA_GetFn( status_msg.arg1 );
					L1_LOGV( "SACCH TX @ FN = ", fn );
				}*/

				break;
			case STATUS_MEAS_REPORT :
				{
					RIPMeasReport_t rip_meas_report;

					SHAREDMEM_ReadMeasReport(
						&rip_meas_report.rxqual_full[ 0 ],
						&rip_meas_report.rxqual_sub[  0 ],
						&rip_meas_report.rxlev_full[  0 ],
						&rip_meas_report.rxlev_sub[   0 ]
					 );

					RIPOUT_GetCB()->ripPostMeasReport( &rip_meas_report, (Boolean)status_msg.arg0, (Boolean)DATA_IsHSCSD() );

#ifdef MST_TEST
					CAL_LOGV4("FER info",total_fr_cnt, err_fr_cnt, 0, 0);
					CAL_LOGV4("RX_qual_lev",rip_meas_report.rxqual_full[ 0 ], rip_meas_report.rxqual_sub[  0 ],
						rip_meas_report.rxlev_full[  0 ], rip_meas_report.rxlev_sub[   0 ]);

#endif

#if defined(DEVELOPMENT_ONLY)

					if(SHAREDMEM_get_fer_logging_flag())
					{
						meas_report_period_cnt++;
						if (meas_report_period_cnt >= 30)		//15sec
						{
							meas_report_period_cnt=0;
							{

							PAGE1_SharedMem_t *pg_mem = SHAREDMEM_GetPage1SharedMemPtr();
							L1_LOGV4("FER info: total   error  ", pg_mem->shared_total_speech_frame, pg_mem->shared_FER_counter,0,0);

							pg_mem->shared_reset_speech_counter_flag = 1;
							}

						}

					}
#endif
				}
				break;

			case STATUS_NCELL_ADJUST :
                {

#ifdef VMF_INCLUDE
				Boolean nonidle = PHYEvent_IsNonIdleMode();
				if(nonidle)
					VMF_switchto_hisr_context((T_VM_ID)PHYEvent_GetCurGl1Sim()+1);
#endif
				RIPOUT_GetCB()->ripNCellAdjust(
					status_msg.arg0,
					status_msg.arg1,
					status_msg.arg2
				);
#ifdef VMF_INCLUDE
				if(nonidle)
					VMF_switchto_hisr_context(E_VM_ID_NONE);
#endif
			    }
				break;
			case STATUS_TX_RESULT :
				if (PHYEvent_IsNonIdleMode())
				{
#ifdef VMF_INCLUDE
					VMF_switchto_hisr_context((T_VM_ID)PHYEvent_GetCurGl1Sim()+1);
#endif
#ifdef DSP_FEATURE_TBF_FASTCMD				
				if ((PHYFRAME_GetFrameState() == FRAMESTATE_GPRS_TBF || PHYFRAME_GetFrameState() == FRAMESTATE_GPRS_MON_DLINK) &&
					(status_msg.arg0 || status_msg.arg1)) // Place or Oder blocks will be transmitted in the next frame					
					PHYFRAME_DelayedMacTxBlocks( );
#endif				
				GPRSDATA_ReportHowManySent( status_msg.arg0, status_msg.arg1 );
				if ( status_msg.arg0 || status_msg.arg1 )
					L1_LOGV("TX_SEQ_DEBUG", status_msg.arg2);
				RIPOUT_GetCB()->ripDelayedProcess( &status_msg );
#ifdef VMF_INCLUDE
					VMF_switchto_hisr_context(E_VM_ID_NONE);
#endif				
				}
				break;

			case STATUS_TIMING_PULSE :
				break;

			case STATUS_SACCH_SNR:  //one interrupt per SACCH/CBCH burst
				if( AMRPROC_GetCurrentValid())
					AMRPROC_SaveSNRperBurst(status_msg.arg0, status_msg.arg1, status_msg.arg2, disable_link_est);
				else
					L1_LOGV4("SACCH or CBCH SNR",status_msg.arg0,status_msg.arg1,status_msg.arg2,0);
				break;
		case STATUS_AMR_CMI:  //one interrupt per one speech frame (20ms)
			AMRPROC_SetCodecMode(status_msg.arg0);
			AMRPROC_SaveSNRperBlock( SHAREDMEM_ReadMetric(), SHAREDMEM_ReadBFI_BEC(),
									 SHAREDMEM_ReadSNR(), status_msg.arg1, status_msg.arg2, disable_link_est );
			break;
		case STATUS_FACCH_SENT:
			L1_LOGV( "FACCH TX @ buf_id | seq_id = ", ((status_msg.arg0 << 16) | (status_msg.arg2)) );
#ifdef STACK_wedge
//	[GNATS 1656][ L1: MS didn't start FACCH T200 at the right time]
#ifdef VMF_INCLUDE
			VMF_switchto_hisr_context((T_VM_ID)PHYEvent_GetCurGl1Sim()+1);
#endif
			PHPRIM_StartFacchT200(status_msg.arg2);
#ifdef VMF_INCLUDE
			VMF_switchto_hisr_context(E_VM_ID_NONE);
#endif
#endif //STACK_wedge
			break;
		case STATUS_RATSCCH_SENT:

			if((UInt8) status_msg.arg0 == ENTRY_RXBUF_RATSCCH)
			{
				if (!ignor_RATSCCH)
				{
					AMRPROC_SetAckActFn((UInt8) status_msg.arg1);
					AMRPROC_StartCounterAckActivation();
				}
				else
				{
					L1_LOG( "ERROR: RATSCCH should be de-actived during AMR loop" );
				}
				// This is for 26.16.9.8 FTA testcase
				// If Req_Activation_Running (i.e we received RATSCCH_REQ and started the req_activation timer)
				// and RATSCCH message response has been transmitted then disable patch5 so that we don't transmit
				// RATSCCH_MARKER for all the other RATSCCH messages received until the Req_Activation times out
				// Patch 5 (For C2) is enabled again when Req_Activation timer times out
			}

			break;
		case STATUS_RATSCCH_MRK_SENT:

			if((UInt8) status_msg.arg0 == ENTRY_RXBUF_RATSCCH)
			{
				Fn_t fn;

				fn = ENTRYDATA_GetFn( status_msg.arg1 );
				L1_LOGV( "RATSCCH Marker TX @ fn = ", fn );
				if (ignor_RATSCCH)
				{
					L1_LOG( "ERROR: RATSCCH should be de-actived during AMR loop" );
				}
			}

			break;
		case STATUS_RX_RATSCCH_MRK:
			{
				Fn_t fn;

				// This is for 26.16.9.8 FTA testcase
				// If Req_Activation_Running (i.e we received RATSCCH_REQ and started the req_activation timer), we shouldn't
				// transmit RATSCCH_MARKER for all the other RATSCCH messages received until the Req_Activation times out

// CQ MobC00103169, Andy Shen, 02/25/10
				// frame index is arg2, was using arg1 incorrectly
				fn = ENTRYDATA_GetFn( status_msg.arg2 );
// CQ MobC00103169, Andy Shen, 02/25/10
				
				L1_LOGV( "RATSCCH Marker RX @ fn = ", fn );
				if (ignor_RATSCCH)
				{
					L1_LOG( "ERROR: RATSCCH should be de-actived during AMR loop" );
				}
			}

			break;

		case STATUS_AMR_PARM_SET:
			AMRPROC_DSP_StartDlinkNewCodecSet( status_msg.arg0, status_msg.arg1, status_msg.arg2 );
			break;

		case STATUS_ONSET_SENT:
			{
				Fn_t fn;

				fn = ENTRYDATA_GetFn( status_msg.arg1 );
				L1_LOGV( "Onset TX @ fn = ", fn );
			}
			break;

		case STATUS_SID_FIRST_SENT:
			{
				Fn_t fn;

				fn = ENTRYDATA_GetFn( status_msg.arg1 );
				L1_LOGV( "SID FIRST TX @ fn = ", fn );
			}
			break;

		case STATUS_SID_FIRST_INH_SENT:
			{
				Fn_t fn;

				fn = ENTRYDATA_GetFn( status_msg.arg1 );
				L1_LOGV( "SID FIRST INH TX @ fn = ", fn );
			}
			break;

		case STATUS_SID_UPDATE_SENT:
			{
				Fn_t fn;

				fn = ENTRYDATA_GetFn( status_msg.arg1 );
				L1_LOGV( "SID UPDATE TX @ fn = ", fn );
			}
			break;

		case STATUS_SID_UPDATE_INH_SENT:
			{
				Fn_t fn;

				fn = ENTRYDATA_GetFn( status_msg.arg1 );
				L1_LOGV( "SID UPDATE INH TX @ fn = ", fn );
			}
			break;
		case STATUS_SYNC:
			RIPOUT_GetCB()->ripDelayedProcess( &status_msg );
			break;


		case STATUS_CTM_READY:
			RIPCMDQ_CTM_Ready(0);
			CTM_Continue(0);
			break;

		case STATUS_DSP_WAKEUP_RSP:
			if ( status_msg.arg0 == 1)
			{
				PHYFRAME_DSPSuspendAck(STATUS_DSP_WAKEUP_RSP, status_msg.arg1 - 1);
			}
			else
			{
				PHYFRAME_DSPSuspendAck(STATUS_DSP_WAKEUP_RSP, 0);
				L1_LOGV4( "Wakeup requested before DSP went to sleep", status_msg.arg0, status_msg.arg1, status_msg.arg2, 0 );
			}
			break;


		case STATUS_ECHO_CANCEL_DONE:
			RIPOUT_GetCB()->ripDelayedProcess( &status_msg );	//MobC00074668, swchen, 4/2009
			break;

		case STATUS_SAIC_SW:
		   //	L1_LOGV4( "SAIC ", status_msg.arg0, status_msg.arg1, status_msg.arg2, 0 );
			break;

#if (defined UMTS)

		case STATUS_FER_TEST:	// For MST CRC passing to OLPC
			{
				VP_RXFrameType_AMR_t rxframe_type;
				rxframe_type = (VP_RXFrameType_AMR_t)( ((status_msg.arg2)>>4) & 0x7);
				//L1_LOGV4("mst frame type", status_msg.arg0, status_msg.arg1, status_msg.arg2,rxframe_type);
				switch (rxframe_type){	  // rxframe_type
					case AMR_RX_SPEECH_GOOD:
					case AMR_RX_SID_FIRST:
					case AMR_RX_SID_UPDATE:
						l1u_olpc_mst_crc_update(0); //pass CRC
						break;
					case AMR_RX_SPEECH_BAD:
					case AMR_RX_SID_BAD:
						l1u_olpc_mst_crc_update(1);	//CRC fail
						break;
					default:
						l1u_olpc_mst_crc_update(0xff);	//CRC default
						break;
				}
			}
			break;
#endif //#ifdef UMTS
		case STATUS_OPEN_LOOP:
			ignor_RATSCCH = FALSE;
			disable_link_est = FALSE;
			L1_LOG("ACK OPEN LOOP CMD");
			break;

		case STATUS_CLOSE_LOOP:
			L1_LOGV4("ACK CLOSE LOOP CMD:", status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);
			TCHLoop_type = status_msg.arg0;
			if (status_msg.arg0 != TCHLOOP_NONE)
			{
				extern void MEASPROC_InvalidateNext(void);
				ignor_RATSCCH = TRUE;
				disable_link_est = TRUE;

				// Invalidate the first measurement report after Close Loop Cmd
				MEASPROC_InvalidateNext();

				//AMRPROC_DisableLinkAdaptation();
				if (status_msg.arg0 == TCHLOOP_I)
					AMRPROC_OddPhaseReq();
			}
			break;

#ifdef MST_TEST
		case STATUS_FER_TEST:

			total_fr_cnt = status_msg.arg0;
			err_fr_cnt   = status_msg.arg1;

			L1_LOGV4("sta_fer_tst", status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);

			break;
#endif

#ifdef EPC
		case STATUS_EPC:
			L1_LOGV4("STATUS_EPC: SACCH Burst, PWR, RxQual", status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);
			LINKCTRL_SetEPCPwr(status_msg.arg1);
			break;
#endif

		case STATUS_AUDIO_STREAM_DATA_READY:

			L1_LOGV4("STATUS_AUDIO_STREAM_DATA_READY",status_msg.arg0, status_msg.arg1, status_msg.arg2, 0);
			break;

		case STATUS_SMC_ERROR:
			assert( FALSE );
			break;

		case STATUS_GPIO_MISMATCH:
			L1_LOGV4("GPIO ERROR",status_msg.status,status_msg.arg0, status_msg.arg1, status_msg.arg2);
			assert( FALSE );
			break;
		case STATUS_DSP_MIPI_WRITE:
			{
				Int16	spisr_type;
				Unpaged_SharedMem_t  *pg_mem = SHAREDMEM_GetUnpagedSharedMemPtr();

				spisr_type = status_msg.arg0;
			//  add corect logging here
				L1_LOGV4("GPIO DSP_MIPI_WRITE",status_msg.status,status_msg.arg0, status_msg.arg1, pg_mem->shared_spi_dump[spisr_type][0]);
			}

			break;
		case STATUS_MODEM_DATA_READY:
			{
				L1_LOGV4("MODEM DATA READY",status_msg.status,status_msg.arg0, status_msg.arg1, status_msg.arg2);
			}
			break;
			
		case STATUS_SHORT_FRAME:
		{
			L1_LOGV4("STATUS SHORT FRAME ",status_msg.status,status_msg.arg0, status_msg.arg1, status_msg.arg2);
		}
		break;
		default :
			if ( ((UInt16)(status_msg.status & 0xf000) == (UInt16)0xe000) || ((UInt16)(status_msg.status & 0xf000) == (UInt16)0xd000) )
			{
				L1_LOGV4("DSP WARNING STATUS",status_msg.status,status_msg.arg0, status_msg.arg1, status_msg.arg2);
				
				if ( status_msg.status == 0xec14 )
				{
					L1_LOG("WARNING: EQ VIT BUSY");
				}
				else if ( status_msg.status == 0xec1a )
				{
					L1_LOG("WARNING: LEFT OVER DATA in TX BUFFER");
				}
				else if	( status_msg.status == 0xec20 || status_msg.status == 0xec21 )
				{
					L1_LOG("WARNING: SW RX pending/delay");
				}
				else if	( status_msg.status == 0xec0f )
				{
					L1_LOG("WARNING: TX buffer is not filled");
				}
				else if ( status_msg.status == 0xdec5 )
				{
					L1_LOG("WARNING: 2AMR DECODER ERROR: WRONG FRAME FORMAT ");
				}
				else if ( status_msg.status == 0xec16 )
				{
					L1_LOG("WARNING: SEARCH FRAME COUNT IS NEGATIVE");
				}
				else
				{
					if (Enable_dsp_assertion_4_WARNING_STATUS_MSG == 0x0)
					{
						char string[0x50];

						sprintf(string, "\r\n ripisr, %02x %04x %04x %04x", status_msg.status, status_msg.arg0,
								status_msg.arg1, status_msg.arg2);
						SIO_PutString(PortB, string);
						// redefined JTAG_FRIENDLY_L1_ASSERT to return zero, compiler warning cleanup
						JTAG_FRIENDLY_DSP_ASSERT(FALSE, (status_msg.arg0 << 16)|status_msg.status);
					}
				}
			}
			else
			{
				msg_processed = FALSE;
			}
			break;
	}
	return msg_processed;
}

//******************************************************************************
//
// Function Name:	RIPISR_COMMS_Init
//
// Description:		
//
// Notes:
//
//******************************************************************************

void RIPISR_COMMS_Init()						// Initialize the RIP ISR
{
	RIPISR_Register((RIPISR_HISR_CB)RIPISR_HISR_Comms,	(RIPISR_PROCESSSTATUS_CB) RIPISR_ProcessMsg_Comms);
}


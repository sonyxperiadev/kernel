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
#include "sharedmem.h"
#include "xassert.h"
#include "log.h"

#include "io.h"
#include "platform_mconfig_rhea.h"

#if defined(DEVELOPMENT_ONLY)
UInt16 at_mdsptst_audio_logging = FALSE;
UInt16 at_mtst_track_logging =FALSE;
UInt16 at_mtst_ncell_logging = FALSE;
UInt16 at_mtst_fer_logging = FALSE;
Boolean at_mtst_frameentry_logging = TRUE;
Boolean at_mtst_fsc_logging = FALSE;
Boolean at_mtst_event_logging = FALSE;
#endif


//#pragma arm section zidata = "shared_rip_mem_sect"
//static AP_SharedMem_t			AP_shared_mememory __attribute__((section("shared_rip_mem_sect")));
      
//#pragma arm section


static AP_SharedMem_t 			*dsp_shared_mem = NULL;
static AP_SharedMem_t 			*global_shared_mem = NULL;

static AP_SharedMem_t			*shared_mem	= NULL; 
static AP_SharedMem_t 			*pg27_shared_mem = NULL;
static AP_SharedMem_t 			*pg28_shared_mem = NULL;

static UInt8 AMR_Codec_Mode_Good = 7;
#if defined(UMTS) && defined(DEVELOPMENT_ONLY)
Boolean is_amr_voice_test = FALSE;
static UInt32 amr_voice_counter =0;
#endif



//******************************************************************************
//
// Function Name:	SHAREDMEM_Init
//
// Description:		Initialize AP Shared Memory
//
// Notes:
//
//******************************************************************************
void AP_SHAREDMEM_Init()
{
	// Clear out shared memory
//	memset(&AP_shared_mememory, 0, sizeof(AP_SharedMem_t));

}

//#if ((defined(FPGA_VERSION) && defined(_BCM21551_)) || defined(_ATHENA_))
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetDspModemAudioSharedMemPtr
//
// Description:		Return pointer to 32-bit aligned shared memory
//
// Notes:
//
//******************************************************************************

AP_SharedMem_t *SHAREDMEM_GetDsp_SharedMemPtr()		// Return pointer to 32-bit aligned shared memory
{
		if(global_shared_mem == NULL)
		{
			global_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
			if (!global_shared_mem) {
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* mapping dsp shared memory failed\n\r");
				return NULL;
			}
	   }
		dsp_shared_mem = global_shared_mem;

	return dsp_shared_mem;
}	

//#endif




//******************************************************************************
//
// Function Name:	SHAREDMEM_GetSharedMemPtr
//
// Description:		Return pointer to shared memory
//
// Notes:
//
//******************************************************************************

AP_SharedMem_t *SHAREDMEM_GetSharedMemPtr()// Return pointer to shared memory
{
	 if(global_shared_mem == NULL)
	 {
		 global_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
		 if (!global_shared_mem) {
			 Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* mapping shared memory failed\n\r");
			 return NULL;
		 }
	}
	shared_mem = global_shared_mem;

	return shared_mem;


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

AP_SharedMem_t *SHAREDMEM_GetPage27SharedMemPtr()// Return pointer to page 27 shared memory
{
    if(global_shared_mem == NULL)
    {
        global_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
        if (!global_shared_mem) {
            Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* mapping shared memory failed\n\r");
            return NULL;
        }
    }
   	pg27_shared_mem = global_shared_mem;
	
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

AP_SharedMem_t *SHAREDMEM_GetPage28SharedMemPtr()// Return pointer to page 28 shared memory
{
	if(global_shared_mem == NULL)
	   {
		   global_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
		   if (!global_shared_mem) {
			   Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* mapping shared memory failed\n\r");
			   return NULL;
		   }
	  }
	  pg28_shared_mem = global_shared_mem;
	  
	  return pg28_shared_mem;
}	


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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();	
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_DL_MainAMR_buf);
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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_DL_MainAMR_buf);

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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_UL_MainAMR_buf);
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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_UL_MainAMR_buf);
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

static const UInt16 sVoIPDataLen[6] = {0, 322, 160, 38, 166, 642};

//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteDL_VoIP_Data
//
// Description:	Write the DL VoIP data to sharedmem
// Notes:
//
//******************************************************************************
void SHAREDMEM_WriteDL_VoIP_Data(UInt16 codec_type, UInt16 *pSrc)
{
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());
	VOIP_Buffer_t *pDst = &(vp_shared_mem->VOIP_DL_buf);
	UInt8 index = 0;
	UInt16 data_len = 0;

	index = (codec_type & 0xf000) >> 12;
	if (index >= 6)
	{
  		Log_DebugPrintf( LOGID_SOC_AUDIO,"=====SHAREDMEM_WriteUL_VoIP_Data, invalid codec type!!!\r\n");
	}
	else
		data_len = sVoIPDataLen[index];

  	//TRACE_Printf_Sio( "=====SHAREDMEM_WriteDL_VoIP_Data codec_type=0x%x dataLen=%d\r\n", codec_type, data_len);

	memcpy(pDst, pSrc, data_len);
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadUL_VoIP_Data
//
// Description:	Read the UL VoIP data from sharedmem
// Notes:
//
//******************************************************************************
UInt8 SHAREDMEM_ReadUL_VoIP_Data(UInt16 codec_type, UInt16 *pDst)
{
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr());
	UInt16 *pSrc = (UInt16*)&(vp_shared_mem->VOIP_UL_buf.voip_vocoder);
	UInt8 index = 0;
	UInt16 data_len = 0;

	index = (codec_type & 0xf000) >> 12;
	if (index >= 6)
	{
  		Log_DebugPrintf( LOGID_SOC_AUDIO,"=====SHAREDMEM_ReadUL_VoIP_Data, invalid codec type!!!\r\n");
	}
	else
		data_len = sVoIPDataLen[index];

  	// TRACE_Printf_Sio( "=====SHAREDMEM_ReadUL_VoIP_Data codec_type=0x%x dataLen=%d\r\n", codec_type, data_len);
	
	memcpy(pDst, pSrc, data_len);
	
	return data_len;
}

//Max WB-AMR frame size is 60 bytes (477 bits rounded to nearest byte) and there is 4 bytes header on the buffer received from DSP. So we allocate 16 DWORDS size buffer
static UInt32 ul_amr_wb_speech_data[16];
//MobC00093269 - AMR-WB version of read buffer
void SHAREDMEM_ReadUL_WB_AMR_Speech(UInt32 **speech_data, Boolean amr_if2_enable)
{
	int i;
	UInt16 UL_AMR_buf[32]; //Max WB-AMR frame size is 60 bytes (477 bits rounded to nearest byte) and there is 4 bytes header on the buffer received from DSP
	UInt8  *temp_buf8 = (UInt8 *)UL_AMR_buf;
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_UL_MainAMR_buf);

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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();

	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_UL_MainAMR_buf);
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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();
	
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_UL_MainAMR_buf);
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
	AP_SharedMem_t *vp_shared_mem = SHAREDMEM_GetSharedMemPtr();

	UInt16 *ptr = (UInt16*)&(vp_shared_mem->AP_UL_MainAMR_buf);
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

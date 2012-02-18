//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   csl_voip.c
*
*   @brief  This file contains DSP VoIP interface
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_voip.h"
#include "csl_apcmd.h"
#include "log.h"

extern AP_SharedMem_t	*vp_shared_mem;

static const UInt16 sVoIPDataLen[7] = {0, 322, 160, 38, 166, 642, 70};


//*********************************************************************
/**
*
*   CSL_WriteDLVoIPData writes VoIP data to DSP shared memory
*
*   @param    codec_type	(in)		codec type
*   @param    pSrc			(in)		source of the data to write
* 
**********************************************************************/
void CSL_WriteDLVoIPData(UInt16 codec_type, UInt16 *pSrc)
{
	VOIP_Buffer_t *pDst = &(vp_shared_mem->VOIP_DL_buf);
	UInt8 index = 0;
	UInt16 data_len = 0;

	index = (codec_type & 0xf000) >> 12;
	if (index >= 7)
  		Log_DebugPrintf(LOGID_AUDIO, "===== CSL_WriteDLVoIPData, invalid codec type!!!\n");
	else
		data_len = sVoIPDataLen[index];

	memcpy(pDst, pSrc, data_len);
}

//*********************************************************************
/**
*
*   CSL_ReadULVoIPData reads VoIP data from DSP shared memory
*
*   @param    codec_type	(in)		codec type
*   @param    pDst			(in)		destination of read data
*   @return   UInt8						size of data in bytes
* 
**********************************************************************/
UInt8 CSL_ReadULVoIPData(UInt16 codec_type, UInt16 *pDst)
{
	UInt16 *pSrc = (UInt16*)&(vp_shared_mem->VOIP_UL_buf.voip_vocoder);
	UInt8 index = 0;
	UInt16 data_len = 0;

	index = (codec_type & 0xf000) >> 12;
	if (index >= 7)
  		Log_DebugPrintf(LOGID_AUDIO, "===== CSL_ReadULVoIPData, invalid codec type!!!\n");
	else
		data_len = sVoIPDataLen[index];
	
	memcpy(pDst, pSrc, data_len);
	
	return data_len;
}

#ifdef VOLTE_SUPPORT

//*********************************************************************
/**
*
*   DJB_Init initializes Jitter Buffer of VoLTE interface
*
**********************************************************************/
void DJB_Init(void)
{
	/* clear payload queue */
	memset(&vp_shared_mem->ajcPayloadQueue, 0, sizeof(DJB_PAYLOADQ));

	/* clear payload buffer */
	memset(&vp_shared_mem->ajcPayloadBuffer[0], 0, DJB_BUFFER_SIZE*sizeof(UInt16));
	
	/* send init command to DSP */
	VPRIPCMDQ_VoLTE_Init();

}

//*********************************************************************
/**
*
*   DJB_Init flushes Jitter Buffer for new stream
* 
**********************************************************************/
void DJB_StartStream(void)
{
	/* send new stream command to DSP */
	VPRIPCMDQ_VoLTE_Start_Stream();

}

/****************************************************************************
*  searchFreeIndex - search for a free slot in the payload buffer
*  PURPOSE: This routine allocates a buffer for frame payload
*  PARAMETERS:
*     payloadqp - payload queue
*  RETURNS:
*     allocated buffer index
*  NOTE:
****************************************************************************/
static Int16 searchFreeIndex(DJB_PAYLOADQ *payloadqp)
{   
   UInt16 *freeListp = &vp_shared_mem->ajcPayloadBuffer[0] + (payloadqp->numEntry * 4);
   UInt16 i, bufferidx = 0;
   UInt16 search = 0;
   Int16 result = -1;

   for(i = 0; i < payloadqp->freeListSize; i++)
   {
      search = 0x8000;
      for(bufferidx = 0; bufferidx < 16; bufferidx++)
      {
         if((*freeListp|search) != *freeListp)
         {
            break;
         }
         search >>= 1;
      }

      if(bufferidx < 16)
         break;

      freeListp++;

   }

   bufferidx += (i*16);
   
   if(bufferidx < payloadqp->numEntry)
   {
      result = (Int16)bufferidx;
      *freeListp |= search;
   }

   return(result);
}


//*********************************************************************
/**
*
*   DJB_PutFrame puts incoming frame into Jitter Buffer of VoLTE interface
*
*   @param    pInputFrame	(in)		input frame
* 
**********************************************************************/
void DJB_PutFrame(DJB_InputFrame *pInputFrame)
{
 DJB_PAYLOADQ *payloadqp;
 UInt16 *pPayload;
 UInt16 timestamp;
 Int16 bufferIndex;

	/* get payload queue pointer */
	payloadqp = &vp_shared_mem->ajcPayloadQueue;

	/* allocate a buffer for frame payload */
	bufferIndex = searchFreeIndex(payloadqp);

	/* find allocated buffer pointer */
	if(bufferIndex >= 0)
    {
		pPayload = &vp_shared_mem->ajcPayloadBuffer[0] + (payloadqp->numEntry * 4) + (payloadqp->numEntry>>4) + 1 + (bufferIndex * payloadqp->entrySizeInWords);
        payloadqp->numAlloc++;
     
	}
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "===== DJB_PutFrame, Jitter Buffer overflow!!!\n");

		return;
	}

	
	/* copy payload content to DSP shared memory */
	memcpy(pPayload, pInputFrame->pFramePayload, pInputFrame->payloadSize);

	/* take 16-bit LSB of RTP timestamp */
	timestamp = (UInt16)pInputFrame->RTPTimestamp;

	/* send message to DSP */
	VPRIPCMDQ_VoLTE_Put_Frame(timestamp, pInputFrame->codecType, pInputFrame->frameType, pInputFrame->frameQuality, pInputFrame->frameIndex, (UInt8)bufferIndex);

}

#endif // VOLTE_SUPPORT


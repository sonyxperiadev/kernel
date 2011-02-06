/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   csl_aud_queue.h
*
*   @brief  APIs for audio queue object
*
****************************************************************************/

#ifndef	__CSL_AUD_QUE_H__
#define	__CSL_AUD_QUE_H__

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

#define AUDQUE_MARGIN 1	//leave at least this amount in the queue during write. only works for 1 for now.

typedef struct AUDQUE_Queue_t
{
	UInt8				*base;
	UInt8				*bottom;
	UInt32				blockNum;
	UInt32				blockSize;
	UInt8				*writePtr;
	UInt8				*readPtr;

	Boolean				isCleared;
} AUDQUE_Queue_t;


/**
*
*  Create an audio queue
*
*  @param		baseAddr	The start address of the audio queue
*  @param		blockNum	The number of blocks in the audio queue
*  @param		blockSize	The size of each block in the audio queue
*
*  @return	   AUDQUE_Queue_t	the created audio queue reference
*
*  @note		An audio queue is to describe and manage a continues memory area.
*				This memory area is divided into a number of(blockNum) blocks and 
*				each block has a size(blockSize).
*
*****************************************************************************/
AUDQUE_Queue_t*	AUDQUE_Create (UInt8 *baseAddr, 
							   UInt32 blockNum, 
							   UInt32 blockSize);


/**
*
*  Destroy an audio queue
*
*  @param		aq	The audio queue reference
*
*  @return	   void
*
*****************************************************************************/
void AUDQUE_Destroy (AUDQUE_Queue_t* aq);


/**
*
*  Write data to an audio queue
*
*  @param		*aq		The audio queue reference
*  @param		*data	The address of the data
*  @param		size	The size of the data
*
*  @return	   UInt32   Number of bytes copied
*
*  @note		This function checks and handles overflow
*****************************************************************************/
UInt32 AUDQUE_Write (AUDQUE_Queue_t *aq, 
					 UInt8 *data, 
					 UInt32 size);
/**
*
*  Write data to an audio queue as interleaved.
*
*  @param		*aq		The audio queue reference
*  @param		*data	The address of the data
*  @param		*data2	The address of the other data
*  @param		size	The size of the data
*  @param		bitPerSample	The format of the data
*
*  @return	   UInt32   Number of bytes copied
*
*  @note		This function checks and handles overflow
*****************************************************************************/
UInt32 AUDQUE_WriteInterleave (AUDQUE_Queue_t *aq,
           					   UInt8 *data, 
		            		   UInt8 *data2, 
        					   UInt32 size,
                               AUDIO_BITS_PER_SAMPLE_t bitPerSample);

/**
*
*  Write data to an audio queue without checking overflow
*
*  @param		*aq		The audio queue reference
*  @param		*data	The address of the data
*  @param		size	The size of the data
*
*  @return	   UInt32   Number of bytes copied
*
*  @note		This function doesn't check overflow and may overwrite previous
*				unused data
*****************************************************************************/
UInt32 AUDQUE_WriteNoCheck (AUDQUE_Queue_t *aq,
							UInt8 *data, 
							UInt32 size);

/**
*
*  Update the writePtr of an audio queue without checking overflow
*
*  @param		*aq		The audio queue reference
*
*  @return	   void
*
*  @note		This function doesn't check overflow, usually called when DMA
*				finishes a transfer to the queue
*****************************************************************************/
void AUDQUE_UpdateWritePtr (AUDQUE_Queue_t *aq);

/**
*
*   Move the writePtr to current plus the size, don't handle underflow
*
*  @param		*aq		The audio queue reference
*
*  @return	   void
*
*  @note		This function doesn't handle underflow
*****************************************************************************/
void AUDQUE_UpdateWritePtrWithSize (AUDQUE_Queue_t *aq, UInt32 size);


/**
*
*  Read data from an audio queue
*
*  @param		*aq		The audio queue reference
*  @param		*data	The address of the data
*  @param		size	The size of the data
*
*  @return	   UInt32   Number of bytes copied
*
*  @note		This function checks and handles underflow
*****************************************************************************/
UInt32 AUDQUE_Read (AUDQUE_Queue_t *aq, 
					UInt8 *data, 
					UInt32 size);

/**
*
*  Read data from an audio queue without checking underflow
*
*  @param		*aq		The audio queue reference
*  @param		*data	The address of the data
*  @param		size	The size of the data
*
*  @return	   UInt32   Number of bytes copied
*
*  @note		This function doesn't check overflow and may read legacy data.
*****************************************************************************/
UInt32 AUDQUE_ReadNoCheck (AUDQUE_Queue_t *aq, 
						   UInt8 *data, 
						   UInt32 size);

/**
*
*   Move the readPtr to current plus the size, don't handle underflow
*
*  @param		*aq		The audio queue reference
*
*  @return	   void
*
*  @note		This function doesn't handle underflow
*****************************************************************************/
void AUDQUE_UpdateReadPtrWithSize (AUDQUE_Queue_t *aq, UInt32 size);

/**
*
*  Update the readPtr of an audio queue with underflow checking
*
*  @param		*aq		The audio queue reference
*
*  @return	   void
*
*  @note		This function checks underflow, usually called when DMA
*				finishes a transfer from the queue. 
*****************************************************************************/
void AUDQUE_UpdateReadPtr (AUDQUE_Queue_t *aq);

/**
*
*	Get the read pointer(first byte) 
*  @param		*aq		The audio queue reference
*
*  @return	   UInt8*	The value of the read pointer
*
*  @note		 
*****************************************************************************/
UInt8* AUDQUE_GetReadPtr (AUDQUE_Queue_t *aq);\

/**
*
*	Get the write pointer(first byte) 
*  @param		*aq		The audio queue reference
*
*  @return	   UInt8*	The value of the read pointer
*
*  @note		 
*****************************************************************************/
UInt8* AUDQUE_GetWritePtr (AUDQUE_Queue_t *aq);

/**
*
*  Get the amount (size) of the remaining data in the queue (FIFO).
*
*  @param		*aq		The audio queue reference
*
*  @return	   UInt32	The size in bytes
*
*  @note		 
*****************************************************************************/
UInt32 AUDQUE_GetLoad (AUDQUE_Queue_t *aq);

/**
*
*  Flush the audio queue with 0's
*
*  @param		*aq		The audio queue reference
*
*  @return	   void
*
*  @note		 
*****************************************************************************/
void AUDQUE_Flush(AUDQUE_Queue_t *aq);

/**
*
*  Get the total size of the queue
*
*  @param		*aq		The audio queue reference
*
*  @return	   UInt32   The size of the queuue
*
*  @note		 
*****************************************************************************/
UInt32 AUDQUE_GetQueueSize(AUDQUE_Queue_t *aq);


/**
*
* Get the size of from the read pointer to the bottom of the queue
*  @param		*aq		The audio queue reference
*
*  @return	   UInt32   The size from read pointer to the bottom
*
*  @note		 
*****************************************************************************/
UInt32 AUDQUE_GetSizeReadPtrToBottom (AUDQUE_Queue_t *aq);

/**
*
* Get the size of from the write pointer to the bottom of the queue
*  @param		*aq		The audio queue reference
*
*  @return	   UInt32   The size from write pointer to the bottom
*
*  @note		 
*****************************************************************************/
UInt32 AUDQUE_GetSizeWritePtrToBottom (AUDQUE_Queue_t *aq);


#ifdef __cplusplus
}
#endif

#endif // __CSL_AUD_QUE_H__

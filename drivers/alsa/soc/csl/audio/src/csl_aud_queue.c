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
*   @file   csl_aud_queue.c
*
*   @brief  APIs for audio queue object
*
****************************************************************************/



/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#include "mobcom_types.h"
#include "audio_consts.h"
#ifdef UNDER_LINUX
#include <linux/kernel.h>
#include <linux/slab.h>
#endif
#include "osdal_os.h"
#include "log.h"
#include "xassert.h"
#include "csl_aud_queue.h"

static void AUDQUE_MemCpyInterleave (UInt8* dest, 
                                    UInt8* source, 
                                    UInt8* source2, 
                                    UInt32 size,
                                    AUDIO_BITS_PER_SAMPLE_t bitPerSample);
// ====================================================================================
//
// Function Name: AUDQUE_MemCpyInterleave
//
// Description: Copy data from two buffers as interleaved into the dest buffer
//
// ===================================================================================

static void AUDQUE_MemCpyInterleave (UInt8* dest, 
                                    UInt8* source, 
                                    UInt8* source2, 
                                    UInt32 size,
                                    AUDIO_BITS_PER_SAMPLE_t bitPerSample)
{
    UInt32 i = 0;
#if 0    
    UInt32 j = 0;
    UInt32 step = 0;
    if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
    {
        step = 2;
    }
    else
    if (bitPerSample == AUDIO_32_BIT_PER_SAMPLE)
    {
        step = 4;
    }
    else
    {
        xassert(0, bitPerSample);
    }

    while (i < size)
    {
        for(j=0; j<step; j++)
        {
            *(dest+j) = *(source+j);
        }
//        memcpy(dest, source, step);
        dest = dest + step;
        for(j=0; j<step; j++)
        {
            *(dest+j) = *(source2+j);
        }
//        memcpy(dest, source2, step);

        source2 = source2 + step;
        source = source + step;
        dest = dest + step;
        i++;
    }

#else


    if(bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
    {
        UInt16 *dest16 = (void*)dest;
        UInt16 *src_16 = (void*)source;
        UInt16 *src2_16 = (void*)source2;
        for(i=0; i<size/2; i++)
        {
            *dest16++ = *src_16++;
            *dest16++ = *src2_16++;
        }
    }
    else
    if(bitPerSample == AUDIO_32_BIT_PER_SAMPLE)
    {
        UInt32 *dest32 = (void*)dest;
        UInt32 *src_32 = (void*)source;
        UInt32 *src2_32 = (void*)source2;
        for(i=0; i<size/4; i++)
        {
            *dest32++ = *src_32++;
            *dest32++ = *src2_32++;
        }
    }
    else
    {
        xassert(0, bitPerSample);
    }


#endif
    return;
}

// ====================================================================================
//
// Function Name: AUDQUE_Create
//
// Description: create a audio queue object
//
// ===================================================================================
AUDQUE_Queue_t* AUDQUE_Create(UInt8 *baseAddr, 
							  UInt32 blockNum, 
							  UInt32 blockSize)
{
	AUDQUE_Queue_t *que = (AUDQUE_Queue_t *)OSDAL_ALLOCHEAPMEM (sizeof(AUDQUE_Queue_t));

	que->base = que->readPtr = que->writePtr = baseAddr;
	que->blockNum = blockNum;
	que->blockSize = blockSize;
	que->bottom = baseAddr + blockNum*blockSize;
	
	memset (que->base, 0, que->bottom - que->base);
	que->isCleared = TRUE;

	return que;
}

// ====================================================================================
//
// Function Name: AUDQUE_Destroy
//
// Description: destroy a audio queue object
//
// ===================================================================================
void AUDQUE_Destroy (AUDQUE_Queue_t *aq)
{
	OSDAL_FREEHEAPMEM (aq);
}


// ====================================================================================
//
// Function Name: AUDQUE_Write
//
// Description: Write data to an audio queue, checking overflow
//
// ===================================================================================
UInt32 AUDQUE_Write (AUDQUE_Queue_t *aq,
					 UInt8 *data, 
					 UInt32 size)
{
	// This function deals with overflow. It doesn't copy data when overflow happens.

	UInt32 copied = 0;
	UInt8	*buf = data;
	
	if (aq->writePtr < aq->readPtr)
	{
		if (aq->writePtr + size < aq->readPtr)
		{
			// copy all	
			memcpy (aq->writePtr, buf, size);
			copied = size;
		}
		else
		{
			// no space to write all, just copy part, don't wite to readPrt, leave AUDQUE_MARGIN byte
			memcpy (aq->writePtr, buf, (aq->readPtr - aq->writePtr - AUDQUE_MARGIN));
			copied = aq->readPtr - aq->writePtr - AUDQUE_MARGIN;
		}	
	}
	else //(writePtr >= readPtr)
	{
		if (aq->writePtr + size < aq->bottom)
		{
			// copy all
			memcpy (aq->writePtr, buf, size);
			copied = size;
		}
		else
		{
			if (aq->base + (aq->writePtr + size - aq->bottom) < aq->readPtr)
			{
				// copy all, but need 2 steps
				memcpy (aq->writePtr, buf, aq->bottom - aq->writePtr);
				buf += aq->bottom - aq->writePtr;
				memcpy (aq->base, buf,  aq->writePtr + size - aq->bottom);
				copied = size;
			}
			else
			{
				// copy part, but need 2 steps, don't wite to readPrt, leave AUDQUE_MARGIN byte
				if (aq->readPtr > aq->base)
				{
					memcpy (aq->writePtr, buf, aq->bottom - aq->writePtr);
					buf += aq->bottom - aq->writePtr;
					memcpy (aq->base, buf, aq->readPtr - aq->base - AUDQUE_MARGIN);
					copied = (aq->bottom - aq->writePtr) + (aq->readPtr - aq->base - AUDQUE_MARGIN);
				}
				else // aq->readPtr == aq->base
				{
					// don't write base
					memcpy (aq->writePtr, buf, aq->bottom - aq->writePtr - AUDQUE_MARGIN);
					buf += aq->bottom - aq->writePtr - AUDQUE_MARGIN;
					copied = aq->bottom - aq->writePtr - AUDQUE_MARGIN;			
				}
			}
		}
	}
	
	// update writePtr
	if (aq->writePtr + copied < aq->bottom)
	{
		aq->writePtr += copied;
	}
	else
	{
		// wrapped
		aq->writePtr = aq->base + (aq->writePtr + copied - aq->bottom);
	}

	return copied;
}

// ====================================================================================
//
// Function Name: AUDQUE_WriteInterleave
//
// Description: Write two data to an audio queue with interleaving format, checking overflow
//
// ===================================================================================
UInt32 AUDQUE_WriteInterleave (AUDQUE_Queue_t *aq,
           					   UInt8 *data, 
		            		   UInt8 *data2, 
        					   UInt32 size,
                               AUDIO_BITS_PER_SAMPLE_t bitPerSample)
{
	// This function deals with overflow. It doesn't copy data when overflow happens.

	UInt32 copied = 0;
	UInt8	*buf = data;
	UInt8	*buf2 = data2;
	
	if (aq->writePtr < aq->readPtr)
	{
		if (aq->writePtr + size < aq->readPtr)
		{
			// copy all	
			AUDQUE_MemCpyInterleave (aq->writePtr, 
                                     buf, 
                                     buf2, 
                                     size, 
                                     bitPerSample);
			copied = size;
		}
		else
		{
			// no space to write all, just copy part, don't wite to readPrt, leave AUDQUE_MARGIN byte
			AUDQUE_MemCpyInterleave (aq->writePtr, 
                                     buf, 
                                     buf2, 
                                     (aq->readPtr - aq->writePtr - AUDQUE_MARGIN), 
                                     bitPerSample);
			copied = aq->readPtr - aq->writePtr - AUDQUE_MARGIN;
		}	
	}
	else //(writePtr >= readPtr)
	{
		if (aq->writePtr + size < aq->bottom)
		{
			// copy all
			AUDQUE_MemCpyInterleave (aq->writePtr, 
                                     buf, 
                                     buf2, 
                                     size,
                                     bitPerSample);
			copied = size;
		}
		else
		{
			if (aq->base + (aq->writePtr + size - aq->bottom) < aq->readPtr)
			{
				// copy all, but need 2 steps
				AUDQUE_MemCpyInterleave (aq->writePtr, 
                                         buf, 
                                         buf2, 
                                         aq->bottom - aq->writePtr,
                                         bitPerSample);
				buf += aq->bottom - aq->writePtr;
				AUDQUE_MemCpyInterleave (aq->base, 
                                         buf, 
                                         buf2, 
                                         aq->writePtr + size - aq->bottom,
                                         bitPerSample);
				copied = size;
			}
			else
			{
				// copy part, but need 2 steps, don't wite to readPrt, leave AUDQUE_MARGIN byte
				if (aq->readPtr > aq->base)
				{
					AUDQUE_MemCpyInterleave (aq->writePtr, 
                                             buf, 
                                             buf2, 
                                             aq->bottom - aq->writePtr,
                                             bitPerSample);
					buf += aq->bottom - aq->writePtr;
					AUDQUE_MemCpyInterleave (aq->base, 
                                             buf, 
                                             buf2, 
                                             aq->readPtr - aq->base - AUDQUE_MARGIN,
                                             bitPerSample);
					copied = (aq->bottom - aq->writePtr) + (aq->readPtr - aq->base - AUDQUE_MARGIN);
				}
				else // aq->readPtr == aq->base
				{
					// don't write base
					AUDQUE_MemCpyInterleave (aq->writePtr, 
                                             buf, 
                                             buf2, 
                                             aq->bottom - aq->writePtr - AUDQUE_MARGIN,
                                             bitPerSample);
					buf += aq->bottom - aq->writePtr - AUDQUE_MARGIN;
					copied = aq->bottom - aq->writePtr - AUDQUE_MARGIN;			
				}
			}
		}
	}
	
	// update writePtr
	if (aq->writePtr + copied < aq->bottom)
	{
		aq->writePtr += copied;
	}
	else
	{
		// wrapped
		aq->writePtr = aq->base + (aq->writePtr + copied - aq->bottom);
	}

	return copied;
}


// ====================================================================================
//
// Function Name: AUDQUE_WriteNoCheck
//
// Description: Write data to an audio queue, without checking overflow
//
// ===================================================================================
UInt32 AUDQUE_WriteNoCheck (AUDQUE_Queue_t *aq,
							UInt8 *data,
							UInt32 size)
{
	memcpy (aq->writePtr, data, size);
	return size;
}


// ====================================================================================
//
// Function Name: AUDQUE_UpdateWritePtr
//
// Description: Move the writePtr to the next block, without handle overflow
//
// ===================================================================================
void AUDQUE_UpdateWritePtr (AUDQUE_Queue_t *aq)
{

	// update writePtr
	if (aq->writePtr + aq->blockSize >= aq->bottom)
	{
		// reach the bottom, set the writePtr to base
		aq->writePtr = aq->base;
	}
	else
	{
		aq->writePtr += aq->blockSize;
	}
}

// ====================================================================================
//
// Function Name: AUDQUE_UpdateWritePtrWithSize
//
// Description: Move the writePtr to currrent plus size, without handle overflow
//
// ===================================================================================
void AUDQUE_UpdateWritePtrWithSize (AUDQUE_Queue_t *aq, UInt32 size)
{
	// update writePtr
	if (aq->writePtr + size >= aq->bottom)
	{
		// reach the bottom, set the writePtr to base
		aq->writePtr = aq->base + (aq->writePtr + size - aq->bottom);
	}
	else
	{
		aq->writePtr += size;
	}
}

// ====================================================================================
//
// Function Name: AUDQUE_Read
//
// Description: Read data from an audio queue, checking underflow
//
// ===================================================================================
UInt32 AUDQUE_Read (AUDQUE_Queue_t *aq,
					UInt8 *data, 
					UInt32 size)
{
	UInt32 copied;
	UInt8	*buf = data;
	
	if (aq->readPtr == aq->writePtr)
	{
		// wait for write start first.
		copied = 0;
	}
	else if (aq->readPtr < aq->writePtr)
	{
		if (aq->readPtr + size <= aq->writePtr)
		{
			// copy all	
			memcpy (buf, aq->readPtr, size);
			copied = size;
		}
		else
		{
			// no enouth data for the read size, just copy available data
			memcpy (buf, aq->readPtr, aq->writePtr - aq->readPtr);
			copied = aq->writePtr - aq->readPtr;
		}	
	}
	else //(aq->readPtr > aq->writePtr)
	{
		if (aq->readPtr + size <= aq->bottom)
		{
			// copy all
			memcpy (buf, aq->readPtr, size);
			copied = size;
		}
		else
		{
			if (aq->base + (aq->readPtr + size - aq->bottom) <= aq->writePtr)
			{
				// copy all, but need 2 steps
				memcpy (buf, aq->readPtr, aq->bottom - aq->readPtr);
				buf += aq->bottom - aq->readPtr;
				memcpy (buf, aq->base, aq->readPtr + size - aq->bottom);
				copied = size;
			}
			else
			{
				// copy part, but also need 2 steps
				memcpy (buf, aq->readPtr, aq->bottom - aq->readPtr);
				buf += aq->bottom - aq->readPtr;
				memcpy (buf, aq->base, aq->writePtr - aq->base);
				copied = (aq->bottom - aq->readPtr) + (aq->writePtr - aq->base);
			}
		}
	}
	
	// update readPtr
	if (aq->readPtr + copied < aq->bottom)
	{
		aq->readPtr += copied;
	}
	else
	{
		// wrapped
		aq->readPtr = aq->base + (aq->readPtr + copied - aq->bottom);
	}

	return copied;
}


// ====================================================================================
//
// Function Name: AUDQUE_ReadNoCheck
//
// Description: Read data from audio queue, without checking underflow
//
// ===================================================================================
UInt32 AUDQUE_ReadNoCheck (AUDQUE_Queue_t *aq, 
						   UInt8 *data, 
						   UInt32 size)
{
	memcpy (data, aq->readPtr, size);
	return size;
}

// ====================================================================================
//
// Function Name: AUDQUE_UpdateReadPtrWithSize
//
// Description: Move the readPtr to current plus the size, don't handle underflow
//
// ===================================================================================
void AUDQUE_UpdateReadPtrWithSize (AUDQUE_Queue_t *aq, UInt32 size)
{
	// update readPtr
	if (aq->readPtr + size >= aq->bottom)
	{
		// reach the bottom, set the readPtr
		aq->readPtr = aq->base + (aq->readPtr + size - aq->bottom);
	}
	else
	{
		aq->readPtr += size;
	}
}

// ====================================================================================
//
// Function Name: AUDQUE_UpdateReadPtr
//
// Description: Move the readPtr to the next block, handle underflow
//
// ===================================================================================
void AUDQUE_UpdateReadPtr (AUDQUE_Queue_t *aq)
{
	// This function deals with underflow. It clears the previous used data when underflow happens.

	UInt8 *nextReadPtr; // the next ReadPtr DMA will set


	// update readPtr
	if (aq->readPtr + aq->blockSize >= aq->bottom)
	{
		// reach the bottom, set the readPtr to base, 
		aq->readPtr = aq->base;
	}
	else
	{
		aq->readPtr += aq->blockSize;
	}


	//set the next readPtr, don't reset next readPtr to base now. 
	nextReadPtr = aq->readPtr + aq->blockSize;

	// deal with underflow now.
	if (aq->readPtr <= aq->writePtr && nextReadPtr > aq->writePtr)
	{
		// Clear the previous used data, need changes to handle non-pcm data
		if (aq->isCleared == FALSE)
		{
			memset (aq->base, 0, aq->bottom - aq->base);
			aq->isCleared = TRUE;
		}

		if (aq->blockNum > 2)
		{
			// we don't do this for ping-pong buffer. It actually causes noise if we do.

			// Reset the writePtr to the next readPtr of DMA interrupt, so writePtr and readPtr
			// will not cross over each other.
			if (nextReadPtr >= aq->bottom)
			{
				nextReadPtr = aq->base;
			}
			aq->writePtr = nextReadPtr;
		}
	}
	else
	{
		aq->isCleared = FALSE;
	}
}

// ====================================================================================
//
// Function Name: AUDQUE_GetReadPtr
//
// Description: Get the read pointer 
//
// ===================================================================================
UInt8* AUDQUE_GetReadPtr (AUDQUE_Queue_t *aq)
{
	return (aq->readPtr);
}

// ====================================================================================
//
// Function Name: AUDQUE_GetWritePtr
//
// Description: Get the write pointer 
//
// ===================================================================================
UInt8* AUDQUE_GetWritePtr (AUDQUE_Queue_t *aq)
{
	return (aq->writePtr);
}

// ====================================================================================
//
// Function Name: AUDQUE_GetLoad
//
// Description: Get the amount (size) of the remaining data in the queue (FIFO).
//
// ===================================================================================
UInt32 AUDQUE_GetLoad (AUDQUE_Queue_t *aq)
{
	UInt32 load;

	if (aq->writePtr < aq->readPtr)
	{
		load = (aq->writePtr - aq->base) + (aq->bottom - aq->readPtr);	
	}
	else
	{
		load = aq->writePtr - aq->readPtr;
	}

	//Log_DebugPrintf(LOGID_AUDIO, "AUDQUE_GetLoad:: aq->writePtr 0x%x, aq->readPtr 0x%x, aq->base 0x%x, aq->bottom = 0x%x, load %d \n",
		//aq->writePtr, aq->readPtr, 
		//aq->base, aq->bottom,
		//load );

	return load;
}


// ====================================================================================
//
// Function Name: AUDQUE_Flush
//
// Description: Flush the queue with 0s
//
// ===================================================================================
void AUDQUE_Flush(AUDQUE_Queue_t *aq)
{
	// Clear the previous used data
	memset (aq->base, 0, aq->bottom - aq->base);

	// reset writePtr and readPtr
 	aq->writePtr = aq->readPtr; 
 	aq->isCleared = TRUE;

}

// ====================================================================================
//
// Function Name: AUDQUE_GetQueueSize
//
// Description: Get the total size of the queue
//
// ===================================================================================
UInt32 AUDQUE_GetQueueSize(AUDQUE_Queue_t *aq)
{
	return aq->blockNum * aq->blockSize;
}

// ====================================================================================
//
// Function Name: AUDQUE_GetSizeReadPtrToBottom
//
// Description: Get the size of from the read pointer to the bottom of the queue
//
// ===================================================================================
UInt32 AUDQUE_GetSizeReadPtrToBottom (AUDQUE_Queue_t *aq)
{
	//Log_DebugPrintf(LOGID_AUDIO, "AUDQUE_GetSizeReadPtrToBottom:: aq->readPtr 0x%x, aq->bottom = 0x%x, (aq->readPtr - aq->bottom) %d \n",
		//aq->readPtr, aq->bottom, (aq->bottom - aq->readPtr) );

    //assert(aq->readPtr <= aq->bottom);

    return(aq->bottom - aq->readPtr);
}

// ====================================================================================
//
// Function Name: AUDQUE_GetSizeWritePtrToBottom
//
// Description: Get the size of from the write pointer to the bottom of the queue
//
// ===================================================================================
UInt32 AUDQUE_GetSizeWritePtrToBottom (AUDQUE_Queue_t *aq)
{
    //assert(aq->writePtr <= aq->bottom);

    return(aq->bottom - aq->writePtr);
}


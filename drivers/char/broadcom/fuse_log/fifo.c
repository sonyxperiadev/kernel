/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#include <linux/string.h>

#include "fifo.h"

/**
 *	Initialize FIFO
 *	@param	pFifo			(inout)	FIFO to be initialized
 *	@param  pFifoBuf		(in)	pointer to FIFO buffer, externally allocated
 *	@param	fifoBufSz		(in)	size of FIFO buffer in bytes
 **/
void BCMLOG_FifoInit( BCMLOG_Fifo_t *pFifo, unsigned char *pFifoBuf, unsigned long fifoBufSz )
{
	pFifo->buf_ptr	= pFifoBuf ;
	pFifo->buf_sz	= fifoBufSz ;
	pFifo->n_avail	= 0 ;
	pFifo->idx_data	= 0 ;
	pFifo->idx_free	= 0 ;
}


/**
 *	Get number of continguous data bytes
 *	@param	pFifo			(inout) the FIFO
 *	@return	Number of available bytes
 **/
unsigned long BCMLOG_FifoGetNumContig( BCMLOG_Fifo_t *pFifo ) 
{
	unsigned long numContig = 0 ;

	if( pFifo->n_avail > 0 )
	{
		if( pFifo->idx_data < pFifo->idx_free )
		{
			numContig = pFifo->n_avail ;
		}
		else
		{
			numContig = pFifo->buf_sz - pFifo->idx_data ;
		}
	}

	return numContig ;
}

/**
 *	Get pointer to first data byte
 *	@param	pFifo			(inout) the FIFO
 *	@return	Pointer to first byte, NULL if no bytes in FIFO
 **/
unsigned char* BCMLOG_FifoGetData( BCMLOG_Fifo_t *pFifo ) 
{
	return pFifo->n_avail > 0 ? pFifo->buf_ptr + pFifo->idx_data : 0 ;
}

/**
 *	Add data to FIFO
 *	@param	pFifo			(inout) the FIFO
 *	@param	pSrcBuf			(in)	pointer to the data to add
 *	@param	srcBufSz		(in)	number of bytes to add
 *	@return	Number of bytes added
 *	@note	If srcBufSz exceeds the number of available FIFO bytes, zero bytes are copied to FIFO
 **/
unsigned long BCMLOG_FifoAdd( BCMLOG_Fifo_t *pFifo, unsigned char *pSrcBuf, unsigned long srcBufSz )
{
	unsigned long numAdded = 0 ;

	if( srcBufSz <= pFifo->buf_sz - pFifo->n_avail )
	{
		numAdded = srcBufSz ;

		if( pFifo->n_avail == 0 )
		{
			memcpy( pFifo->buf_ptr, pSrcBuf, numAdded ) ;
			pFifo->idx_data = 0 ;
			pFifo->idx_free = numAdded ;
			pFifo->n_avail  = numAdded ;
		}

		else
		{
			if( pFifo->idx_data > pFifo->idx_free )
			{
				memcpy( pFifo->buf_ptr+pFifo->idx_free, pSrcBuf, numAdded ) ;
				pFifo->idx_free += numAdded ;
			}
			else
			{
				unsigned long nhi = pFifo->buf_sz - pFifo->idx_free ;
				unsigned long nlo = numAdded - nhi ;

				if( numAdded <= nhi )
				{
					memcpy( pFifo->buf_ptr+pFifo->idx_free, pSrcBuf, numAdded ) ;
					pFifo->idx_free += numAdded ;
				}
				else
				{
					memcpy( pFifo->buf_ptr+pFifo->idx_free, pSrcBuf, nhi ) ;
					memcpy( pFifo->buf_ptr, pSrcBuf+nhi, nlo ) ;
					pFifo->idx_free = nlo ;
				}
			}
			pFifo->n_avail += numAdded ;
		}
			
		pFifo->idx_data %= pFifo->buf_sz ;
		pFifo->idx_free %= pFifo->buf_sz ;
	}
//else 
//should increase a counter for log loss because of out of buffer 
	return numAdded ;
}

/**
 *	Remove up to rmSize oldest bytes from FIFO 
 *	@param	pFifo			(inout)	the FIFO
 *	@param	maxMsgs			(in)	max number of bytes to remove
 *	@return	number of bytes removed
 **/
unsigned long BCMLOG_FifoRemove( BCMLOG_Fifo_t *pFifo, unsigned long rmSize ) 
{
	unsigned long numRemoved = 0 ;

	if( pFifo->n_avail > 0 )
	{
		if( rmSize > pFifo->n_avail )
			numRemoved = pFifo->n_avail ;

		else
			numRemoved = rmSize ;

		pFifo->idx_data += numRemoved ;
		pFifo->idx_data %= pFifo->buf_sz ;

		pFifo->n_avail -= numRemoved ;
	}

	return numRemoved ;
}

unsigned long BCMLOG_FifoNeedOutput( BCMLOG_Fifo_t *pFifo ) 
{
	/* needs output if at least 1/2 buffer is used */
	if (pFifo->n_avail > (pFifo->buf_sz>>1))
		return pFifo->n_avail;
	else
		return 0;
}

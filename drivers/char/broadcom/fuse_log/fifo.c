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
*   software in any way with any other Broadcom software provided under
*   a license other than the GPL, without Broadcom's express prior
*   written consent.
*
****************************************************************************/

#include <linux/string.h>

#include "fifo.h"
#include <linux/kernel.h>
#include "output.h"

/**
 *	Initialize FIFO
 *	@param	pFifo			(inout)	FIFO to be initialized
 *	@param  pFifoBuf		(in)	pointer to FIFO buffer, externally allocated
 *	@param	fifoBufSz		(in)	size of FIFO buffer in bytes
 **/
void BCMLOG_FifoInit(BCMLOG_Fifo_t *pFifo, unsigned char *pFifoBuf,
		     unsigned long fifoBufSz)
{
	pFifo->buf_ptr = pFifoBuf;
	pFifo->buf_sz = fifoBufSz;
	pFifo->idx_read = 0;
	pFifo->idx_write = 0;
}

/**
 *	Get number of continguous data bytes
 *	@param	pFifo			(inout) the FIFO
 *	@return	Number of available bytes
 **/
unsigned long BCMLOG_FifoGetNumContig(BCMLOG_Fifo_t *pFifo)
{
	unsigned long numContig = 0;

	if (pFifo->idx_read <= pFifo->idx_write)
		numContig = pFifo->idx_write - pFifo->idx_read;
	else
		numContig = pFifo->buf_sz - pFifo->idx_read;

	return numContig;
}

/**
 *	Get pointer to first data byte
 *	@param	pFifo			(inout) the FIFO
 *	@return	Pointer to first byte, NULL if no bytes in FIFO
 **/
unsigned char *BCMLOG_FifoGetData(BCMLOG_Fifo_t *pFifo)
{
	return pFifo->buf_ptr + pFifo->idx_read;
}

unsigned long BCMLOG_FifoGetDataSize(BCMLOG_Fifo_t *pFifo)
{
	unsigned long totalsize;

	if (pFifo->idx_read <= pFifo->idx_write)
		totalsize = pFifo->idx_write - pFifo->idx_read;
	else
		totalsize =
		    (pFifo->buf_sz - pFifo->idx_read) + (pFifo->idx_write);

	return totalsize;
}

/**
 *	Get free space log
 *	@param	pFifo			(inout) the FIFO
 *	@return	The size in byte that are free
 **/
unsigned long BCMLOG_FifoGetFreeSize(BCMLOG_Fifo_t *pFifo)
{
	return (pFifo->buf_sz - BCMLOG_FifoGetDataSize(pFifo)) - 1;
}

/**
 *	Add data to FIFO
 *	@param	pFifo			(inout) the FIFO
 *	@param	pSrcBuf			(in)	pointer to the data to add
 *	@param	srcBufSz		(in)	number of bytes to add
 *	@return	Number of bytes added
 *	@note	If srcBufSz exceeds the number of available FIFO bytes, zero bytes are copied to FIFO
 **/
unsigned long BCMLOG_FifoAdd(BCMLOG_Fifo_t *pFifo, unsigned char *pSrcBuf,
			     unsigned long srcBufSz)
{
	unsigned long numAdded = 0;

	if (srcBufSz <= BCMLOG_FifoGetFreeSize(pFifo)) {
		numAdded = srcBufSz;

		if (pFifo->idx_read > pFifo->idx_write) {
			memcpy(pFifo->buf_ptr + pFifo->idx_write, pSrcBuf,
			       numAdded);
			pFifo->idx_write += numAdded;
		} else {
			unsigned long nhi = pFifo->buf_sz - pFifo->idx_write;
			unsigned long nlo = numAdded - nhi;

			if (numAdded <= nhi) {
				memcpy(pFifo->buf_ptr + pFifo->idx_write,
				       pSrcBuf, numAdded);
				pFifo->idx_write += numAdded;
			} else {
				memcpy(pFifo->buf_ptr + pFifo->idx_write,
				       pSrcBuf, nhi);
				memcpy(pFifo->buf_ptr, pSrcBuf + nhi, nlo);
				pFifo->idx_write = nlo;
			}
		}

		pFifo->idx_write %= pFifo->buf_sz;
	}

	return numAdded;
}

/**
 *	Remove up to rmSize oldest bytes from FIFO
 *	@param	pFifo			(inout)	the FIFO
 *	@param	maxMsgs			(in)	max number of bytes to remove
 *	@return	number of bytes removed
 **/

unsigned long BCMLOG_FifoRemove(BCMLOG_Fifo_t *pFifo, unsigned long rmSize)
{
	unsigned long numRemoved = 0;
#ifdef BCMLOG_DEBUG_FLAG
	unsigned long datasize;

	datasize = BCMLOG_FifoGetDataSize(pFifo);
	if (rmSize > datasize) {
		pr_info("BCMLOG avail %d, removed %d", datasize, rmSize);
		numRemoved = datasize;
	} else
#endif
		numRemoved = rmSize;

	pFifo->idx_read += numRemoved;
	pFifo->idx_read %= pFifo->buf_sz;

	return numRemoved;
}

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

#ifndef __BCMLOG_FIFO_H__
#define __BCMLOG_FIFO_H__

/**
 *	FIFO structure
 **/
typedef struct {
	unsigned char		*buf_ptr ;			///< the buffer
	unsigned long		 buf_sz ;			///< buffer size
//	unsigned long		 n_avail ;			///< number of available bytes
	unsigned long		 idx_read ;			///< index of first data byte
	unsigned long		 idx_write ;			///< index of first free byte
}	BCMLOG_Fifo_t ;

/**
 *	Initialize FIFO
 *	@param	pFifo			(inout)	FIFO to be initialized
 *	@param  pFifoBuf		(in)	pointer to FIFO buffer, externally allocated
 *	@param	fifoBufSz		(in)	size of FIFO buffer in bytes
 **/
void BCMLOG_FifoInit( BCMLOG_Fifo_t *pFifo, unsigned char *pFifoBuf, unsigned long fifoBufSz ) ;

/**
 *	Get free space log
 *	@param	pFifo			(inout) the FIFO
 *	@return	The size in byte that are free
 **/
unsigned long BCMLOG_FifoGetFreeSize( BCMLOG_Fifo_t *pFifo ) ;

/**
 *	Get number of continguous data bytes
 *	@param	pFifo			(inout) the FIFO
 *	@return	Number of available bytes
 **/
unsigned long BCMLOG_FifoGetNumContig( BCMLOG_Fifo_t *pFifo ) ;

/**
 *	Get pointer to first data byte
 *	@param	pFifo			(inout) the FIFO
 *	@return	Pointer to first byte, NULL if no bytes in FIFO
 **/
unsigned char* BCMLOG_FifoGetData( BCMLOG_Fifo_t *pFifo ) ;

/**
 *	Add data to FIFO
 *	@param	pFifo			(inout) the FIFO
 *	@param	pSrcBuf			(in)	pointer to the data to add
 *	@param	srcBufSz		(in)	number of bytes to add
 *	@return	Number of bytes added
 *	@note	If srcBufSz exceeds the number of available FIFO bytes, zero bytes are copied to FIFO
 **/
unsigned long BCMLOG_FifoAdd( BCMLOG_Fifo_t *pFifo, unsigned char *pSrcBuf, unsigned long srcBufSz ) ;

/**
 *	Remove up to rmSize oldest bytes from FIFO 
 *	@param	pFifo			(inout)	the FIFO
 *	@param	maxMsgs			(in)	max number of bytes to remove
 *	@return	number of bytes removed
 **/
unsigned long BCMLOG_FifoRemove( BCMLOG_Fifo_t *pFifo, unsigned long rmSize )  ;

unsigned long BCMLOG_FifoGetDataSize( BCMLOG_Fifo_t *pFifo ) ;


#endif // __BCMLOG_FIFO_H__


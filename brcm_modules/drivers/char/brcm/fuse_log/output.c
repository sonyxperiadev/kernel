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


#include <linux/kernel.h>
#include <linux/brcm_console.h>

#include "fifo.h"
#include "output.h"

/**
 *	flow control state for RNDIS, set/reset by flow control callbacks
 **/
static char g_netconsole_on = 0 ;

/**
 *	flow control callback for RNDIS (start flow), called when RNDIS
 *	available to transport data
 **/
static int netconsole_start_cb( void )
{
	g_netconsole_on = 1 ;
	return 0 ;
}

/**
 *	flow control callback for RNDIS (stop flow), called when RNDIS
 *	available to transport data
 **/
static int netconsole_stop_cb( void )
{
	g_netconsole_on = 0 ;
	return 0 ;
}

/**
 *	flow control callback structure for RNDIS
 **/
static struct brcm_netconsole_callbacks _cb = 
{
	.start = netconsole_start_cb,
	.stop  = netconsole_stop_cb 
} ;

extern char brcm_netconsole_register_callbacks(struct brcm_netconsole_callbacks *_cb) ;

/**
 *	Write to RNDIS 
 *	@param	pBytes			(in)	ptr to buffer
 *	@param	nBytes			(out)	number of bytes
 *	@return	number of bytes transfered
 **/
static unsigned long Write_RNDIS( unsigned char *pBytes, unsigned long nBytes )
{
#ifdef CONFIG_USB_ETH_RNDIS
	unsigned long bytes_written = 0 ;
	
	if( g_netconsole_on )
	{
		while( nBytes > 0 )
		{
			int nWrite = (int)nBytes ;
			int rndis_rc ;

			if( nWrite > BCMLOG_OUTPUT_XFER_MAXBYTES )
				nWrite = BCMLOG_OUTPUT_XFER_MAXBYTES ;

			rndis_rc = brcm_klogging( pBytes, nWrite ) ;
		
			if( rndis_rc > 0 )
			{
				bytes_written += rndis_rc ;
				nBytes -= rndis_rc ;
				pBytes += rndis_rc ;
			}
			
			else
				nBytes = 0 ;
		}
	}

	return bytes_written ;

#else // CONFIG_USB_ETH_RNDIS undefined
	return 0;
#endif

}


/**
 *	Write to target 
 *	@param	pBytes			(in)	ptr to buffer
 *	@param	nBytes			(out)	number of bytes
 *	@return	number of bytes transfered
 **/
static unsigned long Write( unsigned char *pBytes, unsigned long nBytes )
{
	return Write_RNDIS( pBytes, nBytes ) ;
}


/**
 *	Drain FIFO; send bytes to target device 
 *	@param	pFifo			(in)	ptr to FIFO
 *	@return	number of bytes remaining in FIFO
 **/
static unsigned long DrainFifo( BCMLOG_Fifo_t *pFifo )
{
	unsigned long bytes_in_fifo = BCMLOG_FifoGetNumContig( pFifo ) ;

	while( bytes_in_fifo > 0 )
	{
		unsigned long bytes_written = 0 ;

		bytes_written = Write( BCMLOG_FifoGetData( pFifo ), bytes_in_fifo ) ;
	
		if( bytes_written > 0 )
			BCMLOG_FifoRemove( pFifo, bytes_written ) ;
		
		else 
			break ;

		bytes_in_fifo = BCMLOG_FifoGetNumContig( pFifo ) ;
	}

	return bytes_in_fifo ;
}

/**
 *	Output bytes to host
 *	@param	pFifo				(inout)	FIFO of messages awaiting output
 *	@param  pBytes				(in)	pointer to user buffer
 *	@param	nBytes				(in)	number of bytes
 **/
void BCMLOG_Output( BCMLOG_Fifo_t *pFifo, unsigned char *pBytes, unsigned long nBytes )
{
	unsigned long bytes_written = 0 ;

	if( pFifo->n_avail >= BCMLOG_OUTPUT_XFER_MINBYTES || nBytes >= BCMLOG_OUTPUT_XFER_MINBYTES ) 
	{
		unsigned long bytes_in_fifo = DrainFifo( pFifo ) ;

		if( bytes_in_fifo == 0 ) 
			if( nBytes >= BCMLOG_OUTPUT_XFER_MINBYTES )
				bytes_written = Write( pBytes, nBytes ) ;
	}

	if( bytes_written < nBytes )
		BCMLOG_FifoAdd( pFifo, pBytes+bytes_written, nBytes-bytes_written ) ;
}

/**
 *	Initialize output module
 **/
void BCMLOG_OutputInit( void )
{
	/*
	 *	register flow control callback functions
	 */
//FixMe -- guojin
//	g_netconsole_on = brcm_netconsole_register_callbacks( &_cb ) ;
}

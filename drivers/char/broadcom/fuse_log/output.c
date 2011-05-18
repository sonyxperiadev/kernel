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
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/brcm_console.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/rtc.h>

#include "bcmlog.h"
#include "fifo.h"
#include "output.h"

/*
 *	extern declarations
 */
extern char brcm_netconsole_register_callbacks(struct brcm_netconsole_callbacks *_cb) ;

#ifdef CONFIG_USB_ETH_RNDIS

/*
 *	forward declarations
 */
static int netconsole_start_cb( void ) ;
static int netconsole_stop_cb( void ) ;

/**
 *	local vars
 **/
static char g_netconsole_on = 0 ;				//	flow control state for RNDIS, set/reset by flow control callbacks

#endif

#define BCMLOG_OUTPUT_FIFO_MAX_BYTES  65536		
static BCMLOG_Fifo_t g_fifo ;					//	output fifo


typedef struct 
{
	struct work_struct	wq ;	
	int					outdev; 
	int					prev_outdev; 
	struct file			*file ;
	u8					busy ;
}	WriteToLogDevParms_t ;

static WriteToLogDevParms_t g_devWrParms =		//	worker thread vars
{
	.file			= 0,
	.busy			= 0,
	.prev_outdev	= BCMLOG_OUTDEV_NONE,
	.outdev			= BCMLOG_OUTDEV_NONE,
} ;

#ifdef CONFIG_USB_ETH_RNDIS

static struct brcm_netconsole_callbacks _cb =	//	RNDIS flow control callbacks
{
	.start = netconsole_start_cb,
	.stop  = netconsole_stop_cb 
} ;
/**
 *	flow control callback for RNDIS (start flow), called when RNDIS
 *	available to transport data
 **/
static int netconsole_start_cb( void )
{
	g_netconsole_on = 1 ;
	printk( "BCMLOG: RNDIS netconsole started\n" ) ;
	return 0 ;
}

/**
 *	flow control callback for RNDIS (stop flow), called when RNDIS
 *	available to transport data
 **/
static int netconsole_stop_cb( void )
{
	g_netconsole_on = 0 ;
	printk( "BCMLOG: RNDIS netconsole stopped\n" ) ;
	return 0 ;
}
#endif

/*
 *	Create a file name for logging, based on system time.
 */
static void GetLogFileName( char *buf, char *rootdev, int size )
{

    struct timespec ts;
    struct rtc_time tm;

    getnstimeofday(&ts);
    rtc_time_to_tm(ts.tv_sec, &tm);
	
    snprintf(buf, size, 
            "%slog-%d_%02d_%02d_%02d_%02d_%02d.bin",
            rootdev, 
			tm.tm_year + 1900, 
			tm.tm_mon + 1, 
			tm.tm_mday,
            tm.tm_hour, 
			tm.tm_min, 
			tm.tm_sec);
}

#define MAX_FS_WRITE_SIZE 16384
#define MAX_LOG_PATHNAME     64

/*
 *	Write log to file system
 */
static void WriteToLogDev_SDCARD( void )
{
	mm_segment_t oldfs ;
	char		 fname[MAX_LOG_PATHNAME] ;

	oldfs = get_fs( ) ;
	set_fs (KERNEL_DS);

	/*
	 *	Attempt to open log file, if not already open
	 */
	if( !g_devWrParms.file )
	{
		GetLogFileName( fname, "/sdcard/", sizeof( fname ) ) ;

		g_devWrParms.file = filp_open( fname, O_WRONLY|O_TRUNC|O_CREAT, 0666); 

		if( IS_ERR( g_devWrParms.file ) )
			g_devWrParms.file = 0 ;
	}

	/*
	 *	If log file open start logging to it
	 */
	if( g_devWrParms.file )
	{
		u32 nFifo ;

		do
		{
			int nWrite ;

			nFifo = BCMLOG_FifoGetNumContig( &g_fifo ) ;
			
			if( nFifo > MAX_FS_WRITE_SIZE )
				nFifo = MAX_FS_WRITE_SIZE ;

			if( nFifo > 0 )
			{
				nWrite = g_devWrParms.file->f_op->write( g_devWrParms.file, 
					BCMLOG_FifoGetData( &g_fifo ), nFifo, &g_devWrParms.file->f_pos ) ;
				
				if( nWrite > 0 )
					BCMLOG_FifoRemove( &g_fifo, nWrite ) ;

				if( nWrite < nFifo )
				{
					nFifo = 0 ;
					filp_close( g_devWrParms.file ,NULL );
					g_devWrParms.file = 0 ;
					/*
					 * in the case of sdcard, redirect output
					 * to 'null' as the card is full or removed at
					 * this point
					 */
					 g_devWrParms.outdev = BCMLOG_OUTDEV_NONE ;
				}
			}
		} while( nFifo > 0 );
	}

	set_fs( oldfs ) ;
}

static void WriteToLogDev_RNDIS( void )
{
#ifdef CONFIG_BRCM_NETCONSOLE

	if( g_netconsole_on )
	{
		u32 nFifo ;

		do
		{
			int nWrite ;

			nFifo = BCMLOG_FifoGetNumContig( &g_fifo ) ;
			
			if( nFifo > MAX_FS_WRITE_SIZE )
				nFifo = MAX_FS_WRITE_SIZE ;

			if( nFifo > 0 )
			{
				nWrite = brcm_klogging( BCMLOG_FifoGetData( &g_fifo ), nFifo ) ;
				
				if( nWrite > 0 )
					BCMLOG_FifoRemove( &g_fifo, nWrite ) ;
				
				if( nWrite < nFifo )
					nFifo = 0 ;
			}
		} while( nFifo > 0 );
	}
#endif	
}

static void WriteToLogDev_UART( void )
{
	mm_segment_t oldfs ;

	oldfs = get_fs( ) ;
	set_fs (KERNEL_DS);

	/*
	 *	Attempt to open log file, if not already open
	 */

	if( !g_devWrParms.file )
	{
		g_devWrParms.file = filp_open( "/dev/ttyS0", O_WRONLY|O_TRUNC|O_CREAT, 0666); 

		if( IS_ERR( g_devWrParms.file ) )
		{
			g_devWrParms.file = 0 ;
		}
	}

	/*
	 *	If log file open start logging to it
	 */
	if( g_devWrParms.file )
	{
		u32 nFifo ;

		do
		{
			int nWrite ;

			nFifo = BCMLOG_FifoGetNumContig( &g_fifo ) ;
			
			if( nFifo > MAX_FS_WRITE_SIZE )
				nFifo = MAX_FS_WRITE_SIZE ;

			if( nFifo > 0 )
			{
				nWrite = g_devWrParms.file->f_op->write( g_devWrParms.file, 
					BCMLOG_FifoGetData( &g_fifo ), nFifo, &g_devWrParms.file->f_pos ) ;
				
				if( nWrite > 0 )
					BCMLOG_FifoRemove( &g_fifo, nWrite ) ;

				if( nWrite < nFifo )
				{
					nFifo = 0 ;
					filp_close( g_devWrParms.file ,NULL );
					g_devWrParms.file = 0 ;
				}
			}
		} while( nFifo > 0 );
	}

	set_fs( oldfs ) ;
}

static void WriteToLogDev_ACM( void )
{
	mm_segment_t oldfs ;

	oldfs = get_fs( ) ;
	set_fs (KERNEL_DS);

	/*
	 *	Attempt to open log file, if not already open
	 */
	if( !g_devWrParms.file )
	{
		g_devWrParms.file = filp_open( "/dev/ttyGS1", O_WRONLY|O_TRUNC|O_CREAT, 0666); 

		if( IS_ERR( g_devWrParms.file ) )
			g_devWrParms.file = 0 ;
	}

	/*
	 *	If log file open start logging to it
	 */
	if( g_devWrParms.file )
	{
		u32 nFifo ;

		do
		{
			int nWrite ;

			nFifo = BCMLOG_FifoGetNumContig( &g_fifo ) ;
			
			if( nFifo > MAX_FS_WRITE_SIZE )
				nFifo = MAX_FS_WRITE_SIZE ;

			if( nFifo > 0 )
			{
				nWrite = g_devWrParms.file->f_op->write( g_devWrParms.file, 
					BCMLOG_FifoGetData( &g_fifo ), nFifo, &g_devWrParms.file->f_pos ) ;
				
				if( nWrite > 0 )
					BCMLOG_FifoRemove( &g_fifo, nWrite ) ;

				if( nWrite < nFifo )
				{
					nFifo = 0 ;
					filp_close( g_devWrParms.file ,NULL );
					g_devWrParms.file = 0 ;
				}
			}
		} while( nFifo > 0 );
	}

	set_fs( oldfs ) ;
}

static void WriteToLogDev( struct work_struct *work )
{
	if( g_devWrParms.prev_outdev != g_devWrParms.outdev )
	{
		if( g_devWrParms.file )
		{
			filp_close( g_devWrParms.file ,NULL );
			g_devWrParms.file = 0 ;
		}
	}
				
	switch( g_devWrParms.outdev )
	{
	case BCMLOG_OUTDEV_SDCARD:
		WriteToLogDev_SDCARD( ) ;
		break ;
	
	case BCMLOG_OUTDEV_RNDIS:
		WriteToLogDev_RNDIS( ) ;
		break ;

	case BCMLOG_OUTDEV_UART:
		WriteToLogDev_UART( ) ;
		break ;

	case BCMLOG_OUTDEV_ACM:
		WriteToLogDev_ACM( ) ;
		break ;

	default:
		break ;
	}

	g_devWrParms.busy = 0 ;
}


/**
 *	Output bytes to host
 *	@param  pBytes				(in)	pointer to user buffer
 *	@param	nBytes				(in)	number of bytes
 **/
void BCMLOG_Output( unsigned char *pBytes, unsigned long nBytes )
{
	/*
	 *	Add to FIFO.  If unable to add (FIFO full) then discard the message; there's nothing
	 *	that can be done to save it.
	 */
	BCMLOG_FifoAdd( &g_fifo, pBytes, nBytes ) ;

	/*
	 *	If output worker thread is not busy then set up arguments and schedule it; otherwise
	 *	we'll retry on the next output to run the thread.
	 */
	if( !g_devWrParms.busy /*&&BCMLOG_FifoNeedOutput( &g_fifo )*/ )
	{
		g_devWrParms.busy   = 1; 
		g_devWrParms.prev_outdev = g_devWrParms.outdev ;
		g_devWrParms.outdev = BCMLOG_GetRunlogDevice( ) ;
		schedule_work( &g_devWrParms.wq ) ;
	}
}

/**
 *	Initialize output module
 **/
int BCMLOG_OutputInit( void )
{
	unsigned char *fifobuf ;

	fifobuf = kmalloc( BCMLOG_OUTPUT_FIFO_MAX_BYTES, GFP_KERNEL ) ;

	if( !fifobuf )
        return -1 ;

	BCMLOG_FifoInit( &g_fifo, fifobuf, BCMLOG_OUTPUT_FIFO_MAX_BYTES ) ;

	g_devWrParms.busy = 0 ;
	g_devWrParms.file = 0 ;


	INIT_WORK( &g_devWrParms.wq, WriteToLogDev ) ;
#ifdef CONFIG_BRCM_NETCONSOLE

	/*
	 *	register flow control callback functions
	 */
	g_netconsole_on = brcm_netconsole_register_callbacks( &_cb ) ;

#endif
	return 0 ;
}

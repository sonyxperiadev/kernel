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


/**
 *
 *  @file   bcmlog.h
 *
 *  @brief  Interface to the Broadcom logging driver.
 *
 *	@note	Use code will access driver through an API layer and typically should not
 *			require use of these symbols.
 *
 **/

#ifndef __BCMLOG_H__
#define __BCMLOG_H__

#include <linux/file.h>

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

/**
 *	Log IDs are in the range 0 to 8191, partitioned as follows:
 *
 *		   0 - 1023 :	reserved to Broadcom
 *		1024 - 32767:	available for custom applications
 *
 *	The following Log IDs are reserved to Broadcom for Android-specific purposes:
 *
 *		1000 - 1023 :	reserved to Broadcom (Android-specific)
 *
 *	See also logapi.h
 *
 **/
 
#define BCMLOG_MAX_LOG_ID		32767		///<	maximum LOG ID
#define BCMLOG_LOG_ULOGGING_ID	1000		///<	Android unified logging (e.g., printk logging)
#define BCMLOG_ANDROID_KRIL_BASIC	1001		///<Android KRIL logging for basic information
#define BCMLOG_ANDROID_KRIL_DETAIL	1002		///<Android KRIL logging for detail information

/**
 *	miscellaneous definitions
 **/
#define BCMLOG_MODULE_NAME		"bcm_log"	///<	public module name

/**
 *
 *  ioctl commands
 *
 **/
#define BCMLOG_IOC_LOGSTR		102			///<	log a string
#define BCMLOG_IOC_ENABLE		103			///<	get or set enable/disable local message logging based on ID
#define BCMLOG_IOC_LOGSIGNAL	104			///<	log a signal
#define BCMLOG_IOC_SAVECONFIG	105			///<	save configuration to PS
#define BCMLOG_IOC_RESETCONFIG	106			///<	reset configuration in PS to default

/**
 *  for ioctl cmd BCMLOG_IOC_LOGSTR, a variable of this type
 *	is passed as the 'arg' to ioctl()
 **/
typedef struct
{
	char*				str ;			///<	pointer to string buffer
	int					size ;			///<	size of buffer including terminating null
	unsigned short      sender;         ///<    ID of sending task
}	BCMLOG_IoctlLogStr_t ;

/**
 *  for ioctl cmd BCMLOG_IOC_ENABLE a variable of this type 
 *	is passed as the 'arg' to ioctl()
 **/
typedef struct
{
	unsigned short		id ;			///<	log ID
	unsigned char		set ;			///<	if set != 0 set (write) the enable/disable status;
										///<	if set == 0 return (read) the enable/disable status
	unsigned char		enable ;		///<	if set != 0 ID logging is enabled based on this member
										///<	if set == 0 ID logging this member contains
										///<	current enabled/disabled status on return from 
										///<	ioctl call
}	BCMLOG_IoctlEnable_t ;

/**
 *  for ioctl cmd BCMLOG_IOC_LOGSIGNAL a variable of this type 
 *	is passed as the 'arg' to ioctl()
 **/
typedef struct
{
	unsigned int		sigCode ;		///<	binary signal code
	void*               sigPtr;         ///<    pointer to signal buffer
	unsigned int        sigBufSize;     ///<    size of signal buffer in bytes
	unsigned short      state;          ///<    receiving task's state information (optional)
	unsigned short      sender;         ///<    ID of sending task
}	BCMLOG_IoctlLogSignal_t ;

/**
 *	Log null terminated string.
 *
 *	@param	inLogString		(in)	null terminated string to be logged
 *	@param	inSender		(in)	ID of sending task
 *
**/
void BCMLOG_LogString( const char* inLogString,
                        unsigned short inSender  );

/**
    Log a formatted list of arguments
      @param            inSender (in) ID of the source module
      @param            *fmt (in) a format string as printf()
      ...               (in) a list of arguments as printf()
	  @note				if the Broadcom Unified Logging option is not
						enabled then printk is called
**/   
//#ifdef CONFIG_BRCM_UNIFIED_LOGGING
void BCMLOG_Printf( unsigned short inSender, char* fmt, ...) ;
//#else
//#define BCMLOG_Printf( inSender, ... ) printk( __VA_ARGS__ )
//#endif

/**
 *	Log binary signal.
 *
 *	@param	inSigCode		(in)	signal code
 *	@param	inSigBuf		(in)	pointer to signal buffer
 *	@param	inSigBufSize	(in)	size of signal buffer in bytes
 *	@param	inState 		(in)	receiving task's state information (optional)
 *	@param	inSender		(in)	ID of sending task
 *
**/
void BCMLOG_LogSignal( unsigned int inSigCode, 
                        void* inSigBuf,
                        unsigned int inSigBufSize,
                        unsigned short inState,
                        unsigned short inSender  );

/**
 *	Handle a CP log message from IPC server.
 *
 *	@param	buf		(in)	message buffer
 *	@param	size	(in)	message length
 *	@note	does not free the IPC message buffer
 **/
void BCMLOG_HandleCpLogMsg( unsigned char *buf, int size ) ;

/**
 *	Prepare to handle CP crash dump. During CP crash
 *  dump, all other logging requests are ignored.
 **/
void BCMLOG_StartCpCrashDump( struct file* inDumpFile );

/**
 *	Done with CP crash dump. Normal logging is resumed.
 **/
void BCMLOG_EndCpCrashDump( void );

/**
 *	Handle CP crash dump data from IPC server.
 *
 *	@param	buf		(in)	message buffer
 *	@param	size	(in)	message length
 *	@note	does not free the IPC message buffer
 **/
void BCMLOG_HandleCpCrashDumpData( const char *buf, int size );

/**
 *	Handle memory dump data from CP crash.
 *
 *	@param	address	(in)	virtual address of memory to dump
 *	@param	size	(in)	size in bytes of buf
 *	
 **/
void BCMLOG_HandleCpCrashMemDumpData( const char* address, int size );

/**
 *	Log null terminated string during crash dump.
 *
 *	@param	inLogString		(in)	null terminated string to be logged
 *
**/
void BCMLOG_LogCPCrashDumpString( const char* inLogString );

/**
 *	Logging output devices.  
 **/
#define BCMLOG_OUTDEV_NONE		0		///<	undefined or disabled
#define BCMLOG_OUTDEV_PANIC		1		///<	panic partition / MTD
#define BCMLOG_OUTDEV_SDCARD		2		///<	sdcard
#define BCMLOG_OUTDEV_RNDIS		3		///<	MTT/RNDIS
#define BCMLOG_OUTDEV_UART		4		///<	UART
#define BCMLOG_OUTDEV_ACM		5		///<	ACM/OBEX
#define BCMLOG_OUTDEV_STM               6		///<    STM	

/**
 *	Get runtime log device
 **/
int BCMLOG_GetRunlogDevice( void ) ;
int BCMLOG_GetCpCrashLogDevice( void ) ;
int BCMLOG_GetApCrashLogDevice( void ) ;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __BCMLOG_H__

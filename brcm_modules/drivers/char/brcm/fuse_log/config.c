//
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

#include "config.h"

static struct delayed_work g_load_config_wq ;			///< work queue for reading config file
static BCMLOG_Config_t g_config ;						///< log configuration

/**
 *	rudimentary parsers use to handle data from /proc/BCMLOG_CONFIG_PROC_FILE
 **/
#define IsSpace( c )     ( (c) == ' ' || (c) == '\t' || (c) == '\n' )

/**
 *	Find token in string. Tokens are characters delimited by spaces.
 *	@parm str (in) string to search
 *	@parm tkn (in) token to match
 *  @return pointer to char one past token if match; 0 if no match
 **/
unsigned char* FindTkn( unsigned char *str, unsigned char *tkn ) 
{
	unsigned short match = 1 ;

	while( *str && IsSpace(*str) )
		++str ;

	while( *str && *tkn )
	{
		if( *str != *tkn )
		{
			match = 0 ;
			break ;
		}
		++str ;
		++tkn ;
	}

	if( match && *str && !IsSpace(*str) )
		match = 0 ;

	return match ? str : 0 ;
}

/**
 *	Accept string from /proc/BCMLOG_CONFIG_PROC_FILE to control logging configuration.
 *	The following strings are recognized:
 *
 *		String			Action
 *		cpdump=sdcard	Set CP dump device to sd card
 *		cpdump=bmtt		Set CP dump device to Broadcom MTT (over RNDIS)
 *		cpdump=mtd		Set CP dump device to MTD (flash partition)
**/
static ssize_t brcm_logcfg_write(struct file *file, const char *buffer, unsigned long count, void *data)
{	
	BCMLOG_CpCrashDumpDevice_t old_cp_crashdump_dev = g_config.cp_crashdump_dev ;
	BCMLOG_CpCrashDumpDevice_t new_cp_crashdump_dev = old_cp_crashdump_dev ;
	
	if( FindTkn( buffer, "cpdump=sdcard" ) ) 
		new_cp_crashdump_dev = BCMLOG_CPCRASH_SDCARD ;

	else if( FindTkn( buffer, "cpdump=bmtt" ) ) 
		new_cp_crashdump_dev = BCMLOG_CPCRASH_BMTT ;

#ifdef CONFIG_BRCM_CP_CRASH_DUMP
	else if( FindTkn( buffer, "cpdump=mtd" ) )
		new_cp_crashdump_dev = BCMLOG_CPCRASH_MTD ;
#endif

	if( new_cp_crashdump_dev != old_cp_crashdump_dev )
	{
		g_config.cp_crashdump_dev = new_cp_crashdump_dev ;
		printk( "BCMLOG: CONFIGURATION CHANGED FROM %d TO %d\n", (int)old_cp_crashdump_dev, (int)new_cp_crashdump_dev ) ;
		BCMLOG_SaveConfig( 1 ) ;
	}

	return count ;
}

static struct file_operations brcm_logcfg_ops = {
  .owner = THIS_MODULE,
  .write = brcm_logcfg_write,
};

/**
 *	Load configuration from persistent storage
 **/
static int LoadConfigFromPersistentStorage( void )
{
	mm_segment_t oldfs ;
	struct file* config_file ;
	int rc = 0 ;

	oldfs = get_fs( ) ;
	set_fs (KERNEL_DS);

	config_file = filp_open( BCMLOG_CONFIG_PS_FILE, O_RDONLY, 0 ); 

	if( IS_ERR( config_file ) )
	{
		rc = -1 ;
	}

	else
	{
		if( sizeof(BCMLOG_Config_t)!= config_file->f_op->read( config_file, (void*)&g_config, 
				sizeof(BCMLOG_Config_t), &config_file->f_pos ) )
		{
			rc = -1 ;
		}

		filp_close( config_file ,NULL );
	}
		
	set_fs( oldfs ) ;

	return rc ;
}

/**
 *	Work thread to read configuration file.  Called at driver
 *	initialization, will reschedule itself periodically until
 *	configuration is successfully loaded, up to predetermined
 *	number of attempts.  Delay is necessary to wait until
 *	file system is available.
 *   
 *	@param	(in)	ptr to work structure
 *	@note
 *		Function prototype as required by INIT_DELAYED_WORK macro.
 *	@note
 *		There is a one-second delay between each attemt.  It was 
 *		determined emperically that 5 - 6 seconds is about
 *		the time required for FS to come up, so we'll give 
 *		it up to 10 seconds.
 **/
static void try_load_config( struct work_struct *work )
{
	static int tries = 10 ;

	if( --tries > 0 )
		if( LoadConfigFromPersistentStorage( ) < 0 )
			schedule_delayed_work( &g_load_config_wq, 1*HZ ) ;

}

/**
 *	Save or reset configuration persistent storage
 *	@param		saveFlag [in] if nonzero save configuration else reset
 *				configuration
 *	@return		0 on success, -1 on error
 **/
int BCMLOG_SaveConfig( int saveFlag )
{
	mm_segment_t oldfs ;
	struct file* config_file ;
	int rc = 0 ;

	oldfs = get_fs( ) ;
	set_fs (KERNEL_DS);

	config_file = filp_open( BCMLOG_CONFIG_PS_FILE, O_WRONLY|O_TRUNC|O_CREAT, 666); 

	if( IS_ERR( config_file ) )
		rc = -1 ;

	else
	{
		if( saveFlag )
			if( sizeof(BCMLOG_Config_t) != config_file->f_op->write( config_file, (void*)&g_config, 
				sizeof(BCMLOG_Config_t), &config_file->f_pos ) )
				rc = -1 ;

		// if !saveFlag the file is truncated to zero bytes, invalidating the configuration
		filp_close( config_file ,NULL );
	}
		
	set_fs( oldfs ) ;

	return rc ;
}

/**
 *	Enable or disable log ID
 *	@param		inLogId (in) log id
 *	@param		inEnable (in) nonzero to enable; zero to disable log ID
 **/
void BCMLOG_EnableLogId( unsigned short inLogId, int inEnable )
{
	unsigned long index ;
	unsigned long bit ;
	
	if( inLogId < BCMLOG_MAX_LOG_ID )
	{
		index = inLogId / BITS_PER_LONG ;
		bit   = inLogId % BITS_PER_LONG ;
						
		if( inEnable )
			g_config.id_enable[index] |= ( 1 << bit ) ;
		else
			g_config.id_enable[index] &= ~( 1 << bit ) ;
	}
}

/**
 *	Check if log id (sender) is enabled
 *	@param		inLogId (in) log id
 *	@return		1 if enabled; 0 if not enabled or invalid ID
 **/
int BCMLOG_LogIdIsEnabled( unsigned short inLogId )
{
	unsigned long index ;
	unsigned long bit ;
	
	if( inLogId >= BCMLOG_MAX_LOG_ID )
	{
		return 0 ;
	}
	
	index = inLogId / BITS_PER_LONG ;
	bit   = inLogId % BITS_PER_LONG ;
					
	return g_config.id_enable[index] & ( 1 << bit ) ? 1 : 0 ;
}

/**
 *	Initialize logging configuration.  Schedules a work thread to
 *	load the configuration file once the file system is readable.
 **/
void BCMLOG_InitConfig( void ) 
{
	/*
	 *	/proc file accepts strings from scripts to configure logging
	 */
	struct proc_dir_entry *pentry_brcm_logcfg = proc_create( BCMLOG_CONFIG_PROC_FILE, 0666, NULL, &brcm_logcfg_ops);

	/*
	 *	disable all AP logging (CP logging is handled by CP) [MobC00126731]
	 */
	memset( &g_config, 0x00, sizeof( g_config ) ) ;
	
	/*
	 *	set default CP crash dump device
	 */
#ifdef CONFIG_BRCM_CP_CRASH_DUMP
	g_config.cp_crashdump_dev = BCMLOG_CPCRASH_MTD ;
#else
	g_config.cp_crashdump_dev = BCMLOG_CPCRASH_SDCARD ;
#endif
	
	/*
	 *	start a thread to attempt to load configuration when filesystem ready
	 */
	INIT_DELAYED_WORK( &g_load_config_wq, try_load_config ) ;
	
	schedule_delayed_work( &g_load_config_wq, 1*HZ ) ;
}

BCMLOG_CpCrashDumpDevice_t BCMLOG_GetCpCrashDumpDevice( void ) 
{
	return g_config.cp_crashdump_dev ;
}


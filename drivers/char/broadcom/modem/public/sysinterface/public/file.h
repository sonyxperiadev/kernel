//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   file.h
*
*   @brief  This file defines structures and prototypes for File System
*
****************************************************************************/
/**

*   @defgroup   FFSGroup   File System
*   @ingroup    FileSystemGroup
*
*   @brief      This group defines the interfaces to the file system.
*
********************************************************************************************/
/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/

#ifndef __FILE_H__
#define __FILE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
                                Constant
*****************************************************************************/
#define FS_INVALID		-1		///< Invalid file system
#define	FS_OK	 0				///< File system OK
#define	FS_EOF	 1				///< End of File
#define FS_ERROR 2				///< File system Error

/*
 * FS_Seek() flags
 */
#define	FS_SEEK_SET		0		///< seek from beginning of file
#define FS_SEEK_CUR		1		///< seek from current position
#define FS_SEEK_END		2		///< seek from seek from end-of-file

/*
 * FS_Access() flags
 */
#define FS_ACCESS_RD	4		///< Read access flag.
#define FS_ACCESS_WR	2		///< Write access flag.
#define FS_EXISTENCE	0		///< Existance flag.

#define FS_DIR_FLAG		0x4000 //This should be the same value as S_DIR which is defined in posix.h

/*
 * FS_ChangeMode() flags
 */
#define FS_S_IREAD         000400	///< Read permission flag.
#define FS_S_IWRITE        000200	///< Write permission flag.
#define FS_S_IEXECUTE	   000100	///< Execute permission flag.

/*****************************************************************************
                               Data Structure
*****************************************************************************/

typedef void FSFILE;

#define FS_FILENAME_MAX 128

#define FS_FOPEN_MAX 64	

typedef struct dirent *FS_INFO_t;

typedef	void*		HDIR;	///< Directory Handle

/// File system status.
struct FS_STAT_t	// Be sure target and simulator work for any change.
{
	UInt16	fs_mode;	///< The file's permission bits and should be a bit-wise OR of one or more of FS_S_IREAD, FS_S_IWRITE, and FS_DIR_FLAG.
	UInt32	fs_size;	///< File size
	UInt32	fs_atime;	///< Time of last access of file.
	UInt32	fs_mtime;	///< Time of last modification of file.
	UInt32	fs_ctime;	///< Time of creation of file.
};

/**
	File System Drive Type
**/
typedef enum
{
	FS_DRIVE_FLASH,			///< Internal flash (FFS format)
	FS_DRIVE_SDCARD,		///< SD card
#ifdef UDISK	
	FS_DRIVE_UDISK,
#endif	
#ifdef ALLOW_EXTERNAL_USB_DISK
	FS_DRIVE_EXT_USB_DISK,
#endif
	FS_DRIVE_RAMDISK,		///< RAMDISK
	FS_NUM_DRIVES			///< Max number of drives supported
} FS_Drive_t; 

/**
	File System Drive Type (legacy)
**/
#define FS_DRIVE_0		FS_DRIVE_FLASH		///< Drive 0	
#define FS_DRIVE_1		FS_DRIVE_SDCARD		///< Drive 1
#ifdef UDISK
#define FS_DRIVE_2		FS_DRIVE_UDISK		///< Drive 2
#endif
#ifdef ALLOW_EXTERNAL_USB_DISK
#define FS_DRIVE_3		FS_DRIVE_EXT_USB_DISK
#endif
#define NUM_FS_DRIVE	FS_NUM_DRIVES		///< Max number drives
/**
	Content of MSG_FS_VOLUME_USE_IND message broadcast when volume utilization
	exceeds a threshold
**/
typedef struct {
    UInt8      pct ;		///< percent used volume capacity
    FS_Drive_t volIdx ;		///< volume (drive) ID
} FsVolumeUseInd_t ;

/// File system volume
typedef struct {
		char	* name ;						///< Name of the volume
		int		mountOnInit ;					///< 1 to mount on initialization, 0 not
		int		formatIfMountFails ;			///< 1 to format if mount fails, 0 not
		int		downloadContentAfterFormat ;	///< 1 to download content after format, 0 not
		void    *media;                         ///< media control block
} FS_VolList_t ;

/// Removable media insert/remove messagse
typedef struct {
	FS_Drive_t	driveType ;						///< Drive type
	Boolean		inserted ;						///< TRUE=inserted; FALSE=removed
}	FS_RemovableMediaInsertEvent_t ;

/**
 * @addtogroup FFSGroup
 * @{
 */

/*****************************************************************************
                                 Functions
*****************************************************************************/

//**************************************************************************************
/**
	Function to Initialize the File System
	@param		volList (in) null-terminate list of volume entries
	@return		Int32
	@note
		This function is used to initialize the file system.  Each volume
		in volList is initialized and its associated driver installed. 
		Each volume also has associated flags indicating whether to mount
		the volume on FS_Init and if the optional mount fails whether to
		initialize the volume.  If the volList argument is NULL then a
		default volume list is used, in which only the ffs volume is initialized.
		Returns 0 on success and -1 on error.
**/

Int32 FS_Init( const FS_VolList_t * volList ) ;

//**************************************************************************************
/**
	Function to Run File System
	@return		none
	@note
		This function is used to run file system

**/

void FS_Run(void);

//**************************************************************************************
/**
	Function to Shut down the File System
	@return		none
	@note
		This function is used to shut down file system

**/

void FS_Shutdown(void);

//**************************************************************************************
/**
	Function to determine whether removable media
	is inserted
	@param		driveNum (in) the drive number
	@return		Boolean
	@note
		Returns TRUE if removable media device is 
		inserted else FALSE.  For non-removable media 
		will return TRUE. 

**/

Boolean FS_RemovableMediaInserted( FS_Drive_t driveNum ) ;

//**************************************************************************************
/**
	Function to format a filesystem volume
	@param		name (in) the volume name
	@return		Int32
	@note
		Returns 0 on success and -1 on error.

**/

Int32 FS_Format( const char * name ) ;

//**************************************************************************************
/**
	Function to format a filesystem volume
	@param		driveNum (in) the drive number
	@return		Int32
	@note
		Returns 0 on success and -1 on error.

**/

Int32 FS_FormatDrive( FS_Drive_t driveNum ) ;

//**************************************************************************************
/**
	Function to mount a filesystem volume
	@param		name (in) the volume name
	@return		Int32
	@note
		Returns 0 on success and -1 on error.

**/

Int32 FS_Mount( const char * name ) ;

//**************************************************************************************
/**
	Function to mount a filesystem volume
	@param		driveNum (in) the drive number
	@return		Int32
	@note
		Returns 0 on success and -1 on error.

**/

Int32 FS_MountDrive( FS_Drive_t driveNum ) ;

//**************************************************************************************
/**
	Function to query drive mount status
	@param		driveNum (in) the drive number
	@return		Boolean
	@note
		Returns TRUE if drive mounted else FALSE

**/

Boolean FS_DriveMounted( FS_Drive_t driveNum ) ;

//**************************************************************************************
/**
	Function to unmount a filesystem volume
	@param		name (in) the volume name
	@return		Int32
	@note
		Returns 0 on success and -1 on error.

**/

Int32 FS_Unmount( const char * name ) ;

//**************************************************************************************
/**
	Function to unmount a filesystem volume
	@param		driveNum (in) the drive number
	@return		Int32
	@note
		Returns 0 on success and -1 on error.

**/

Int32 FS_UnmountDrive( FS_Drive_t driveNum ) ;

//**************************************************************************************
/**
	Function that Opens the named file, and returns a stream, or NULL if the attempt fails.
	@param		name (in) pointer to directory name to be opened
	@return		HDIR
**/

HDIR FS_OpenDir(const char *name);


//**************************************************************************************
/**
	Function that Opens the named file, and returns a stream, or NULL if the attempt fails.
	@param		name (in) pointer to file name to be opened
	@param		mode (in) mode in which it has to be opened
	@return		FSFILE
	@note
		Legal values for mode include:  
				 "r":  open file for reading;
                 "w":  create file for writing, discard previous contents if any;
                 "a":  append, open or create file for writing at end of file;
                 "r+": open file for update (reading and writing);
                 "w+": create file for update, discard previous contents if any;
                 "a+": append, open or create file for update, writing at end

**/

FSFILE* FS_Open(const char *name, const char *mode);

//**************************************************************************************
/**
	Function that flushes output streams. 
	

	@param		stream (in) Pointer to the file system
	@return		Int32
	@note
		On an output stream, causes any buffered but unwritten data to be written. 
		It returns zero on success.FS_Flash(FS_NULL) won't flush all output streams.

**/

Int32 FS_Flush(FSFILE* stream);

//**************************************************************************************
/**
	Function that closes the stream. 
	

	@param		stream (in) Pointer to the file system
	@return		Int32
	@note
		Flushes any unwritten data for stream then closes the stream. 
		It returns FS_EOF if any error occurred and zero otherwise
**/

Int32 FS_Close(FSFILE* stream);

//**************************************************************************************
/**
	Function that removes a file. 
	

	@param		name (in) Pathname of file to be removed
	@return		Int32
	@note
		Removes the named file. It returns non-zero if the attempt fails.
**/

Int32 FS_Remove(const char *name);

//**************************************************************************************
/**
	Function that re-names a file. 
	

	@param		oldname (in) Pointer to the old name
	@param		newname (in) Pointer to the new name

	@return		Int32
	@note
		Changes the name of a file. It returns non-zero if the attempt fails.
**/

Int32 FS_Rename(const char *oldname, const char *newname);


//**************************************************************************************
/**
	Function that reads from a stream 
	

	@param		ptr (in) Pointer to the buffer to receive data
	@param		size (in) Size of an element
	@param		nmemb (in) Number of elements to read
	@param		stream (in) Pointer to the file stream from which to read

	@return		Int32 ///<  Number of elements read
	@note
		Reads from stream into the array ptr at most nBytes bytes. 
        It returns the number of bytes reads which may be less than the 
        number requested.
**/


Int32 FS_Read(void *ptr, UInt32 size, UInt32 nmemb, FSFILE* stream);

//**************************************************************************************
/**
	Function that writes on stream 
	

	@param		ptr (in) Pointer to the buffer to receive data
	@param		size (in) Size of an element
	@param		nmemb (in) Number of elements to write
	@param		stream (in) Pointer to the file stream.

	@return		Int32 ///<  Number of elements written
	@note
		Writes, from the array ptr, nBytes bytes on stream. It returns 
        the number of bytes written which may be less than the number 
        requested.
 **/


Int32 FS_Write(const void *ptr, UInt32 size, UInt32 nmemb, FSFILE* stream);

//**************************************************************************************
/**
	Function that sets the file position for a stream 
	

	@param		stream (in) Stream from which to read
	@param		offset (in) Offset from the designated origin
	@param		origin (in) Specifies origin: Beginning, current position, end of file

	@return		Int32	///< Non zero on error
	@note
		Sets the file position for stream; a subsequent read or write 
        will access data beginning at the new position.  The position is 
        set to offset bytes from origin. It returns non-zero on error.
		Legal values for origin includes:  
		FS_SEEK_SET: seek from beginning 
		FS_SEEK_CUR: seek from current position 
		FS_SEEK_END: seek from end-of-file
 **/


Int32 FS_Seek(FSFILE* stream, Int32 offset, Int32 origin);


//**************************************************************************************
/**
	Function that Repositions the file pointer to the beginning of a file. 
	

	@param		stream (in) Pointer to stream

	@return		Int32

**/

Int32 FS_Rewind(FSFILE* stream);

//**************************************************************************************
/**
	Function that Returns the current file position for stream, or -1 on error.
	

	@param		stream (in) Pointer to stream

	@return		Int32

**/


Int32 FS_Tell(FSFILE* stream);

//**************************************************************************************
/**
	Function to	Create a new directory.
	

	@param		name (in) Pointer to directory name.

	@return		Int32

**/

Int32 FS_MakeDir(const char *name); 

//**************************************************************************************
/**
	Function to	Delete a directory.
	

	@param		name (in) Pointer to directory name to be deleted.

	@return		Int32

**/

Int32 FS_RemoveDir(const char *name); 

//**************************************************************************************
/**
	Function to	obtain first entry to a directory 
	

	@param		pInfo (in) Pointer to buffer that contains status of a file search
	@param		pattern (in) Pointer to path name to match

	@return		HDIR ///< Handle to directory, otherwise NULL on error.
	@note
		This function obtains the first entry in a directory to match the pattern. 
        It returns NULL on error.

**/


HDIR FS_GetFileFirst(FS_INFO_t *pInfo, const char *pattern);

//**************************************************************************************
/**
	Function to	Get the next entry in a directory that matches a pattern. 
	

	@param		handle (in) Handle to directory
	@param		pInfo (in) Pointer to buffer that contains status of a file search

	@return		Boolean ///< TRUE if find a file, otherwise NULL if the end of the file list
	@note
		This function gets the next entry in a directory that matches a pattern
        It returns TRUE if find a file, otherwise NULL if the end of the file list.
		pInfo is returned by a precious call to a FS-FS_GetFileFirst or FS_GetFileNext 

**/


Boolean FS_GetFileNext(HDIR handle, FS_INFO_t *pInfo);


//**************************************************************************************
/**
	Function to	free internal resources. 
	

	@param		handle (in) Handle to directory

	@return		void
	@note
		This function frees internal resources used by FS_GetFileFirst and FS_GetFileNext.
        This must be called when done searching through a directory.

**/


void FS_GetFileDone(HDIR handle);


//**************************************************************************************
/**
	Function to	return free space available. 
	

	@param		pathname (in) Pointer to valid drive

	@return		Int32	///< Returns the available free space
	@note
		Given the path containing a valid drive, returns free space available.

**/
UInt64 FS_GetFreeSpace(const char *pathname);

//**************************************************************************************
/**
	Function to	determine file access permission
	

	@param		name (in) Pointer to file name
	@param		mode (in) The file's access bits and should be a bit-wise OR of one or more of the following values.
							- FS_ACCESS_RD
							- FS_ACCESS_WR
							.

	@return		Int32 ///< Returns 0 if the file has the given mode, otherwise error.
	@note
		This function determine file-access permission.
		Returns 0 if the file has the given mode, otherwise error.

**/


Int32 FS_Access(const char *name, UInt32 mode);


//**************************************************************************************
/**
	Function to	change the file-permission settings
	

	@param		name (in) Pointer to file name
	@param		mode (in) The file's permission bits and should be a bit-wise OR of one or more of the following values.
							- FS_S_IREAD
							- FS_S_IWRITE
							.

	@return		Int32 ///< Returns 0 if the permission setting is successfully changed, otherwise error.
	@note
		This function changes the file-permission settings.
		Returns 0 if the permission setting is successfully changed, otherwise error.

**/

Int32 FS_ChangeMode(const char *name, int mode);


//**************************************************************************************
/**
	Function to	get status information on a file.
	

	@param		name (in) Pointer to file name
	@param		buf (in) File status buffer

	@return		Int32 ///< Returns 0 if the file-status information is obtained, otherwise error.
	@note
		This function gets status information on a file.
		Returns 0 if the file-status information is obtained, otherwise error.

**/


Int32 FS_GetStatus(const char *name, struct FS_STAT_t *buf);

//**************************************************************************************
/**
	Function to	get directory name.
	

	@param		dirInfo (in) Directory Information

	@return		char ///< Pointer to the directory name
	@note
		This function returns path names based on result of FS_GetFileFirst or FS_GetFileNext
		

**/


const char * FS_GetPathName(FS_INFO_t dirInfo);


//**************************************************************************************
/**
	Function to	get drive name.
	

	@param		driveType (in) Type of Drive

	@return		char ///< Pointer to the drive name
	@note
		This function gets the drive name. Returns NULL if
		driveType is invalid.
		

**/


const char * FS_GetDriveName(FS_Drive_t driveType);

//**************************************************************************************
/**
	Function to	read one character from the specified stream.
	

	@param		file (in) Pointer to the file stream

	@return		Int32 
	@note
		This function reads one character from the specified file
		stream or EOF if an error occurs.
		

**/

Int32 FS_Getc(FSFILE *file);

//**************************************************************************************
/**
	Function to	read characters from the specified stream and store them in 's' 
	

	@param		buf (in) Pointer to buffer
	@param		n	(in) Number of characters to be read
	@param		file (in) Pointer to file stream

	@return		Int32 
	@note
		This function  reads up to 'n' characters from a string, up to and including 
		the newline character from the file stream or EOF if an error occurs.
		
		

**/


char * FS_Gets(char *buf, Int32 n, FSFILE *file);


//**************************************************************************************
/**
	Function to	write a character to file stream 
	

	@param		ch (in) Character to be written
	@param		file (in) Pointer to file 

	@return		Int32 
	@note
		This function writes one character to a specified file
		It returns ch if successful, EOF otherwise	
		

**/

Int32 FS_Putc(Int32 ch, FSFILE *file);

//**************************************************************************************
/**
	Function to	write a string to file stream 
	

	@param		s (in) Pointer to string to be written
	@param		file (in) Pointer to file stream

	@return		Int32 
	@note
		This function writes one character to a specified file
		It returns a non-negative value if successful, EOF otherwise	
		

**/

Int32 FS_Puts(const char *s, FSFILE *file);

//**************************************************************************************
/**
	Function to	create a temporary file
	

	@return		FSFILE ///<  temporary file handle or NULL	
	@note
		This function creates a temporary file.
		It returns a file handle if successful, NULL otherwise	
		

**/

FSFILE *FS_Tmpfile(void);

//**************************************************************************************
/**
	Function to	check for EOF
	
	@param		stream (in) Pointer to file stream

	@return		Int32 ///<  < 0 if error, = 0 if not EOF, > 0 if EOF
	@note
		This function checks for End Of File.
		It returns < 0 if error, = 0 if not EOF, > 0 if EOF
		

**/


Int32 FS_Eof(FSFILE *stream);

//**************************************************************************************
/**
	Function to	query error code status
	
	@param		stream (in) Pointer to file stream

	@return		Int32 ///<  < 0 if query failure, else errcode (= 0 if no error, > 0 if error)
	@note
		This function will return a value based on a query of the errcode status.
		It returns < 0 if error, = 0 if not EOF, > 0 if EOF
		

**/


Int32 FS_Error(FSFILE *stream);

//**************************************************************************************
/**
	Function to	query last filesystem error (FS-specific errno)
	
	@return		Int32 ///< errno
	@note
		This function returns the last filesystem-specific error (errno)
		

**/


Int32 FS_GetErrno(void);

//**************************************************************************************
/**
	Function to	set filesystem error (FS-specific errno)
	
	@note
		This function is typically called from filesystem code.
		

**/

void FS_SetErrno(int err_val,char *filename, int linenum);

/** @} */

/*    FS_Truncate: Truncate the file size to the specified value
*                                                               
*      Inputs: path = pointer to the file to truncate           
*              length = desired file size                       
*                                                               
*     Returns: 0 on success, -1 on failure                      
*                                                               
*/
Int32 FS_Truncate(const char *path, UInt32 length);


/*
 * Function:	FS_GetTotalSpace
 *
 * Description:	 Given the path containing a valid drive, returns total space of this volume 
 *
 * Return:	  	
 *
 * Note:
 */
Int64 FS_GetTotalSpace(const char *volume);

/*
 * Function:	FS_DownloadContent
 *
 * Description:	 Download FS content over serial port A
 *
 * Return:	  	0 on success; -1 on error
 *
 * Note:
 */
//Int32 FS_DownloadContent( void ) ;

/*
 * Function:	FFSGC_TriggerCleanup
 *
 * Description:	 Triggers a volume cleanup (used block recovery).
 *               valid for NOR flash; ignored for NAND flash
 *
 * Return:	  	None
 *
 */
void FFSGC_TriggerCleanup( void ) ;

/*
 * Function:	FFSGC_StopCleanup
 *
 * Description:	 Stops a volume cleanup in progress.
 *
 * Return:	  	None
 *
 * Note:        No effect if volume cleanup not in progress.
 *              Does not affect page erase in progress.
 */
void FFSGC_StopCleanup( void ) ;

/*
 * Function:	FFSGC_SetCleanupCycle
 *
 * Description:	 Sets default volume cleanup cycle in seconds.
 *               The volume cleanup cycle is the interval
 *               between successive volume cleanups (used block
 *               recovery).  If FFSGC_SetCleanupCycle is not called
 *               a default cycle time is used.
 *
 * Return:	  	None
 *
 * Note:
 */
void FFSGC_SetCleanupCycle( UInt32 seconds ) ;

/**
 * @addtogroup FFSGroup
 * @{
 */

//**************************************************************************************
/**
	Enable broadcast of message (type MSG_FS_VOLUME_USE_IND) when
	volume use exceeds a threshold
	
	@param		volIdx (in) volume index
	@param		enable (in) TRUE to enable message broadcast or FALSE to disable.

	@return		None
**/
void FS_EnablePercentUsedIndicator( FS_Drive_t volIdx, Boolean enable ) ;

//**************************************************************************************
/**
	Get enable status of MSG_FS_VOLUME_USE_IND broadcast
	
	@param		volIdx (in) volume index

	@return		None
**/
Boolean FS_GetEnablePercentUsedIndicator( FS_Drive_t volIdx ) ;

//**************************************************************************************
/**
	Return threshold used to to send MSG_FS_VOLUME_USE_IND message when volume use
	exceeds threshold
	
	@param		volIdx (in) volume index
	@return		UInt8
	@note
		Returns threshold value in percent (0 to 100)
**/
UInt8 FS_GetPercentUsed( FS_Drive_t volIdx ) ;

//**************************************************************************************
/**
	Set threshold used to to send MSG_FS_VOLUME_USE_IND message when volume use
	exceeds threshold
	
	@param		volIdx (in) volume index
	@param		threshPct (in) threshold value (percent) from 0 to 100.

	@return		None
**/
void FS_SetPercentUsedThreshold( FS_Drive_t volIdx, UInt8 threshPct );

//**************************************************************************************
/**
	Get current volume use in percent
	
	@param		volIdx (in) volume index

	@return		UInt8
**/
UInt8 FS_GetPercentUsedThreshold( FS_Drive_t volIdx ) ;

//**************************************************************************************
/**
	Query USB Mass Storage class status
	@return		Boolean
	@note
		Returns TRUE if USB Mass Storage class active else FALSE.
**/
Boolean FS_UsbMassStorageClassActive( void ) ;

//**************************************************************************************
/**
	Mount drive as USB Mass Storage Class volume
	@param		drive (in) the drive number
	@return		Int32
	@note
		Returns 0 on success and -1 on error.
**/
Int32 FS_MountUsbDrive( FS_Drive_t drive ) ;

//**************************************************************************************
/**
	Function to query USB Mass Storage Class volume status
	@param		drive (in) the drive number
	@return		Boolean
	@note
		Returns TRUE if drive mounted USB-MSC else FALSE

**/
Boolean FS_UsbDriveMounted( FS_Drive_t drive ) ;

//**************************************************************************************
/**
	Unmount drive as USB Mass Storage Class volume
	@param		drive (in) the drive number
	@return		Int32
	@note
		Returns 0 on success and -1 on error.
**/
Int32 FS_UnmountUsbDrive( FS_Drive_t drive ) ;

//**************************************************************************************
/**
	Load ramdisk image from ffs file.
	@return		Int32
	@note
		Returns 0 on success and -1 on error.
	@note
		This is for testing only and should not be considered permanent.
**/
Int32 FS_LoadRamdiskImage( void ) ;

//**************************************************************************************
/**
	Copy the ramdisk image to a file on the flash filesystem.  
	Automatically called on shutdown.  
	This file will be used to initialize the ramdisk next startup.
	@return		Int32
	@note
		Returns 0 on success and -1 on error.
	@note
		This is for testing only and should not be considered permanent.  The ramdisk 
		size and ffs image name are fixed.
**/

Int32 FS_SyncRamdiskImage( void ) ;

//**************************************************************************************
/**
	Get the filesystem software version based on symbol FS_VERSION.
	@return		const UInt8*
	@note
		This information is intended for display purposes only.  Symbol
		FS_VERSION should be used to determine the filesystem version.
**/
const UInt8* FS_GetFsVersionStr( void ) ;


//**************************************************************************************
/**
	Get the filesystem flash type based on symbols FLASH_TYPE_NAND and
	FLASH_TYPE_NOR.  
	@return		const UInt8*
	@note
		This information is intended for display purposes only.  Symbols
		FLASH_TYPE_NAND and FLASH_TYPE_NOR should be used to determine 
		the filesystem flash type.
**/
const UInt8 *FS_GetFlashTypeStr( void ) ;

//**************************************************************************************
/**
	Get the total number of FAT sectors on removable media.
	@return		UInt32
	@note
		Returns total number of FAT sectors if a removable media device is inserted, 
		or zero (0) if no device inserted or device not supported.
**/
UInt32 FS_FatTotalSectors(UInt32 media);

//**************************************************************************************
/**
	Get the FAT sector size.
	@return		UInt32
	@note
		Microsoft recommends only 512-byte sectors be used for
		compatability with virtually all the FAT file systems in use today
		(source: by Blunk Micro, 1/08).  This function is implemented as a macro.

**/
#define FS_FatSectorSize() ((UInt32)512)


#ifdef __cplusplus
}
#endif
/** @} */
#endif	/* __FILE_H__ */


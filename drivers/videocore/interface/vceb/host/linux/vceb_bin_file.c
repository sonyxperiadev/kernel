/*****************************************************************************
* Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

#include "vceb_bin_file.h"

typedef struct
{
    struct  list_head   list;
    VCEB_BIN_FILE_T    *binFile;

} VCEB_BIN_FILE_ENTRY_T;

static LIST_HEAD( gBinFileList );
static DEFINE_MUTEX( gBinFileMutex );

/***************************************************************************** 
* 
*    Finds a bin file base on instance name and file type.
*  
*****************************************************************************/

VCEB_BIN_FILE_T *vceb_find_bin_file( const char *instance_name, const char *type )
{
    struct list_head   *entry;
    VCEB_BIN_FILE_T    *binFile = NULL;

    mutex_lock( &gBinFileMutex );
    list_for_each( entry, &gBinFileList )
    {
        VCEB_BIN_FILE_ENTRY_T *binEntry = list_entry( entry, VCEB_BIN_FILE_ENTRY_T, list );

        if (( strcmp( instance_name, binEntry->binFile->instance_name ) == 0 )
        &&  ( strcmp( type, binEntry->binFile->type ) == 0 ))
        {
            binFile = binEntry->binFile;
            break;
        }
    }
    mutex_unlock( &gBinFileMutex );

    return binFile;
}

/***************************************************************************** 
* 
*    Registers a bin file of a parituclar type against a particular instance
*  
*****************************************************************************/

int vceb_register_bin_file( VCEB_BIN_FILE_T *binFile )
{
    VCEB_BIN_FILE_ENTRY_T   *entry;

    if (( entry = kzalloc( sizeof( *entry ), GFP_KERNEL )) == NULL )
    {
        return -1;
    }
    entry->binFile = binFile;

    mutex_lock( &gBinFileMutex );
    list_add( &entry->list, &gBinFileList );
    mutex_unlock( &gBinFileMutex );

    return 0;
}

/***************************************************************************** 
* 
*    Releases the memory for a bin file and de-registers it as well.
*  
*****************************************************************************/

void vceb_free_bin_file( VCEB_BIN_FILE_T *binFile )
{
    struct list_head   *entry;
    struct list_head   *temp;

    // See if the binFile is registered, and remove it from the list if it is.

    mutex_lock( &gBinFileMutex );
    list_for_each_safe( entry, temp, &gBinFileList )
    {
        VCEB_BIN_FILE_ENTRY_T *binEntry = list_entry( entry, VCEB_BIN_FILE_ENTRY_T, list );

        if ( binEntry->binFile == binFile )
        {
            list_del( entry );
            kfree( binEntry );
        }
    }
    mutex_unlock( &gBinFileMutex );

    // Free up memory used

    vceb_free_bin_file_data( binFile );
    binFile->filename[0] = '\0';
}

/***************************************************************************** 
* 
*    Allocate the memory for the data portion of the bin file
*  
*****************************************************************************/

int vceb_alloc_bin_file_data( VCEB_BIN_FILE_T *binFile, size_t numBytes )
{
    // Make sure that numBytes is rounded up to a multiple of 16
    numBytes = ( numBytes + 15 ) & ~15;

    if (( binFile->data = vmalloc( numBytes )) == NULL )
    {
        binFile->len = 0;
        return -1;
    }
    memset( binFile->data, 0, numBytes );
    binFile->len = numBytes;
    return 0;
}

/***************************************************************************** 
* 
*    Release the memory occupied by the data portion of the bin file.
*  
*****************************************************************************/

void vceb_free_bin_file_data( VCEB_BIN_FILE_T *binFile )
{
    if ( binFile->data != NULL )
    {
        vfree( binFile->data );
        binFile->data = NULL;
        binFile->len = 0;
    }
}


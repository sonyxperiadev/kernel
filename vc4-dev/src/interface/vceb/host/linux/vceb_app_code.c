/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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

#include "vceb_types.h"
#include "../common/vceb_app_code.h"
#include "vceb_bin_file.h"

/******************************************************************************
External defs of symbols in the object file
******************************************************************************/

/******************************************************************************
Global functions.
******************************************************************************/

void vceb_app_code_init( const char * const instance_name )
{
   //do nothing
}

void *vceb_app_code_get_ptr( const char * const instance_name )
{
    VCEB_BIN_FILE_T *binFile;

    if (( binFile = vceb_find_bin_file( instance_name, "vceb" )) == NULL )
    {
        return NULL;
    }

    printk( KERN_INFO "VCEB: Loading app code '%s'\n", binFile->filename );

    return binFile->data;
}

uint32_t vceb_app_code_get_size( const char * const instance_name )
{
    VCEB_BIN_FILE_T *binFile;

    if (( binFile = vceb_find_bin_file( instance_name, "vceb" )) == NULL )
    {
        return 0;
    }
    return binFile->len;
}

/* ************************************ The End ***************************************** */

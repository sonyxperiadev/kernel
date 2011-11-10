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

#include <linux/module.h>
#include <linux/kernel.h>

#include "vceb_types.h"
#include "../common/vceb_2nd_stage_code.h"
#include "vceb_bin_file.h"

/******************************************************************************
External defs of symbols in the object file
******************************************************************************/

/******************************************************************************
Global functions.
******************************************************************************/

void vceb_2nd_stage_code_init( const char * const instance_name )
{
   //do nothing
}

void *vceb_2nd_stage_code_get_ptr( const char * const instance_name )
{
    VCEB_BIN_FILE_T *binFile;

    if (( binFile = vceb_find_bin_file( instance_name, "2nd-stage" )) == NULL )
    {
        return NULL;
    }
    printk( KERN_INFO "VCEB: Loading bootloader code '%s'\n", binFile->filename );

    return binFile->data;
}

uint32_t vceb_2nd_stage_code_get_size( const char * const instance_name )
{
    VCEB_BIN_FILE_T *binFile;

    if (( binFile = vceb_find_bin_file( instance_name, "2nd-stage" )) == NULL )
    {
        return 0;
    }
    return binFile->len;
}

// These may be needed by the suspend/resume code in vchiq_kernel.c

EXPORT_SYMBOL( vceb_2nd_stage_code_init );
EXPORT_SYMBOL( vceb_2nd_stage_code_get_ptr );
EXPORT_SYMBOL( vceb_2nd_stage_code_get_size );

/* ************************************ The End ***************************************** */


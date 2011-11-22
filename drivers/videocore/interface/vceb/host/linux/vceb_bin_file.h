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

#ifndef VCEB_BIN_FILE_H_
#define VCEB_BIN_FILE_H_

#include "vceb_types.h"

#define VCEB_MAX_BIN_FILENAME_LEN   256

typedef struct
{
    const char  *instance_name;
    const char  *type;

    char         filename[ VCEB_MAX_BIN_FILENAME_LEN ];
    void        *data;
    size_t       len;

} VCEB_BIN_FILE_T;


VCEB_BIN_FILE_T *vceb_find_bin_file( const char *instance_name, const char *type );
int vceb_register_bin_file( VCEB_BIN_FILE_T *binFile );
void vceb_free_bin_file( VCEB_BIN_FILE_T *binFile );
int vceb_alloc_bin_file_data( VCEB_BIN_FILE_T *binFile, size_t numBytes );
void vceb_free_bin_file_data( VCEB_BIN_FILE_T *binFile );

#endif /* VCEB_BIN_FILE_H_ */


// #############################################################################
// START #######################################################################
/*****************************************************************************
* Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.
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

#include "interface/vcos/vcos.h"

#if defined( __KERNEL__ )
#include <linux/string.h>
#include <linux/module.h>
#else
#include <string.h>
#endif

/***************************************************************************** 
* 
*  vcos_strcpy
*  
*****************************************************************************/

char *vcos_strcpy(char *dst, const char *src)
{
    return strcpy( dst, src );
}

/***************************************************************************** 
* 
*  vcos_strncpy
*  
*****************************************************************************/

char *vcos_strncpy(char *dst, const char *src, size_t count)
{
    return strncpy( dst, src, count );
}

/***************************************************************************** 
* 
*  vcos_strncmp
*  
*****************************************************************************/

int vcos_strncmp(const char *cs, const char *ct, size_t count)
{
    return strncmp( cs, ct, count );
}

/***************************************************************************** 
* 
*  vcos_memcpy
*  
*****************************************************************************/

void *vcos_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy( dst, src, n );
}

/***************************************************************************** 
* 
*  vcos_memset
*  
*****************************************************************************/

void *vcos_memset(void *p, int c, size_t n)
{
    return memset( p, c, n );
}

#if defined( __KERNEL__ )
EXPORT_SYMBOL( vcos_strcpy );
EXPORT_SYMBOL( vcos_strncpy );
EXPORT_SYMBOL( vcos_strncmp );
EXPORT_SYMBOL( vcos_memcpy );
EXPORT_SYMBOL( vcos_memset );
#endif
// END #########################################################################
// #############################################################################
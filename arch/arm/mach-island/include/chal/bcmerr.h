/*****************************************************************************
* Copyright (c) 2003-2008 Broadcom Corporation.  All rights reserved.
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

#ifndef BCM_ERR__
#define BCM_ERR__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BCM_SUCCESS = 0,
    BCM_ERROR   = -1
} BCM_ERR_CODE;

//#define BCM_FAIL_STOP( x ) if ((x) != BCM_SUCCESS) { BCM_DBG_ERR(( "%s [%s @%d] '%s'", __FUNCTION__, __FILE__, __LINE__, #x)); return (x); }

#ifdef __cplusplus
}
#endif

#endif /* BCM_ERR__ */

/*****************************************************************************
*  Copyright 2003 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef _BCM_VSP_DEBUG_H
#define _BCM_VSP_DEBUG_H

#define DBG_ERROR   0x01
#define DBG_INFO    0x02
#define DBG_TRACE   0x04
#define DBG_TRACE2  0x08
#define DBG_DATA    0x10
#define DBG_DATA2   0x20

#define DBG_DEFAULT_LEVEL (DBG_ERROR|DBG_INFO)

static int logLevel = DBG_DEFAULT_LEVEL;

#define VSP_DEBUG(level,fmt,args...) if (level & logLevel) printk( "%s:: " fmt, __FUNCTION__, ##args )

#endif //_BCM_VSP_DEBUG_H
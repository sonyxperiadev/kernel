/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*   at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a license
*  other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#ifndef _BCM_IPC_DEBUG_H
#define _BCM_IPC_DEBUG_H
#ifdef __KERNEL__

#include <linux/printk.h>
#define DBG_ERROR   KERN_ERR
#define DBG_WARN    KERN_WARNING
#define DBG_INFO    KERN_INFO
#define DBG_TRACE   KERN_DEBUG
#define DBG_TRACE2  KERN_DEBUG
#define DBG_DATA    KERN_DEBUG
#define DBG_DATA2   KERN_DEBUG
#define IPC_DEBUG(level, fmt, args...)   \
			printk(level "ipc:%s(): " fmt, __func__, ##args)
#else
#error "Error: IPC_DEBUG() macro is not defined for this platform!"
#endif

#endif /* _BCM_IPC_DEBUG_H */

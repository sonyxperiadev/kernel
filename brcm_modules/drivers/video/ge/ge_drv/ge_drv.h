/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef _GE_DRV_H_
#define _GE_DRV_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */
#undef PDEBUG			/* undef it, just in case */
#ifdef GE_DEBUG
#  ifdef __KERNEL__
    /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_INFO "x170: " fmt, ## args)
#  else
    /* This one for user space */
#    define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__ , ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)	/* not debugging: nothing */
#endif


#define GE_IOC_MAGIC  'g'
#define GE_IOC_RESERVE           _IO(GE_IOC_MAGIC,  6)
#define GE_IOC_UNRESERVE           _IO(GE_IOC_MAGIC,  7)
#define GE_IOC_WAIT           _IO(GE_IOC_MAGIC,  8)

#define GE_IOC_MAXNR 9


#define	GE_REG_OFFSET			((volatile char*)(pRegBase))

#define	GE_GESR				((volatile int*)(GE_REG_OFFSET+0x0000))
#define	GE_GECR				((volatile int*)(GE_REG_OFFSET+0x0004))
#define	GE_GEER				((volatile int*)(GE_REG_OFFSET+0x0008))
#define	GE_GECER			((volatile int*)(GE_REG_OFFSET+0x000c))
#define	GE_GEOPR			((volatile int*)(GE_REG_OFFSET+0x0010))
#define	GE_GEABR			((volatile int*)(GE_REG_OFFSET+0x0014))
#define	GE_GESSAR			((volatile int*)(GE_REG_OFFSET+0x0020))
#define	GE_GESCAR			((volatile int*)(GE_REG_OFFSET+0x0024))
#define	GE_GEDBAR			((volatile int*)(GE_REG_OFFSET+0x0028))
#define	GE_GEDCAR			((volatile int*)(GE_REG_OFFSET+0x002c))
#define	GE_GECFR			((volatile int*)(GE_REG_OFFSET+0x0030))
#define	GE_GESPR			((volatile int*)(GE_REG_OFFSET+0x0034))
#define	GE_GEDPR			((volatile int*)(GE_REG_OFFSET+0x0038))
#define	GE_GESOR			((volatile int*)(GE_REG_OFFSET+0x003c))
#define	GE_GEWR				((volatile int*)(GE_REG_OFFSET+0x0040))
#define	GE_GEHR				((volatile int*)(GE_REG_OFFSET+0x0044))
#define	GE_GESBA1R			((volatile int*)(GE_REG_OFFSET+0x0050))
#define	GE_GESBA2R			((volatile int*)(GE_REG_OFFSET+0x0054))
#define	GE_GEDSA1R			((volatile int*)(GE_REG_OFFSET+0x0058))
#define	GE_GEDSA2R			((volatile int*)(GE_REG_OFFSET+0x005c))
#define	GE_DST_FETCH_ADDR	((volatile int*)(GE_REG_OFFSET+0x0060))
#define	GE_TRANS_WIDTH_NUM	((volatile int*)(GE_REG_OFFSET+0x0064))
#define	GE_FSM_STATE_24B	((volatile int*)(GE_REG_OFFSET+0x0068))
#define	GE_FSM_STATES		((volatile int*)(GE_REG_OFFSET+0x006c))


#endif /* !_GE_H_ */

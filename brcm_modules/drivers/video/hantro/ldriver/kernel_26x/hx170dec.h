/* 
 * Decoder device driver (kernel module headers)
 *
 * Copyright (C) 2009  Hantro Products Oy.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: hx170dec.h,v $
--  $Date: 2009/11/05 11:01:43 $
--  $Revision: 1.1 $
--
------------------------------------------------------------------------------*/
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

#ifndef _HX170DEC_H_
#define _HX170DEC_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */
#undef PDEBUG			/* undef it, just in case */
#ifdef HX170DEC_DEBUG
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

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define HX170DEC_IOC_MAGIC  'k'
/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */
 /*
    #define HX170DEC_IOCGBUFBUSADDRESS _IOR(HX170DEC_IOC_MAGIC,  1, unsigned long *)
    #define HX170DEC_IOCGBUFSIZE       _IOR(HX170DEC_IOC_MAGIC,  2, unsigned int *)
    #define HX170DEC_IOCGHWOFFSET      _IOR(HX170DEC_IOC_MAGIC,  3, unsigned long *)
    #define HX170DEC_IOCGHWIOSIZE      _IOR(HX170DEC_IOC_MAGIC,  4, unsigned int *)
  */
#define HX170DEC_IOC_CLI           _IO(HX170DEC_IOC_MAGIC,  5)
#define HX170DEC_IOC_STI           _IO(HX170DEC_IOC_MAGIC,  6)
/*
#define HX170DEC_IOCXVIRT2BUS      _IOWR(HX170DEC_IOC_MAGIC,  7, unsigned long *)
*/
#define HX170DEC_IOCHARDRESET      _IO(HX170DEC_IOC_MAGIC, 8)	/* debugging tool */
#define HX170DEC_PP_INSTANCE       _IO(HX170DEC_IOC_MAGIC, 1)	/* the client is pp instance */
#define HX170DEC_DEC_INSTANCE       _IO(HX170DEC_IOC_MAGIC, 2)   /* the client is dec instance */

#define HX170DEC_IRQ_WAIT          _IO(HX170DEC_IOC_MAGIC, 9)	/* wait for Hantro irq */

#define HX170DEC_DEC_CLK_DISABLE   _IO(HX170DEC_IOC_MAGIC, 10)	/* disable clk after decode */
#define HX170DEC_DEC_CLK_ENABLE    _IO(HX170DEC_IOC_MAGIC, 11)	/* enable clk before decode */

/* Reserve Decoder Hantro */
#define HX170DEC_DEC_RESV_WAIT          _IO(HX170DEC_IOC_MAGIC, 12)
/* UnReserve Decoder Hantro */
#define HX170DEC_DEC_UNRESV          _IO(HX170DEC_IOC_MAGIC, 13)
/* Reserve PP Hantro */
#define HX170DEC_PP_RESV_WAIT          _IO(HX170DEC_IOC_MAGIC, 14)
/* Reserve PP Hantro */
#define HX170DEC_PP_UNRESV          _IO(HX170DEC_IOC_MAGIC, 15)

#define HX170DEC_IOC_MAXNR 19

#endif /* !_HX170DEC_H_ */

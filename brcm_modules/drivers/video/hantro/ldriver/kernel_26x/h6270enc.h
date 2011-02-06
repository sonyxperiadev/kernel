/* 
 * 6270 Encoder device driver (kernel module headers)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,USA.
 *
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: h6270enc.h,v $
--  $Date: 2009/11/09 07:33:57 $
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

#ifndef _H6270ENC_H_
#define _H6270ENC_H_
#include <linux/ioctl.h>    /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG   /* undef it, just in case */
#ifdef H6270ENC_DEBUG
#  ifdef __KERNEL__
    /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_INFO "hmp4e: " fmt, ## args)
#  else
    /* This one for user space */
#    define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__ , ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)  /* not debugging: nothing */
#endif

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define H6270ENC_IOC_MAGIC  'k'
/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */
 /*
  * #define H6270ENC_IOCGBUFBUSADDRESS _IOR(H6270ENC_IOC_MAGIC,  1, unsigned long *)
  * #define H6270ENC_IOCGBUFSIZE       _IOR(H6270ENC_IOC_MAGIC,  2, unsigned int *)
  */
#define H6270ENC_IOCGHWOFFSET      _IOR(H6270ENC_IOC_MAGIC,  3, unsigned long *)
#define H6270ENC_IOCGHWIOSIZE      _IOR(H6270ENC_IOC_MAGIC,  4, unsigned int *)
#define H6270ENC_IOC_CLI           _IO(H6270ENC_IOC_MAGIC,  5)
#define H6270ENC_IOC_STI           _IO(H6270ENC_IOC_MAGIC,  6)
#define H6270ENC_IOCXVIRT2BUS      _IOWR(H6270ENC_IOC_MAGIC,  7, unsigned long *)

#define H6270ENC_IOCHARDRESET      _IO(H6270ENC_IOC_MAGIC, 8)   /* debugging tool */

#define H6270ENC_IRQ_WAIT          _IO(H6270ENC_IOC_MAGIC, 9)   /* wait for Hantro irq */

#define H6270ENC_IOC_MAXNR 9

#endif /* !_H6270ENC_H_ */

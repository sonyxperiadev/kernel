/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   xdr_porting_layer.h
*
*   @brief  This file contains type definitions and the functions needed by xdr.
*
*
****************************************************************************/
#ifndef __XDR_PORTING_LAYER_H__
#define __XDR_PORTING_LAYER_H__

#ifdef LINUX_RPC_KERNEL
#include <linux/slab.h>
typedef int32_t bool_t;
typedef int32_t enum_t;
#else

typedef signed char int8_t;
typedef unsigned char u_int8_t;
typedef short int16_t;
typedef unsigned short u_int16_t;
typedef int int32_t;
typedef unsigned int u_int32_t;

typedef unsigned int u_int;
typedef int32_t bool_t;
typedef int32_t enum_t;
typedef unsigned long u_long;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef char *caddr_t;
#endif

#ifdef _LONG_LONG_
typedef long double int64_t;
typedef unsigned long double u_int64_t;
typedef long double longlong_t;
typedef unsigned long double u_longlong_t;
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef LINUX_RPC_KERNEL
#define	UINT_MAX	0xffffffff
#else
#define	XDR_UINT_MAX	0xffffffff
#endif

#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define	__END_DECLS }
#else
#define __BEGIN_DECLS
#define	__END_DECLS
#endif

#ifndef __P
#define __P(x)	x
#endif

#define _DIAGASSERT(x)

#define __dontcare__	-1

#ifndef UNDER_LINUX_MODEM
#ifdef __BIG_ENDIAN
#define lswap()			/* Not needed on big-endian machines */
#define      htonl(l) (l)
#define      ntohl(l) (l)
#define      htons(s) (s)
#define      ntohs(s) (s)
#else
extern unsigned long lswap(unsigned long l);	/* swap bytes in 32 bit long */
#define      htonl(l) lswap(l)
#define      ntohl(l) lswap(l)
#define      htons(s) ((u_short)(((u_short)(s) >> 8) | ((u_short)(s) << 8)))
#define      ntohs(s) htons(s)
#endif
#endif

#if defined(UNDER_CE) || defined(WIN32)
#define snprintf _snprintf
#endif

#ifdef LINUX_RPC_KERNEL
#define mem_free(a, b)	kfree(a)
#define mem_alloc(a)	kmalloc(a, GFP_KERNEL)

#define strlen(a)		strlen(a)
#else
#define mem_free(a, b)	free(a)
#define mem_alloc(a)	malloc(a)
#define strlen(a)		strlen(a)
#endif
#define warnx

#ifndef LINUX_RPC_KERNEL
#include <stdio.h>
#include <stdlib.h>
#endif

#if defined(UNDER_LINUX) || defined(LINUX_RPC_KERNEL)
#ifndef NULL
#define NULL 0
#endif
#endif

#ifdef WIN32
#include <malloc.h>		/* for malloc/free */
#include <memory.h>		/* for memmove */
#endif
#ifndef LINUX_RPC_KERNEL
#include <string.h>		/* for strlen */
#endif
#endif /*  __XDR_PORTING_LAYER_H__ */

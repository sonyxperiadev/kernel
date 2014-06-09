/**
 * \file    portable.h
 *  
 * \authors Pete Skeggs 
 *
 * \brief   portability macros
 *
 * \copyright  (C) 2013 EM Microelectronic
 *
 */


#ifndef _PORTABLE_H_
#define _PORTABLE_H_

#if defined(__KERNEL__)
#define PREPACK
#define MIDPACK __attribute__((__packed__))
#define POSTPACK
#define VOLATILE __volatile__
#define ATOMIC_INT int
#include <linux/swab.h> // for swab32 (swap bytes within a 32 bit word)

#elif defined(_MSC_VER)
#include "windows.h"
#define PREPACK __pragma(pack(push, 1))
#define MIDPACK
#define POSTPACK __pragma(pack(pop))
#define VOLATILE volatile
#define ATOMIC_INT int
#define swab32(x) ((u32)(				\
	(((u32)(x) & (u32)0x000000ffUL) << 24) |		\
	(((u32)(x) & (u32)0x0000ff00UL) <<  8) |		\
	(((u32)(x) & (u32)0x00ff0000UL) >>  8) |		\
	(((u32)(x) & (u32)0xff000000UL) >> 24)))

#elif defined(__GNUC__)
#define PREPACK
#define MIDPACK __attribute__((__packed__))
#define POSTPACK
#define VOLATILE __volatile__
#define ATOMIC_INT int
#if defined(__gnu_linux__) || defined(__CYGWIN__)
#include <linux/swab.h> // for swab32 (swap bytes within a 32 bit word)
#else
#define swab32(x) ((u32)(				\
	(((u32)(x) & (u32)0x000000ffUL) << 24) |		\
	(((u32)(x) & (u32)0x0000ff00UL) <<  8) |		\
	(((u32)(x) & (u32)0x00ff0000UL) >>  8) |		\
	(((u32)(x) & (u32)0xff000000UL) >> 24)))
#endif
#endif

#endif


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
/**
*
*   @file   audiohal_bits.c
*
*   @brief  This file BITxx micros for hal_audio_access.c and hal_audio_core.c
*              These BIT?? defines conflict with arch/arm/mach-bcm116x/include/mach/bits.h,
*              So please include this file only to hal_audio_access.c and hal_audio_core.c, put it in the last 
*		 '# include' list.
****************************************************************************/

#ifndef	__AUDIOHAL_BITS_H
#define __AUDIOHAL_BITS_H
//Below BIT?? defines conflict with arch/arm/mach-bcm116x/include/mach/bits.h:
//So we undef it then redefine
#undef	BIT00
#undef	BIT01
#undef	BIT02
#undef	BIT03
#undef	BIT04
#undef	BIT05
#undef	BIT06
#undef	BIT07
#undef	BIT08
#undef	BIT09
#undef	BIT10
#undef	BIT11
#undef	BIT12
#undef	BIT13
#undef	BIT14
#undef	BIT15
#undef	BIT16
#undef	BIT17
#undef	BIT18
#undef	BIT19
#undef	BIT20
#undef	BIT21
#undef	BIT22
#undef	BIT23
#undef	BIT24
#undef	BIT25
#undef	BIT26
#undef	BIT27
#undef	BIT28
#undef	BIT29
#undef	BIT30
#undef	BIT31

#define	BIT00			0
#define	BIT01			1
#define	BIT02			2
#define	BIT03			3
#define	BIT04			4
#define	BIT05			5
#define	BIT06			6
#define	BIT07			7
#define	BIT08			8
#define	BIT09			9
#define	BIT10			10
#define	BIT11			11
#define	BIT12			12
#define	BIT13			13
#define	BIT14			14
#define	BIT15			15
#define	BIT16			16
#define	BIT17			17
#define	BIT18			18
#define	BIT19			19
#define	BIT20			20
#define	BIT21			21
#define	BIT22			22
#define	BIT23			23
#define	BIT24			24
#define	BIT25			25
#define	BIT26			26
#define	BIT27			27
#define	BIT28			28
#define	BIT29			29
#define	BIT30			30
#define	BIT31			31

#endif //__AUDIOHAL_BITS_H

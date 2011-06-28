/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

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


#ifndef __PLAT_DEFCONFIG_H__
#define __PLAT_DEFCONFIG_H__

// to define different the platforms specific features

#ifdef CONFIG_ARCH_RHEA
#define	CONFIG_DEPENDENCY_READY_CAPI
#define	CONFIG_DEPENDENCY_ENABLE_SSP34
#define	CONFIG_DEPENDENCY_READY_PMU
//#define	CONFIG_DEPENDENCY_READY_SYSPARM
#endif


#ifdef CONFIG_ARCH_ISLAND
#define	CONFIG_DEPENDENCY_READY_CAPI
//#define CONFIG_DEPENDENCY_ENABLE_SSP34
//#define	CONFIG_DEPENDENCY_READY_PMU
//#define	CONFIG_DEPENDENCY_READY_SYSPARM
#endif


#ifdef CONFIG_ARCH_SAMOA
//#define	CONFIG_DEPENDENCY_READY_CAPI
//#define	CONFIG_DEPENDENCY_ENABLE_SSP34
//#define	CONFIG_DEPENDENCY_READY_PMU
//#define	CONFIG_DEPENDENCY_READY_SYSPARM
#endif

#endif //__PLAT_DEFCONFIG_H__


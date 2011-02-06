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

#ifndef __BRCM_ALSA_PCG_H__
#define __BRCM_ALSA_PCG_H__



#define PCG_IOCTL_SETMODE  101
#define PCG_IOCTL_GETMODE  102
#define PCG_IOCTL_SETVOL  103
#define PCG_IOCTL_GETVOL  104
#define PCG_IOCTL_SetExtPAVol  105
#define PCG_IOCTL_MAUDTST 106

typedef struct{
  UInt32 val1;
  UInt32 val2;
  UInt32 val3;
  UInt32 val4;
  UInt32 val5;
  UInt32 val6;
}arg_t;

#endif

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.


 *  Copyright (C) 2009-2014 Broadcom Corporation
 */


/************************************************************************************
 *
 *  Filename:      fm_public.h
 *
 *  Description:   FM Driver public header file.
 *
 ***********************************************************************************/

#ifndef _FM_PUBLIC_H
#define _FM_PUBLIC_H

/*******************************************************************************
**  Constants & Macros
*******************************************************************************/

#define    FM_REGION_EUR    0x00
#define    FM_REGION_JP     0x01
#define    FM_REGION_NA     0x02
#define    FM_REGION_RUS    0x03
#define    FM_REGION_CHN    0x04
#define    FM_REGION_IT     0x05

#define    FM_RDS_BIT       1<<4
#define    FM_RBDS_BIT      1<<5
#define    FM_AF_BIT        1<<6

#define     FM_REGION_MAX           FM_REGION_JP
/* low 3 bits (bit0, 1)of FUNC mask is region code */
#define     FM_REGION_MASK          (FM_REGION_NA | FM_REGION_EUR)

/* FM audio output mode */
enum
{
    FM_AUTO_MODE = 1,   /* auto blend by default */
    FM_STEREO_MODE,     /* manual stereo switch */
    FM_MONO_MODE,       /* manual mono switch */
    FM_SWITCH_MODE      /* auto stereo, and switch activated */
};

/* FM audio routing configuration */
#define FM_AUDIO_NONE       0x00  /* No FM audio output */
#define FM_AUDIO_DAC        0x01  /* routing FM over analog output */
#define FM_AUDIO_I2S        0x02  /* routing FM over digital (I2S) output */
#define FM_AUDIO_BT_MONO    0x04  /* routing FM over SCO */
#define FM_AUDIO_BT_STEREO  0x08  /* routing FM over BT Stereo */

#define     FM_DEEMPHA_50U      0       /* 6th bit in FM_AUDIO_CTRL0 set to 0, Europe default */
#define     FM_DEEMPHA_75U      (1<<6)  /* 6th bit in FM_AUDIO_CTRL0 set to 1, US  default */

#define  FM_STEP_50KHZ      0x00
#define  FM_STEP_100KHZ     0x01
#define  FM_STEP_200KHZ     0x02

#define  CHL_SPACE_ONE      0x01
#define  CHL_SPACE_TWO      0x02
#define  CHL_SPACE_FOUR     0x04



#define FM_GET_FREQ(x) ((unsigned short) ((x * 10) - 64000))
#define FM_SET_FREQ(x) ((unsigned int) ((x + 64000)/10))
enum
{
    FM_RDS,
    FM_RBDS
};

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#endif

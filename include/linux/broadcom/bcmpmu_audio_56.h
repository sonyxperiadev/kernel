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
f
Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/


#ifndef __BCMPMU_AUDIO_H__
#define __BCMPMU_AUDIO_H__

#define BCMPMU_IHFTOP_IDDQ		0x01
#define BCMPMU_IHFLDO_PUP		0x01
#define BCMPMU_IHFPOP_PUP		0x08
#define BCPMU_IHFPOP_BYPASS		0x20
#define BCMPMU_IHFPOP_AUTOSEQ	0x40
#define BCMPMU_HSPUP1_IDDQ_PWRDN	0x10
#define BCMPMU_HSPUP2_HS_PWRUP	0x40
#define BCMPMU_HS_GAIN_MASK		0x3F
#define BCMPMU_IHF_GAIN_MASK	0x3F
#define BCMPMU_PLL_EN			0x02
#define BCMPMU_PLL_AUDIO_EN		0x20

/* HSPGA1 */
#define BCMPMU_HSPGA1_PGA_GAINR		0x80
#define BCMPMU_HSPGA1_PGA_GAINL		0x40
#define BCMPMU_PGA_CTL_MASK		0x3F
/* HSPGA2 */
#define BCMPMU_PGA_CPCTL_BIT		6
#define BCMPMU_PGA_CPCTL_MASK		0x3

/* HSPGA3 */
#define BCMPMU_HSPGA3_PGA_INSHORT	0x40
#define BCMPMU_HSPGA3_PGA_ACINADJ	0x20
#define BCMPMU_HSPGA3_PGA_ENACCPL	0x04
#define BCMPMU_HSPGA3_PGA_PULLDNSJ	0x02
#define BCMPMU_HSPGA3_PGA_CMCTL		0x01

typedef enum {
	PMU_AUDIO_HS_LEFT,
	PMU_AUDIO_HS_RIGHT,
	PMU_AUDIO_HS_BOTH
} bcmpmu_hs_path_t;

typedef enum {
	PMU_HSGAIN_MUTE,
	PMU_HSGAIN_66DB_N,
	PMU_HSGAIN_63DB_N,
	PMU_HSGAIN_60DB_N,
	PMU_HSGAIN_57DB_N,
	PMU_HSGAIN_54DB_N,
	PMU_HSGAIN_51DB_N,
	PMU_HSGAIN_48DB_N,
	PMU_HSGAIN_45DB_N,
	PMU_HSGAIN_42DB_N,
	PMU_HSGAIN_40P5DB_N,
	PMU_HSGAIN_39DB_N,
	PMU_HSGAIN_37P5DB_N,
	PMU_HSGAIN_36DB_N,
	PMU_HSGAIN_34P5DB_N,
	PMU_HSGAIN_33DB_N,
	PMU_HSGAIN_31P5DB_N,
	PMU_HSGAIN_30DB_N,
	PMU_HSGAIN_28P5DB_N,
	PMU_HSGAIN_27DB_N,
	PMU_HSGAIN_25P5DB_N,
	PMU_HSGAIN_24DB_N,
	PMU_HSGAIN_22P5DB_N,
	PMU_HSGAIN_22DB_N,
	PMU_HSGAIN_21P5DB_N,
	PMU_HSGAIN_21DB_N,
	PMU_HSGAIN_20P5DB_N,
	PMU_HSGAIN_20DB_N,
	PMU_HSGAIN_19P5DB_N,
	PMU_HSGAIN_19DB_N,
	PMU_HSGAIN_18P5DB_N,
	PMU_HSGAIN_18DB_N,
	PMU_HSGAIN_17P5DB_N,
	PMU_HSGAIN_17DB_N,
	PMU_HSGAIN_16P5DB_N,
	PMU_HSGAIN_16DB_N,
	PMU_HSGAIN_15P5DB_N,
	PMU_HSGAIN_15DB_N,
	PMU_HSGAIN_14P5DB_N,
	PMU_HSGAIN_14DB_N,
	PMU_HSGAIN_13P5DB_N,
	PMU_HSGAIN_13DB_N,
	PMU_HSGAIN_12P5DB_N,
	PMU_HSGAIN_12DB_N,
	PMU_HSGAIN_11P5DB_N,
	PMU_HSGAIN_11DB_N,
	PMU_HSGAIN_10P5DB_N,
	PMU_HSGAIN_10DB_N,
	PMU_HSGAIN_9P5DB_N,
	PMU_HSGAIN_9DB_N,
	PMU_HSGAIN_8P5DB_N,
	PMU_HSGAIN_8DB_N,
	PMU_HSGAIN_7P5DB_N,
	PMU_HSGAIN_7DB_N,
	PMU_HSGAIN_6P5DB_N,
	PMU_HSGAIN_6DB_N,
	PMU_HSGAIN_5P5DB_N,
	PMU_HSGAIN_5DB_N,
	PMU_HSGAIN_4P5DB_N,
	PMU_HSGAIN_4DB_N,
	PMU_HSGAIN_3P5DB_N,
	PMU_HSGAIN_3DB_N,
	PMU_HSGAIN_2P5DB_N,
	PMU_HSGAIN_2DB_N,
	PMU_HSGAIN_NUM
} bcmpmu_hs_gain_t ;

typedef enum {
	PMU_IHFGAIN_MUTE,
	PMU_IHFGAIN_60DB_N,
	PMU_IHFGAIN_57DB_N,
	PMU_IHFGAIN_54DB_N,
	PMU_IHFGAIN_51DB_N,
	PMU_IHFGAIN_48DB_N,
	PMU_IHFGAIN_45DB_N,
	PMU_IHFGAIN_42DB_N,
	PMU_IHFGAIN_39DB_N,
	PMU_IHFGAIN_36DB_N,
	PMU_IHFGAIN_34P5DB_N,
	PMU_IHFGAIN_33DB_N,
	PMU_IHFGAIN_31P5DB_N,
	PMU_IHFGAIN_30DB_N,
	PMU_IHFGAIN_28P5DB_N,
	PMU_IHFGAIN_27DB_N,
	PMU_IHFGAIN_25P5DB_N,
	PMU_IHFGAIN_24DB_N,
	PMU_IHFGAIN_22P5DB_N,
	PMU_IHFGAIN_21DB_N,
	PMU_IHFGAIN_19P5DB_N,
	PMU_IHFGAIN_18DB_N,
	PMU_IHFGAIN_16P5DB_N,
	PMU_IHFGAIN_16DB_N,
	PMU_IHFGAIN_15P5DB_N,
	PMU_IHFGAIN_15DB_N,
	PMU_IHFGAIN_14P5DB_N,
	PMU_IHFGAIN_14DB_N,
	PMU_IHFGAIN_13P5DB_N,
	PMU_IHFGAIN_13DB_N,
	PMU_IHFGAIN_12P5DB_N,
	PMU_IHFGAIN_12DB_N,
	PMU_IHFGAIN_11P5DB_N,
	PMU_IHFGAIN_11DB_N,
	PMU_IHFGAIN_10P5DB_N,
	PMU_IHFGAIN_10DB_N,
	PMU_IHFGAIN_9P5DB_N,
	PMU_IHFGAIN_9DB_N,
	PMU_IHFGAIN_8P5DB_N,
	PMU_IHFGAIN_8DB_N,
	PMU_IHFGAIN_7P5DB_N,
	PMU_IHFGAIN_7DB_N,
	PMU_IHFGAIN_6P5DB_N,
	PMU_IHFGAIN_6DB_N,
	PMU_IHFGAIN_5P5DB_N,
	PMU_IHFGAIN_5DB_N,
	PMU_IHFGAIN_4P5DB_N,
	PMU_IHFGAIN_4DB_N,
	PMU_IHFGAIN_3P5DB_N,
	PMU_IHFGAIN_3DB_N,
	PMU_IHFGAIN_2P5DB_N,
	PMU_IHFGAIN_2DB_N,
	PMU_IHFGAIN_1P5DB_N,
	PMU_IHFGAIN_1DB_N,
	PMU_IHFGAIN_P5DB_N,
	PMU_IHFGAIN_0DB,
	PMU_IHFGAIN_P5DB_P,
	PMU_IHFGAIN_1DB_P,
	PMU_IHFGAIN_1P5DB_P,
	PMU_IHFGAIN_2DB_P,
	PMU_IHFGAIN_2P5DB_P,
	PMU_IHFGAIN_3DB_P,
	PMU_IHFGAIN_3P5DB_P,
	PMU_IHFGAIN_4DB_P,
	PMU_IHFGAIN_NUM
} bcmpmu_ihf_gain_t;

/* HS/IHF Test Mode */
#define PMU_TEST_READ_AND_ENABLE  0x03
#define PMU_TEST_READ_AND_DISABLE 0x02
#define PMU_TEST_ENABLE_NO_READ   0x01
#define PMU_TEST_DISABLE_NO_READ  0x00

enum
{
	PMU_HS_DIFFERENTIAL_DC_COUPLED,
	PMU_HS_DIFFERENTIAL_AC_COUPLED,
	PMU_HS_SINGLE_ENDED_AC_COUPLED
};

extern void bcmpmu_audio_init(void);
extern void bcmpmu_audio_deinit(void);
extern void bcmpmu_hs_power(bool on);
extern void bcmpmu_ihf_power(bool on);
extern void bcmpmu_hs_set_gain(bcmpmu_hs_path_t path, bcmpmu_hs_gain_t gain);
extern void bcmpmu_ihf_set_gain(bcmpmu_ihf_gain_t gain);
extern void bcmpmu_hi_gain_mode_en(bool en);
extern void bcmpmu_hi_gain_mode_hs_en(bcmpmu_hs_path_t path, bool en);
extern int bcmpmu_audio_ihf_selftest_stimulus_input(int stimulus);
extern int bcmpmu_audio_ihf_selftest_stimulus_output(int stimulus);
extern void bcmpmu_audio_ihf_selftest_result(u8 *result);
extern int bcmpmu_audio_ihf_testmode(int Mode);
extern int bcmpmu_audio_hs_selftest_stimulus(int stimulus);
extern void bcmpmu_audio_hs_selftest_result(u8 *result);
extern int bcmpmu_audio_hs_testmode(int Mode);
extern int bcmpmu_hs_set_input_mode(int HSgain, int HSInputmode);
void bcmpmu_audio_hs_selftest_backup(bool Enable);
void bcmpmu_audio_ihf_selftest_backup(bool Enable);

#endif

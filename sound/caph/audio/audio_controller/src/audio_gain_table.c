/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
* @file   audio_gain_table.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================
#include <string.h>
#include "mobcom_types.h"
#include "audio_consts.h"

#ifdef PMU_BCM59055
#include "linux/broadcom/bcm59055-audio.h"
#elif defined(CONFIG_BCMPMU_AUDIO)
#include "bcmpmu_audio.h"
#endif

#include "audio_gain_table.h"

#define END_OF_TABLE  0xFFFF
typedef struct
{
	UInt16 valueQ14_1;
	UInt16 valueQ1_14;
	UInt16 slopGain;
} AUDTABL_SlopGain_LUT_t;

/* The following lookup table defines the gains in Q14.1 format and 
 * in Q1.14 format as well as TARGETGAIN in register SLOPGAIN register. 
 * It is used to do the translatioin from Q14.1 or Q1.14 gain to 
 * HW register gain.
 */ 
AUDTABL_SlopGain_LUT_t slopGain_lut[]=
{
/*	0dB */	{ 0x0,	16384,	0x1ff},
/*	 -0.50dB */	{ 0xffff,	15467,	0x1fd},
/*	 -1.00dB */	{ 0xfffe,	14602,	0x1fb},
/*	 -1.50dB */	{ 0xfffd,	13785,	0x1f9},
/*	 -2.00dB */	{ 0xfffc,	13014,	0x1f7},
/*	 -2.50dB */	{ 0xfffb,	12286,	0x1f5},
/*	 -3.00dB */	{ 0xfffa,	11599,	0x1f3},
/*	 -3.50dB */	{ 0xfff9,	10950,	0x1f1},
/*	 -4.00dB */	{ 0xfff8,	10338,	0x1ef},
/*	 -4.50dB */	{ 0xfff7,	9759,	0x1ed},
/*	 -5.00dB */	{ 0xfff6,	9213,	0x1eb},
/*	 -5.50dB */	{ 0xfff5,	8698,	0x1e9},
/*	 -6.00dB */	{ 0xfff4,	8211,	0x1e7},
/*	 -6.50dB */	{ 0xfff3,	7752,	0x1e5},
/*	 -7.00dB */	{ 0xfff2,	7301,	0x1e3},
/*	 -7.50dB */	{ 0xfff1,	6892,	0x1e1},
/*	 -8.00dB */	{ 0xfff0,	6507,	0x1df},
/*	 -8.50dB */	{ 0xffef,	6143,	0x1dd},
/*	 -9.00dB */	{ 0xffee,	5799,	0x1db},
/*	 -9.50dB */	{ 0xffed,	5475,	0x1d9},
/*	 -10.00dB */	{ 0xffec,	5169,	0x1d7},
/*	 -10.50dB */	{ 0xffeb,	4879,	0x1d5},
/*	 -11.00dB */	{ 0xffea,	4606,	0x1d3},
/*	 -11.50dB */	{ 0xffe9,	4349,	0x1d1},
/*	 -12.00dB */	{ 0xffe8,	4105,	0x1cf},
/*	 -12.50dB */	{ 0xffe7,	3876,	0x1cd},
/*	 -13.00dB */	{ 0xffe6,	3650,	0x1cb},
/*	 -13.50dB */	{ 0xffe5,	3446,	0x1c9},
/*	 -14.00dB */	{ 0xffe4,	3253,	0x1c7},
/*	 -14.50dB */	{ 0xffe3,	3071,	0x1c5},
/*	 -15.00dB */	{ 0xffe2,	2899,	0x1c3},
/*	 -15.50dB */	{ 0xffe1,	2737,	0x1c1},
/*	 -16.00dB */	{ 0xffe0,	2584,	0x1bf},
/*	 -16.50dB */	{ 0xffdf,	2439,	0x1bd},
/*	 -17.00dB */	{ 0xffde,	2303,	0x1bb},
/*	 -17.50dB */	{ 0xffdd,	2174,	0x1b9},
/*	 -18.00dB */	{ 0xffdc,	2052,	0x1b7},
/*	 -18.50dB */	{ 0xffdb,	1938,	0x1b5},
/*	 -19.00dB */	{ 0xffda,	1825,	0x1b3},
/*	 -19.50dB */	{ 0xffd9,	1723,	0x1b1},
/*	 -20.00dB */	{ 0xffd8,	1626,	0x1af},
/*	 -20.50dB */	{ 0xffd7,	1535,	0x1ad},
/*	 -21.00dB */	{ 0xffd6,	1449,	0x1ab},
/*	 -21.50dB */	{ 0xffd5,	1368,	0x1a9},
/*	 -22.00dB */	{ 0xffd4,	1292,	0x1a7},
/*	 -22.50dB */	{ 0xffd3,	1219,	0x1a5},
/*	 -23.00dB */	{ 0xffd2,	1151,	0x1a3},
/*	 -23.50dB */	{ 0xffd1,	1087,	0x1a1},
/*	 -24.00dB */	{ 0xffd0,	1026,	0x19f},
/*	 -24.50dB */	{ 0xffcf,	969,	0x19d},
/*	 -25.00dB */	{ 0xffce,	912,	0x19b},
/*	 -25.50dB */	{ 0xffcd,	861,	0x199},
/*	 -26.00dB */	{ 0xffcc,	813,	0x197},
/*	 -26.50dB */	{ 0xffcb,	767,	0x195},
/*	 -27.00dB */	{ 0xffca,	724,	0x193},
/*	 -27.50dB */	{ 0xffc9,	684,	0x191},
/*	 -28.00dB */	{ 0xffc8,	646,	0x18f},
/*	 -28.50dB */	{ 0xffc7,	609,	0x18d},
/*	 -29.00dB */	{ 0xffc6,	575,	0x18b},
/*	 -29.50dB */	{ 0xffc5,	543,	0x189},
/*	 -30.00dB */	{ 0xffc4,	513,	0x187},
/*	 -30.50dB */	{ 0xffc3,	484,	0x185},
/*	 -31.00dB */	{ 0xffc2,	456,	0x183},
/*	 -31.50dB */	{ 0xffc1,	430,	0x181},
/*	 -32.00dB */	{ 0xffc0,	406,	0x17f},
/*	 -32.50dB */	{ 0xffbf,	383,	0x17d},
/*	 -33.00dB */	{ 0xffbe,	362,	0x17b},
/*	 -33.50dB */	{ 0xffbd,	342,	0x179},
/*	 -34.00dB */	{ 0xffbc,	323,	0x177},
/*	 -34.50dB */	{ 0xffbb,	304,	0x175},
/*	 -35.00dB */	{ 0xffba,	287,	0x173},
/*	 -35.50dB */	{ 0xffb9,	271,	0x171},
/*	 -36.00dB */	{ 0xffb8,	256,	0x16f},
/*	 -36.50dB */	{ 0xffb7,	242,	0x16d},
/*	 -37.00dB */	{ 0xffb6,	228,	0x16b},
/*	 -37.50dB */	{ 0xffb5,	215,	0x169},
/*	 -38.00dB */	{ 0xffb4,	203,	0x167},
/*	 -38.50dB */	{ 0xffb3,	191,	0x165},
/*	 -39.00dB */	{ 0xffb2,	181,	0x163},
/*	 -39.50dB */	{ 0xffb1,	171,	0x161},
/*	 -40.00dB */	{ 0xffb0,	161,	0x15f},
/*	 -40.50dB */	{ 0xffaf,	152,	0x15d},
/*	 -41.00dB */	{ 0xffae,	143,	0x15b},
/*	 -41.50dB */	{ 0xffad,	135,	0x159},
/*	 -42.00dB */	{ 0xffac,	128,	0x157},
/*	 -42.50dB */	{ 0xffab,	121,	0x155},
/*	 -43.00dB */	{ 0xffaa,	114,	0x153},
/*	 -43.50dB */	{ 0xffa9,	107,	0x151},
/*	 -44.00dB */	{ 0xffa8,	101,	0x14f},
/*	 -44.50dB */	{ 0xffa7,	95,	0x14d},
/*	 -45.00dB */	{ 0xffa6,	90,	0x14b},
/*	 -45.50dB */	{ 0xffa5,	85,	0x149},
/*	 -46.00dB */	{ 0xffa4,	80,	0x147},
/*	 -46.50dB */	{ 0xffa3,	76,	0x145},
/*	 -47.00dB */	{ 0xffa2,	71,	0x143},
/*	 -47.50dB */	{ 0xffa1,	67,	0x141},
/*	 -48.00dB */	{ 0xffa0,	64,	0x13f},
/*	 -48.50dB */	{ 0xff9f,	60,	0x13d},
/*	 -49.00dB */	{ 0xff9e,	57,	0x13b},
/*	 -49.50dB */	{ 0xff9d,	53,	0x139},
/*	 -50.00dB */	{ 0xff9c,	50,	0x137},
/*	 -50.50dB */	{ 0xff9b,	47,	0x135},
/* End of table*/   { 0xff9a,   0,	END_OF_TABLE}
};


static AUDTABL_GainMapping_t GainMapping_Table[] =
{
#if defined(PMU_BCM59055) || defined(CONFIG_BCMPMU_AUDIO) 
	//Audio Mode        // Total Gain(miliBel)   // Gain in PMU    // Gain in AudioHW(miliBel)
/* Earpiece Mode */    
	{AUDIO_MODE_HANDSET,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
/* Headset Mode */    
	{AUDIO_MODE_HEADSET,	/*-60.00dB */ 0xffffe890,	PMU_HSGAIN_60DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-59.50dB */ 0xffffe8c2,	PMU_HSGAIN_57DB_N,	0xffffff06},
	{AUDIO_MODE_HEADSET,	/*-59.00dB */ 0xffffe8f4,	PMU_HSGAIN_57DB_N,	0xffffff38},
	{AUDIO_MODE_HEADSET,	/*-58.50dB */ 0xffffe926,	PMU_HSGAIN_57DB_N,	0xffffff6a},
	{AUDIO_MODE_HEADSET,	/*-58.00dB */ 0xffffe958,	PMU_HSGAIN_57DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-57.50dB */ 0xffffe98a,	PMU_HSGAIN_57DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-57.00dB */ 0xffffe9bc,	PMU_HSGAIN_57DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-56.50dB */ 0xffffe9ee,	PMU_HSGAIN_54DB_N,	0xffffff06},
	{AUDIO_MODE_HEADSET,	/*-56.00dB */ 0xffffea20,	PMU_HSGAIN_54DB_N,	0xffffff38},
	{AUDIO_MODE_HEADSET,	/*-55.50dB */ 0xffffea52,	PMU_HSGAIN_54DB_N,	0xffffff6a},
	{AUDIO_MODE_HEADSET,	/*-55.00dB */ 0xffffea84,	PMU_HSGAIN_54DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-54.50dB */ 0xffffeab6,	PMU_HSGAIN_54DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-54.00dB */ 0xffffeae8,	PMU_HSGAIN_54DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-53.50dB */ 0xffffeb1a,	PMU_HSGAIN_51DB_N,	0xffffff06},
	{AUDIO_MODE_HEADSET,	/*-53.00dB */ 0xffffeb4c,	PMU_HSGAIN_51DB_N,	0xffffff38},
	{AUDIO_MODE_HEADSET,	/*-52.50dB */ 0xffffeb7e,	PMU_HSGAIN_51DB_N,	0xffffff6a},
	{AUDIO_MODE_HEADSET,	/*-52.00dB */ 0xffffebb0,	PMU_HSGAIN_51DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-51.50dB */ 0xffffebe2,	PMU_HSGAIN_51DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-51.00dB */ 0xffffec14,	PMU_HSGAIN_51DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-50.50dB */ 0xffffec46,	PMU_HSGAIN_48DB_N,	0xffffff06},
	{AUDIO_MODE_HEADSET,	/*-50.00dB */ 0xffffec78,	PMU_HSGAIN_48DB_N,	0xffffff38},
	{AUDIO_MODE_HEADSET,	/*-49.50dB */ 0xffffecaa,	PMU_HSGAIN_48DB_N,	0xffffff6a},
	{AUDIO_MODE_HEADSET,	/*-49.00dB */ 0xffffecdc,	PMU_HSGAIN_48DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-48.50dB */ 0xffffed0e,	PMU_HSGAIN_48DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-48.00dB */ 0xffffed40,	PMU_HSGAIN_48DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-47.50dB */ 0xffffed72,	PMU_HSGAIN_45DB_N,	0xffffff06},
	{AUDIO_MODE_HEADSET,	/*-47.00dB */ 0xffffeda4,	PMU_HSGAIN_45DB_N,	0xffffff38},
	{AUDIO_MODE_HEADSET,	/*-46.50dB */ 0xffffedd6,	PMU_HSGAIN_45DB_N,	0xffffff6a},
	{AUDIO_MODE_HEADSET,	/*-46.00dB */ 0xffffee08,	PMU_HSGAIN_45DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-45.50dB */ 0xffffee3a,	PMU_HSGAIN_45DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-45.00dB */ 0xffffee6c,	PMU_HSGAIN_45DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-44.50dB */ 0xffffee9e,	PMU_HSGAIN_42DB_N,	0xffffff06},
	{AUDIO_MODE_HEADSET,	/*-44.00dB */ 0xffffeed0,	PMU_HSGAIN_42DB_N,	0xffffff38},
	{AUDIO_MODE_HEADSET,	/*-43.50dB */ 0xffffef02,	PMU_HSGAIN_42DB_N,	0xffffff6a},
	{AUDIO_MODE_HEADSET,	/*-43.00dB */ 0xffffef34,	PMU_HSGAIN_42DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-42.50dB */ 0xffffef66,	PMU_HSGAIN_42DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-42.00dB */ 0xffffef98,	PMU_HSGAIN_42DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-41.50dB */ 0xffffefca,	PMU_HSGAIN_40P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-41.00dB */ 0xffffeffc,	PMU_HSGAIN_40P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-40.50dB */ 0xfffff02e,	PMU_HSGAIN_40P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-40.00dB */ 0xfffff060,	PMU_HSGAIN_39DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-39.50dB */ 0xfffff092,	PMU_HSGAIN_39DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-39.00dB */ 0xfffff0c4,	PMU_HSGAIN_39DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-38.50dB */ 0xfffff0f6,	PMU_HSGAIN_37P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-38.00dB */ 0xfffff128,	PMU_HSGAIN_37P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-37.50dB */ 0xfffff15a,	PMU_HSGAIN_37P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-37.00dB */ 0xfffff18c,	PMU_HSGAIN_36DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-36.50dB */ 0xfffff1be,	PMU_HSGAIN_36DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-36.00dB */ 0xfffff1f0,	PMU_HSGAIN_36DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-35.50dB */ 0xfffff222,	PMU_HSGAIN_34P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-35.00dB */ 0xfffff254,	PMU_HSGAIN_34P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-34.50dB */ 0xfffff286,	PMU_HSGAIN_34P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-34.00dB */ 0xfffff2b8,	PMU_HSGAIN_33DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-33.50dB */ 0xfffff2ea,	PMU_HSGAIN_33DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-33.00dB */ 0xfffff31c,	PMU_HSGAIN_33DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-32.50dB */ 0xfffff34e,	PMU_HSGAIN_31P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-32.00dB */ 0xfffff380,	PMU_HSGAIN_31P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-31.50dB */ 0xfffff3b2,	PMU_HSGAIN_31P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-31.00dB */ 0xfffff3e4,	PMU_HSGAIN_30DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-30.50dB */ 0xfffff416,	PMU_HSGAIN_30DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-30.00dB */ 0xfffff448,	PMU_HSGAIN_30DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-29.50dB */ 0xfffff47a,	PMU_HSGAIN_28P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-29.00dB */ 0xfffff4ac,	PMU_HSGAIN_28P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-28.50dB */ 0xfffff4de,	PMU_HSGAIN_28P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-28.00dB */ 0xfffff510,	PMU_HSGAIN_27DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-27.50dB */ 0xfffff542,	PMU_HSGAIN_27DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-27.00dB */ 0xfffff574,	PMU_HSGAIN_27DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-26.50dB */ 0xfffff5a6,	PMU_HSGAIN_25P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-26.00dB */ 0xfffff5d8,	PMU_HSGAIN_25P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-25.50dB */ 0xfffff60a,	PMU_HSGAIN_25P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-25.00dB */ 0xfffff63c,	PMU_HSGAIN_24DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-24.50dB */ 0xfffff66e,	PMU_HSGAIN_24DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-24.00dB */ 0xfffff6a0,	PMU_HSGAIN_24DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-23.50dB */ 0xfffff6d2,	PMU_HSGAIN_22P5DB_N,	0xffffff9c},
	{AUDIO_MODE_HEADSET,	/*-23.00dB */ 0xfffff704,	PMU_HSGAIN_22P5DB_N,	0xffffffce},
	{AUDIO_MODE_HEADSET,	/*-22.50dB */ 0xfffff736,	PMU_HSGAIN_22P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-22.00dB */ 0xfffff768,	PMU_HSGAIN_22DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-21.50dB */ 0xfffff79a,	PMU_HSGAIN_21P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-21.00dB */ 0xfffff7cc,	PMU_HSGAIN_21DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-20.50dB */ 0xfffff7fe,	PMU_HSGAIN_20P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-20.00dB */ 0xfffff830,	PMU_HSGAIN_20DB_N,	0x00},
	{AUDIO_MODE_HEADSET,	/*-19.50dB */ 0xfffff862,	PMU_HSGAIN_19P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-19.00dB */ 0xfffff894,	PMU_HSGAIN_19DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-18.50dB */ 0xfffff8c6,	PMU_HSGAIN_18P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-18.00dB */ 0xfffff8f8,	PMU_HSGAIN_18DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-17.50dB */ 0xfffff92a,	PMU_HSGAIN_17P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-17.00dB */ 0xfffff95c,	PMU_HSGAIN_17DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-16.50dB */ 0xfffff98e,	PMU_HSGAIN_16P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-16.00dB */ 0xfffff9c0,	PMU_HSGAIN_16DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-15.50dB */ 0xfffff9f2,	PMU_HSGAIN_15P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-15.00dB */ 0xfffffa24,	PMU_HSGAIN_15DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-14.50dB */ 0xfffffa56,	PMU_HSGAIN_14P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-14.00dB */ 0xfffffa88,	PMU_HSGAIN_14DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-13.50dB */ 0xfffffaba,	PMU_HSGAIN_13P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-13.00dB */ 0xfffffaec,	PMU_HSGAIN_13DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-12.50dB */ 0xfffffb1e,	PMU_HSGAIN_12P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-12.00dB */ 0xfffffb50,	PMU_HSGAIN_12DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-11.50dB */ 0xfffffb82,	PMU_HSGAIN_11P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-11.00dB */ 0xfffffbb4,	PMU_HSGAIN_11DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-10.50dB */ 0xfffffbe6,	PMU_HSGAIN_10P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-10.00dB */ 0xfffffc18,	PMU_HSGAIN_10DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-9.50dB */ 0xfffffc4a,	PMU_HSGAIN_9P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-9.00dB */ 0xfffffc7c,	PMU_HSGAIN_9DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-8.50dB */ 0xfffffcae,	PMU_HSGAIN_8P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-8.00dB */ 0xfffffce0,	PMU_HSGAIN_8DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-7.50dB */ 0xfffffd12,	PMU_HSGAIN_7P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-7.00dB */ 0xfffffd44,	PMU_HSGAIN_7DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-6.50dB */ 0xfffffd76,	PMU_HSGAIN_6P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-6.00dB */ 0xfffffda8,	PMU_HSGAIN_6DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-5.50dB */ 0xfffffdda,	PMU_HSGAIN_5P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-5.00dB */ 0xfffffe0c,	PMU_HSGAIN_5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-4.50dB */ 0xfffffe3e,	PMU_HSGAIN_4P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-4.00dB */ 0xfffffe70,	PMU_HSGAIN_4DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-3.50dB */ 0xfffffea2,	PMU_HSGAIN_3P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-3.00dB */ 0xfffffed4,	PMU_HSGAIN_3DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-2.50dB */ 0xffffff06,	PMU_HSGAIN_2P5DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-2.00dB */ 0xffffff38,	PMU_HSGAIN_2DB_N,	0x0},
	{AUDIO_MODE_HEADSET,	/*-1.50dB */ 0xffffff6a,	PMU_HSGAIN_2DB_N,	0x32},
	{AUDIO_MODE_HEADSET,	/*-1.00dB */ 0xffffff9c,	PMU_HSGAIN_2DB_N,	0x64},
	{AUDIO_MODE_HEADSET,	/*-0.50dB */ 0xffffffce,	PMU_HSGAIN_2DB_N,	0x96},
	{AUDIO_MODE_HEADSET,	/*0.00dB */ 0x0,	PMU_HSGAIN_2DB_N,	0xc8},
	{AUDIO_MODE_HEADSET,	/*0.50dB */ 0x32,	PMU_HSGAIN_2DB_N,	0xfa},
	{AUDIO_MODE_HEADSET,	/*1.00dB */ 0x64,	PMU_HSGAIN_2DB_N,	0x12c},
	{AUDIO_MODE_HEADSET,	/*1.50dB */ 0x96,	PMU_HSGAIN_2DB_N,	0x15e},
	{AUDIO_MODE_HEADSET,	/*2.00dB */ 0xc8,	PMU_HSGAIN_2DB_N,	0x190},
	{AUDIO_MODE_HEADSET,	/*2.50dB */ 0xfa,	PMU_HSGAIN_2DB_N,	0x1c2},
	{AUDIO_MODE_HEADSET,	/*3.00dB */ 0x12c,	PMU_HSGAIN_2DB_N,	0x1f4},
	{AUDIO_MODE_HEADSET,	/*3.50dB */ 0x15e,	PMU_HSGAIN_2DB_N,	0x226},
	{AUDIO_MODE_HEADSET,	/*4.00dB */ 0x190,	PMU_HSGAIN_2DB_N,	0x258},
	{AUDIO_MODE_HEADSET,	/*4.50dB */ 0x1c2,	PMU_HSGAIN_2DB_N,	0x28a},
	{AUDIO_MODE_HEADSET,	/*5.00dB */ 0x1f4,	PMU_HSGAIN_2DB_N,	0x2bc},
	{AUDIO_MODE_HEADSET,	/*5.50dB */ 0x226,	PMU_HSGAIN_2DB_N,	0x2ee},
	{AUDIO_MODE_HEADSET,	/*6.00dB */ 0x258,	PMU_HSGAIN_2DB_N,	0x320},
	{AUDIO_MODE_HEADSET,	/*6.50dB */ 0x28a,	PMU_HSGAIN_2DB_N,	0x352},
	{AUDIO_MODE_HEADSET,	/*7.00dB */ 0x2bc,	PMU_HSGAIN_2DB_N,	0x384},
	{AUDIO_MODE_HEADSET,	/*7.50dB */ 0x2ee,	PMU_HSGAIN_2DB_N,	0x3b6},
	{AUDIO_MODE_HEADSET,	/*8.00dB */ 0x320,	PMU_HSGAIN_2DB_N,	0x3e8},
	{AUDIO_MODE_HEADSET,	/*8.50dB */ 0x352,	PMU_HSGAIN_2DB_N,	0x41a},
	{AUDIO_MODE_HEADSET,	/*9.00dB */ 0x384,	PMU_HSGAIN_2DB_N,	0x44c},
	{AUDIO_MODE_HEADSET,	/*9.50dB */ 0x3b6,	PMU_HSGAIN_2DB_N,	0x47e},
	{AUDIO_MODE_HEADSET,	/*10.00dB */ 0x3e8,	PMU_HSGAIN_2DB_N,	0x4b0},
	{AUDIO_MODE_HEADSET,	/*10.50dB */ 0x41a,	PMU_HSGAIN_2DB_N,	0x4e2},
	{AUDIO_MODE_HEADSET,	/*11.00dB */ 0x44c,	PMU_HSGAIN_2DB_N,	0x514},
	{AUDIO_MODE_HEADSET,	/*11.50dB */ 0x47e,	PMU_HSGAIN_2DB_N,	0x546},
	{AUDIO_MODE_HEADSET,	/*12.00dB */ 0x4b0,	PMU_HSGAIN_2DB_N,	0x578},
	{AUDIO_MODE_HEADSET,	/*12.50dB */ 0x4e2,	PMU_HSGAIN_2DB_N,	0x5aa},
	{AUDIO_MODE_HEADSET,	/*13.00dB */ 0x514,	PMU_HSGAIN_2DB_N,	0x5dc},
	{AUDIO_MODE_HEADSET,	/*13.50dB */ 0x546,	PMU_HSGAIN_2DB_N,	0x60e},
	{AUDIO_MODE_HEADSET,	/*14.00dB */ 0x578,	PMU_HSGAIN_2DB_N,	0x640},
	{AUDIO_MODE_HEADSET,	/*14.50dB */ 0x5aa,	PMU_HSGAIN_2DB_N,	0x672},
	{AUDIO_MODE_HEADSET,	/*15.00dB */ 0x5dc,	PMU_HSGAIN_2DB_N,	0x6a4},
	{AUDIO_MODE_HEADSET,	/*15.50dB */ 0x60e,	PMU_HSGAIN_2DB_N,	0x6d6},
	{AUDIO_MODE_HEADSET,	/*16.00dB */ 0x640,	PMU_HSGAIN_2DB_N,	0x708},
	{AUDIO_MODE_HEADSET,	/*16.50dB */ 0x672,	PMU_HSGAIN_2DB_N,	0x73a},
	{AUDIO_MODE_HEADSET,	/*17.00dB */ 0x6a4,	PMU_HSGAIN_2DB_N,	0x76c},
	{AUDIO_MODE_HEADSET,	/*17.50dB */ 0x6d6,	PMU_HSGAIN_2DB_N,	0x79e},
	{AUDIO_MODE_HEADSET,	/*18.00dB */ 0x708,	PMU_HSGAIN_2DB_N,	0x7d0},
	{AUDIO_MODE_HEADSET,	/*18.50dB */ 0x73a,	PMU_HSGAIN_2DB_N,	0x802},
	{AUDIO_MODE_HEADSET,	/*19.00dB */ 0x76c,	PMU_HSGAIN_2DB_N,	0x834},
	{AUDIO_MODE_HEADSET,	/*19.50dB */ 0x79e,	PMU_HSGAIN_2DB_N,	0x866},
	{AUDIO_MODE_HEADSET,	/*20.00dB */ 0x7d0,	PMU_HSGAIN_2DB_N,	0x898},
	{AUDIO_MODE_HEADSET,	/*20.50dB */ 0x802,	PMU_HSGAIN_2DB_N,	0x8ca},
	{AUDIO_MODE_HEADSET,	/*21.00dB */ 0x834,	PMU_HSGAIN_2DB_N,	0x8fc},
	{AUDIO_MODE_HEADSET,	/*21.50dB */ 0x866,	PMU_HSGAIN_2DB_N,	0x92e},
	{AUDIO_MODE_HEADSET,	/*22.00dB */ 0x898,	PMU_HSGAIN_2DB_N,	0x960},
	{AUDIO_MODE_HEADSET,	/*22.50dB */ 0x8ca,	PMU_HSGAIN_2DB_N,	0x992},
	{AUDIO_MODE_HEADSET,	/*23.00dB */ 0x8fc,	PMU_HSGAIN_2DB_N,	0x9c4},
	{AUDIO_MODE_HEADSET,	/*23.50dB */ 0x92e,	PMU_HSGAIN_2DB_N,	0x9f6},
	{AUDIO_MODE_HEADSET,	/*24.00dB */ 0x960,	PMU_HSGAIN_2DB_N,	0xa28},
	{AUDIO_MODE_HEADSET,	/*24.50dB */ 0x992,	PMU_HSGAIN_2DB_N,	0xa5a},
	{AUDIO_MODE_HEADSET,	/*25.00dB */ 0x9c4,	PMU_HSGAIN_2DB_N,	0xa8c},
	{AUDIO_MODE_HEADSET,	/*25.50dB */ 0x9f6,	PMU_HSGAIN_2DB_N,	0xabe},
	{AUDIO_MODE_HEADSET,	/*26.00dB */ 0xa28,	PMU_HSGAIN_2DB_N,	0xaf0},
	{AUDIO_MODE_HEADSET,	/*26.50dB */ 0xa5a,	PMU_HSGAIN_2DB_N,	0xb22},
	{AUDIO_MODE_HEADSET,	/*27.00dB */ 0xa8c,	PMU_HSGAIN_2DB_N,	0xb54},
	{AUDIO_MODE_HEADSET,	/*27.50dB */ 0xabe,	PMU_HSGAIN_2DB_N,	0xb86},
	{AUDIO_MODE_HEADSET,	/*28.00dB */ 0xaf0,	PMU_HSGAIN_2DB_N,	0xbb8},
	{AUDIO_MODE_HEADSET,	/*28.50dB */ 0xb22,	PMU_HSGAIN_2DB_N,	0xbea},
	{AUDIO_MODE_HEADSET,	/*29.00dB */ 0xb54,	PMU_HSGAIN_2DB_N,	0xc1c},
	{AUDIO_MODE_HEADSET,	/*29.50dB */ 0xb86,	PMU_HSGAIN_2DB_N,	0xc4e},
	{AUDIO_MODE_HEADSET,	/*30.00dB */ 0xbb8,	PMU_HSGAIN_2DB_N,	0xc80},
	{AUDIO_MODE_HEADSET,	/*30.50dB */ 0xbea,	PMU_HSGAIN_2DB_N,	0xcb2},
	{AUDIO_MODE_HEADSET,	/*31.00dB */ 0xc1c,	PMU_HSGAIN_2DB_N,	0xce4},
	{AUDIO_MODE_HEADSET,	/*31.50dB */ 0xc4e,	PMU_HSGAIN_2DB_N,	0xd16},
	{AUDIO_MODE_HEADSET,	/*32.00dB */ 0xc80,	PMU_HSGAIN_2DB_N,	0xd48},
	{AUDIO_MODE_HEADSET,	/*32.50dB */ 0xcb2,	PMU_HSGAIN_2DB_N,	0xd7a},
	{AUDIO_MODE_HEADSET,	/*33.00dB */ 0xce4,	PMU_HSGAIN_2DB_N,	0xdac},
	{AUDIO_MODE_HEADSET,	/*33.50dB */ 0xd16,	PMU_HSGAIN_2DB_N,	0xdde},
	{AUDIO_MODE_HEADSET,	/*34.00dB */ 0xd48,	PMU_HSGAIN_2DB_N,	0xe10},
	{AUDIO_MODE_HEADSET,	/*34.50dB */ 0xd7a,	PMU_HSGAIN_2DB_N,	0xe42},
	{AUDIO_MODE_HEADSET,	/*35.00dB */ 0xdac,	PMU_HSGAIN_2DB_N,	0xe74},
	{AUDIO_MODE_HEADSET,	/*35.50dB */ 0xdde,	PMU_HSGAIN_2DB_N,	0xea6},
	{AUDIO_MODE_HEADSET,	/*36.00dB */ 0xe10,	PMU_HSGAIN_2DB_N,	0xed8},
	{AUDIO_MODE_HEADSET,	/*36.50dB */ 0xe42,	PMU_HSGAIN_2DB_N,	0xf0a},
	{AUDIO_MODE_HEADSET,	/*37.00dB */ 0xe74,	PMU_HSGAIN_2DB_N,	0xf3c},
	{AUDIO_MODE_HEADSET,	/*37.50dB */ 0xea6,	PMU_HSGAIN_2DB_N,	0xf6e},
	{AUDIO_MODE_HEADSET,	/*38.00dB */ 0xed8,	PMU_HSGAIN_2DB_N,	0xfa0},
	{AUDIO_MODE_HEADSET,	/*38.50dB */ 0xf0a,	PMU_HSGAIN_2DB_N,	0xfd2},
	{AUDIO_MODE_HEADSET,	/*39.00dB */ 0xf3c,	PMU_HSGAIN_2DB_N,	0x1004},
	{AUDIO_MODE_HEADSET,	/*39.50dB */ 0xf6e,	PMU_HSGAIN_2DB_N,	0x1036},
	{AUDIO_MODE_HEADSET,	/*40.00dB */ 0xfa0,	PMU_HSGAIN_2DB_N,	0x1068},
	{AUDIO_MODE_HEADSET,	/*40.50dB */ 0xfd2,	PMU_HSGAIN_2DB_N,	0x109a},
	{AUDIO_MODE_HEADSET,	/*41.00dB */ 0x1004,	PMU_HSGAIN_2DB_N,	0x10cc},
	{AUDIO_MODE_HEADSET,	/*41.50dB */ 0x1036,	PMU_HSGAIN_2DB_N,	0x10fe},
	{AUDIO_MODE_HEADSET,	/*42.50dB */ 0x1036,	PMU_HSGAIN_2DB_N,	0x1130},
	{AUDIO_MODE_HEADSET,	/*42.00dB */ 0x1068,	PMU_HSGAIN_2DB_N,	0x1162},
	{AUDIO_MODE_HEADSET,	/*43.00dB */ 0x10cc,	PMU_HSGAIN_2DB_N,	0x1194},
	{AUDIO_MODE_HEADSET,	/*43.50dB */ 0x10fe,	PMU_HSGAIN_2DB_N,	0x11c6},
	{AUDIO_MODE_HEADSET,	/*44.00dB */ 0x1130,	PMU_HSGAIN_2DB_N,	0x11f8},
	{AUDIO_MODE_HEADSET,	/*44.50dB */ 0x1162,	PMU_HSGAIN_2DB_N,	0x122a},
	{AUDIO_MODE_HEADSET,	/*45.00dB */ 0x1194,	PMU_HSGAIN_2DB_N,	0x125c},
	{AUDIO_MODE_HEADSET,	/*45.50dB */ 0x11c6,	PMU_HSGAIN_2DB_N,	0x128e},
	{AUDIO_MODE_HEADSET,	/*46.00dB */ 0x11f8,	PMU_HSGAIN_2DB_N,	0x12c0},
	{AUDIO_MODE_HEADSET,	/*46.50dB */ 0x122a,	PMU_HSGAIN_2DB_N,	0x12f2},
	{AUDIO_MODE_HEADSET,	/*47.00dB */ 0x125c,	PMU_HSGAIN_2DB_N,	0x1324},
	{AUDIO_MODE_HEADSET,	/*47.50dB */ 0x128e,	PMU_HSGAIN_2DB_N,	0x1356},
	{AUDIO_MODE_HEADSET,	/*48.00dB */ 0x12c0,	PMU_HSGAIN_2DB_N,	0x1388},
	{AUDIO_MODE_HEADSET,	/*48.50dB */ 0x12f2,	PMU_HSGAIN_2DB_N,	0x13ba},
	{AUDIO_MODE_HEADSET,	/*49.00dB */ 0x1324,	PMU_HSGAIN_2DB_N,	0x13ec},
	{AUDIO_MODE_HEADSET,	/*49.50dB */ 0x1356,	PMU_HSGAIN_2DB_N,	0x141e},
	{AUDIO_MODE_HEADSET,	/*50.00dB */ 0x1388,	PMU_HSGAIN_2DB_N,	0x1450},
	{AUDIO_MODE_HEADSET,	/*50.50dB */ 0x13ba,	PMU_HSGAIN_2DB_N,	0x1482},
	{AUDIO_MODE_HEADSET,	/*51.00dB */ 0x13ec,	PMU_HSGAIN_2DB_N,	0x14b4},
	{AUDIO_MODE_HEADSET,	/*51.50dB */ 0x141e,	PMU_HSGAIN_2DB_N,	0x14e6},
	{AUDIO_MODE_HEADSET,	/*52.00dB */ 0x1450,	PMU_HSGAIN_2DB_N,	0x1518},
	{AUDIO_MODE_HEADSET,	/*52.50dB */ 0x1482,	PMU_HSGAIN_2DB_N,	0x154a},
	{AUDIO_MODE_HEADSET,	/*53.00dB */ 0x14b4,	PMU_HSGAIN_2DB_N,	0x157c},
	{AUDIO_MODE_HEADSET,	/*53.50dB */ 0x14e6,	PMU_HSGAIN_2DB_N,	0x15ae},
	{AUDIO_MODE_HEADSET,	/*54.00dB */ 0x1518,	PMU_HSGAIN_2DB_N,	0x15e0},
	{AUDIO_MODE_HEADSET,	/*54.50dB */ 0x154a,	PMU_HSGAIN_2DB_N,	0x1612},
	{AUDIO_MODE_HEADSET,	/*55.00dB */ 0x157c,	PMU_HSGAIN_2DB_N,	0x1644},
	{AUDIO_MODE_HEADSET,	/*55.50dB */ 0x15ae,	PMU_HSGAIN_2DB_N,	0x1676},
	{AUDIO_MODE_HEADSET,	/*56.00dB */ 0x15e0,	PMU_HSGAIN_2DB_N,	0x16a8},
	{AUDIO_MODE_HEADSET,	/*56.50dB */ 0x1612,	PMU_HSGAIN_2DB_N,	0x16da},
	{AUDIO_MODE_HEADSET,	/*57.00dB */ 0x1644,	PMU_HSGAIN_2DB_N,	0x170c},
	{AUDIO_MODE_HEADSET,	/*57.50dB */ 0x1676,	PMU_HSGAIN_2DB_N,	0x173e},
	{AUDIO_MODE_HEADSET,	/*58.00dB */ 0x16a8,	PMU_HSGAIN_2DB_N,	0x1770},
	{AUDIO_MODE_HEADSET,	/*58.50dB */ 0x16da,	PMU_HSGAIN_2DB_N,	0x17a2},
	{AUDIO_MODE_HEADSET,	/*59.00dB */ 0x170c,	PMU_HSGAIN_2DB_N,	0x17d4},
	{AUDIO_MODE_HEADSET,	/*59.50dB */ 0x173e,	PMU_HSGAIN_2DB_N,	0x1806},
	{AUDIO_MODE_HEADSET,	/*60.00dB */ 0x1770,	PMU_HSGAIN_2DB_N,	0x1838},

/* IHF Speaker Mode */
	{AUDIO_MODE_SPEAKERPHONE,	/*-60.00dB*/ 0xffffe890,	PMU_IHFGAIN_60DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-59.50dB*/ 0xffffe8c2,	PMU_IHFGAIN_57DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-59.00dB*/ 0xffffe8f4,	PMU_IHFGAIN_57DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-58.50dB*/ 0xffffe926,	PMU_IHFGAIN_57DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-58.00dB*/ 0xffffe958,	PMU_IHFGAIN_57DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-57.50dB*/ 0xffffe98a,	PMU_IHFGAIN_57DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-57.00dB*/ 0xffffe9bc,	PMU_IHFGAIN_57DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-56.50dB*/ 0xffffe9ee,	PMU_IHFGAIN_54DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-56.00dB*/ 0xffffea20,	PMU_IHFGAIN_54DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-55.50dB*/ 0xffffea52,	PMU_IHFGAIN_54DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-55.00dB*/ 0xffffea84,	PMU_IHFGAIN_54DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-54.50dB*/ 0xffffeab6,	PMU_IHFGAIN_54DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-54.00dB*/ 0xffffeae8,	PMU_IHFGAIN_54DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-53.50dB*/ 0xffffeb1a,	PMU_IHFGAIN_51DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-53.00dB*/ 0xffffeb4c,	PMU_IHFGAIN_51DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-52.50dB*/ 0xffffeb7e,	PMU_IHFGAIN_51DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-52.00dB*/ 0xffffebb0,	PMU_IHFGAIN_51DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-51.50dB*/ 0xffffebe2,	PMU_IHFGAIN_51DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-51.00dB*/ 0xffffec14,	PMU_IHFGAIN_51DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-50.50dB*/ 0xffffec46,	PMU_IHFGAIN_48DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-50.00dB*/ 0xffffec78,	PMU_IHFGAIN_48DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-49.50dB*/ 0xffffecaa,	PMU_IHFGAIN_48DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-49.00dB*/ 0xffffecdc,	PMU_IHFGAIN_48DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-48.50dB*/ 0xffffed0e,	PMU_IHFGAIN_48DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-48.00dB*/ 0xffffed40,	PMU_IHFGAIN_48DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-47.50dB*/ 0xffffed72,	PMU_IHFGAIN_45DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-47.00dB*/ 0xffffeda4,	PMU_IHFGAIN_45DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-46.50dB*/ 0xffffedd6,	PMU_IHFGAIN_45DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-46.00dB*/ 0xffffee08,	PMU_IHFGAIN_45DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-45.50dB*/ 0xffffee3a,	PMU_IHFGAIN_45DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-45.00dB*/ 0xffffee6c,	PMU_IHFGAIN_45DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-44.50dB*/ 0xffffee9e,	PMU_IHFGAIN_42DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-44.00dB*/ 0xffffeed0,	PMU_IHFGAIN_42DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-43.50dB*/ 0xffffef02,	PMU_IHFGAIN_42DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-43.00dB*/ 0xffffef34,	PMU_IHFGAIN_42DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-42.50dB*/ 0xffffef66,	PMU_IHFGAIN_42DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-42.00dB*/ 0xffffef98,	PMU_IHFGAIN_42DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-41.50dB*/ 0xffffefca,	PMU_IHFGAIN_39DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-41.00dB*/ 0xffffeffc,	PMU_IHFGAIN_39DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-40.50dB*/ 0xfffff02e,	PMU_IHFGAIN_39DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-40.00dB*/ 0xfffff060,	PMU_IHFGAIN_39DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-39.50dB*/ 0xfffff092,	PMU_IHFGAIN_39DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-39.00dB*/ 0xfffff0c4,	PMU_IHFGAIN_39DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-38.50dB*/ 0xfffff0f6,	PMU_IHFGAIN_36DB_N,	0xffffff06},
	{AUDIO_MODE_SPEAKERPHONE,	/*-38.00dB*/ 0xfffff128,	PMU_IHFGAIN_36DB_N,	0xffffff38},
	{AUDIO_MODE_SPEAKERPHONE,	/*-37.50dB*/ 0xfffff15a,	PMU_IHFGAIN_36DB_N,	0xffffff6a},
	{AUDIO_MODE_SPEAKERPHONE,	/*-37.00dB*/ 0xfffff18c,	PMU_IHFGAIN_36DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-36.50dB*/ 0xfffff1be,	PMU_IHFGAIN_36DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-36.00dB*/ 0xfffff1f0,	PMU_IHFGAIN_36DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-35.50dB*/ 0xfffff222,	PMU_IHFGAIN_34P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-35.00dB*/ 0xfffff254,	PMU_IHFGAIN_34P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-34.50dB*/ 0xfffff286,	PMU_IHFGAIN_34P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-34.00dB*/ 0xfffff2b8,	PMU_IHFGAIN_33DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-33.50dB*/ 0xfffff2ea,	PMU_IHFGAIN_33DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-33.00dB*/ 0xfffff31c,	PMU_IHFGAIN_33DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-32.50dB*/ 0xfffff34e,	PMU_IHFGAIN_31P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-32.00dB*/ 0xfffff380,	PMU_IHFGAIN_31P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-31.50dB*/ 0xfffff3b2,	PMU_IHFGAIN_31P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-31.00dB*/ 0xfffff3e4,	PMU_IHFGAIN_30DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-30.50dB*/ 0xfffff416,	PMU_IHFGAIN_30DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-30.00dB*/ 0xfffff448,	PMU_IHFGAIN_30DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-29.50dB*/ 0xfffff47a,	PMU_IHFGAIN_28P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-29.00dB*/ 0xfffff4ac,	PMU_IHFGAIN_28P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-28.50dB*/ 0xfffff4de,	PMU_IHFGAIN_28P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-28.00dB*/ 0xfffff510,	PMU_IHFGAIN_27DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-27.50dB*/ 0xfffff542,	PMU_IHFGAIN_27DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-27.00dB*/ 0xfffff574,	PMU_IHFGAIN_27DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-26.50dB*/ 0xfffff5a6,	PMU_IHFGAIN_25P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-26.00dB*/ 0xfffff5d8,	PMU_IHFGAIN_25P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-25.50dB*/ 0xfffff60a,	PMU_IHFGAIN_25P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-25.00dB*/ 0xfffff63c,	PMU_IHFGAIN_24DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-24.50dB*/ 0xfffff66e,	PMU_IHFGAIN_24DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-24.00dB*/ 0xfffff6a0,	PMU_IHFGAIN_24DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-23.50dB*/ 0xfffff6d2,	PMU_IHFGAIN_22P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-23.00dB*/ 0xfffff704,	PMU_IHFGAIN_22P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-22.50dB*/ 0xfffff736,	PMU_IHFGAIN_22P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-22.00dB*/ 0xfffff768,	PMU_IHFGAIN_21DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-21.50dB*/ 0xfffff79a,	PMU_IHFGAIN_21DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-21.00dB*/ 0xfffff7cc,	PMU_IHFGAIN_21DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-20.50dB*/ 0xfffff7fe,	PMU_IHFGAIN_19P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-20.00dB*/ 0xfffff830,	PMU_IHFGAIN_19P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-19.50dB*/ 0xfffff862,	PMU_IHFGAIN_19P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-19.00dB*/ 0xfffff894,	PMU_IHFGAIN_18DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-18.50dB*/ 0xfffff8c6,	PMU_IHFGAIN_18DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-18.00dB*/ 0xfffff8f8,	PMU_IHFGAIN_18DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-17.50dB*/ 0xfffff92a,	PMU_IHFGAIN_16P5DB_N,	0xffffff9c},
	{AUDIO_MODE_SPEAKERPHONE,	/*-17.00dB*/ 0xfffff95c,	PMU_IHFGAIN_16P5DB_N,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*-16.50dB*/ 0xfffff98e,	PMU_IHFGAIN_16P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-16.00dB*/ 0xfffff9c0,	PMU_IHFGAIN_16DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-15.50dB*/ 0xfffff9f2,	PMU_IHFGAIN_15P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-15.00dB*/ 0xfffffa24,	PMU_IHFGAIN_15DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-14.50dB*/ 0xfffffa56,	PMU_IHFGAIN_14P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-14.00dB*/ 0xfffffa88,	PMU_IHFGAIN_14DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-13.50dB*/ 0xfffffaba,	PMU_IHFGAIN_13P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-13.00dB*/ 0xfffffaec,	PMU_IHFGAIN_13DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-12.50dB*/ 0xfffffb1e,	PMU_IHFGAIN_12P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-12.00dB*/ 0xfffffb50,	PMU_IHFGAIN_12DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-11.50dB*/ 0xfffffb82,	PMU_IHFGAIN_11P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-11.00dB*/ 0xfffffbb4,	PMU_IHFGAIN_11DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-10.50dB*/ 0xfffffbe6,	PMU_IHFGAIN_10P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-10.00dB*/ 0xfffffc18,	PMU_IHFGAIN_10DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-9.50dB*/ 0xfffffc4a,	PMU_IHFGAIN_9P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-9.00dB*/ 0xfffffc7c,	PMU_IHFGAIN_9DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-8.50dB*/ 0xfffffcae,	PMU_IHFGAIN_8P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-8.00dB*/ 0xfffffce0,	PMU_IHFGAIN_8DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-7.50dB*/ 0xfffffd12,	PMU_IHFGAIN_7P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-7.00dB*/ 0xfffffd44,	PMU_IHFGAIN_7DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-6.50dB*/ 0xfffffd76,	PMU_IHFGAIN_6P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-6.00dB*/ 0xfffffda8,	PMU_IHFGAIN_6DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-5.50dB*/ 0xfffffdda,	PMU_IHFGAIN_5P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-5.00dB*/ 0xfffffe0c,	PMU_IHFGAIN_5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-4.50dB*/ 0xfffffe3e,	PMU_IHFGAIN_4P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-4.00dB*/ 0xfffffe70,	PMU_IHFGAIN_4DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-3.50dB*/ 0xfffffea2,	PMU_IHFGAIN_3P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-3.00dB*/ 0xfffffed4,	PMU_IHFGAIN_3DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-2.50dB*/ 0xffffff06,	PMU_IHFGAIN_2P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-2.00dB*/ 0xffffff38,	PMU_IHFGAIN_2DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-1.50dB*/ 0xffffff6a,	PMU_IHFGAIN_1P5DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-1.00dB*/ 0xffffff9c,	PMU_IHFGAIN_1DB_N,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*-0.50dB*/ 0xffffffce,	PMU_IHFGAIN_0DB,	0xffffffce},
	{AUDIO_MODE_SPEAKERPHONE,	/*0.00dB*/ 0x0,	PMU_IHFGAIN_0DB,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*0.50dB*/ 0x32,	PMU_IHFGAIN_P5DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*1.00dB*/ 0x64,	PMU_IHFGAIN_1DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*1.50dB*/ 0x96,	PMU_IHFGAIN_1P5DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*2.00dB*/ 0xc8,	PMU_IHFGAIN_2DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*2.50dB*/ 0xfa,	PMU_IHFGAIN_2P5DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*3.00dB*/ 0x12c,	PMU_IHFGAIN_3DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*3.50dB*/ 0x15e,	PMU_IHFGAIN_3P5DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*4.00dB*/ 0x190,	PMU_IHFGAIN_4DB_P,	0x0},
	{AUDIO_MODE_SPEAKERPHONE,	/*4.50dB*/ 0x1c2,	PMU_IHFGAIN_4DB_P,	0x32},
	{AUDIO_MODE_SPEAKERPHONE,	/*5.00dB*/ 0x1f4,	PMU_IHFGAIN_4DB_P,	0x64},
	{AUDIO_MODE_SPEAKERPHONE,	/*5.50dB*/ 0x226,	PMU_IHFGAIN_4DB_P,	0x96},
	{AUDIO_MODE_SPEAKERPHONE,	/*6.00dB*/ 0x258,	PMU_IHFGAIN_4DB_P,	0xc8},
	{AUDIO_MODE_SPEAKERPHONE,	/*6.50dB*/ 0x28a,	PMU_IHFGAIN_4DB_P,	0xfa},
	{AUDIO_MODE_SPEAKERPHONE,	/*7.00dB*/ 0x2bc,	PMU_IHFGAIN_4DB_P,	0x12c},
	{AUDIO_MODE_SPEAKERPHONE,	/*7.50dB*/ 0x2ee,	PMU_IHFGAIN_4DB_P,	0x15e},
	{AUDIO_MODE_SPEAKERPHONE,	/*8.00dB*/ 0x320,	PMU_IHFGAIN_4DB_P,	0x190},
	{AUDIO_MODE_SPEAKERPHONE,	/*8.50dB*/ 0x352,	PMU_IHFGAIN_4DB_P,	0x1c2},
	{AUDIO_MODE_SPEAKERPHONE,	/*9.00dB*/ 0x384,	PMU_IHFGAIN_4DB_P,	0x1f4},
	{AUDIO_MODE_SPEAKERPHONE,	/*9.50dB*/ 0x3b6,	PMU_IHFGAIN_4DB_P,	0x226},
	{AUDIO_MODE_SPEAKERPHONE,	/*10.00dB*/ 0x3e8,	PMU_IHFGAIN_4DB_P,	0x258},
	{AUDIO_MODE_SPEAKERPHONE,	/*10.50dB*/ 0x41a,	PMU_IHFGAIN_4DB_P,	0x28a},
	{AUDIO_MODE_SPEAKERPHONE,	/*11.00dB*/ 0x44c,	PMU_IHFGAIN_4DB_P,	0x2bc},
	{AUDIO_MODE_SPEAKERPHONE,	/*11.50dB*/ 0x47e,	PMU_IHFGAIN_4DB_P,	0x2ee},
	{AUDIO_MODE_SPEAKERPHONE,	/*12.00dB*/ 0x4b0,	PMU_IHFGAIN_4DB_P,	0x320},
	{AUDIO_MODE_SPEAKERPHONE,	/*12.50dB*/ 0x4e2,	PMU_IHFGAIN_4DB_P,	0x352},
	{AUDIO_MODE_SPEAKERPHONE,	/*13.00dB*/ 0x514,	PMU_IHFGAIN_4DB_P,	0x384},
	{AUDIO_MODE_SPEAKERPHONE,	/*13.50dB*/ 0x546,	PMU_IHFGAIN_4DB_P,	0x3b6},
	{AUDIO_MODE_SPEAKERPHONE,	/*14.00dB*/ 0x578,	PMU_IHFGAIN_4DB_P,	0x3e8},
	{AUDIO_MODE_SPEAKERPHONE,	/*14.50dB*/ 0x5aa,	PMU_IHFGAIN_4DB_P,	0x41a},
	{AUDIO_MODE_SPEAKERPHONE,	/*15.00dB*/ 0x5dc,	PMU_IHFGAIN_4DB_P,	0x44c},
	{AUDIO_MODE_SPEAKERPHONE,	/*15.50dB*/ 0x60e,	PMU_IHFGAIN_4DB_P,	0x47e},
	{AUDIO_MODE_SPEAKERPHONE,	/*16.00dB*/ 0x640,	PMU_IHFGAIN_4DB_P,	0x4b0},
	{AUDIO_MODE_SPEAKERPHONE,	/*16.50dB*/ 0x672,	PMU_IHFGAIN_4DB_P,	0x4e2},
	{AUDIO_MODE_SPEAKERPHONE,	/*17.00dB*/ 0x6a4,	PMU_IHFGAIN_4DB_P,	0x514},
	{AUDIO_MODE_SPEAKERPHONE,	/*17.50dB*/ 0x6d6,	PMU_IHFGAIN_4DB_P,	0x546},
	{AUDIO_MODE_SPEAKERPHONE,	/*18.00dB*/ 0x708,	PMU_IHFGAIN_4DB_P,	0x578},
	{AUDIO_MODE_SPEAKERPHONE,	/*18.50dB*/ 0x73a,	PMU_IHFGAIN_4DB_P,	0x5aa},
	{AUDIO_MODE_SPEAKERPHONE,	/*19.00dB*/ 0x76c,	PMU_IHFGAIN_4DB_P,	0x5dc},
	{AUDIO_MODE_SPEAKERPHONE,	/*19.50dB*/ 0x79e,	PMU_IHFGAIN_4DB_P,	0x60e},
	{AUDIO_MODE_SPEAKERPHONE,	/*20.00dB*/ 0x7d0,	PMU_IHFGAIN_4DB_P,	0x640},
	{AUDIO_MODE_SPEAKERPHONE,	/*20.50dB*/ 0x802,	PMU_IHFGAIN_4DB_P,	0x672},
	{AUDIO_MODE_SPEAKERPHONE,	/*21.00dB*/ 0x834,	PMU_IHFGAIN_4DB_P,	0x6a4},
	{AUDIO_MODE_SPEAKERPHONE,	/*21.50dB*/ 0x866,	PMU_IHFGAIN_4DB_P,	0x6d6},
	{AUDIO_MODE_SPEAKERPHONE,	/*22.00dB*/ 0x898,	PMU_IHFGAIN_4DB_P,	0x708},
	{AUDIO_MODE_SPEAKERPHONE,	/*22.50dB*/ 0x8ca,	PMU_IHFGAIN_4DB_P,	0x73a},
	{AUDIO_MODE_SPEAKERPHONE,	/*23.00dB*/ 0x8fc,	PMU_IHFGAIN_4DB_P,	0x76c},
	{AUDIO_MODE_SPEAKERPHONE,	/*23.50dB*/ 0x92e,	PMU_IHFGAIN_4DB_P,	0x79e},
	{AUDIO_MODE_SPEAKERPHONE,	/*24.00dB*/ 0x960,	PMU_IHFGAIN_4DB_P,	0x7d0},
	{AUDIO_MODE_SPEAKERPHONE,	/*24.50dB*/ 0x992,	PMU_IHFGAIN_4DB_P,	0x802},
	{AUDIO_MODE_SPEAKERPHONE,	/*25.00dB*/ 0x9c4,	PMU_IHFGAIN_4DB_P,	0x834},
	{AUDIO_MODE_SPEAKERPHONE,	/*25.50dB*/ 0x9f6,	PMU_IHFGAIN_4DB_P,	0x866},
	{AUDIO_MODE_SPEAKERPHONE,	/*26.00dB*/ 0xa28,	PMU_IHFGAIN_4DB_P,	0x898},
	{AUDIO_MODE_SPEAKERPHONE,	/*26.50dB*/ 0xa5a,	PMU_IHFGAIN_4DB_P,	0x8ca},
	{AUDIO_MODE_SPEAKERPHONE,	/*27.00dB*/ 0xa8c,	PMU_IHFGAIN_4DB_P,	0x8fc},
	{AUDIO_MODE_SPEAKERPHONE,	/*27.50dB*/ 0xabe,	PMU_IHFGAIN_4DB_P,	0x92e},
	{AUDIO_MODE_SPEAKERPHONE,	/*28.00dB*/ 0xaf0,	PMU_IHFGAIN_4DB_P,	0x960},
	{AUDIO_MODE_SPEAKERPHONE,	/*28.50dB*/ 0xb22,	PMU_IHFGAIN_4DB_P,	0x992},
	{AUDIO_MODE_SPEAKERPHONE,	/*29.00dB*/ 0xb54,	PMU_IHFGAIN_4DB_P,	0x9c4},
	{AUDIO_MODE_SPEAKERPHONE,	/*29.50dB*/ 0xb86,	PMU_IHFGAIN_4DB_P,	0x9f6},
	{AUDIO_MODE_SPEAKERPHONE,	/*30.00dB*/ 0xbb8,	PMU_IHFGAIN_4DB_P,	0xa28},
	{AUDIO_MODE_SPEAKERPHONE,	/*30.50dB*/ 0xbea,	PMU_IHFGAIN_4DB_P,	0xa5a},
	{AUDIO_MODE_SPEAKERPHONE,	/*31.00dB*/ 0xc1c,	PMU_IHFGAIN_4DB_P,	0xa8c},
	{AUDIO_MODE_SPEAKERPHONE,	/*31.50dB*/ 0xc4e,	PMU_IHFGAIN_4DB_P,	0xabe},
	{AUDIO_MODE_SPEAKERPHONE,	/*32.00dB*/ 0xc80,	PMU_IHFGAIN_4DB_P,	0xaf0},
	{AUDIO_MODE_SPEAKERPHONE,	/*32.50dB*/ 0xcb2,	PMU_IHFGAIN_4DB_P,	0xb22},
	{AUDIO_MODE_SPEAKERPHONE,	/*33.00dB*/ 0xce4,	PMU_IHFGAIN_4DB_P,	0xb54},
	{AUDIO_MODE_SPEAKERPHONE,	/*33.50dB*/ 0xd16,	PMU_IHFGAIN_4DB_P,	0xb86},
	{AUDIO_MODE_SPEAKERPHONE,	/*34.00dB*/ 0xd48,	PMU_IHFGAIN_4DB_P,	0xbb8},
	{AUDIO_MODE_SPEAKERPHONE,	/*34.50dB*/ 0xd7a,	PMU_IHFGAIN_4DB_P,	0xbea},
	{AUDIO_MODE_SPEAKERPHONE,	/*35.00dB*/ 0xdac,	PMU_IHFGAIN_4DB_P,	0xc1c},
	{AUDIO_MODE_SPEAKERPHONE,	/*35.50dB*/ 0xdde,	PMU_IHFGAIN_4DB_P,	0xc4e},
	{AUDIO_MODE_SPEAKERPHONE,	/*36.00dB*/ 0xe10,	PMU_IHFGAIN_4DB_P,	0xc80},
	{AUDIO_MODE_SPEAKERPHONE,	/*36.50dB*/ 0xe42,	PMU_IHFGAIN_4DB_P,	0xcb2},
	{AUDIO_MODE_SPEAKERPHONE,	/*37.00dB*/ 0xe74,	PMU_IHFGAIN_4DB_P,	0xce4},
	{AUDIO_MODE_SPEAKERPHONE,	/*37.50dB*/ 0xea6,	PMU_IHFGAIN_4DB_P,	0xd16},
	{AUDIO_MODE_SPEAKERPHONE,	/*38.00dB*/ 0xed8,	PMU_IHFGAIN_4DB_P,	0xd48},
	{AUDIO_MODE_SPEAKERPHONE,	/*38.50dB*/ 0xf0a,	PMU_IHFGAIN_4DB_P,	0xd7a},
	{AUDIO_MODE_SPEAKERPHONE,	/*39.00dB*/ 0xf3c,	PMU_IHFGAIN_4DB_P,	0xdac},
	{AUDIO_MODE_SPEAKERPHONE,	/*39.50dB*/ 0xf6e,	PMU_IHFGAIN_4DB_P,	0xdde},
	{AUDIO_MODE_SPEAKERPHONE,	/*40.00dB*/ 0xfa0,	PMU_IHFGAIN_4DB_P,	0xe10},
	{AUDIO_MODE_SPEAKERPHONE,	/*40.50dB*/ 0xfd2,	PMU_IHFGAIN_4DB_P,	0xe42},
	{AUDIO_MODE_SPEAKERPHONE,	/*41.00dB*/ 0x1004,	PMU_IHFGAIN_4DB_P,	0xe74},
	{AUDIO_MODE_SPEAKERPHONE,	/*41.50dB*/ 0x1036,	PMU_IHFGAIN_4DB_P,	0xea6},
	{AUDIO_MODE_SPEAKERPHONE,	/*42.00dB*/ 0x1068,	PMU_IHFGAIN_4DB_P,	0xed8},
	{AUDIO_MODE_SPEAKERPHONE,	/*42.50dB*/ 0x109a,	PMU_IHFGAIN_4DB_P,	0xf0a},
	{AUDIO_MODE_SPEAKERPHONE,	/*43.00dB*/ 0x10cc,	PMU_IHFGAIN_4DB_P,	0xf3c},
	{AUDIO_MODE_SPEAKERPHONE,	/*43.50dB*/ 0x10fe,	PMU_IHFGAIN_4DB_P,	0xf6e},
	{AUDIO_MODE_SPEAKERPHONE,	/*44.00dB*/ 0x1130,	PMU_IHFGAIN_4DB_P,	0xfa0},
	{AUDIO_MODE_SPEAKERPHONE,	/*44.50dB*/ 0x1162,	PMU_IHFGAIN_4DB_P,	0xfd2},
	{AUDIO_MODE_SPEAKERPHONE,	/*45.00dB*/ 0x1194,	PMU_IHFGAIN_4DB_P,	0x1004},
	{AUDIO_MODE_SPEAKERPHONE,	/*45.50dB*/ 0x11c6,	PMU_IHFGAIN_4DB_P,	0x1036},
	{AUDIO_MODE_SPEAKERPHONE,	/*46.00dB*/ 0x11f8,	PMU_IHFGAIN_4DB_P,	0x1068},
	{AUDIO_MODE_SPEAKERPHONE,	/*46.50dB*/ 0x122a,	PMU_IHFGAIN_4DB_P,	0x109a},
	{AUDIO_MODE_SPEAKERPHONE,	/*47.00dB*/ 0x125c,	PMU_IHFGAIN_4DB_P,	0x10cc},
	{AUDIO_MODE_SPEAKERPHONE,	/*47.50dB*/ 0x128e,	PMU_IHFGAIN_4DB_P,	0x10fe},
	{AUDIO_MODE_SPEAKERPHONE,	/*48.00dB*/ 0x12c0,	PMU_IHFGAIN_4DB_P,	0x1130},
	{AUDIO_MODE_SPEAKERPHONE,	/*48.50dB*/ 0x12f2,	PMU_IHFGAIN_4DB_P,	0x1162},
	{AUDIO_MODE_SPEAKERPHONE,	/*49.00dB*/ 0x1324,	PMU_IHFGAIN_4DB_P,	0x1194},
	{AUDIO_MODE_SPEAKERPHONE,	/*49.50dB*/ 0x1356,	PMU_IHFGAIN_4DB_P,	0x11c6},
	{AUDIO_MODE_SPEAKERPHONE,	/*50.00dB*/ 0x1388,	PMU_IHFGAIN_4DB_P,	0x11f8},
	{AUDIO_MODE_SPEAKERPHONE,	/*50.50dB*/ 0x13ba,	PMU_IHFGAIN_4DB_P,	0x122a},
	{AUDIO_MODE_SPEAKERPHONE,	/*51.00dB*/ 0x13ec,	PMU_IHFGAIN_4DB_P,	0x125c},
	{AUDIO_MODE_SPEAKERPHONE,	/*51.50dB*/ 0x141e,	PMU_IHFGAIN_4DB_P,	0x128e},
	{AUDIO_MODE_SPEAKERPHONE,	/*52.00dB*/ 0x1450,	PMU_IHFGAIN_4DB_P,	0x12c0},
	{AUDIO_MODE_SPEAKERPHONE,	/*52.50dB*/ 0x1482,	PMU_IHFGAIN_4DB_P,	0x12f2},
	{AUDIO_MODE_SPEAKERPHONE,	/*53.00dB*/ 0x14b4,	PMU_IHFGAIN_4DB_P,	0x1324},
	{AUDIO_MODE_SPEAKERPHONE,	/*53.50dB*/ 0x14e6,	PMU_IHFGAIN_4DB_P,	0x1356},
	{AUDIO_MODE_SPEAKERPHONE,	/*54.00dB*/ 0x1518,	PMU_IHFGAIN_4DB_P,	0x1388},
	{AUDIO_MODE_SPEAKERPHONE,	/*54.50dB*/ 0x154a,	PMU_IHFGAIN_4DB_P,	0x13ba},
	{AUDIO_MODE_SPEAKERPHONE,	/*55.00dB*/ 0x157c,	PMU_IHFGAIN_4DB_P,	0x13ec},
	{AUDIO_MODE_SPEAKERPHONE,	/*55.50dB*/ 0x15ae,	PMU_IHFGAIN_4DB_P,	0x141e},
	{AUDIO_MODE_SPEAKERPHONE,	/*56.00dB*/ 0x15e0,	PMU_IHFGAIN_4DB_P,	0x1450},
	{AUDIO_MODE_SPEAKERPHONE,	/*56.50dB*/ 0x1612,	PMU_IHFGAIN_4DB_P,	0x1482},
	{AUDIO_MODE_SPEAKERPHONE,	/*57.00dB*/ 0x1644,	PMU_IHFGAIN_4DB_P,	0x14b4},
	{AUDIO_MODE_SPEAKERPHONE,	/*57.50dB*/ 0x1676,	PMU_IHFGAIN_4DB_P,	0x14e6},
	{AUDIO_MODE_SPEAKERPHONE,	/*58.00dB*/ 0x16a8,	PMU_IHFGAIN_4DB_P,	0x1518},
	{AUDIO_MODE_SPEAKERPHONE,	/*58.50dB*/ 0x16da,	PMU_IHFGAIN_4DB_P,	0x154a},
	{AUDIO_MODE_SPEAKERPHONE,	/*59.00dB*/ 0x170c,	PMU_IHFGAIN_4DB_P,	0x157c},
	{AUDIO_MODE_SPEAKERPHONE,	/*59.50dB*/ 0x173e,	PMU_IHFGAIN_4DB_P,	0x15ae},
	{AUDIO_MODE_SPEAKERPHONE,	/*60.00dB*/ 0x1770,	PMU_IHFGAIN_4DB_P,	0x15e0},
#else
/* Headset Mode */    
	{AUDIO_MODE_HEADSET,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
/* IHF Speaker Mode */    
	{AUDIO_MODE_SPEAKERPHONE,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
#endif
/* Earpiece Mode */    
	{AUDIO_MODE_HANDSFREE,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
	{AUDIO_MODE_BLUETOOTH,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
	{AUDIO_MODE_TTY,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
	{AUDIO_MODE_HAC,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
	{AUDIO_MODE_USB,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
	{AUDIO_MODE_RESERVE,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN},
	{AUDIO_MODE_INVALID,     TOTAL_GAIN,          NO_PMU_NEEDED,       TOTAL_GAIN}
};



//============================================================================
//
// Function Name: AUDTABL_getQ14_1HWSlopGain
//
// Description:   Read the Slopgain value from the table based on Q14.1 gain
//
//============================================================================

UInt16 AUDTABL_getQ14_1HWSlopGain(UInt16 valueQ14_1)
{
    UInt8 i = 0;
    while(slopGain_lut[i].slopGain != END_OF_TABLE)
    {
        if ( slopGain_lut[i].valueQ14_1 == valueQ14_1)
        {
            return slopGain_lut[i].slopGain;
        }
        i ++;
    }
    if((Int16)valueQ14_1 > (Int16)(slopGain_lut[0].valueQ14_1))
    {
        return slopGain_lut[0].slopGain;
    }
	else
    if((Int16)valueQ14_1 <= (Int16)(slopGain_lut[i-1].valueQ14_1))
    {
        return slopGain_lut[i-1].slopGain;
    }
    return END_OF_TABLE;
}

//============================================================================
//
// Function Name: AUDTABL_getQ1_14HWSlopGain
//
// Description:   Read the Slopgain value from the table based on Q1.14 gain
//
//============================================================================

UInt16 AUDTABL_getQ1_14HWSlopGain(UInt16 valueQ1_14)
{
    UInt16 i = 0;
    if(valueQ1_14 > slopGain_lut[0].valueQ1_14)
    {
        return slopGain_lut[0].slopGain;
    }
    while(slopGain_lut[i].slopGain != END_OF_TABLE)
    {
        if ( slopGain_lut[i].valueQ1_14 == valueQ1_14)
        {
            return slopGain_lut[i].slopGain;
        }

		if (slopGain_lut[i+1].slopGain == END_OF_TABLE) 
		{
			return slopGain_lut[i].slopGain;
		}
		
// If the value falls in between, the following code will
// choose the closet value.
        if (( slopGain_lut[i].valueQ1_14 > valueQ1_14 )
            &&( slopGain_lut[i+1].valueQ1_14 <= valueQ1_14 ))
        {
            if (( valueQ1_14 - slopGain_lut[i+1].valueQ1_14 )
                <=( slopGain_lut[i].valueQ1_14 - valueQ1_14 ))
            {
                return slopGain_lut[i+1].slopGain;
            }
            else
            {
                return slopGain_lut[i].slopGain;
            }
        }
        i ++;
    }
    return END_OF_TABLE;
}



//============================================================================
//
// Function Name: AUDTABL_getGainDistribution
//
// Description:   Get the gain distribution (How much PMU gain, HOW much Audio HW gain)
// for the requested gain in milibel.
//
//============================================================================

AUDTABL_GainMapping_t AUDTABL_getGainDistribution(AudioMode_t audioMode, UInt32 gainQ31)
{
    AUDTABL_GainMapping_t gainMapping;
    UInt16 i = 0;
    memset(&gainMapping, 0, sizeof(AUDTABL_GainMapping_t));
    while (GainMapping_Table[i].audioMode !=AUDIO_MODE_INVALID)
    {
        if (GainMapping_Table[i].audioMode == audioMode)
        {
            if ((audioMode == AUDIO_MODE_HEADSET)||(audioMode == AUDIO_MODE_SPEAKERPHONE))
            {
                if (GainMapping_Table[i].gainTotal == gainQ31)
                // the gain is found in the table
                {
                    return GainMapping_Table[i];
                }
                else
                if(((Int32)(GainMapping_Table[i].gainTotal) <= (Int32)gainQ31)
                   &&((Int32)(GainMapping_Table[i+1].gainTotal) > (Int32)gainQ31))
                // the gain fits between two adjacent gains in the table for Headset/IHF Mode
                // return the smaller gain
                {
                    return GainMapping_Table[i];
                }
                else
				//index 0 defines the total gain. hence use index 1
                if((Int32)(GainMapping_Table[0].gainTotal) > (Int32)gainQ31)
                // the gain is smaller than the smallest gain in the table for Headset/IHF Mode
                // return the smallest available gain
                {
                    return GainMapping_Table[0];
                }
                else
                if(GainMapping_Table[i+1].audioMode != audioMode)
                // the gain is larger than the largest gain in the table for Headset/IHF Mode
                // return the largest available gain
                {
                    return GainMapping_Table[i];
                }
            }
            else
            {
                return GainMapping_Table[i];
            }
        }
        i++;
    }
    // return the last line in the table, AUDIO_MODE_INVALID.
    return GainMapping_Table[i-1];
}





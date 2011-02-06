/******************************************************************************
Copyright 2009 Broadcom Corporation.  All rights reserved.

This program is the proprietary software of Broadcom Corporation and/or its 
licensors, and may only be used, duplicated, modified or distributed pursuant 
to the terms and conditions of a separate, written license agreement executed 
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to the 
Software, and Broadcom expressly reserves all rights in and to the Software and 
all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, 
THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY 
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, 
constitutes the valuable trade secrets of Broadcom, and you shall use all 
reasonable efforts to protect the confidentiality thereof, and to use this 
information only in connection with your use of Broadcom integrated circuit 
products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE 
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
ANY LIMITED REMEDY.
******************************************************************************/
/**
*
* @file   audio_gain_table.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================
#include "audio_gain_table.h"

#define END_OF_TABLE  0xFFFF

typedef struct AUDTABL_SlopGain_LUT_t
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








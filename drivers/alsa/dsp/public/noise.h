/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated, 
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between 
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license 
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all 
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU 
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
* ALL USE OF THE SOFTWARE.	
* 
* Except as expressly set forth in the Authorized License,
* 
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
* 
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO 
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE. 
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, 
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR 
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY 
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/
#ifndef __NOISE_H__
#define __NOISE_H__

#include "mobcom_types.h"

#if (CHIPVERSION >= CHIP_VERSION(BCM2132,30) || CHIPVERSION >= CHIP_VERSION(ML2021,23))/* BCM2132C0 and later || ML2021 B3 and later */

//******************************************************************************
// Global typedefs
//******************************************************************************

typedef struct {							// Adaptive Noise Suppresser Control state memory
	UInt32 Epeak;							// The peak channel energy
	Int32 SPNR;								// The long term Signal Peak To Noise Ratio db_q5
	UInt32 frame_num;						// The current 20 ms speech frame being executed
} NOISE_Statmem_t;


//******************************************************************************
// Global function declarations
//******************************************************************************

NOISE_Statmem_t *NOISE_GetAdapMemPtr(void);	// Returns a pointer to the adaptive noise suppresser control memory

void NOISE_AdapInit(void);					// Initialises the adaptive noise suppresser control memory

void NOISE_AdapRun(void);					// The main adaptive noise suppresser control function

void SIGPROC_mac_32_16_64(					// Performs a 32 bit x 32 bit MAC on data stored as 16 bit words
	UInt16	*data_1_ptr,					// Pointer to 32 bit data values stored as 16 bits in form ..hi,lo,hi..16 bit words
	const	UInt16 *data_2_ptr,				// Pointer to 16 bit data values
	UInt8	loop_count,						// Number of MACs to perform
	UInt32	*output_ptr						// Final 64 bit output result after loop_count MACs
	);


//******************************************************************************
// Function Name:	NOISE_Set_SBNLP_On
//
// Description:		set the flag
//******************************************************************************
void NOISE_Set_SBNLP_On( Boolean on );


//******************************************************************************
// Function Name:	NOISE_Get_SBNLP_On
//
// Description:		get the flag
//******************************************************************************
Boolean NOISE_Get_SBNLP_On( void );

//******************************************************************************
// Function Name:	NOISE_Set_NS_On
//
// Description:		check if NS is turned on.
//******************************************************************************
void NOISE_Set_NS_On( Boolean on );

//******************************************************************************
// Function Name:	NOISE_Get_NS_On
//
// Description:		check if NS is turned on.
//******************************************************************************
Boolean NOISE_Get_NS_On(void);


#endif	//#if (CHIPVERSION >= CHIP_VERSION(BCM2132,30) || CHIPVERSION >= CHIP_VERSION(ML2021,23))/* BCM2132C0 and later || ML2021 B3 and later */

#endif	// __NOISE_H__


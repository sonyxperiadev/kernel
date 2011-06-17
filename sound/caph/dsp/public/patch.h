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
#ifndef _OAK_PATCH_H
#define _OAK_PATCH_H

#include "sysparm.h"

#if CHIPVERSION == CHIP_VERSION(BCM2132,24)
#define PATCH_COMMAND_SIZE	1296
#else
#if CHIPVERSION >= CHIP_VERSION(BCM2133,10)
#define PATCH_COMMAND_SIZE	16		// ??? do not need 1296 back to 16
#else
#define PATCH_COMMAND_SIZE	16
#endif
#endif

#if CHIPVERSION >= CHIP_VERSION(BCM2133,10)
#if CHIPVERSION == CHIP_VERSION(BCM2133,12)
#ifdef DSP_FEATURE_2133A2_TWO_LAYER_OVERLAY_PATCH
#define PATCH_MEMORY_SIZE	8249			// both the patch and overlay in one file, extent size to 4 pages
#else
#define PATCH_MEMORY_SIZE	6144			// both the patch and overlay in one file, extent size to 3 pages
#endif
#else
#define PATCH_MEMORY_SIZE	2048
#endif
#else
#define PATCH_MEMORY_SIZE	1024
#endif


#define SIGNATURE	"PATCH2029"
typedef struct
{
	UInt16 value;
	UInt16 addr;
} Patch_GenericCmd_t;

typedef struct				// moved variables around to align pram_write_cmd.value to 4kw boundary for easy dsp paged access
{
	
#ifdef DSP_FEATURE_2133A2_TWO_LAYER_OVERLAY_PATCH
	UInt16				pram_write_cmd_value[ PATCH_MEMORY_SIZE ];
	UInt16				pram_write_cmd_addr[ PATCH_MEMORY_SIZE ];
#else	
	Patch_GenericCmd_t	pram_write_cmd[ PATCH_MEMORY_SIZE ];
#endif
	Patch_GenericCmd_t	write_init_cmd[ PATCH_COMMAND_SIZE ];
	UInt8				signature[16];
	UInt8				date[16];
	UInt8				revision[ 16 ];
	UInt8				changelist[16];
	UInt16				enabled;
	UInt32				dspchangelist;
	UInt16				write_init_cmd_size;
	UInt16				pram_write_cmd_size;
	UInt16				checksum;
	UInt16				fill_in;

} PatchData_t;

typedef struct
{
	PatchData_t patch1;
#ifndef DSP_FEATURE_2133A2_TWO_LAYER_OVERLAY_PATCH
	PatchData_t patch2;
#endif
}Patch_Flash_t;

void PATCH_SetBaseAddr(UInt32 patch_base_addr);
void PATCH_Config(void);

Boolean PATCH_Check(void);
UInt8* PATCH_GetDate( void );	
UInt8* PATCH_GetRevision( void );	
UInt8* PATCH_GetChangelist( void );	
UInt16 PATCH_GetEnabled( void );	
UInt16 PATCH_GetWriteInitCmdSize( void );
UInt16 PATCH_GetPramWriteCmdSize( void );

#endif


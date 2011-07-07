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

#ifndef _PHY_RIPISR_H_
#define _PHY_RIPISR_H_

#include "mobcom_types.h"
#include "consts.h"

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

#ifdef DEFINE_FIQ_MEM
#	define	FIQEXTERN
#else
#	define	FIQEXTERN	extern
#endif

FIQEXTERN UInt16 rfic_phase1;
FIQEXTERN UInt16 rfic_phase2;

typedef void (*RIPISR_HISR_CB)(void);
typedef Boolean (*RIPISR_PROCESSSTATUS_CB)(void * msg);

typedef void (*Audio_ISR_Handler_t)(StatQ_t status_msg);
//typedef void (*VPU_ProcessStatus_t)(VPStatQ_t vp_status_msg);
typedef void (*VPU_ProcessStatus_t)( void );

//******************************************************************************
// Global Function prototypes
//******************************************************************************
void RIPISR_Init( void );				// Initialize the RIP ISR
void RIPISR_Register(	RIPISR_HISR_CB hisr_cb,	RIPISR_PROCESSSTATUS_CB msg_cb);

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb );
void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb );

//will figure out how to avoid vpu.c call this function.
//and remove this from public interface...
void Audio_ISR_Handler(StatQ_t msg);

#endif
		
#endif


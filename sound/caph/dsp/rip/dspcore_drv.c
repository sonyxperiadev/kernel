//****************************************************************************
//
//     Copyright (c) 2006 Broadcom Corporation
//                All Rights Reserved
//
//     No portions of this material may be reproduced in any form without the
//     written permission of:
//
//           Broadcom Corporation
//           16215 Alton Parkway
//           P.O. Box 57013
//           Irvine, California 92619-7013
//
//     All information contained in this document is Broadcom Corporation
//     company private, proprietary, and trade secret.
//****************************************************************************
//
//   ABSTRACT: This file contains DSP driver implementation
//
//   TARGET:   BCM2124, BCM2133, BCM2152, BCM213x1,BCM2153
//
//   TOOLSET:
//
//   HISTORY:
//   DATE       AUTHOR           DESCDSPTION
//  01/08/07                   Initial version
//****************************************************************************
//!
//! \file   dspcore_drv.c 
//! \brief  dsp driver implementation
//! \brief  
//!
//****************************************************************************
#include "mobcom_types.h"
#include "chip_version.h"
#ifdef DSP_COMMS_INCLUDED
//#include "ms.h"
#endif
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "dsp_feature_def.h"
#include "msconsts.h"
#include "sysparm.h"
#include "ostask.h"
#include "osqueue.h"

#include "memmap.h"
#include "dspcmd.h"

#include "dspcore_drv.h"
#include "dsp_public_cp.h"
#include "rficisr.h"
#include "osheap.h"
#include "assert.h"
#include "xassert.h"
#if defined(_ATHENA_)
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_ahb_tl3r.h"
#include "brcm_rdb_syscfg.h"
#include "brcm_rdb_dsp_audio.h"
#include "chal_types.h"
#include "chal_tl3r.h"
#include "chal_syscfg.h"
#else
#if !defined(_HERA_)  // covers Rhea and Samoa 
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_dsp_tl3r.h"
#include "syscfg_drv.h"
#endif
#endif


//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************
extern	UInt32			isTestPointEnabled;

//****************************************
// global variable definitions
//****************************************

//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************
// local macro declarations
//****************************************

//****************************************
// local typedef declarations
//****************************************

//****************************************
// local variable definitions
//****************************************
static Queue_t  dspcore_q;                  // Queue for AUDIO task
static Task_t   dspcore_task;
#if defined(DROM_LOAD_JTAG)
#pragma arm section zidata="er_drom_patch"
static UInt8	mem_drom_patch[0x10000];
#pragma arm section
#elif defined(DROM_LOAD_EMBEDDED)
static UInt32 mem_drom_patch[] =
{
#include "drom_dsp.h"
};
#endif

//****************************************
// local function declarations
//****************************************
static void	DSP_PRAM_flash(void);
static void ConfigDebug(void);

//****************************************
// local function definitions
//****************************************
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSP_PRAM_flash
//!
//! Description:	download DSP code to DSP PRAM and restart DSP
//!
//! Notes: 
//! 
/////////////////////////////////////////////////////////////////////////////
static void	DSP_PRAM_flash(void)
{
	UInt32 chipid;


	
#ifndef WIN32
	#if defined(_ATHENA_)
	chipid = GET_CHIPID();
	#else
	#if defined(_SAMOA_) && defined(FPGA_VERSION)
	chipid = SAMOA;
	#else
	UInt32 FamilyID, ProductID, RevID;
	
	SYSCFGDRV_Get_Chipid(&FamilyID, &ProductID, &RevID);
	chipid = (FamilyID*16)+ProductID;
	#endif
	#endif
	switch ( chipid )
	{
		case BCM2133 :
		case BCM2124 :
		case BCM2152 :
		case BCM2153 :
		case BCM213x1 :
		case BCM213x1v :
        case BCM213x1j:
			//	resetting DSP, different from 2131
			*(UInt8 *)DSPCTRL = 0x40;
			break;
		case BCM21551 :	
			//	resetting DSP, different from 2153

#if (defined(FPGA_VERSION) && defined(_BCM21551_))
	{
	   UInt32 temp2;		
			
			temp2 = *(unsigned long *)0x2940003C;
		    temp2 = temp2 & 0x7f;
			temp2 = temp2 | 0x10060000;
			*(unsigned long *)0x3040003C = temp2;


	 }
#else
			*(UInt8 *)DSPCTRL = 0;
#endif

			break;
		case ATHENA :	
#if defined(_ATHENA_)
			//	resetting DSP, different from 21551
			tl3_dsp_cleanup();
			*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_DSPCTRL_OFFSET) &= 0xFFFFFE7F;
			*((UInt32 *)(AHB_TL3R_TL3_A2D_ACCESS_EN_R))	&= 0xFFFFFFFC;
			*((UInt32 *)AHB_TL3R_TL3_CTRL_REG) |= AHB_TL3R_TL3_CTRL_REG_dsp2apb_clk_2to1;
#endif

			break;
		case HERA: //RHEA has same chip ID as Hera
#if defined(_RHEA_)

#ifdef DEBUG_DSP_ENABLE_TRACE
/**
;setup the ETM pins for DSP
DATA.SET 0x35004FF0 %LE %LONG 0xa5a501
DATA.SET 0x35004f80 %LE %LONG 0x0
DATA.SET 0x35004FF0 %LE %LONG 0xa5a501
DATA.SET 0x35004f84 %LE %LONG 0x0
DATA.SET 0x35004FF0 %LE %LONG 0xa5a501
DATA.SET 0x35004f88 %LE %LONG 0x0
DATA.SET 0x35004FF0 %LE %LONG 0xa5a501
DATA.SET 0x35004f8c %LE %LONG 0x0
DATA.SET 0x35004FF0 %LE %LONG 0xa5a501
DATA.SET 0x35004f90 %LE %LONG 0x0

DATA.SET 0x35004a10 %LE %LONG 0x301

DATA.SET 0x35004a14 %LE %LONG 0x301

DATA.SET 0x35004a18 %LE %LONG 0x301

DATA.SET 0x35004a1c %LE %LONG 0x301

DATA.SET 0x35004a20 %LE %LONG 0x301
DATA.SET 0x35004a24 %LE %LONG 0x301

DATA.SET 0x35004a28 %LE %LONG 0x301

DATA.SET 0x35004a2c %LE %LONG 0x301

DATA.SET 0x35004a30 %LE %LONG 0x301
*/

#pragma o0
*(volatile unsigned long *)0x35004FF0 = 0xa5a501;
*(volatile unsigned long *)0x35004f80 = 0x0;
*(volatile unsigned long *)0x35004FF0 = 0xa5a501;
*(volatile unsigned long *)0x35004f84 = 0x0;
*(volatile unsigned long *)0x35004FF0 = 0xa5a501;
*(volatile unsigned long *)0x35004f88 = 0x0;
*(volatile unsigned long *)0x35004FF0 = 0xa5a501;
*(volatile unsigned long *)0x35004f8c = 0x0;
*(volatile unsigned long *)0x35004FF0 = 0xa5a501;
*(volatile unsigned long *)0x35004f90 = 0x0;

*(volatile unsigned long *)0x35004a10 = 0x301;
*(volatile unsigned long *)0x35004a14 = 0x301;
*(volatile unsigned long *)0x35004a18 = 0x301;
*(volatile unsigned long *)0x35004a1c = 0x301;
*(volatile unsigned long *)0x35004a20 = 0x301;
*(volatile unsigned long *)0x35004a24 = 0x301;
*(volatile unsigned long *)0x35004a28 = 0x301;
*(volatile unsigned long *)0x35004a2c = 0x301;
*(volatile unsigned long *)0x35004a30 = 0x301;
#endif



		//Resetting DSP
		tl3_dsp_cleanup();

		//Disable ARM to DSP Access for Program and Data Memory
		*((UInt32 *)(AHB_DSP_TL3R_BASE_ADDR + 0x0000002C))	&= 0xFFFFFFFC;

		//DSP to APB Clock Ratio 2 to 1 : clock ratio is 2:1
		*((UInt32 *)(AHB_DSP_TL3R_BASE_ADDR + 0x0000003C)) |= 0x04000000;
#endif
		break;
		case SAMOA: //RHEA has same chip ID as Hera
#if defined(_SAMOA_)
			//Resetting DSP
			tl3_dsp_cleanup();

			//Disable ARM to DSP Access for Program and Data Memory
			*((UInt32 *)(AHB_DSP_TL3R_BASE_ADDR + 0x0000002C))	&= 0xFFFFFFFC;

			//DSP to APB Clock Ratio 2 to 1 : clock ratio is 2:1
			*((UInt32 *)(AHB_DSP_TL3R_BASE_ADDR + 0x0000003C)) |= 0x04000000;
#endif
		break;

		default:
			xassert (0, chipid);		//dsp code started from address 0
			break;
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	ConfigDebug
//!
//! Description:	trigger DSP debug mode for CRC check etc.
//!
//! Notes: 
//! 
/////////////////////////////////////////////////////////////////////////////

#if defined(DROM_LOAD_FLASH) || defined(DROM_LOAD_JTAG) || defined(DROM_LOAD_EMBEDDED)
#define DSP_REG_DDMACF2		0xE08A
#define DSP_DROM_BASE_ADDR	0x4000
#endif

static void ConfigDebug()						// Configure the debug mode
{
#ifdef DSP_COMMS_INCLUDED
	UInt16 i, k;
	volatile UInt16 j;
	CmdQ_t msg;
    SysCalDataInd_t *sys_data_ind = SYSPARM_GetIndParm();
		msg.arg0 = 0;
		msg.arg1 = 2;
		msg.arg2 = 0;		// init to avoid coverity warning

#if defined(DROM_LOAD_FLASH) || defined(DROM_LOAD_JTAG) || defined(DROM_LOAD_EMBEDDED)
	//************************
	// Download the DROM patch
	//************************
	{
		UInt16	i;
		UInt8	*ptr;
		UInt16	*addr;

		// set DRAM page
		msg.cmd = COMMAND_WRITE;
		msg.arg0 = DSP_REG_DDMACF2;
		msg.arg1 = 2;
		msg.arg2 = 0;		// init to avoid coverity warning
		SHAREDMEM_PostCmdQ( &msg );

		msg.cmd = COMMAND_WRITE;
#if defined(DROM_LOAD_JTAG)
		addr = (UInt16 *)mem_drom_patch;
#elif defined(DROM_LOAD_EMBEDDED)
		addr = (UInt16 *)&mem_drom_patch[4];
#elif defined(DROM_LOAD_FLASH)
#if defined(FLASH_TYPE_NAND)
		addr = (UInt16 *)OSHEAP_Alloc( MS_GetDromLen() );
		FlashLoadImage( MS_GetDromAddr(), MS_GetDromLen(), (UInt8 *)addr );
#else // else, FLASH_TYPE_NOR
		addr = (UInt16 *)MS_GetDromAddr();
#endif
#endif
		ptr = (UInt8 *)addr;
		for ( i = 0 ; i < MS_GetDromLen() / 2 ; i++ )
		{ 
			msg.arg0 = i + DSP_DROM_BASE_ADDR;
			msg.arg1 = (*ptr++) << 8;		//endian safe
			msg.arg1 |= *ptr++;
			SHAREDMEM_PostCmdQ( &msg );
		}
#if defined(DROM_LOAD_JTAG)
		// no memory to free
#elif defined(DROM_LOAD_EMBEDDED)
		// no memory to free
#elif defined(DROM_LOAD_FLASH)
#if defined(FLASH_TYPE_NAND)
		OSHEAP_Delete(addr);
#else // else, FLASH_TYPE_NOR
		// no memory to free
#endif
#endif
	}
#endif // defined(DROM_LOAD_FLASH) || defined(DROM_LOAD_JTAG) || defined(DROM_LOAD_EMBEDDED)

	msg.cmd = COMMAND_DEBUG_MODE;
	msg.arg0 = sys_data_ind->debug_mode;
	SHAREDMEM_PostCmdQ( &msg );

	isTestPointEnabled = sys_data_ind->debug_la;
	for ( k = 0 ; k < 20 ; k++ )
		for ( i = 0 ; i < 20000 ; i++ ){ j = 0; }
#endif	//DSP_COMMS_INCLUDED
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	dspcore_Entry
//!
//! Description:	dspcore task
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

static void dspcore_Entry( void )
{
    DSPCMD_t msg;

    while( 1 ) {
        OSQUEUE_Pend( dspcore_q, (QMsg_t *)&msg, TICKS_FOREVER );

		ProcessDspCmd( &msg );
    }
    
}   

//****************************************
// global function definitions
//****************************************

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCore_Init
//!
//! Description:	Initialize dsp driver
//!
//! Notes: 
//! 
/////////////////////////////////////////////////////////////////////////////
void DSPCore_Init(void)
{
#ifdef DSP_COMMS_INCLUDED
	RFICISR_ConfigStatic();
#endif// DSP_COMMS_INCLUDED

	DSP_PRAM_flash();

	{
		UInt16 i;
		volatile UInt16 j;

		// give the DSP some time to come out of reset , initialize memory, etc.
		for ( i = 0 ; i < 20000 ; i++ ){ j = 0; }
		for ( i = 0 ; i < 20000 ; i++ ){ j = 0; }
	}

#if !(defined(_ATHENA_))
	IRQ_EnableRIPInt();
#endif

	IRQ_TriggerRIPInt();	// so the DSP will start processing commands from the Arm

	ConfigDebug();

    dspcore_q = OSQUEUE_Create( QUEUESIZE_DSPCORE, sizeof( DSPCMD_t ), OSSUSPEND_FIFO );
	OSQUEUE_ChangeName(dspcore_q, "DSPCOR_Q");	// for debug convenience, Hui Luo, 7/13/07
	dspcore_task = OSTASK_Create((TEntry_t) dspcore_Entry, (TName_t) TASKNAME_DSPCORE, (TPriority_t)TASKPRI_DSPCORE,  (TStackSize_t) STACKSIZE_DSPCORE );

	DSPCMD_Initialize();

}

void DSPCore_Shutdown( void )
{
	OSTASK_Destroy( dspcore_task );
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCore_PostCmd
//!
//! Description:	Send a command to DSP
//!
//! Notes: It would pending the command to virtual command queue and send it out later in dspdrv task
//! 
/////////////////////////////////////////////////////////////////////////////
Boolean DSPCore_PostCmd(DSPCMD_t *pMsg)
{
	return OSQUEUE_Post( dspcore_q, (QMsg_t *)pMsg, TICKS_FOREVER ) == OSSTATUS_SUCCESS;
}

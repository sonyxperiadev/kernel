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
//
//   ABSTRACT: DSP IF driver code
//
//   TARGET:
//
//   TOOLSET:   RVDS2.2
//
//   HISTORY:
//   DATE       AUTHOR           DESCRIPTION
//****************************************************************************
//!
//! \file   dspif_drv.c
//! \brief  DSP IF driver
//!
//****************************************************************************
#define 	NEW_FRAME_ENTRY
#include "mobcom_types.h"
//#include "hal_audio.h"
#include <linux/broadcom/chip_version.h>
#include <linux/string.h>
#include <linux/kernel.h>

//-- #include <stdlib.h>

//-- #include "types.h"
#include "consts.h"

//-- #include "osheap.h"
//-- #include "ostask.h"
//-- #include "osinterrupt.h"
//-- #include "msconsts.h"

#include <mach/reg_dsp_dma.h>
#include <mach/reg_smi.h>

#include "memmap.h"
//-- #include "ram_memmap.h"
#include "sharedmem.h"

#ifdef POLY_INCLUDED
#include "poly_ringer.h"
#endif

//-- #include "dbg.h"
//-- #include "xassert.h"
#define 	xassert(a,b)

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************
//****************************************
// global variable definitions
//****************************************


//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************
// local macro declarations
//****************************************

//#define	CHK_DSP_INTR

//****************************************
// local variable definitions
//****************************************

#ifdef NEW_FRAME_ENTRY
static Unpaged_SharedMem_t	*unpaged_shared_mem = (Unpaged_SharedMem_t *)SHARED_RAM_BASE;
static SharedMem_t	*shared_mem = (SharedMem_t *)(SHARED_RAM_BASE+0x2000);
#else
SharedMem_t shared_mem_region;
static SharedMem_t	*shared_mem = (SharedMem_t *)SHARED_RAM_BASE;
#endif

#if (defined(_BCM2133_) && CHIP_REVISION >= 11) /* 2133A1 */|| defined(_BCM2124_) || defined(_BCM2152_) || defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_)
static PAGE1_SharedMem_t *pg1_shared_mem = (PAGE1_SharedMem_t *)(SHARED_RAM_BASE+0x4000);	//base address of page 1 shared RAM
static Shared_poly_events_t *event_page = (Shared_poly_events_t *)(SHARED_RAM_BASE+0x8000);	//base address of page 3 shared RAM
static Shared_poly_audio_t *pg4_shared_mem = (Shared_poly_audio_t *)(SHARED_RAM_BASE+0xA000);	//base address of page 4 shared RAM
static PAGE5_SharedMem_t *pg5_shared_mem = (PAGE5_SharedMem_t *)(SHARED_RAM_BASE+0xC000);		//base address of page 5 shared RAM
static PAGE6_SharedMem_t *pg6_shared_mem = (PAGE6_SharedMem_t *)(SHARED_RAM_BASE+0xe000);		//base address of page 6 shared RAM

#endif

#if (defined(_BCM2152_) && CHIP_REVISION >= 13) || defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_)
static PAGE7_SharedMem_t *pg7_shared_mem = (PAGE7_SharedMem_t *)(SHARED_RAM_BASE+0x10000);		//base address of page 7 shared RAM
static PAGE10_SharedMem_t *pg10_shared_mem = (PAGE10_SharedMem_t *)(SHARED_RAM_BASE+0x16000);		//base address of page 10 shared RAM
static PAGE11_SharedMem_t *pg11_shared_mem = (PAGE11_SharedMem_t *)(SHARED_RAM_BASE+0x18000);		//base address of page 11 shared RAM
static PAGE14_SharedMem_t *pg14_shared_mem = (PAGE14_SharedMem_t *)(SHARED_RAM_BASE+0x1E000);		//base address of page 14 shared RAM
#if (defined(_BCM213x1_) && CHIP_REVISION >= 20) || (defined(_BCM2153_) && CHIP_REVISION >= 40) || defined(_BCM21551_)
static PAGE15_SharedMem_t *pg15_shared_mem = (PAGE15_SharedMem_t *)(SHARED_RAM_BASE+0x20000);		//base address of page 15 shared RAM
static PAGE16_SharedMem_t *pg16_shared_mem = (PAGE16_SharedMem_t *)(SHARED_RAM_BASE+0x22000);		//base address of page 16 shared RAM
static PAGE17_SharedMem_t *pg17_shared_mem = (PAGE17_SharedMem_t *)(SHARED_RAM_BASE+0x24000);		//base address of page 17 shared RAM
static PAGE18_SharedMem_t *pg18_shared_mem = (PAGE18_SharedMem_t *)(SHARED_RAM_BASE+0x26000);		//base address of page 18 shared RAM
static PAGE19_SharedMem_t *pg19_shared_mem = (PAGE19_SharedMem_t *)(SHARED_RAM_BASE+0x28000);		//base address of page 19 shared RAM
static PAGE20_SharedMem_t *pg20_shared_mem = (PAGE20_SharedMem_t *)(SHARED_RAM_BASE+0x2a000);		//base address of page 20 shared RAM
static PAGE25_SharedMem_t *pg25_shared_mem = (PAGE25_SharedMem_t *)(SHARED_RAM_BASE+0x34000);		//base address of page 20 shared RAM
static PAGE27_SharedMem_t *pg27_shared_mem = (PAGE27_SharedMem_t *)(SHARED_RAM_BASE+0x38000);		//base address of page 27 shared RAM
static PAGE28_SharedMem_t *pg28_shared_mem = (PAGE28_SharedMem_t *)(SHARED_RAM_BASE+0x3a000);		//base address of page 28 shared RAM
static PAGE29_SharedMem_t *pg29_shared_mem = (PAGE29_SharedMem_t *)(SHARED_RAM_BASE+0x3c000);		//base address of page 29 shared RAM
static PAGE30_SharedMem_t *pg30_shared_mem = (PAGE30_SharedMem_t *)(SHARED_RAM_BASE+0x3e000);		//base address of page 30 shared RAM
#endif
#endif

static UInt16 value_written_to_dsp;

//****************************************
// local function declarations
//****************************************
void SHAREDMEM_Init();
void SYSPARM_Init();
void SHAREDMEM_PostCmdQ(CmdQ_t *cmd_msg);

extern void DSPCore_Init(void);
//****************************************
// local function definitions
//****************************************


//******************************************************************************
//
// Function Name:	RIPCMDQ_GetVersionID
//
// Description:		Get version ID
//
// Notes: From DSPISR.C
//
//******************************************************************************

void RIPCMDQ_GetVersionID()				// Get version ID
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_GET_VERSION_ID;
	msg.arg0 = 0;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}

#if 0 //#endif //CONFLICT with audvoc_dsp.c

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
void SHAREDMEM_Init()
{
  // The size of shared memory is defined at link
  // time.  Currently 8K bytes are reserved.

  UInt32 struct_size;

  //dprintf(DBG_L2, "SHAREDMEM_Init\n");

  //dprintf(DBG_L2, "SHARED_RAM_BASE = 0x%x\n",SHARED_RAM_BASE);

  struct_size = sizeof( SharedMem_t );

 #ifdef NEW_FRAME_ENTRY
	memset( unpaged_shared_mem, 0, sizeof(Unpaged_SharedMem_t) );
#endif

  memset( shared_mem, 0, sizeof(SharedMem_t) );
  memset( pg1_shared_mem, 0, sizeof(PAGE1_SharedMem_t) );
		
#if defined(_BCM2152_) || defined(_BCM213x1_) || defined(_BCM2153_)
  *((UInt32 *)(PDMACONF)) = SDRAM_BASE | 0x10;	//0x16:11bits from DSP ADDR_H; //0x0E: 7 bits from DSP ADDR_H
  *((UInt32 *)(DDMACONF)) = SDRAM_BASE | 0x10;
#endif

#ifndef _BCM2124_
  *((UInt32 *)(SMICONF0)) = SHARED_RAM_BASE | 0x0c; 			// 2133 now uses 32-bit register, so do not add 2 to the base address
  *((UInt32 *)(SMICONF2)) = SHARED_RAM_BASE | 0x0c | 0x01; 	// enable pre-fetch buffer mode for polyringer shared mem usage
#else
  *((UInt32 *)(SMICONF0)) = SHARED_RAM_BASE | 0x0c; 			// 2133 now uses 32-bit register, so do not add 2 to the base address
  *((UInt32 *)(SMICONF2)) = SHARED_RAM_BASE | 0x0c | 0x01; 	// enable pre-fetch buffer mode for polyringer shared mem usage
#endif

#if 0
#ifdef POLY_INCLUDED
#if (defined(_BCM2133_) && CHIP_REVISION >= 11) || defined(_BCM2124_) || defined(_BCM2152_) || defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_)
  *((UInt32 *)(SMICONF1))   = ( (UInt32)DlsFile ) | 0x0A | 0x01;
#else
  *((UInt32 *)(0x08130004)) = ( (UInt32)DlsFile ) | 0x0A | 0x01;
#endif
#endif	// POLY_INCLUDED

#endif

  //
  // patch address
  //
//  *((UInt32 *)(SMICONF3)) = 0x00400000 | 0x06 | 0x01; 	// used by 2133a2 patch init function to overlay itself also enable pre-fetch buffer mode
  *((UInt32 *)(SMICONF3)) = SHARED_RAM_BASE | 0x1C | 0x01 | 0x20; 	// DSP soft downloadable codec, max 14bits for sharedmem page, pre-fetch buffer read/write

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
void SYSPARM_Init()
{
  SharedMem_t *mem = SHAREDMEM_GetSharedMemPtr();

  PAGE1_SharedMem_t *pg1_mem = SHAREDMEM_GetPage1SharedMemPtr();
  PAGE5_SharedMem_t *pg5_mem = SHAREDMEM_GetPage5SharedMemPtr();

  //dprintf(DBG_L2, "SYSPARM_Init\n");

  //mem->shared_frame_out   = 0;
  pg5_mem->shared_cmdq_in     = 0;
  pg5_mem->shared_cmdq_out    = 0;
  pg5_mem->shared_statusq_in  = 0;
  pg5_mem->shared_statusq_out = 0;

	mem->shared_audio_output_gain = 32767;	// sys_data_ind->audio_output_gain;
#if (defined(_BCM2152_) && CHIP_REVISION >= 14) || (defined(_BCM2153_) && CHIP_REVISION >= 40) || (CHIPVERSION >= CHIP_VERSION(BCM213x1v,21)) || defined(_BCM21551_)
	pg1_mem->shared_polyringer_out_gain_dl = 64;
	pg1_mem->shared_polyringer_out_gain_ul = 64;
#else
#if (defined(_BCM213x1_) && CHIP_REVISION >= 12)
	pg1_mem->shared_polyringer_out_gain = 64;
	pg1_mem->shared_second_amr_out_gain = 64;
#endif
#endif


#ifdef VPU_INCLUDED
	VPSHAREDMEM_Init();
#endif

#ifndef BSP_IPC_AP
  DSPCore_Init();

//#ifdef INCLUDE_DSP_PATCH
  RIPCMDQ_GetVersionID();
//#endif
#endif
}
#endif //#endif //CONFLICT with audvoc_dsp.c

#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)// && defined(BSP_IPC_AP)  //{
//extern void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2);
extern void SHAREDMEM_PostCmdQ(CmdQ_t *cmd_msg);

#else //} {

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
void SHAREDMEM_PostCmdQ(CmdQ_t *cmd_msg)
{
  CmdQ_t	*p;
  UInt8	next_cmd_in;
  IRQMask_t mask;
  int i, MoreChances;

  do
  {
    if( (UInt8)(( pg5_shared_mem->shared_cmdq_in + 1 ) % CMDQ_SIZE) == pg5_shared_mem->shared_cmdq_out )
    {
      dprintf(DBG_L1, "DSP command queue full!!!\n");
#if	0
      break;
#else
	  mdelay(5);
	  continue;
#endif
    }

    IRQ_DisableAll(&mask);
    next_cmd_in = (UInt8)(( pg5_shared_mem->shared_cmdq_in + 1 ) % CMDQ_SIZE);
    if(next_cmd_in == pg5_shared_mem->shared_cmdq_out)
    {
      dprintf(DBG_L1, "Error: next_cmd_in(0x%x) == shared_cmdq_out(0x%x)\n",
        next_cmd_in, pg5_shared_mem->shared_cmdq_out);
      break;
    }

#if 0
    dprintf(DBG_L2, "cmdq: 0x%x, 0x%x\n",
      pg5_shared_mem->shared_cmdq_in,
      pg5_shared_mem->shared_cmdq_out);
#endif

    p = &pg5_shared_mem->shared_cmdq[ pg5_shared_mem->shared_cmdq_in ];

    p->cmd = cmd_msg->cmd;
    p->arg0 = cmd_msg->arg0;
    p->arg1 = cmd_msg->arg1;
    p->arg2 = cmd_msg->arg2;
    pg5_shared_mem->shared_cmdq_in = next_cmd_in;

    IRQ_Restore( mask );
    IRQ_TriggerRIPInt();
	break;
  } while(1);
}
#endif //}
//******************************************************************************
//
// Function Name:	SHAREDMEM_PostFastCmdQ
//
// Description:		Post an entry to the fast command queue
//
// Notes:
//
//******************************************************************************
#ifdef DSP_FAST_COMMAND
void SHAREDMEM_PostFastCmdQ(		// Post an entry to the fast command queue
	CmdQ_t *cmd_msg					// Entry to post
	)
{
  SHAREDMEM_PostCmdQ(cmd_msg);
#if 0
	CmdQ_t	*p;
	UInt8	next_cmd_in;
	IRQMask_t mask;

	UInt32 Enter_Time = TIMER_GetValue();

	// wait until we have room.

	while( TRUE )
	{
		if( (UInt8)(( pg5_shared_mem->shared_fast_cmdq_in + 1 ) % FAST_CMDQ_SIZE) == pg5_shared_mem->shared_fast_cmdq_out )
		{
			if( TIMER_GetValue() - Enter_Time > TIMEOUT_WAIT_FOR_CMDQ_AVAIABLE )
			{
				// timeout. We will know something wrong here by assertion below.
				xassert(FALSE,Enter_Time);
			}
		}
		else
		{
			break;
		}
	}

	IRQ_DisableAll(&mask);
	next_cmd_in = (UInt8)(( pg5_shared_mem->shared_fast_cmdq_in + 1 ) % FAST_CMDQ_SIZE);
	assert( next_cmd_in != pg5_shared_mem->shared_fast_cmdq_out );

	p = &pg5_shared_mem->shared_fast_cmdq[ pg5_shared_mem->shared_fast_cmdq_in ];

	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;
	pg5_shared_mem->shared_fast_cmdq_in = next_cmd_in;

	IRQ_Restore( mask );
	TriggerInterrupt();
#if defined(DEVELOPMENT_ONLY)
   if ( at_mtst_audio_logging & 0x8000 )
	L1_LOGV4("Fast CMD",(pg5_shared_mem->shared_fast_cmdq_in<<16 | pg5_shared_mem->shared_fast_cmdq_out<<8 | p->cmd),p->arg0,p->arg1,p->arg2);
#endif

#endif
}
#endif

#if 0 ////CONFLICT with audvoc_dsp.c
#ifdef NEW_FRAME_ENTRY
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetSharedMemPtr
//
// Description:		Return pointer to shared memory
//
// Notes:
//
//******************************************************************************

Unpaged_SharedMem_t *SHAREDMEM_GetUnpagedSharedMemPtr()// Return pointer to unpaged shared memory
{
	return unpaged_shared_mem;
}
#endif

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
SharedMem_t *SHAREDMEM_GetSharedMemPtr()
{
  return shared_mem;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
Shared_poly_events_t *SHAREDMEM_Get_poly_events_MemPtr()
{
  return event_page;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
PAGE1_SharedMem_t *SHAREDMEM_GetPage1SharedMemPtr()
{
  return pg1_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage5SharedMemPtr
//
// Description:		Return pointer to Page5 shared memory
//
// Notes:
//
//******************************************************************************

Shared_poly_audio_t *SHAREDMEM_GetPage4SharedMemPtr()// Return pointer to page 5 shared memory
{
	return pg4_shared_mem;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
PAGE5_SharedMem_t *SHAREDMEM_GetPage5SharedMemPtr()
{
  return pg5_shared_mem;
}

PAGE6_SharedMem_t *SHAREDMEM_GetPage6SharedMemPtr()
{
  return pg6_shared_mem;
}

#if (defined(_BCM2152_) && CHIP_REVISION >= 13) || defined(_BCM213x1_)  || defined(_BCM2153_) || defined(_BCM21551_)
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
PAGE7_SharedMem_t *SHAREDMEM_GetPage7SharedMemPtr()
{
  return pg7_shared_mem;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
PAGE10_SharedMem_t *SHAREDMEM_GetPage10SharedMemPtr()
{
  return pg10_shared_mem;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
PAGE11_SharedMem_t *SHAREDMEM_GetPage11SharedMemPtr()
{
  return pg11_shared_mem;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
PAGE14_SharedMem_t *SHAREDMEM_GetPage14SharedMemPtr()
{
  return pg14_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage15SharedMemPtr
//
// Description:		Return pointer to Page15 shared memory
//
// Notes:
//
//******************************************************************************
#if (defined(_BCM213x1_) && CHIP_REVISION >= 20) || (defined(_BCM2153_) && CHIP_REVISION >= 40) || defined(_BCM21551_)

PAGE15_SharedMem_t *SHAREDMEM_GetPage15SharedMemPtr()// Return pointer to page 15 shared memory
{
	return pg15_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage18SharedMemPtr
//
// Description:		Return pointer to Page18 shared memory
//
// Notes:
//
//******************************************************************************

PAGE18_SharedMem_t *SHAREDMEM_GetPage18SharedMemPtr()// Return pointer to page 18 shared memory
{
	return pg18_shared_mem;
}
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage25SharedMemPtr
//
// Description:		Return pointer to Page25 shared memory
//
// Notes:
//
//******************************************************************************

PAGE25_SharedMem_t *SHAREDMEM_GetPage25SharedMemPtr()// Return pointer to page 25 shared memory
{
	return pg25_shared_mem;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage27SharedMemPtr
//
// Description:		Return pointer to Page27 shared memory
//
// Notes:
//
//******************************************************************************

PAGE27_SharedMem_t *SHAREDMEM_GetPage27SharedMemPtr()// Return pointer to page 27 shared memory
{
	return pg27_shared_mem;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage28SharedMemPtr
//
// Description:		Return pointer to Page28 shared memory
//
// Notes:
//
//******************************************************************************

PAGE28_SharedMem_t *SHAREDMEM_GetPage28SharedMemPtr()// Return pointer to page 28 shared memory
{
	return pg28_shared_mem;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage29SharedMemPtr
//
// Description:		Return pointer to Page29 shared memory
//
// Notes:
//
//******************************************************************************

PAGE29_SharedMem_t *SHAREDMEM_GetPage29SharedMemPtr()// Return pointer to page 29 shared memory
{
	return pg29_shared_mem;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_GetPage30SharedMemPtr
//
// Description:		Return pointer to Page30 shared memory
//
// Notes:
//
//******************************************************************************

PAGE30_SharedMem_t *SHAREDMEM_GetPage30SharedMemPtr()// Return pointer to page 30 shared memory
{
	return pg30_shared_mem;
}
#endif


#endif
#endif //CONFLICT with audvoc_dsp.c
static UInt8 AMR_Codec_Mode_Good = 7;

void SHAREDMEM_DLAMR_Speech_Init(void)
{
	int i;
#ifndef L1TEST
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr() + 4*((sizeof(SharedMem_t)+3)/4) );
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->DL_MainAMR_buf);
#else
	UInt16 dummy;
	UInt16 *ptr = &dummy;
#endif
	ptr[0] = AMR_RX_NO_DATA;
	ptr[1] = AMR_Codec_Mode_Good;
	memset( &ptr[2], 0, 32);
}

void SHAREDMEM_ULAMR_Speech_Init(void)
{
	int i;
#ifndef L1TEST
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr() + 4*((sizeof(SharedMem_t)+3)/4) );
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
#else
	UInt16 dummy;
	UInt16 *ptr = &dummy;
#endif
	ptr[0] = AMR_TX_NO_DATA;
	ptr[1] = AMR_Codec_Mode_Good;
	memset( &ptr[2], 0, 32);
}

static UInt32 ul_speech_data[8];
void SHAREDMEM_ReadUL_AMR_Speech(UInt32 **speech_data, Boolean amr_if2_enable)
{
	int i;

	UInt16 UL_AMR_buf[18];
	UInt8  *temp_buf8 = (UInt8  *)UL_AMR_buf;
#ifndef L1TEST
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr() + 4*((sizeof(SharedMem_t)+3)/4) );
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
#else
	UInt16 dummy;
	UInt16 *ptr = &dummy;
#endif
	//Note: ptr[0] RX_Frame_type
	//      ptr[1] AMR code rate
	//      ptr[2]~ptr[17] speech data
//	L1_LOG_ARRAY("AMR UL", ptr,36);
#ifdef __BIG_ENDIAN
	memcpy(UL_AMR_buf, ptr,36);
#else
	UL_AMR_buf[0]=ptr[0];
	UL_AMR_buf[1]=ptr[1];
	for ( i = 2; i < 18; i++)
	{
		temp_buf8[i*2] = ptr[i] >> 8;
		temp_buf8[i*2 + 1] = ptr[i] & 0xFF;
	}
#endif

	if(UL_AMR_buf[0] == AMR_TX_SID_FIRST)
	{
		memset(&UL_AMR_buf[2],0,32);
#ifdef __BIG_ENDIAN
		UL_AMR_buf[4] = ((ptr[1]&0x1)<<11)|((ptr[1]&0x2)<<9)|((ptr[1]&0x4)<<7);
#else
		UL_AMR_buf[4] = ((ptr[1]&0x1)<<3)|((ptr[1]&0x2)<<1)|((ptr[1]&0x4)<<15);
#endif
	}
	if( UL_AMR_buf[0] == AMR_TX_SID_UPDATE)
	{
		memset(&UL_AMR_buf[5],0,26);
#ifdef __BIG_ENDIAN
		UL_AMR_buf[4] |= ((ptr[1]&0x1)<<11)|((ptr[1]&0x2)<<9)|((ptr[1]&0x4)<<7)|0x1000;
#else
		UL_AMR_buf[4] |= ((ptr[1]&0x1)<<3)|((ptr[1]&0x2)<<1)|((ptr[1]&0x4)<<15)|0x10;
#endif
	}
	if (amr_if2_enable)
	{
		UInt16 *pDst = (UInt16*)ul_speech_data;
		for ( i = 0; i < 16; i++)
		{
			pDst[i] = ((UL_AMR_buf[i+2] & 0xff00)) >> 8 | ((UL_AMR_buf[i+2] & 0x00ff) << 8);
		}
	}
	else
	{
		UInt32 * temp_buf32 = (UInt32  *)UL_AMR_buf;
		for( i =0; i < 8; i++)
		{
			ul_speech_data[i] = temp_buf32[i+1];
		}
	}
#if defined(UMTS) && defined(DEVELOPMENT_ONLY)
	if(is_amr_voice_test)
	{
		for( i = 1; i < 8; i++)
		{
			ul_speech_data[i] = 99000*(i+31)*(i+1)+99*i+987543;
		}
		amr_voice_counter ++;
		ul_speech_data[0] = amr_voice_counter;
		L1_LOG_ARRAY("AMR TEST UL", ul_speech_data ,32);
	}
#endif
	*speech_data = ul_speech_data;

//	L1_LOGV4("UL AMR speech", ul_speech_data[0],ul_speech_data[1],ul_speech_data[2],ul_speech_data[3]);
}

UInt16 SHAREDMEM_ReadUL_AMR_Mode(void)
{
#ifndef L1TEST
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr() + 4*((sizeof(SharedMem_t)+3)/4) );
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->UL_MainAMR_buf);
#else
	UInt16 dummy;
	UInt16 *ptr = &dummy;
#endif
	UInt16 temp;

	if( ptr[0] == AMR_TX_SPEECH_GOOD)
	{
		temp = ptr[1];
	}
	else if( (ptr[0] == AMR_TX_SID_FIRST)|| (ptr[0]==AMR_TX_SID_UPDATE))
	{
		temp = 8;
	}
	else if( ptr[0] == AMR_TX_NO_DATA)
	{
		temp = 15;
	}
	else
		xassert( FALSE, ptr[0]);
	//L1_LOGV4("UL type,mode, mac_frame_type", ptr[0],ptr[1],temp,0);
	return(temp);
}

static UInt8 good_speech_cnt =0;
static UInt8 bad_speech_cnt = 0;
static Boolean replace_bad_sid = FALSE;

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132 C3 above*/
#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || (CHIPVERSION >= CHIP_VERSION(BCM2152,10))
void SHAREDMEM_WriteDL_AMR_Speech(UInt8 rx_type,UInt8 frame_type, UInt32 *data, UInt8 length, Boolean amr_if2_enable, UInt16 *softA, UInt32 *deciphbit)
#else
void SHAREDMEM_WriteDL_AMR_Speech(UInt8 rx_type,UInt8 frame_type, UInt32 *data, UInt8 length, Boolean amr_if2_enable)
#endif
{
	int i;
#ifndef L1TEST
	VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) ((unsigned int)SHAREDMEM_GetSharedMemPtr() + 4*((sizeof(SharedMem_t)+3)/4) );
	UInt16 *ptr = (UInt16*)&(vp_shared_mem->DL_MainAMR_buf);
#else
	UInt16 dummy;
	UInt16 *ptr = &dummy;
#endif
#ifndef __BIG_ENDIAN
 	UInt8 *data8 = (UInt8*)data;
#endif
	if (amr_if2_enable)
	{
		UInt16 *pSrc = (UInt16*)data;
	
		for ( i = 0; i < length*2; i++)
		{
		  //			ptr[i+2] = ((pSrc[i] & 0xff00)) >> 8 | ((pSrc[i] & 0x00ff) << 8);
		  ptr[i+2]=pSrc[i];
		}
	}
	else
	{
#ifndef L1TEST
#if defined(UMTS)
	if( SYSPARM_GetMSTWCDMA())
		ptr = (UInt16*)&(vp_shared_mem->DL_wcdma_mst_amr_buf);
#endif
#endif
		for( i = 0 ; i < length; i++)
		{
#ifdef __BIG_ENDIAN
			ptr[2*i+2] = (data[i] >> 16 )&0xffff;
			ptr[2*i+3] = data[i]&0xffff;
#else
			ptr[2*i+2] = data8[i * 4]<<8 | data8[i * 4 +1];
			ptr[2*i+3] = data8[i * 4 + 2]<<8 | data8[i * 4 + 3];
#endif
		}

	}
	if( rx_type == 0)
	{
#if defined(UMTS)
		if( SYSPARM_GetMSTWCDMA()&&(!amr_if2_enable))
		{
			for( i=0; i< 38; i++)
			{
#ifdef __BIG_ENDIAN
				ptr[18 +2*i]= softA[2*i+1];
				ptr[18 + 2*i+1] = softA[2*i];
#else
				ptr[18 +2*i] = softA[2*i];
				ptr[18 + 2*i+1] = softA[2*i+1];
#endif
			}
	
			 if( deciphbit !=NULL)
			 {
#ifndef __BIG_ENDIAN
				data8 = (UInt8*)deciphbit;
#endif
				for( i=0; i <3; i++)
				{
#ifdef __BIG_ENDIAN
					ptr[94+2*i]=  (deciphbit[i]>>16)&0xffff;
					ptr[94+2*i+1]=deciphbit[i]&0xffff;
#else
				ptr[94+2*i] = data8[i * 4]<<8 | data8[i * 4 +1];
				ptr[94+2*i+1] = data8[i * 4 + 2]<<8 | data8[i * 4 + 3];
#endif
				}
#ifdef __BIG_ENDIAN	
				   ptr[99] |= 0x1;
#else
				   ptr[99] |= 0x100;
#endif
			 }
			 else
			 {
				for( i=0; i< 6; i++)
					ptr[94+i]=0;
			}
	//			L1_LOG_ARRAY("cipher", &ptr[94],12);
		}
#endif

		bad_speech_cnt ++;
		if(bad_speech_cnt >= 5)
		{
			replace_bad_sid = TRUE;
			good_speech_cnt = 0;
		}
		if(frame_type < 8)
		{
			ptr[0] = AMR_RX_SPEECH_BAD;
			ptr[1] = frame_type;
	
		}
		else if( frame_type == 8)
		{
			 if ( replace_bad_sid)
				ptr[0] = AMR_RX_SPEECH_BAD;
			  else
			ptr[0] = AMR_RX_SID_BAD;
			ptr[1] = AMR_Codec_Mode_Good;
		}
		else if( (frame_type <=11 )&&(frame_type >=9 ))
		{
			xassert(FALSE, frame_type);
		}
		else if( frame_type == 15)
		{
			ptr[0] = AMR_RX_NO_DATA;
			ptr[1] = AMR_Codec_Mode_Good;
		}
		else
			xassert( FALSE, frame_type);
	}
	else if( rx_type == 1)
	{
		good_speech_cnt ++;
		if(good_speech_cnt >= 2)
		{
			replace_bad_sid = FALSE;
			bad_speech_cnt = 0;
		}
		if(frame_type < 8)
		{
			ptr[0] = AMR_RX_SPEECH_GOOD;
			ptr[1] = frame_type;
			AMR_Codec_Mode_Good = frame_type;

		}
		else if( frame_type == 8)
		{
			if( ptr[4]&0x1000)
				ptr[0] = AMR_RX_SID_UPDATE;
			else
				ptr[0] = AMR_RX_SID_FIRST; //AMR_RX_SID_UPDATE
			ptr[1] = AMR_Codec_Mode_Good ;
		//	L1_LOGV("SID codec", ptr[4]&0x0f00);
		}
		else if( (frame_type <=11 )&&(frame_type >=9 ))
		{
			xassert(FALSE, frame_type);
		}
		else if( frame_type == 15)
		{
			ptr[0] = AMR_RX_NO_DATA;
			ptr[1] = AMR_Codec_Mode_Good;
		}
		else
			xassert( FALSE, frame_type);
	}
	else
		xassert( FALSE, rx_type);

//	 L1_LOGV4("DL length,type,mode,speech", length,ptr[0],ptr[1],ptr[2]);
//	 L1_LOG_ARRAY("AMR DL", ptr,36);
//	 L1_LOG_ARRAY("Softbit",&ptr[18],152);
}
#endif

#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteDL_VoIP_Data
//
// Description:	Write the DL VoIP data to sharedmem
// Notes:
//
//******************************************************************************
void SHAREDMEM_WriteDL_VoIP_Data(UInt16 codec_type, UInt16 *pSrc)
{
	Unpaged_SharedMem_t *unpage_sm = SHAREDMEM_GetUnpagedSharedMemPtr();
	UInt16 *pDst = (UInt16*)&(unpage_sm->VOPI_DL_buf.voip_frame.frame_pcm[0]);
	UInt8 data_len = 0;

	if ((codec_type & 0xf000) == VOIP_PCM)		//PCM
  		data_len = 160;
  	else if ((codec_type & 0xf000) == VOIP_FR)	// FR
  		data_len = 79;
  	else if ((codec_type & 0xf000) == VOIP_AMR475)	// AMRNB
		data_len = 18;

	// copy the data to dsp. may need some format conversion for FR and AMRNB. 
	// this depends on the codec type and format we received from ip network.
	memcpy(pDst, pSrc, data_len<<1);
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadUL_VoIP_Data
//
// Description:	Read the UL VoIP data from sharedmem
// Notes:
//
//******************************************************************************
UInt8 SHAREDMEM_ReadUL_VoIP_Data(UInt16 codec_type, UInt16 *pDst)
{
	Unpaged_SharedMem_t *unpage_sm = SHAREDMEM_GetUnpagedSharedMemPtr();
	UInt16 *pSrc = (UInt16*)&(unpage_sm->VOPI_UL_buf.voip_frame.frame_pcm[0]);
	UInt8 data_len = 0;
	
	if ((codec_type & 0xf000) == VOIP_PCM)		//PCM
  		data_len = 160;
  	else if ((codec_type & 0xf000) == VOIP_FR)	// FR
  		data_len = 79;
  	else if ((codec_type & 0xf000) == VOIP_AMR475)	// AMRNB
		data_len = 18;

	// copy the data to arm. may need some format conversion for FR and AMRNB. 
	// this depends on the codec type and format from ip network.	
	memcpy(pDst, pSrc, data_len<<1);
	
	return data_len;
}
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:
//!
//! Description:
//!
/////////////////////////////////////////////////////////////////////////////
Boolean dsp_read_message(StatQ_t *status_msg)
{
  StatQ_t *p;
  UInt16	status_out;
  UInt16	status_in;
  Boolean	result;

  status_out = pg5_shared_mem->shared_statusq_out;
  status_in = pg5_shared_mem->shared_statusq_in;
  if ( status_out == status_in )
  {
#ifdef	CHK_DSP_INTR
    dprintf(DBG_L2, "no msg from DSP\n");
#endif
    result = FALSE;
  }
  else
  {
#ifdef	CHK_DSP_INTR
	  dprintf(DBG_L2, "a msg from DSP\n");
#endif
    p = &pg5_shared_mem->shared_statusq[ status_out ];
    status_msg->status = p->status;
    status_msg->arg0 = (UInt16)p->arg0;
    status_msg->arg1 = (UInt16)p->arg1;
    status_msg->arg2 = (UInt16)p->arg2;

    pg5_shared_mem->shared_statusq_out = ( status_out + 1 ) % STATUSQ_SIZE;
    result = TRUE;
  }
  return(result);
}

#ifdef INCLUDE_DSP_PRAM_LOAD

#pragma arm section zidata="er_pram_load"
static UInt8	mem_pram_load[0x80000];
#pragma arm section

#endif

#ifdef INCLUDE_DSP_PATCH

#include "flash_image_def.h"

#if 0
static UInt32 embedded_patch[] =
{
#include "patch_dsp.h"
};
#endif

static UInt8 *prom_patch_addr = NULL;
UInt32 dsp_changelist;
char DSP_version[80] = {0};

//******************************************************************************
// Function Name: MS_GetPatchAddr
//
// Description: Return the base address of DSP patch
//
// Notes:
//******************************************************************************
UInt32 MS_GetPatchAddr( void )
{
  return PROM_IMAGE_ADDR;
}

//******************************************************************************
// Function Name: MS_GetPatchLen
//
// Description: Return the size of DSP patch
//
// Notes:
//******************************************************************************
UInt32 MS_GetPatchLen( void )
{
  return PROM_IMAGE_SIZE;
}

UInt32 SYSPARM_GetPatchAddr()
{
#if defined(FLASH_TYPE_NAND)
  prom_patch_addr = (UInt8*)OSHEAP_Alloc(MS_GetPatchLen());
  FlashLoadImage( MS_GetPatchAddr(), MS_GetPatchLen(), prom_patch_addr );
#else // else, FLASH_TYPE_NOR
  prom_patch_addr = (UInt8 *)MS_GetPatchAddr();
#endif
  return (UInt32)prom_patch_addr;
}

void LoadDSPPatch(UInt16	v1, UInt16	v2, UInt16	v3)
{
  PATCH_SetBaseAddr( SYSPARM_GetPatchAddr() );

  dsp_changelist = ((v2&0xf000)>>12)*10000000 + \
		 ((v2&0x0f00)>>8 )*1000000  + \
		 ((v2&0x00f0)>>4 )*100000   + \
		 ((v2&0x000f)    )*10000    + \
		 ((v3&0xf000)>>12)*1000	 + \
		 ((v3&0x0f00)>>8 )*100	 + \
		 ((v3&0x00f0)>>4 )*10	 + \
		 ((v3&0x000f)    );


  if (PATCH_Check() == TRUE)
  {
    PATCH_Config();
    // make this show up on release build as well
    sprintf(DSP_version, "DSP ver/CL: %d %d, Patch ver/CL/on: %s %s %d\r\n",
      v1, dsp_changelist,
      (char*)PATCH_GetRevision() ,(char*)PATCH_GetChangelist(), PATCH_GetEnabled());
  }
  else
  {
    sprintf(DSP_version, "DSP ver/CL: %d %d, No patch\r\n", v1, dsp_changelist);
  }

  dprintf(1, "DSP_version: %s\n", DSP_version);
}
#endif

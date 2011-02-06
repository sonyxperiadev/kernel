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

#include <linux/module.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <mach/reg_dsp_dma.h>
#include <mach/reg_smi.h>

#include "audio_ipc_consts.h"
#include "audvoc_consts.h"
#include "shared.h"
#include "memmap.h"
#include "audvoc_drv.h"
#include <rpc_internal_api.h>

#define __AUD "AUD:DSP: "

#define NEW_FRAME_ENTRY //BCM2153 uses new frame entry

extern void audvoc_isr_handler(StatQ_t status_msg);

//vpu.c
extern void AP_Process_VPU_Status( VPStatQ_t vp_status_msg);
extern void VPU_ProcessStatusMainAMRDone(StatQ_t status_msg);
//aacenc_dev.c
extern	void	DspCodecMsgHandler(StatQ_t *pstatus_msg);


#ifdef NEW_FRAME_ENTRY
static Unpaged_SharedMem_t __iomem * unpaged_shared_mem;
static SharedMem_t * shared_mem;
#else
static SharedMem_t shared_mem_region;
static SharedMem_t * shared_mem;
#endif

static PAGE1_SharedMem_t * pg1_shared_mem;	//base address of page 1 shared RAM
static Shared_poly_events_t * event_page;	//base address of page 3 shared RAM
static Shared_poly_audio_t * pg4_shared_mem;	//base address of page 4 shared RAM
static PAGE5_SharedMem_t * pg5_shared_mem;		//base address of page 5 shared RAM
static PAGE6_SharedMem_t * pg6_shared_mem;		//base address of page 6 shared RAM
static PAGE7_SharedMem_t * pg7_shared_mem;		//base address of page 7 shared RAM
static PAGE10_SharedMem_t * pg10_shared_mem;		//base address of page 10 shared RAM
static PAGE11_SharedMem_t * pg11_shared_mem;		//base address of page 11 shared RAM
static PAGE14_SharedMem_t * pg14_shared_mem;		//base address of page 14 shared RAM
static PAGE15_SharedMem_t * pg15_shared_mem;		//base address of page 15 shared RAM
static PAGE16_SharedMem_t * pg16_shared_mem;		//base address of page 16 shared RAM
static PAGE17_SharedMem_t * pg17_shared_mem;		//base address of page 17 shared RAM
static PAGE18_SharedMem_t * pg18_shared_mem;		//base address of page 18 shared RAM
static PAGE19_SharedMem_t * pg19_shared_mem;		//base address of page 19 shared RAM
static PAGE20_SharedMem_t * pg20_shared_mem;		//base address of page 20 shared RAM
static PAGE25_SharedMem_t * pg25_shared_mem;		//base address of page 20 shared RAM
static PAGE27_SharedMem_t * pg27_shared_mem;		//base address of page 27 shared RAM
static PAGE28_SharedMem_t * pg28_shared_mem;		//base address of page 28 shared RAM
static PAGE29_SharedMem_t * pg29_shared_mem;		//base address of page 29 shared RAM
static PAGE30_SharedMem_t * pg30_shared_mem;		//base address of page 30 shared RAM

//static UInt16 value_written_to_dsp;
extern wait_queue_head_t audHalShutdown;
extern Boolean AudHalShutdownRespReceived;

extern size_t csl_aud_play(const char __user * buffer, size_t size);

// non-static
void SHAREDMEM_Init(void);
void SYSPARM_Init(void);
void SHAREDMEM_PostCmdQ(CmdQ_t *cmd_msg);
SharedMem_t * SHAREDMEM_GetSharedMemPtr(void);
PAGE7_SharedMem_t *SHAREDMEM_GetPage7SharedMemPtr(void);
void brcm_dsp_interrupt_hisr(void *, UInt32);

// static
static void SHAREDMEM_MapAddress(void);

#ifdef NEW_FRAME_ENTRY
Unpaged_SharedMem_t *SHAREDMEM_GetUnpagedSharedMemPtr()// Return pointer to unpaged shared memory
{
        return unpaged_shared_mem;
}
#endif


SharedMem_t * SHAREDMEM_GetSharedMemPtr()
{
        return shared_mem;
}


Shared_poly_events_t *SHAREDMEM_Get_poly_events_MemPtr()
{
        return event_page;
}

PAGE1_SharedMem_t * SHAREDMEM_GetPage1SharedMemPtr()
{
        return pg1_shared_mem;
}


Shared_poly_audio_t *SHAREDMEM_GetPage4SharedMemPtr()// Return pointer to page 5 shared memory
{
        return pg4_shared_mem;
}

PAGE5_SharedMem_t *SHAREDMEM_GetPage5SharedMemPtr()
{
        return pg5_shared_mem;
}

PAGE6_SharedMem_t *SHAREDMEM_GetPage6SharedMemPtr()
{
        return pg6_shared_mem;
}

PAGE7_SharedMem_t *SHAREDMEM_GetPage7SharedMemPtr()
{
        return pg7_shared_mem;
}

PAGE10_SharedMem_t *SHAREDMEM_GetPage10SharedMemPtr()
{
        return pg10_shared_mem;
}

PAGE11_SharedMem_t *SHAREDMEM_GetPage11SharedMemPtr()
{
        return pg11_shared_mem;
}

PAGE14_SharedMem_t *SHAREDMEM_GetPage14SharedMemPtr()
{
        return pg14_shared_mem;
}

PAGE15_SharedMem_t *SHAREDMEM_GetPage15SharedMemPtr()// Return pointer to page 15 shared memory
{
        return pg15_shared_mem;
}


PAGE18_SharedMem_t *SHAREDMEM_GetPage18SharedMemPtr()// Return pointer to page 18 shared memory
{
        return pg18_shared_mem;
}

PAGE25_SharedMem_t *SHAREDMEM_GetPage25SharedMemPtr()// Return pointer to page 25 shared memory
{
        return pg25_shared_mem;
}

PAGE27_SharedMem_t *SHAREDMEM_GetPage27SharedMemPtr()// Return pointer to page 27 shared memory
{
        return pg27_shared_mem;
}

PAGE28_SharedMem_t *SHAREDMEM_GetPage28SharedMemPtr()// Return pointer to page 28 shared memory
{
        return pg28_shared_mem;
}

PAGE29_SharedMem_t *SHAREDMEM_GetPage29SharedMemPtr()// Return pointer to page 29 shared memory
{
        return pg29_shared_mem;
}

PAGE30_SharedMem_t *SHAREDMEM_GetPage30SharedMemPtr()// Return pointer to page 30 shared memory
{
        return pg30_shared_mem;
}


void SHAREDMEM_Init(void)
{
        pr_info(__AUD "SHAREDMEM_Init\n");

        SHAREDMEM_MapAddress();

//#ifdef NEW_FRAME_ENTRY
        //memset(unpaged_shared_mem, 0, sizeof(Unpaged_SharedMem_t));
//#endif
        //memset(shared_mem, 0, sizeof(SharedMem_t));
        //memset(pg1_shared_mem, 0, sizeof(PAGE1_SharedMem_t));

        writel(SDRAM_BASE|0x10, &PDMACONF); // 0x16:11bits from DSP ADDR_H; //0x0E: 7 bits from DSP ADDR_H
        writel(SDRAM_BASE|0x10, &DDMACONF);

        writel(SHARED_RAM_BASE|0x0c, &SMICONF0); // 2133 now uses 32-bit register, so do not add 2 to the base address
        writel(SHARED_RAM_BASE|0x0c|0x01, &SMICONF2); // enable pre-fetch buffer mode for polyringer shared mem usage

        //
        // patch address
        //
        writel(SHARED_RAM_BASE | 0x1C | 0x01 | 0x20, &SMICONF3); // DSP soft downloadable codec, max 14bits for sharedmem page, pre-fetch buffer read/write

        pr_info(__AUD"PDMACONF=0x%x\n", (unsigned int) readl(&PDMACONF));
        pr_info(__AUD"DDMACONF=0x%x\n", (unsigned int) readl(&DDMACONF));
        pr_info(__AUD"SMICONF0=0x%x\n", (unsigned int) readl(&SMICONF0));
        pr_info(__AUD"SMICONF2=0x%x\n", (unsigned int) readl(&SMICONF2));
        pr_info(__AUD"SMICONF3=0x%x\n", (unsigned int) readl(&SMICONF3));

}

static void SHAREDMEM_MapAddress(void)
{
        pr_info(__AUD"SHAREDMEM_MapAddress\n");

        unpaged_shared_mem = (Unpaged_SharedMem_t *)(ioremap_nocache(SHARED_RAM_BASE, DSP_SH_SIZE));

        if (!unpaged_shared_mem) {
                pr_info(__AUD"unpaged_shared_mem iomap failed!\n");
        }

        shared_mem = (SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x2000);
        pg1_shared_mem = (PAGE1_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x4000);	//base address of page 1 shared RAM
        event_page = (Shared_poly_events_t *)(((UInt8 *)unpaged_shared_mem)+0x8000);	//base address of page 3 shared RAM
        pg4_shared_mem = (Shared_poly_audio_t *)(((UInt8 *)unpaged_shared_mem)+0xA000);	//base address of page 4 shared RAM
        pg5_shared_mem = (PAGE5_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0xC000);		//base address of page 5 shared RAM
        pg6_shared_mem = (PAGE6_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0xe000);		//base address of page 6 shared RAM
        pg7_shared_mem = (PAGE7_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x10000);		//base address of page 7 shared RAM
        pg10_shared_mem = (PAGE10_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x16000);		//base address of page 10 shared RAM
        pg11_shared_mem = (PAGE11_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x18000);		//base address of page 11 shared RAM
        pg14_shared_mem = (PAGE14_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x1E000);		//base address of page 14 shared RAM
        pg15_shared_mem = (PAGE15_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x20000);		//base address of page 15 shared RAM
        pg16_shared_mem = (PAGE16_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x22000);		//base address of page 16 shared RAM
        pg17_shared_mem = (PAGE17_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x24000);		//base address of page 17 shared RAM
        pg18_shared_mem = (PAGE18_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x26000);		//base address of page 18 shared RAM
        pg19_shared_mem = (PAGE19_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x28000);		//base address of page 19 shared RAM
        pg20_shared_mem = (PAGE20_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x2a000);		//base address of page 20 shared RAM
        pg25_shared_mem = (PAGE25_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x34000);		//base address of page 20 shared RAM
        pg27_shared_mem = (PAGE27_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x38000);		//base address of page 27 shared RAM
        pg28_shared_mem = (PAGE28_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x3a000);		//base address of page 28 shared RAM
        pg29_shared_mem = (PAGE29_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x3c000);		//base address of page 29 shared RAM
        pg30_shared_mem = (PAGE30_SharedMem_t *)(((UInt8 *)unpaged_shared_mem)+0x3e000);		//base address of page 30 shared RAM
}

void SYSPARM_Init(void)
{
        SharedMem_t * const mem = SHAREDMEM_GetSharedMemPtr();
        PAGE1_SharedMem_t * const pg1_mem = SHAREDMEM_GetPage1SharedMemPtr();
        PAGE5_SharedMem_t * const pg5_mem = SHAREDMEM_GetPage5SharedMemPtr();

        pr_info(__AUD"SYSPARM_Init\n");

        //mem->shared_frame_out   = 0;
        pg5_mem->shared_cmdq_in = 0;
        pg5_mem->shared_cmdq_out = 0;
        pg5_mem->shared_statusq_in = 0;
        pg5_mem->shared_statusq_out = 0;

        mem->shared_audio_output_gain = 32767;	// sys_data_ind->audio_output_gain;
        pg1_mem->shared_polyringer_out_gain_dl = 64;
        pg1_mem->shared_polyringer_out_gain_ul = 64;

//#ifdef VPU_INCLUDED
        VPSHAREDMEM_Init();
//#endif
}


void brcm_dsp_interrupt_hisr(void * buffer, UInt32 length)
{
        StatQ_t * status_msg;
 	  VPStatQ_t vp_status_msg;

	if(length == sizeof(vp_status_msg))
	{
		memcpy(&vp_status_msg, buffer, sizeof(vp_status_msg));
//		printk("\n brcm_dsp_interrupt_hisr VPU : msg = 0x%x,arg0 = %d,arg1 = %d,arg2 = %d,arg3 = %d\n", vp_status_msg.status, vp_status_msg.arg0, vp_status_msg.arg1, vp_status_msg.arg2,vp_status_msg.arg3);		
		AP_Process_VPU_Status(vp_status_msg);
		return;
	}


        if (length != sizeof(StatQ_t))
                return;

        status_msg = (StatQ_t *)buffer;
//printk("\n brcm_dsp_interrupt_hisr : msg = 0x%x,arg0 = %04x,arg1 = %04xd,arg2 = %04x\n", status_msg->status, status_msg->arg0, status_msg->arg1, status_msg->arg2);		
		switch ( status_msg->status )
		{
			case STATUS_NEWAUDFIFO_CANCELPLAY:
			case STATUS_NEWAUDFIFO_DONEPLAY:
				pr_info("\n brcm_dsp_interrupt_hisr : msg = 0x%x,arg0 = %d,arg1 = %d,arg2 = %d\n", status_msg->status, status_msg->arg0, status_msg->arg1, status_msg->arg2);		
			case STATUS_NEWAUDFIFO_SW_FIFO_LOW:
			case STATUS_NEWAUDFIFO_SW_FIFO_EMPTY:
				audvoc_isr_handler(*status_msg);
				break;
			case STATUS_MAIN_AMR_DONE:
               VPU_ProcessStatusMainAMRDone(*status_msg);
			     break;
				 
			case STATUS_TEST_PDMA:
			case STATUS_PRAM_CODEC_INPUT_LOW:
			case STATUS_PRAM_CODEC_INPUT_EMPTY:
			case STATUS_PRAM_CODEC_OUTPUT_LOW:
			case STATUS_PRAM_CODEC_OUTPUT_FULL:
			case STATUS_PRAM_CODEC_DONEPLAY:
			case STATUS_PRAM_CODEC_CANCELPLAY:
			case STATUS_PRAM_CODEC_DONE_RECORD:
				DspCodecMsgHandler(status_msg);
				break;
			case STATUS_READ_STATUS:
				{
				//printk("\nDSP read:[0x%04x]=0x%04x\n", status_msg->arg0, status_msg->arg2);
				if (status_msg->arg1 == 0x99)
					{
					   AudHalShutdownRespReceived = TRUE;
                       wake_up(&audHalShutdown);
					}
				 }
				break;
	
			case STATUS_AUDIO_STREAM_DATA_READY:
            	process_Log_Channel(*status_msg);		
		default:
				break;
		}

}




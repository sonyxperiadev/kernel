/*******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
 ******************************************************************************/
#ifndef _VCE_REG_H_
#define _VCE_REG_H_

/*From hardware_video_codec.h*/
#define HW_REGISTER_RW(x) (x)

#define INTERRUPT_CODEC0               (INTERRUPT_HW_OFFSET + 4)
#define INTERRUPT_CODEC1               (INTERRUPT_HW_OFFSET + 5)
#define INTERRUPT_CODEC2               (INTERRUPT_HW_OFFSET + 6)
#define VIDEOCODEC_BASE_ADDRESS        0x3d000000
/*---------------------------------------------------------------------------*/
/* Video Codec */

#define VCSIGNAL0           HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x4408b4)
#define VCINTMASK0          HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x4408b8)
#define VCSIGNAL1           HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x4408bc)
#define VCINTMASK1          HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x4408c0)

#ifndef VCODEC_VERSION
/* Set default to old A0 version */
#define VCODEC_VERSION 821
#endif

#if (VCODEC_VERSION >= 800)

#define VCE_BASE 0x3d100000
#define VCE_DATA_MEM_OFFSET 0
#define VCE_DATA_MEM_SIZE 0x2000
#define VCE_PROGRAM_MEM_OFFSET 0x10000
#define VCE_PROGRAM_MEM_SIZE 0x4000
#define VCE_REGISTERS_OFFSET 0x20000
#define VCE_REGISTERS_COUNT 63
#define VCE_STATUS_OFFSET 0x40000
#define VCE_STATUS_BUSYBITS_MASK 0xffff
#define VCE_STATUS_REASON_POS 16
#define VCE_STATUS_REASON_MASK 0x1f
#define VCE_BUSY_BKPT 0x00
/* up to 0x07 inclusive */
#define VCE_BUSY_USER 0x01
#define VCE_BUSY_DMAIN 0x08
#define VCE_BUSY_DMAOUT 0x09
#define VCE_BUSY_MEMSYNC 0x0a
#define VCE_BUSY_SLEEP 0x0b
#define VCE_REASON_STOPPED 0x10
#define VCE_REASON_RUNNING 0x11
#define VCE_REASON_RESET 0x12
#define VCE_REASON_SINGLE 0x13
#define VCE_STATUS_RUNNING_POS 24
#define VCE_STATUS_NANOFLAG_POS 25
#define VCE_STATUS_INTERRUPT_POS 31
#define VCE_VERSION_OFFSET 0x40004
#define VCE_PC_PF0_OFFSET 0x40008
#define VCE_PC_IF0_OFFSET 0x4000c
#define VCE_PC_RD0_OFFSET 0x40010
#define VCE_PC_EX0_OFFSET 0x40014
#define VCE_CONTROL_OFFSET 0x40020
#define VCE_CONTROL_CLEAR_RUN 0
#define VCE_CONTROL_SET_RUN 1
#define VCE_CONTROL_SINGLE_STEP 3
#define VCE_BAD_ADDR_OFFSET 0x40030
#define VCE_PC_ERR_OFFSET 0x40034
#define VCE_SEMA_CLEAR_OFFSET 0x40024
#define VCE_SEMA_SET_OFFSET 0x40028
#define VCE_SEMA_COUNT 8
#define VCE_SIM_DEBUG_OPTIONS_OFFSET 0x40100

#define VCE_DATA_MEM_BASE     HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x100000)
#define VCE_PROGRAM_MEM_BASE  HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x110000)
#define VCE_REGISTERS_BASE    HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x120000)
#define VCE_STATUS            HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140000)
#define VCE_VERSION           HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140004)
#define VCE_PC_PF0            HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140008)
#define VCE_PC_IF0            HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x14000C)
#define VCE_PC_RD0            HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140010)
#define VCE_PC_EX0            HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140014)
#define VCE_CONTROL           HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140020)
#define VCE_SEMA_CLEAR        HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140024)
#define VCE_SEMA_SET          HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140028)
#define VCE_BAD_ADDR          HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140030)
#define VCE_SIM_DEBUG_OPTIONS HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x140100)

#else
#define PP_PC               HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x110000)
#define PP_CNTL             HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x110004)
#define PP_ACC              HW_REGISTER_RW(VIDEOCODEC_BASE_ADDRESS + 0x110008)
#endif

#endif /*_VCE_REG_H_*/

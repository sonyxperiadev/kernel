/*****************************************************************************
* Copyright 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _MQOS_H_
#define _MQOS_H_


/* include file declarations */
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <mach/rdb/brcm_rdb_include.h>
#include <mach/io_map.h>


#ifndef MEMC1_OPEN_BASE_ADDR
#define MEMC1_OPEN_BASE_ADDR	VC4_EMI_OPEN_BASE_ADDR
#endif

#ifndef MEMC0_OPEN_BASE_ADDR
#define MEMC0_OPEN_BASE_ADDR	SYS_EMI_OPEN_BASE_ADDR
#endif
/*--------Macro Definitions-----------------------------------------------------*/
/* - If CAM_LATENCY_0 == 0 && the incoming AXI command associated with CAM ENTRY_0 is a read,
      the Demesh will treat the command as a ""bandwidth"" type command. 
   - All other accesses are considered ""Latency"" type commands.
   - If the command is considered a bandwidth command a history count is kept in CAM_ENTRY_0. 
   - When a command is received CAM_URG_CTR_0 is added to the history count. 
   - During clock cycles when a command is not received that uses CAM_ENTRY_0 the history count is decremented.
   - CAM_ENTRY_0 is the default CAM entry register. 
   - If the incoming command's ""AXI ID"" does not match a CAM tag in the other CAM_ENTRY registers the command will use this register's fields.*/

#define CHAL_MQOS_CAM_ENTRY_CFG_STR(CAM_READ_ENABLE,	/*RW This field specifies if a read command, using CAM_ENTRY_0 for its QoS parameters, will enable its QoS parameters to be used to prioritize arbitration for the DRAM interface.0x1: Use QoS to prioritize arbitration for DRAM reads0x0: Do not use QoS to prioritize arbitration for DRAM reads*/\
									CAM_WRITE_ENABLE,	/*RWThis field specifies if a write command, using CAM_ENTRY_0 for its QoS parameters, will enable its QoS parameters to be used to prioritize arbitration for the DRAM interface.0x1: Use QoS to prioritize arbitration for DRAM writes0x0: Do not use QoS to prioritize arbitration for DRAM writes*/\
									CAM_PRIORITY,		/*RW CAM Priority for Register 0NOTE: This field is not used. It is included to make this register orthogonal with the other CAM_ENTRY registers.Regardless of how this field is programmed an access that uses this CAM entry will always be considered low priority.*/\
									CAM_LATENCY,		/*RW CAM Latency for Register 0NOTE: This field is not used. It is included to make this register orthogonal with the other CAM_ENTRY registers.Regardless of how this field is programmed an access that uses this CAM entry will always be considered a latency access.*/\
									CAM_URG_CTR,		/*RW CAM Urgent Counter for Register 0IF an incoming command is considered a ""latency"" command this value will be loaded into a Demesh entry's entry_urgent counter.IF an incoming command is considered a ""bandwidth"" command this value will be added to the bandwidth history counter and loaded into a Demesh entry's entry_urgent counter.Once a Demesh entry_urgent_counter is non-zero it will decrement, using the 156Mhz ""aon_clk"" until it is zero.Demesh Entry = urgent for arbitration when entry_urgent_counter <= entry_thold*/\
									CAM_THOLD			/*RW CAM Threshold for Register 0 */)\
			(((CAM_READ_ENABLE << CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_SHIFT) &      /*  27*/\
								  CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK) |         /*0x08000000*/\
			((CAM_WRITE_ENABLE << CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_SHIFT) &     /*  26*/\
								  CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK) |        /*0x04000000*/\
			((CAM_PRIORITY	   << CSR_CAM_ENTRY_0_CAM_PRIORITY_0_SHIFT) &         /*  25*/\
								  CSR_CAM_ENTRY_0_CAM_PRIORITY_0_MASK) |            /*0x02000000*/\
			((CAM_LATENCY	   << CSR_CAM_ENTRY_0_CAM_LATENCY_0_SHIFT) &          /*  24*/\
								  CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK) |             /*0x01000000*/\
			((CAM_URG_CTR	   << CSR_CAM_ENTRY_0_CAM_URG_CTR_0_SHIFT) &          /*  12*/\
								  CSR_CAM_ENTRY_0_CAM_URG_CTR_0_MASK) |            /* 0x003FF000*/\
			((CAM_THOLD		   << CSR_CAM_ENTRY_0_CAM_THOLD_0_SHIFT) &           /*   0*/\
								  CSR_CAM_ENTRY_0_CAM_THOLD_0_MASK)) 

/* - CAM_ENABLE_0 is part of the default CAM entry. 
   - This register's fields are not actually used to choose the CAM entry fields to load into the Demesh. 
   - It is provided to make the CAM registers orthogonal. 
   - If the incoming command's ""AXI ID"" does not match a CAM tag in the other CAM_ENABLE registers,
      the command will use CAM_ENTRY_0's register fields. */

#define CHAL_MQOS_CAM_TAG_CFG_STR( CAM_TAG_ENABLE,	/*CAM Enable for CAM Entry 15The contents of CAM Entry 15 will be loaded into a Demesh entry if an incoming command's {PORT_ID, AXI ID} matches the value in the field CAM_TAG_15 excluding the CAM_TAG_ENABLE_15 bits that are zero:(({PORT_ID, AXI ID} XOR CAM_TAG_15) AND CAM_TAG_ENABLE_15) == 0 */\
								   CAM_TAG			/*CAM Tag for CAM Entry 15 */)\
			(((CAM_TAG_ENABLE << CSR_CAM_ENABLE_0_CAM_TAG_ENABLE_0_SHIFT ) &     /*16*/\
								 CSR_CAM_ENABLE_0_CAM_TAG_ENABLE_0_MASK)   |      /*0x7FFF0000*/\
			(( CAM_TAG		  << CSR_CAM_ENABLE_0_CAM_TAG_0_SHIFT) &            /*0*/\
								 CSR_CAM_ENABLE_0_CAM_TAG_0_MASK))             /*0x00007FFF*/

/* default configuration string */
#define CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY	1
#define CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY	1
#define CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY		1
#define CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY		1
#define CHAL_MQOS_CAM_URG_CTR_DEFAULT_LATENCY		5 	/* at 156MHz clock, 5*6.4nsec = 32 nsec */
#define CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY			0	/* this must be zero */
#define CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_LATENCY		CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,\
																				CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,\
																				CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,\
																				CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,\
																				CHAL_MQOS_CAM_URG_CTR_DEFAULT_LATENCY,\
																				CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY)

#define CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH		1
#define CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH	1
#define CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH		1
#define CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH			0
#define CHAL_MQOS_CAM_URG_CTR_DEFAULT_BANDWIDTH			39	/* average time between access = 250ns@156Mhz = 39*/
#define CHAL_MQOS_CAM_THOLD_DEFAULT_BANDWIDTH			156 /* Min number of access remaining in the FIFO before MEMC must set urgent. 1us @156Mhz */
#define CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH	CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH,\
									  											CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH,\
									  											CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH,\
									  											CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH	,\
																				CHAL_MQOS_CAM_URG_CTR_DEFAULT_BANDWIDTH,\
																				CHAL_MQOS_CAM_THOLD_DEFAULT_BANDWIDTH)			
													
#define ISLAND_HANA
#ifdef ISLAND_HANA
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CATCHALL		0x0000		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_CATCHALL				0x0000		/* A9 connected to Port 1.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CATCHALL	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CATCHALL,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_CATCHALL)
/*
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_UNUSED			0x1FFF		
#define CHAL_MQOS_CAM_TAG_DEFAULT_UNUSED				0x1FFF		
*/
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_UNUSED			0x7FFF		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_UNUSED				0x0		/*   */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_UNUSED	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_UNUSED,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_UNUSED)

/* PL310 AXI ID */
/* SYS EMI (DDR3): identical to SYS LPDDR2 */
/* SYS EMI (DDR3): not identical to MM LPDDR2 */
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_A9				0x7F00		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_A9					0x2000		/* A9 connected to Port 1 of SYS DDR2 and DDR3.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_A9		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_A9,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_A9)
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_MM_PL310		0x7603		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_MM_PL310				0x4000		/* A9 connected to Port 1 of MM DDR.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_MM_PL310		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_MM_PL310,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_MM_PL310)
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_SDMAC			0x7FE3 		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_SDMAC					0x4001		/* Sys DMA connected to Port 2.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_SDMAC		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_SDMAC,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_SDMAC)

/*Com Subsys bus master AXI ID*/
/* SYS EMI (LPDDR2): identical to whether SYS DDR3 */
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_R4				0x7F87		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_R4					0x6003 		/* R4 connected to Port .  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_R4		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_R4,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_R4)
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_MODEM_ETC		0x7FE2 		/* Accept all AXI ID from DSP WCDMA HSDPA HUCM, EDGE... */
#define CHAL_MQOS_CAM_TAG_DEFAULT_MODEM_ETC				0x6000		/*  connected to Port .  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_MODEM_ETC		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_MODEM_ETC,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_MODEM_ETC)

/*Videocore bus master AXI ID*/
/*  15'b{port_id, 00000, 5bit, 3bit} 
// AXI Bus Master ids

// 5 bits for master == 32 masters
// 3 bits for each master to use as needed?  Up to 8 outstanding transactions per master?
// *** DCACHE0 and DCACHE1 masters must only differ by the top bit so that each cache can
// *** construct the other cache's ID.
`define NOONE        5'b00000  // Leave Unused
`define CORE0_V      5'b00001
`define ICACHE0      5'b00010
`define DCACHE0      5'b00011
`define CORE1_V      5'b00100
`define ICACHE1      5'b00101
`define DCACHE1      5'b00110
`define L2_MAIN      5'b00111  // L2 Cache
`define HOST_PORT    5'b01000  // MPHI tx channel
`define HOST_PORT2   5'b01001  // MPHI rx channel
`define HVS          5'b01010
`define ISP          5'b01011
`define VIDEO_DCT    5'b01100  // MPEG4 DCT data
`define VIDEO_SD2AXI 5'b01101
`define CAM0         5'b01110  // Dual lane camera
`define CAM1         5'b01111  // Quad lane camera
`define DMA0         5'b10000  // DMA0 and DMA1 must be adjacent and aligned (DMA0[0]= 0 and DMA0[4:1] = DMA1[4:1])
`define DMA1         5'b10001
`define DMA2         5'b10010  // VPU DMA
`define JPEG         5'b10011
`define VIDEO_CME    5'b10100  // coarse motion estimation, video encoder
`define TRANSPOSER   5'b10101
`define VIDEO_FME    5'b10110
`define CCP2TX       5'b10111  // CCP2 transmitter - host port
`define CAM2         5'b11000  // Quad lane cam_light
// `define USB_ID       5'b11000
`define V3D0         5'b11001  // V3D  - need to be kept together
`define V3D1         5'b11010
`define V3D2         5'b11011
`define IPC          5'b11100
`define DEBUG        5'b11101
`define MMU          5'b11110
`define DVI_ID       5'b11111
*/
/* MM EMI: identical whether Lpddr2 or DDR3 */
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC4			0x6000		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC4					0x0000		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC4		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC4,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC4)
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_L2			0x7FF8		/* Accept all AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC_L2					0x0038		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_L2		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_L2,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC_L2)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CORE0_V		0x7FF8		/* Accept all VC4 core0 vector */
#define CHAL_MQOS_CAM_TAG_DEFAULT_CORE0_V				0x0008 		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CORE0_V	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CORE0_V,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_CORE0_V)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CORE1_V		0x7FF8 		/* Accept all VC4 core1 vector */
#define CHAL_MQOS_CAM_TAG_DEFAULT_CORE1_V				0x0020		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CORE1_V	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CORE1_V,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_CORE1_V)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_HVS			0x7FF8 		/* Accept VC4 HVS  AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_HVS					0x0050		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_HVS		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_HVS,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_HVS)
#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_ISP			0x7FF8 		/* Accept VC4 ISP  AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_ISP					0x0058		/* VC4 ISP.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_ISP		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_ISP,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_ISP)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CAM			0x7FF0 		/* Accept VC4 CAMERA0/1 an dCAM1 AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_CAM					0x0070		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CAM		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CAM,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_CAM)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_DMA			0x7FF0 		/* Accept VC4 DMA0/1 an dCAM1 AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC_DMA				0x0080		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_DMA		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_DMA,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC_DMA)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_DMA2		0x7FF8 		/* Accept VC4 DMA0/1 an dCAM1 AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC_DMA2				0x0090		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_DMA2		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_DMA2,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC_DMA2)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_V3D		0x7FF8 		/* Accept VC4 V3D0, V3D1, V3D2 AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_V3D0				0x00C8		/* VC4 connected to Port 0.  */	
#define CHAL_MQOS_CAM_TAG_DEFAULT_V3D12				0x00D0		/* VC4 connected to Port 0.  */		
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_V3D0		CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_V3D,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_V3D0)

#define CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_VENC		0x7FF8 		/* Accept VC4 DMA0/1 an dCAM1 AXI ID */
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC_CME				0x00A0		/* Video Coarse ME.  */	
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC_FME				0x00B0		/* Video Fine ME.  */
#define CHAL_MQOS_CAM_TAG_DEFAULT_VC_DCT				0x0060		/* Video MPEG4 DCT.  */
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_CME	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_VENC,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC_CME)
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_FME	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_VENC,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC_FME)
#define CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_DCT	CHAL_MQOS_CAM_TAG_CFG_STR(CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_VC_VENC,\
																		CHAL_MQOS_CAM_TAG_DEFAULT_VC_DCT)
#else

#endif


/*-------Type Definitions-----------------------------------------*/			
typedef enum																	
{
	CHAL_MQOS_MEMTYPE_VC4_EMI_SECURE = 0,	/* ddr2 emi */
	CHAL_MQOS_MEMTYPE_VC4_EMI_OPEN, 
	CHAL_MQOS_MEMTYPE_SYS_EMI_SECURE,		/* ddr3 emi */
	CHAL_MQOS_MEMTYPE_SYS_EMI_OPEN,
}CHAL_MQOS_MEMTYPE_e;

typedef enum {
	CHAL_MQOS_CAM_ENTRY_ID_0 = 0,
	CHAL_MQOS_CAM_ENTRY_ID_1,
	CHAL_MQOS_CAM_ENTRY_ID_2,
	CHAL_MQOS_CAM_ENTRY_ID_3,
	CHAL_MQOS_CAM_ENTRY_ID_4,
	CHAL_MQOS_CAM_ENTRY_ID_5,
	CHAL_MQOS_CAM_ENTRY_ID_6,
	CHAL_MQOS_CAM_ENTRY_ID_7,
	CHAL_MQOS_CAM_ENTRY_ID_8,
	CHAL_MQOS_CAM_ENTRY_ID_9,
	CHAL_MQOS_CAM_ENTRY_ID_10,
	CHAL_MQOS_CAM_ENTRY_ID_11,
	CHAL_MQOS_CAM_ENTRY_ID_12,
	CHAL_MQOS_CAM_ENTRY_ID_13,
	CHAL_MQOS_CAM_ENTRY_ID_14,
	CHAL_MQOS_CAM_ENTRY_ID_15,
	CHAL_MQOS_CAM_ENTRY_ID_MAX = 16,
	CHAL_MQOS_CAM_ENTRY_ID_STUFF = 0x7FFFFFFF	/* this makes 32bit datatype*/
} CHAL_MQOS_CAM_ENTRY_ID_e;

/*Default preassignment of CAM for each bus master. */
/*NB: this assignment is random order, and can be changed w/o any HW conflict.*/
typedef enum {
	CHAL_MQOS_CAM_ENTRY_ASSGN_0_DEFAULT = 0,  /* not assignable */
	CHAL_MQOS_CAM_ENTRY_ASSGN_1_VCL2MAIN,	/* VC4 L2_main */
	CHAL_MQOS_CAM_ENTRY_ASSGN_2_SDMAC,	/* sys dmac */
	CHAL_MQOS_CAM_ENTRY_ASSGN_3_VCISP,
	CHAL_MQOS_CAM_ENTRY_ASSGN_4_VCV3D0,
	CHAL_MQOS_CAM_ENTRY_ASSGN_5_A9,
	CHAL_MQOS_CAM_ENTRY_ASSGN_6_R4,
	CHAL_MQOS_CAM_ENTRY_ASSGN_7_MODEM,
	CHAL_MQOS_CAM_ENTRY_ASSGN_8_SS_PL310,  /* sys switch PL310 for MM DDR */
	CHAL_MQOS_CAM_ENTRY_ASSGN_9_SS_DMAC,   /* sys switch and SDMAC */
	CHAL_MQOS_CAM_ENTRY_ASSGN_10_VCHVS,
	CHAL_MQOS_CAM_ENTRY_ASSGN_11_VCCAM01,  /* VC_CAM0 and VC_CAM1 */
	CHAL_MQOS_CAM_ENTRY_ASSGN_12_VCDMA01,
	CHAL_MQOS_CAM_ENTRY_ASSGN_13_VCDMA2,
	CHAL_MQOS_CAM_ENTRY_ASSGN_14_VCCORE0,
	CHAL_MQOS_CAM_ENTRY_ASSGN_15_VCCORE1,
	CHAL_MQOS_CAM_ENTRY_ASSGN_MAX = 16,
	CHAL_MQOS_CAM_ENTRY_ASSGN_STUFF = 0x7FFFFFFF	/* this makes 32bit datatype*/
} CHAL_MQOS_CAM_ENTRY_ID_ASSGN_e;


typedef struct {
	volatile unsigned int CAM_UPDATE;  /*CAM Update Register 0 for Memory Controller module RW 0x0000_0000 32 */
	volatile unsigned int CAM_ENTRY_0; /*CAM ENTRY Register 0 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_1; /*CAM ENTRY Register 1 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_2; /*CAM ENTRY Register 2 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_3; /*CAM ENTRY Register 3 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_4; /*CAM ENTRY Register 4 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_5; /*CAM ENTRY Register 5 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_6; /*CAM ENTRY Register 6 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_7; /*CAM ENTRY Register 7 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_8; /*CAM ENTRY Register 8 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_9; /*CAM ENTRY Register 9 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_10; /*CAM ENTRY Register 10 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_11; /*CAM ENTRY Register 11 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_12; /*CAM ENTRY Register 12 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_13; /*CAM ENTRY Register 13 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_14; /*CAM ENTRY Register 14 RW 0x0C3F_F000 32 */
	volatile unsigned int CAM_ENTRY_15; /*CAM ENTRY Register 15 RW 0x0C3F_F000 32 */
} CHAL_MQOS_CAM_ENTRY_t;

typedef struct {
	volatile unsigned int  CAM_ENABLE_0; /*CAM Enable Register 0 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_1; /*CAM Enable Register 1 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_2; /*CAM Enable Register 2 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_3; /*CAM Enable Register 3 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_4; /*CAM Enable Register 4 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_5; /*CAM Enable Register 5 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_6; /*CAM Enable Register 6 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_7; /*CAM Enable Register 7 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_8; /*CAM Enable Register 8 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_9; /*CAM Enable Register 9 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_10; /*CAM Enable Register 10 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_11; /*CAM Enable Register 11 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_12; /*CAM Enable Register 12 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_13; /*CAM Enable Register 13 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_14; /*CAM Enable Register 14 RW 0x7FFF_0000 32 */
	volatile unsigned int  CAM_ENABLE_15; /*CAM Enable Register 15 RW 0x7FFF_0000 32  */
} CHAL_MQOS_CAM_ENABLE_t;

typedef struct {
	CHAL_MQOS_MEMTYPE_e		  memc_type;
	CHAL_MQOS_CAM_ENTRY_t	* entry_block;
	CHAL_MQOS_CAM_ENABLE_t	* enable_block;
} CHAL_MQOS_t;



/*-----Function Prototypes--------------------------------------------------*/
void chal_mqos_get_handle(CHAL_MQOS_t * QoS_data);

void chal_mqos_update_enable(CHAL_MQOS_t * mqos_data);
void chal_mqos_update_disable(CHAL_MQOS_t * mqos_data);

void chal_mqos_entry_config(
		CHAL_MQOS_t * mqos_data,
		unsigned int entry_cfg_str,
		unsigned int enable_cfg_str,
		CHAL_MQOS_CAM_ENTRY_ID_e id);

void chal_mqos_enable(
		CHAL_MQOS_t * mqos_data,
		unsigned int enable_cfg_str,
		CHAL_MQOS_CAM_ENTRY_ID_e id);

void chal_mqos_stop(CHAL_MQOS_t * mqos_data);

void chal_mqos_print_raw(
			CHAL_MQOS_t * mqos_data,
			int (*fpPrint) (const char *, ...));

#endif /*_MQOS_H_*/

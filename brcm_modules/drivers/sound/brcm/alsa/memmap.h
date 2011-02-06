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
// Description: This include file constants defining the memory and irq map
//				for the MS.
//
// Reference:  	BAG Hardware/Software ICD
//

#ifndef _DEV_MEM_MAP_INC_
#define _DEV_MEM_MAP_INC_

// ------------------------------------------------------------------------
// Defines belows are for naming compatibility with the original codebase
#define PDMACONF REG_DSP_DMA_PROG
#define DDMACONF REG_DSP_DMA_DATA
#define SMICONF0 REG_SMI_CR0
#define SMICONF1 REG_SMI_CR1
#define SMICONF2 REG_SMI_CR2
#define SMICONF3 REG_SMI_CR3
// ------------------------------------------------------------------------

#define SDRAM_BASE  0x80000000 // SDRAM BASE INFO
#define DSP_SH_BASE 0x80000000 // 320k DSP sharedmem
#define DSP_SH_SIZE 0x00050000
#define SHARED_RAM_BASE (DSP_SH_BASE) // for compatibility with existing code


#define	IOCR0_REG			(&(REG_SYS_IOCR0))
#define	IOCR8_REG			(&(REG_SYS_IOCR8))

//DSP registers access via command queue

#define RIPREG_AMCR				0xe540
#define RIPREG_VCOEFR_BASE		0xe5d0
#define RIPREG_FCWR				0xe50f
#define RIPREG_AUDATT			0xe545
#define RIPREG_AUDSCALE_FACTOR	0xe543

#define DSP_AUDIO_REG_AMCR						0xE540
#define DSP_AUDIO_REG_VIPATH_CTRL				0xE541
#define DSP_AUDIO_REG_AUXCOMP					0xE543
#define DSP_AUDIO_REG_AUDIOINPATH_CTRL			0xE544
#define DSP_AUDIO_REG_AUDIOINCM_ADDR			0xE547
#define DSP_AUDIO_REG_AUDIOINCM_DATA			0xE548
#define DSP_AUDIO_REG_AUDIOLOOPBACK_CTRL		0xE549
#define DSP_AUDIO_REG_VCOEFR_0					0xE5D0
#define DSP_AUDIO_REG_VCOEFR_1					0xE5D1
#define DSP_AUDIO_REG_VCOEFR_2					0xE5D2
#define DSP_AUDIO_REG_VCOEFR_3					0xE5D3
#define DSP_AUDIO_REG_VCOEFR_4					0xE5D4
#define DSP_AUDIO_REG_VCOEFR_5					0xE5D5
#define DSP_AUDIO_REG_VCOEFR_6					0xE5D6
#define DSP_AUDIO_REG_VCOEFR_7					0xE5D7
#define DSP_AUDIO_REG_VCOEFR_8					0xE5D8
#define DSP_AUDIO_REG_VCOEFR_9					0xE5D9
#define DSP_AUDIO_REG_VCOEFR_10					0xE5DA
#define DSP_AUDIO_REG_VCOEFR_11					0xE5DB
#define DSP_AUDIO_REG_VCOEFR_12					0xE5DC
#define DSP_AUDIO_REG_VCOEFR_13					0xE5DD
#define DSP_AUDIO_REG_VCOEFR_14					0xE5DE
#define DSP_AUDIO_REG_VCOEFR_15					0xE5DF
#define DSP_AUDIO_REG_VCOEFR_16					0xE5E0
#define DSP_AUDIO_REG_VCOEFR_17					0xE5E1
#define DSP_AUDIO_REG_VCOEFR_18					0xE5E2
#define DSP_AUDIO_REG_VCOEFR_19					0xE5E3
#define DSP_AUDIO_REG_VCOEFR_20					0xE5E4
#define DSP_AUDIO_REG_VCOEFR_21					0xE5E5
#define DSP_AUDIO_REG_VCOEFR_22					0xE5E6
#define DSP_AUDIO_REG_VCOEFR_23					0xE5E7
#define DSP_AUDIO_REG_VCOEFR_24					0xE5E8
#define DSP_AUDIO_REG_VCOEFR_25					0xE5E9
#define DSP_AUDIO_REG_VCOEFR_26					0xE5EA
#define DSP_AUDIO_REG_VCOEFR_27					0xE5EB
#define DSP_AUDIO_REG_VCOEFR_28					0xE5EC
#define DSP_AUDIO_REG_VCOEFR_29					0xE5ED
#define DSP_AUDIO_REG_VCOEFR_30					0xE5EE
#define DSP_AUDIO_REG_VCOEFR_31					0xE5EF
#define DSP_AUDIO_REG_VCOEFR_32					0xE5F0
#define DSP_AUDIO_REG_VCOEFR_33					0xE5F1
#define DSP_AUDIO_REG_VCOEFR_34					0xE5F2
#define DSP_AUDIO_REG_AUDIR2					0xE7A0
#define DSP_AUDIO_REG_AUDIOINFIFO_STATUS		0xE7A1
#define DSP_AUDIO_REG_VCFGR						0xE7A3
#define DSP_AUDIO_REG_AUDIR						0xE7A4
#define DSP_AUDIO_REG_AUDOR						0xE7A5
#define DSP_AUDIO_REG_VMUT						0xE7A8
#define DSP_AUDIO_REG_VAFIFOCTRL				0xE7A9
#define DSP_AUDIO_REG_VSLOPGAIN					0xE7AB
#define DSP_AUDIO_REG_ADCCONTROL				0xE7AC
#define DSP_AUDIO_REG_APCTRK					0xE7AD
#define DSP_AUDIO_REG_AIR						0xE7AE
#define DSP_AUDIO_REG_APRR						0xE7AF
#define DSP_AUDIO_REG_POLYAUDMOD				0xE7B0
#define DSP_AUDIO_REG_PFIFOCTRL					0xE7B1
#define DSP_AUDIO_REG_PIFIFOST					0xE7B2
#define DSP_AUDIO_REG_PLRCH						0xE7B3
#define DSP_AUDIO_REG_PEQPATHOFST0				0xE7B4
#define DSP_AUDIO_REG_PEQPATHOFST1				0xE7B5
#define DSP_AUDIO_REG_PEQPATHOFST2				0xE7B6
#define DSP_AUDIO_REG_PEQPATHGAIN5				0xE7B7
#define DSP_AUDIO_REG_PLSLOPGAIN				0xE7B8
#define DSP_AUDIO_REG_PRSLOPGAIN				0xE7B9
#define DSP_AUDIO_REG_VIBRA_EN					0xE7BA
#define DSP_AUDIO_REG_VIBRA_DUPDATA				0xE7BB
#define DSP_AUDIO_REG_PEQPATHGAIN1				0xE7BC
#define DSP_AUDIO_REG_PEQPATHGAIN2				0xE7BD
#define DSP_AUDIO_REG_PEQPATHGAIN3				0xE7BE
#define DSP_AUDIO_REG_PEQPATHGAIN4				0xE7BF
#define DSP_AUDIO_REG_PIFIFODATA0				0xE7C1
#define DSP_AUDIO_REG_PIFIFODATA1				0xE7C2
#define DSP_AUDIO_REG_PIFIFODATA2				0xE7C3
#define DSP_AUDIO_REG_PEQCOFADD					0xE7C4
#define DSP_AUDIO_REG_PEQCOFDATA				0xE7C5

#define DSP_AUDIO_REG_PCOMPFIRCOFADD			0xE7C6
#define DSP_AUDIO_REG_PCOMPFIRCOFDATA			0xE7C7
#define DSP_AUDIO_REG_PCOMPIIRCOFADD			0xE7C8
#define DSP_AUDIO_REG_PCOMPIIRCOFDATA			0xE7C9
#define DSP_AUDIO_REG_VIBRA_DGAINMODE			0xE7CA
#define DSP_AUDIO_REG_VIBRA_FIFOMODE			0xE7CB
#define DSP_AUDIO_REG_PEQEVT					0xE7CC
#define DSP_AUDIO_REG_MPMBIQUAD_COEF_ADDR		0xE7CD
#define DSP_AUDIO_REG_MPMBIQUAD_COEF_WDATAH		0xE7CE
#define DSP_AUDIO_REG_MPMBIQUAD_COEF_WDATAL		0xE7CF
#define DSP_AUDIO_REG_STEREOAUDMOD				0xE7D0
#define DSP_AUDIO_REG_AFIFOCTRL					0xE7D1
#define DSP_AUDIO_REG_AIFIFOST					0xE7D2
#define DSP_AUDIO_REG_ALRCH						0xE7D3
#define DSP_AUDIO_REG_AEQPATHOFST0				0xE7D4
#define DSP_AUDIO_REG_AEQPATHOFST1				0xE7D5
#define DSP_AUDIO_REG_AEQPATHOFST2				0xE7D6
#define DSP_AUDIO_REG_AEQEVT					0xE7D7
#define DSP_AUDIO_REG_ALSLOPGAIN				0xE7D8
#define DSP_AUDIO_REG_ARSLOPGAIN				0xE7D9
#define DSP_AUDIO_REG_APVLMXGAIN				0xE7DA
#define DSP_AUDIO_REG_APVRMXGAIN				0xE7DB
#define DSP_AUDIO_REG_BTMIXER_CFG				0xE7DC
#define DSP_AUDIO_REG_BTMIXER_CFG2				0xE7DD
#define DSP_AUDIO_REG_BTMIXER_GAIN_L			0xE7DE
#define DSP_AUDIO_REG_BTMIXER_GAIN_R			0xE7DF
#define DSP_AUDIO_REG_AEQPATHGAIN1				0xE7E1
#define DSP_AUDIO_REG_AEQPATHGAIN2				0xE7E2
#define DSP_AUDIO_REG_AEQPATHGAIN3				0xE7E3
#define DSP_AUDIO_REG_AEQPATHGAIN4				0xE7E4
#define DSP_AUDIO_REG_AEQPATHGAIN5				0xE7E5
#define DSP_AUDIO_REG_LSDMSEEDH					0xE7E7
#define DSP_AUDIO_REG_LSDMSEEDL					0xE7E8
#define DSP_AUDIO_REG_LSDMPOLYH					0xE7E9
#define DSP_AUDIO_REG_LSDMPOLYL					0xE7EA
#define DSP_AUDIO_REG_RSDMSEEDH					0xE7EB
#define DSP_AUDIO_REG_RSDMSEEDL					0xE7EC
#define DSP_AUDIO_REG_RSDMPOLYH					0xE7ED
#define DSP_AUDIO_REG_RSDMPOLYL					0xE7EE
#define DSP_AUDIO_REG_SDMDTHER					0xE7EF
#define DSP_AUDIO_REG_AUDVOC_ISR				0xE7F0
#define DSP_AUDIO_REG_AIFIFODATA0				0xE7F1
#define DSP_AUDIO_REG_AIFIFODATA1				0xE7F2
#define DSP_AUDIO_REG_AIFIFODATA2				0xE7F3
#define DSP_AUDIO_REG_AEQCOFADDR				0xE7F4
#define DSP_AUDIO_REG_AEQCOFDATA				0xE7F5
#define DSP_AUDIO_REG_ACOMPFIRCOFADD			0xE7F6
#define DSP_AUDIO_REG_ACOMPFIRCOFDATA			0xE7F7
#define DSP_AUDIO_REG_ACOMPIIRCOFADD			0xE7F8
#define DSP_AUDIO_REG_ACOMPIIRCOFDATA			0xE7F9

#endif //#ifndef _DEV_MEM_MAP_INC_


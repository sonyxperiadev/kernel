/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   chip_irq.h
*
*   @brief  This file contains the chip related IRQ defines.
*
****************************************************************************/
/**
*
* @defgroup IRQAPIGroup Interrupt Controller
*
* @brief This group defines the APIs for the interrupt controller
*
* @ingroup CSLGroup
*****************************************************************************/

#ifndef _CHIP_IRQ_H_
#define _CHIP_IRQ_H_
/**
Interrupt source number. The max number depends on the chip.
**/

#define NUM_KONAIRQs          224
#define LAST_KONAIRQ          (NUM_KONAIRQs-1)

#define FIRST_BMIRQ           (LAST_KONAIRQ+1)
#define NUM_BMIRQs            56
#define FIRST_BMIRQ           (LAST_KONAIRQ+1)
#define IRQ_TO_BMIRQ(irq)         ((irq)-BMIRQ0)
#define IRQ_TO_BMREG_INDEX(irq)   (IRQ_TO_BMIRQ(irq)/32)
#define IRQ_TO_BMREG_MASK(irq)    (1<<(IRQ_TO_BMIRQ(irq)%32))

#define IS_KONA_IRQ(irq) ((irq) <= LAST_KONAIRQ)
#define IRQ_TO_KONAIRQ(irq)       ((irq)-IRQ32)
#define IRQ_TO_KONAREG_INDEX(irq) (IRQ_TO_KONAIRQ(irq)/32)
#define IRQ_TO_KONAREG_MASK(irq)  (1<<(IRQ_TO_KONAIRQ(irq)%32))

#if defined(FUSE_APPS_PROCESSOR)
#define NUM_IRQs              (NUM_KONAIRQs)
#define	TOTAL_NUM_IRQs        (NUM_IRQs)
#else
#define NUM_IRQs              (NUM_BMIRQs+NUM_KONAIRQs)
#define	TOTAL_NUM_IRQs        (NUM_IRQs+NUM_WIRQs)
#endif

typedef enum {
	IRQ0, IRQ1, IRQ2, IRQ3, IRQ4, IRQ5, IRQ6, IRQ7, IRQ8, IRQ9,
	IRQ10, IRQ11, IRQ12, IRQ13, IRQ14, IRQ15, IRQ16, IRQ17, IRQ18, IRQ19,
	IRQ20, IRQ21, IRQ22, IRQ23, IRQ24, IRQ25, IRQ26, IRQ27, IRQ28, IRQ29,
	IRQ30, IRQ31, IRQ32, IRQ33, IRQ34, IRQ35, IRQ36, IRQ37, IRQ38, IRQ39,
	IRQ40, IRQ41, IRQ42, IRQ43, IRQ44, IRQ45, IRQ46, IRQ47, IRQ48, IRQ49,
	IRQ50, IRQ51, IRQ52, IRQ53, IRQ54, IRQ55, IRQ56, IRQ57, IRQ58, IRQ59,
	IRQ60, IRQ61, IRQ62, IRQ63, IRQ64, IRQ65, IRQ66, IRQ67, IRQ68, IRQ69,
	IRQ70, IRQ71, IRQ72, IRQ73, IRQ74, IRQ75, IRQ76, IRQ77, IRQ78, IRQ79,
	IRQ80, IRQ81, IRQ82, IRQ83, IRQ84, IRQ85, IRQ86, IRQ87, IRQ88, IRQ89,
	IRQ90, IRQ91, IRQ92, IRQ93, IRQ94, IRQ95, IRQ96, IRQ97, IRQ98, IRQ99,
	IRQ100, IRQ101, IRQ102, IRQ103, IRQ104, IRQ105, IRQ106, IRQ107, IRQ108,
	    IRQ109,
	IRQ110, IRQ111, IRQ112, IRQ113, IRQ114, IRQ115, IRQ116, IRQ117, IRQ118,
	    IRQ119,
	IRQ120, IRQ121, IRQ122, IRQ123, IRQ124, IRQ125, IRQ126, IRQ127, IRQ128,
	    IRQ129,
	IRQ130, IRQ131, IRQ132, IRQ133, IRQ134, IRQ135, IRQ136, IRQ137, IRQ138,
	    IRQ139,
	IRQ140, IRQ141, IRQ142, IRQ143, IRQ144, IRQ145, IRQ146, IRQ147, IRQ148,
	    IRQ149,
	IRQ150, IRQ151, IRQ152, IRQ153, IRQ154, IRQ155, IRQ156, IRQ157, IRQ158,
	    IRQ159,
	IRQ160, IRQ161, IRQ162, IRQ163, IRQ164, IRQ165, IRQ166, IRQ167, IRQ168,
	    IRQ169,
	IRQ170, IRQ171, IRQ172, IRQ173, IRQ174, IRQ175, IRQ176, IRQ177, IRQ178,
	    IRQ179,
	IRQ180, IRQ181, IRQ182, IRQ183, IRQ184, IRQ185, IRQ186, IRQ187, IRQ188,
	    IRQ189,
	IRQ190, IRQ191
	    /* #if removed as Hera & Rhea both have 192 shared Peripheral interrupts
	       starting from IRQ32 */
	    , IRQ192, IRQ193, IRQ194, IRQ195, IRQ196, IRQ197, IRQ198, IRQ199,
	IRQ200, IRQ201, IRQ202, IRQ203, IRQ204, IRQ205, IRQ206, IRQ207, IRQ208,
	    IRQ209,
	IRQ210, IRQ211, IRQ212, IRQ213, IRQ214, IRQ215, IRQ216, IRQ217, IRQ218,
	    IRQ219,
	IRQ220, IRQ221, IRQ222, IRQ223
	    /* BMODEM interrupts */
	, BMIRQ0 =
	    FIRST_BMIRQ, BMIRQ1, BMIRQ2, BMIRQ3, BMIRQ4, BMIRQ5, BMIRQ6,
	    BMIRQ7, BMIRQ8, BMIRQ9, BMIRQ10, BMIRQ11, BMIRQ12, BMIRQ13,
	    BMIRQ14, BMIRQ15, BMIRQ16, BMIRQ17, BMIRQ18, BMIRQ19, BMIRQ20,
	    BMIRQ21, BMIRQ22, BMIRQ23, BMIRQ24, BMIRQ25, BMIRQ26, BMIRQ27,
	    BMIRQ28, BMIRQ29, BMIRQ30, BMIRQ31, BMIRQ32, BMIRQ33, BMIRQ34,
	    BMIRQ35, BMIRQ36, BMIRQ37, BMIRQ38, BMIRQ39, BMIRQ40, BMIRQ41,
	    BMIRQ42, BMIRQ43, BMIRQ44, BMIRQ45, BMIRQ46, BMIRQ47, BMIRQ48,
	    BMIRQ49, BMIRQ50, BMIRQ51, BMIRQ52, BMIRQ53, BMIRQ54, BMIRQ55
} InterruptId_t;

/* aliases for modem interrupts coming from CHIPREGS */
#define DMAC_INT              BMIRQ0
#define BSIM_IRQ              BMIRQ1	/* matches name in modem baseline */
#define BSIM2_IRQ             BMIRQ2
#define HSIRX_INT             BMIRQ3
#define HSITX_INT             BMIRQ4
#define FHOST_INT             BMIRQ5
#define FHOST_WUP_INT         BMIRQ6
#define AUDIOH_NORM_INT       BMIRQ7
#define AUDIOH_ERR_INT        BMIRQ8
#define CHIPREGS_9_TBD        BMIRQ9
#define CHIPREGS_10_TBD       BMIRQ10
#define CHIPREGS_11_IPC       BMIRQ11
#define CHIPREGS_12_TBD       BMIRQ12
#define CHIPREGS_13_TBD       BMIRQ13
#define CHIPREGS_14_TBD       BMIRQ14
#define BM_KONA_IRQ           BMIRQ15	/* controlled by CHIPREGS */

#if defined(FUSE_APPS_PROCESSOR)
typedef struct {
	UInt32 mask[(NUM_IRQs + 31) / 32];
} IRQMask_t;
#define RIP_IRQ 0
#else // #if defined(FUSE_APPS_PROCESSOR)
typedef struct {
	UInt32 mask[(NUM_KONAIRQs + 31) / 32];
} KonaIRQMask_t;

typedef struct {
	UInt32 mask[(NUM_BMIRQs + 31) / 32];
} BMIRQMask_t;

typedef BMIRQMask_t IRQMask_t;

/* aliases for modem interrupts coming from DSP */
#define DSP_A5_INT            BMIRQ16
#define DSP_EQ_INT            BMIRQ17
#define DSP_BR_INT            BMIRQ18
#define DSP_BT_INT            BMIRQ19
#define DSP_BTEN_INT          BMIRQ20
#define DSP_SMC_INT           BMIRQ21
#define DSP_OTOAFINT          BMIRQ22
#define DSP_OTOAINT           BMIRQ23
#define DSP_OTOACFINT         BMIRQ24
#define DSP_OTOCINT           BMIRQ25
#define SMC_INT_IRQ           DSP_SMC_INT

#define   RIP_IRQ			  DSP_OTOCINT
#define   FIQ                 MP_DONE_INT

/* aliases for modem interrupts coming from WCDMA core */
#define WCDMA_nIRQ            BMIRQ28
#define WCDMA_nFIQ            BMIRQ29
#define WCDMA_L2_IRQ          BMIRQ30	/* matches name in modem baseline */

/* aliases for modem interrupts coming from within the modem subsystem */
#define GEA3_IRQ              BMIRQ31	/* matches name in modem baseline */
#define MP_DONE_INT           BMIRQ32
#define CIPHER_IRQ            BMIRQ33	/* matches name in modem baseline */
#define PACKER_IRQ            BMIRQ34	/* matches name in modem baseline */
#define HUCM_IRQ              BMIRQ35	/* matches name in modem baseline */
#define WDTIMER_INT           BMIRQ36
#define GPTIMER_INT0          BMIRQ37
#define GPTIMER_INT1          BMIRQ38
#define SLPTIMER_INT          BMIRQ39
#define BM_SleepModeTimer_IRQ SLPTIMER_INT	/* alias for RTOS/osdriver */
#define INT_TWIF_SWITCH0      BMIRQ40
#define INT_TWIF_SWITCH1      BMIRQ41
#define INT_RFSPI_SWITCH0     BMIRQ42
#define INT_RFSPI_SWITCH1     BMIRQ43
#define CTIIRQ                BMIRQ44
#define BMDM_CCU_ACT_INT      BMIRQ45
#define BMDM_CCU_TGT_INT      BMIRQ46
#define DSP_CCU_ACT_INT       BMIRQ47
#define DSP_CCU_TGT_INT       BMIRQ48

/* aliases for "spare" interrupts */
#define IPC_WCDMA0            BMIRQ52
#define IPC_WCDMA1            BMIRQ53
#define IPC_DSP0              BMIRQ54
#define IPC_DSP1              BMIRQ55

#endif /* FUSE_APPS_PROCESSOR */
#define   DSP2AP_IRQ		  BMIRQ22

#define AP_RIP_IRQ            BMIRQ53	//using a reserved source for AP RIP
#define CP_RIP_IRQ            BMIRQ54	//using a reserved source for CP RIP

#ifdef FUSE_DUAL_PROCESSOR_ARCHITECTURE
#define IPC_C2A_SOFTINT       COMMS_SUBS7_IRQ
#define IPC_C2A_SOFTINT_BINTC BMIRQ55
#define IPC_A2C_SOFTINT_BINTC BMIRQ11

#define IPC_A2C_SOFTINT IPC_A2C_SOFTINT_BINTC
#endif

//Re-map of IRQ numbers for backward compatibilities. Please remap your own meaningful names
//in module include file.
//--- IRQ map-- //HERA
// Software Generated Interrupts (SGI)
#define		SGI_ID0					IRQ0
#define		SGI_ID1					IRQ1
#define		SGI_ID2					IRQ2
#define		SGI_ID3					IRQ3
#define		SGI_ID4					IRQ4
#define		SGI_ID5					IRQ5
#define		SGI_ID6					IRQ6
#define		SGI_ID7					IRQ7
#define		SGI_ID8					IRQ8
#define		SGI_ID9					IRQ9
#define		SGI_ID10				IRQ10
#define		SGI_ID11				IRQ11
#define		SGI_ID12				IRQ12
#define		SGI_ID13				IRQ13
#define		SGI_ID14				IRQ14
#define		SGI_ID15				IRQ15
#define		GLOBAL_TIMER_IRQ		IRQ27
#define		nFIQ_IRQ				IRQ28
#define		PRIVATE_TIMER_IRQ		IRQ29
#define		WDT_IRQ					IRQ30
#define		nIRQ_IRQ				IRQ31

// Shared Peripheral Interrupts (SPI)
#define		L2CCINTR_IRQ			IRQ32
#define		HUB_CTI_IRQ				IRQ36
#define		SECURE_TRAP8_IRQ		IRQ37
#define		HUB_TIMER2_IRQ			IRQ38
#define		HUB_TIMER1_IRQ			IRQ39
#define		PERIPH_TIMER2_IRQ		IRQ40
#define		PERIPH_TIMER1_IRQ		IRQ41
#define		DMAC_ABORT_IRQ			IRQ45
#define		I2C_PMU_IRQ				IRQ46
#define		PWRMGR_IRQ				IRQ47
#define		FABRIC_CTI_IRQ			IRQ50
#define		SECURE_TRAP1_IRQ		IRQ51
#define		SECURE_TRAP2_IRQ		IRQ52
#define		SECURE_TRAP3_IRQ		IRQ53
#define		SECURE_TRAP4_IRQ		IRQ54
#define		SECURE_TRAP7_IRQ		IRQ55
#define		SECURE_TRAP5_IRQ		IRQ56
#define		SECURE_SPUM_IRQ			IRQ58
#define		OPEN_SPUM_IRQ			IRQ59
#if defined(FUSE_APPS_PROCESSOR)
#define		SIM2_IRQ				IRQ60
#else
/* CP has its own definition */
#define		SIM2_IRQ				BSIM2_IRQ
#endif
#define		COMMTX0_IRQ				IRQ61
#define		COMMRX0_IRQ				IRQ63
#define		PMUIRQ0_IRQ				IRQ65
#define		CTIIRQ0_IRQ				IRQ67
#define		DMAC_MUXING_SECURE_IRQ	IRQ69
#define		DMAC_MUXING_OPEN_IRQ	IRQ70
#define		FMON_LOW_IRQ			IRQ71
#define		FMON_HIGH_IRQ			IRQ72
#define		UTMI_BVALID_IRQ			IRQ73
#define		UTMI_DRWBUS_IRQ			IRQ74
#define		UTMI_CHRGVBUS_IRQ		IRQ75
#define		UTMI_DISCHRGVBUS_IRQ	IRQ76
#define		OTG_ENDP_IRQ			IRQ77
#define		FSHOST_IRQ				IRQ78
#define		FSHOST_IRQ				IRQ78
#define		USB_MEGA_IRQ			FSHOST_IRQ
#define		USB_OTG_IRQ				IRQ79
#define		USB_IRQ					USB_OTG_IRQ
#define		HUB_TIMER4_IRQ			IRQ82
#define		HUB_TIMER3_IRQ			IRQ83
#define		PERIPH_TIMER4_IRQ		IRQ84
#define		PERIPH_TIMER3_IRQ		IRQ85
//#define               ROOT_CCU_IRQ                    IRQ88
#define		UARTB_B_IRQ				IRQ89
#define		SYS_EMI_SECURE_IRQ		IRQ92
#define		SYS_EMI_OPEN_IRQ		IRQ93
#define		UARTC_IRQ				IRQ97
#define		UARTB_IRQ				IRQ98
#define		UARTA_IRQ				IRQ99
#if defined(FUSE_APPS_PROCESSOR)
#define		SIM_IRQ					IRQ100
#else
#define		SIM_IRQ					BSIM_IRQ
#endif
#define		UARTB2_B_IRQ			IRQ101
#define		AON_ACT_INT_IRQ			IRQ103
#define		AON_TGT_INT_IRQ			IRQ104
//#define       SDIO3_IRQ                               IRQ106
//#define       SDIO2_IRQ                               IRQ108
//#define       SDIO1_IRQ                               IRQ109
#define		SDIO2_IRQ				IRQ106
#define		SDIO1_IRQ				IRQ108
#define		SDIO0_IRQ				IRQ109
#define		FAB_ACT_INT_IRQ			IRQ111
#define		SLIM_IRQ				IRQ112
#define		FAB_TGT_INT_IRQ			IRQ114
#define     SYS_EMI_PWRWDOG_IRQ     IRQ105
#define		DEFLAGS0_IRQ			IRQ116
#define		ARM_ACT_INT_IRQ			IRQ119
#define		ARM_TGT_INT_IRQ			IRQ120
#define		SSP1_IRQ				IRQ121
#define		SSP0_IRQ				IRQ122
#define		SEC_WD_TIMER_IRQ		IRQ123
#define		SEC_WD_UTC_IRQ			IRQ124
#define		SEC_WD_CHK_IRQ			IRQ125
#define		HSM_PKA_IRQ				IRQ130
#define		HSM_RNG_IRQ				IRQ131
#define		HSM6_IRQ				IRQ133
#define		I2C2_IRQ				IRQ134
#define		I2C_IRQ					IRQ135
#define		UARTB3_B_IRQ			IRQ137
#define		GPIO1_IRQ				IRQ138
#define		PER_ACT_INT_IRQ			IRQ139
#define		PER_TGT_INT_IRQ			IRQ140
#define		GPIO8_IRQ				IRQ141
#define		GPIO7_IRQ				IRQ142
#define		GPIO6_IRQ				IRQ143
#define		GPIO5_IRQ				IRQ144
#define		GPIO4_IRQ				IRQ145
#define		GPIO3_IRQ				IRQ146
#define		GPIO2_IRQ				IRQ147
#define		HSM7_IRQ				IRQ148
#define		HSM8_IRQ				IRQ149
#define		HSM9_IRQ				IRQ150
#define     HUB_ACT_INT_IRQ         IRQ151
#define		ACI_IRQ					IRQ152
#define		IHF1_IRQ				IRQ153
#define		SSP2_IRQ				IRQ154
#define		SIDETONECTRL_IRQ		IRQ155

//In Rhea Audio FPGA verification, IRQ156 is used for CAPH.
//In the final product, this may be changed.
/*#define		HEADSET_IRQ				IRQ156*/
#define     CAPH_NORM_IRQ           IRQ156
#define     HUB_TGT_INT_IRQ         IRQ157
#define		KEYPAD_IRQ				IRQ159
//#define               ANALOG_MIC_IRQ                  IRQ160
#define     HSOTG_WAKEUP_IRQ        IRQ160
#define     FSOTG_WAKEUP_IRQ        IRQ161
//#define               DIGITAL_MIC_IRQ                 IRQ161
//#define               VIBRA_IRQ                               IRQ162
#define     ACI_WAKEUP_IRQ          IRQ162
//#define               IHF0_IRQ                                IRQ163
//#define               EAR_PIECE_IRQ                   IRQ164
#define     AUXMIC_COMP1_IRQ        IRQ163
#define     AUXMIC_COMP2_IRQ        IRQ164
#define     HSI_TX_IRQ              IRQ165
#define		MM_ACT_INT_IRQ			IRQ176
#define		MM_TGT_INT_IRQ			IRQ177
#define		MM_SUBS2_IRQ			IRQ178
#define		MM_VCE_IRQ				IRQ179
#define		MM_V3D_IRQ				IRQ180
#define		MM_SPI_IRQ				IRQ181
#define		MM_SMI_IRQ				IRQ182
#define		MM_DMA_CHAN1_IRQ		IRQ183
#define		MM_ISP_EXP_IRQ			IRQ184
#define		MM_ISP_IRQ				IRQ185
#define		MM_DSI0_IRQ				IRQ186
#define		MM_DSI1_IRQ				IRQ187
#define		MM_CSI_IRQ				IRQ188
#define		MM_SUBS13_IRQ			IRQ189
#define		MM_DMA_CHAN2_IRQ		IRQ190
#define		MM_DMA_CHAN3_IRQ		IRQ191

/* FIXME!! */
#define ISP_MEMORY_INTF_IRQ MM_ISP_EXP_IRQ

#define		COMMS_SUBS0_IRQ			IRQ192
#define		COMMS_SUBS1_IRQ			IRQ193
#define		COMMS_SUBS2_IRQ			IRQ194
#define		COMMS_SUBS3_IRQ			IRQ195
#define		COMMS_SUBS4_IRQ			IRQ196
#define		COMMS_SUBS5_IRQ			IRQ197
#define		COMMS_SUBS6_IRQ			IRQ198
#define		COMMS_SUBS7_IRQ			IRQ199

// In Rhea Audio FPGA verification, IRQ200 is used by CAPH.
// In the final product, this may be changed.
/*#define		COMMS_SUBS8_IRQ			IRQ200*/
#define     CAPH_ERR_IRQ            IRQ200

#define		COMMS_SUBS9_IRQ			IRQ201
#define		COMMS_SUBS10_IRQ		IRQ202
#define		COMMS_SUBS11_IRQ		IRQ203
//#define               COMMS_SUBS12_IRQ                IRQ204
#define     FIXED_SPM_IRQ           IRQ204
//#define               COMMS_SUBS13_IRQ                IRQ205
#define     VAR_SPM_IRQ             IRQ204
#define		COMMS_ACT_INT_IRQ		IRQ206
#define		COMMS_TGT_INT_IRQ		IRQ207
#define		DSP_SUBS0_IRQ			IRQ208
#define		DSP_SUBS1_IRQ			IRQ209
#define		DSP_SUBS2_IRQ			IRQ210
#define		DSP_SUBS3_IRQ			IRQ211
#define		DSP_SUBS4_IRQ			IRQ212
#define		DSP_SUBS5_IRQ			IRQ213
#define		DSP_SUBS6_IRQ			IRQ214
#define		DSP_SUBS7_IRQ			IRQ215
#define		DMAC0_IRQ				IRQ216
#define		DMAC1_IRQ				IRQ217
#define		DMAC2_IRQ				IRQ218
#define		DMAC3_IRQ				IRQ219
#define		DMAC4_IRQ				IRQ220
#define		DMAC5_IRQ				IRQ221
#define		DMAC6_IRQ				IRQ222
#define		DMAC7_IRQ				IRQ223

#define		USBOTG_WAKE_IRQ			HSOTG_WAKEUP_IRQ

//HERA to be remove 
#define		GPIO_IRQ				IRQ6

#endif /* _CHIP_IRQ_H_ */

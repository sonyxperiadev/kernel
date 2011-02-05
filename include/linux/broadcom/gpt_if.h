/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/timer.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  timer.h
*
*  PURPOSE:
*
*
*
*  NOTES:
*
*****************************************************************************/
	
#ifndef _GPT_IF_INC_
#define _GPT_IF_INC_
	
	//******************************************************************************
	//							Include block
	//******************************************************************************
	
	// virtual IRQ number for GP timer
#define IRQ_GPT_TIMER0        128 //timer0 is used as system timer. Not available to driver
#define IRQ_GPT_TIMER1        129
#define IRQ_GPT_TIMER2        130
#define IRQ_GPT_TIMER3        131
#define IRQ_GPT_TIMER4        132
#define IRQ_GPT_TIMER5        133
										
#define GPT_CLOCK_32KHZ       0
#define	GPT_CLOCK_26MHZ       1
#define GPT_ISR_GPT0_INT_FLAG           1
#define GPT_ISR_GPT1_INT_FLAG           1<<1
#define GPT_ISR_GPT2_INT_FLAG           1<<2
#define GPT_ISR_GPT3_INT_FLAG           1<<3
#define GPT_ISR_GPT4_INT_FLAG           1<<4
#define GPT_ISR_GPT5_INT_FLAG           1<<5
#define GPT_ISR_GPT0_INT_ASSIGN         1<<16
#define GPT_ISR_GPT1_INT_ASSIGN         1<<17
#define GPT_ISR_GPT2_INT_ASSIGN         1<<18
#define GPT_ISR_GPT3_INT_ASSIGN         1<<19
#define GPT_ISR_GPT4_INT_ASSIGN         1<<20
#define GPT_ISR_GPT5_INT_ASSIGN         1<<21
	//CSR Reg
#define GPT_CSR_TIMER_ENABLE             1<<31
#define GPT_CSR_CLKSEL                  1<<30
#define GPT_CSR_SW_SLPMODE               1<<29
#define GPT_CSR_HW_SLEEP_MODE_DIS        1<<28
#define GPT_CSR_INT_ENABLE               1<<27
#define GPT_CSR_INT2_ASSIGN              1<<26
#define GPT_CSR_CLKSEL1                  1<<25
#define GPT_CSR_TIMER_PWRON              1<<24
#define GPT_CSR_SW_PEDEMODE              1<<23
#define GPT_CSR_HW_PEDEMODE_DIS          1<<22
#define GPT_CSR_PEDESTAL_STATE           1<<9
#define GPT_CSR_SLEEP_STATE              1<<8
#define GPT_CSR_INT_FLG                  1
	
	typedef struct _GPT_Register {
		volatile uint32_t gpt_isr;
		volatile uint32_t gpt_csr;
		volatile uint32_t gpt_rr;
		volatile uint32_t gpt_vr;
	} GPT_Register_t;
	
	typedef struct _GPT_REG_T {
		GPT_Register_t gpt_reg[6];
	}GPT_REG_T;
	
	typedef void (*GPT_CB_FUNC)(void);
	typedef struct _GPT_CONFIG_ {
		unsigned int reload_value;
		unsigned int no_repeat;
		unsigned int clock;
		GPT_CB_FUNC callback;
		unsigned int int_group;
		unsigned int sw_sleep_mode;
		unsigned int sw_pedemode;
	} GPT_Config_t;
	
	
	//******************************************************************************
	//
	// Function Name:	GPT_Timer_Update
	//
	// Description: 	Update timer reload value for an intialized timer
	//
	// Return;			0-failed; 1-OK
	// Notes:
	//
	//******************************************************************************
	int GPT_Timer_Update(unsigned int timer, unsigned int val);
	
	//******************************************************************************
	//
	// Function Name:	GPT_Timer_deinit
	//
	// Description: 	Individual timer init
	// Input:			timer: 0-5
	// Return;			
	// Notes:
	//
	//******************************************************************************	
	void GPT_Timer_deinit(unsigned int timer);
	
	//******************************************************************************
	//
	// Function Name:	GPT_Timer_init
	//
	// Description: 	Individual timer init
	// Input:			timer: 0-5; pConfig:  
	// Return;			0-failed; 1-OK
	// Notes:
	//
	//******************************************************************************
	int GPT_Timer_init(unsigned int timer, GPT_Config_t *pConfig);
#endif // _GPT_IF_INC_


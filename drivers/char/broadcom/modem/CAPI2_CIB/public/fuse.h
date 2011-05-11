/*****************************************************************************
*
*    (c) 2007 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/

/* Function Prototypes */
void FUSE_platform_initialize(void *);
// !< --- PreProcess --- >!
#define INT_GSM						0x00000080              // BIT 39
#define INT_L2_SOFT4				0x00000040              // BIT 38
#define INT_L2_SOFT3				0x00000020              // BIT 37
#define INT_GP_DAC					0x00000010              // BIT 36
#define INT_CTL3_LOGIC				0x00000008              // BIT 35
#define INT_CL1						0x00000004              // BIT 34
#define INT_EXT1					0x00000002              // BIT 33
#define INT_RX_POWER_DOWN			0x00000001	              // BIT 32
#define INT_HTDM					0x80000000              // BIT 31
#define INT_TWIF					0x40000000              // BIT 30
#define INT_HSDPA_SCCH_SLOT23		0x20000000          // BIT 29
#define INT_HSDPA_SCCH_SLOT1		0x10000000          // BIT 28
#define INT_HSDPA_HARQ				0x08000000              // BIT 27
#define INT_RX_CMB30				0x04000000              // BIT 26
#define INT_RX_COMB_TPC				0x02000000              // BIT 25
#define INT_RX_COMB2			 	0x01000000              // BIT 24
#define INT_RX_COMB1			 	0x00800000              // BIT 23
#define INT_RX_COMB0			 	0x00400000              // BIT 22
#define INT_SEARCHER_SLOT_STRT		0x00200000        // BIT 21
#define INT_RF_LOAD_CMND			0x00100000              // BIT 20
#define INT_TX_PWR_CTRL				0x00080000              // BIT 19
#define INT_TX_SLOT_STRT			0x00040000              // BIT 18
#define INT_TX_TRCH_FIN				0x00020000              // BIT 17
#define INT_RX_SLOT_STRT			0x00010000              // BIT 16
#define INT_RX_TRCH_FIN				0x00008000              // BIT 15
#define INT_SLEEP_END				0x00004000              // BIT 14
#define INT_M_TIMER_GP0				0x00002000              // BIT 13
#define INT_M_TIMER_GP1				0x00001000              // BIT 12
#define INT_HSUPA_DL				INT_M_TIMER_GP1	  // BIT 12
#define INT_GPIO_LATCH_MT			0x00000800              // BIT 11
#define INT_TX_HSDPA				0x00000400              // BIT 10
#define INT_UART0					0x00000200              // BIT 9
#define INT_USB_GSM					0x00000100              // BIT 8
#define INT_TIMER					0x00000080              // BIT 7
#define INT_HTLASTACCESSIRQ			0x00000040          // BIT 6
#define INT_HTFIRSTACCESSIRQ		0x00000020          	// BIT 5
#define INT_L2_WAKEUP				0x00000010          // BIT 4
#define INT_L2_SOFT1				0x00000008          // BIT 3
#define INT_L2_SOFT0				0x00000004          // BIT 2
#define INT_SW1						0x00000002	       // BIT 1
#define INT_SW0						0x00000001              // BIT 0


void EnableASICDeepSleepHW(void);
void DisableASICDeepSleepHW(void);

#ifdef IPCInterface_h
void CommsIpcPowerSavingInitialise(IPC_PlatformSpecificPowerSavingInfo_T *);
#endif



// !< --- PreProcess --- >!


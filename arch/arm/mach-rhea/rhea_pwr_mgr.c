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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <mach/io_map.h>
#include <linux/io.h>
#include <linux/module.h>
#include <mach/irqs.h>
#include<mach/clock.h>
#include<plat/pi_mgr.h>
#include<mach/pi_mgr.h>
#include<mach/pwr_mgr.h>
#include <mach/memory.h>
#include<plat/pwr_mgr.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_bmdm_pwrmgr.h>
#include <plat/kona_cpufreq_drv.h>
#include <plat/kona_reset_reason.h>
#ifdef CONFIG_DEBUG_FS
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#endif
#include "pm_params.h"
#ifdef CONFIG_RHEA_AVS
#include <mach/rhea_avs.h>
#endif

#ifdef CONFIG_DEBUG_FS
/*GPIO0-15 debug bus select values*/
#define DBG_BUS_PM_DBG_BUS_SEL		0x2
#define DBG_BUS_BMDB_DBG_BUS_SEL	0xD

#endif

#define VLT_LUT_SIZE	16
/*PM policy definitions for Rhea */
#define PM_OFF		0
#define PM_RET		1
#define	PM_ECO		4
#define	PM_DFS		5

static int delayed_init_complete;

#ifdef CONFIG_RHEA_DELAYED_PM_INIT
struct pi_mgr_qos_node delay_arm_lpm;

struct pm_late_init {
	int dummy;
};

#define __param_check_pm_late_init(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_pm_late_init(name, p) \
	__param_check_pm_late_init(name, p, pm_late_init)

static int param_set_pm_late_init(const char *val,
		const struct kernel_param *kp);
static struct kernel_param_ops param_ops_pm_late_init = {
	.set = param_set_pm_late_init,
};
static struct pm_late_init pm_late_init;
module_param_named(pm_late_init, pm_late_init, pm_late_init,
				S_IWUSR | S_IWGRP);
#endif

#ifdef CONFIG_DEBUG_FS
const char *_rhea__event2str[] = {
	__stringify(LCDTE_EVENT),
	__stringify(SSP2SYN_EVENT),
	__stringify(SSP2DI_EVENT),
	__stringify(SSP2CK_EVENT),
	__stringify(SSP1SYN_EVENT),
	__stringify(SSP1DI_EVENT),
	__stringify(SSP1CK_EVENT),
	__stringify(SSP0SYN_EVENT),
	__stringify(SSP0DI_EVENT),
	__stringify(SSP0CK_EVENT),
	__stringify(DIGCLKREQ_EVENT),
	__stringify(ANA_SYS_REQ_EVENT),
	__stringify(SYSCLKREQ_EVENT),
	__stringify(UBRX_EVENT),
	__stringify(UBCTSN_EVENT),
	__stringify(UB2RX_EVENT),
	__stringify(UB2CTSN_EVENT),
	__stringify(SIMDET_EVENT),
	__stringify(SIM2DET_EVENT),
	__stringify(MMC0D3_EVENT),
	__stringify(MMC0D1_EVENT),
	__stringify(MMC1D3_EVENT),
	__stringify(MMC1D1_EVENT),
	__stringify(SDDAT3_EVENT),
	__stringify(SDDAT1_EVENT),
	__stringify(SLB1CLK_EVENT),
	__stringify(SLB1DAT_EVENT),
	__stringify(SWCLKTCK_EVENT),
	__stringify(SWDIOTMS_EVENT),
	__stringify(KEY_R0_EVENT),
	__stringify(KEY_R1_EVENT),
	__stringify(KEY_R2_EVENT),
	__stringify(KEY_R3_EVENT),
	__stringify(KEY_R4_EVENT),
	__stringify(KEY_R5_EVENT),
	__stringify(KEY_R6_EVENT),
	__stringify(KEY_R7_EVENT),
	__stringify(CAWAKE_EVENT),
	__stringify(CAREADY_EVENT),
	__stringify(CAFLAG_EVENT),
	__stringify(BATRM_EVENT),
	__stringify(USBDP_EVENT),
	__stringify(USBDN_EVENT),
	__stringify(RXD_EVENT),
	__stringify(GPIO29_A_EVENT),
	__stringify(GPIO32_A_EVENT),
	__stringify(GPIO33_A_EVENT),
	__stringify(GPIO43_A_EVENT),
	__stringify(GPIO44_A_EVENT),
	__stringify(GPIO45_A_EVENT),
	__stringify(GPIO46_A_EVENT),
	__stringify(GPIO47_A_EVENT),
	__stringify(GPIO48_A_EVENT),
	__stringify(GPIO71_A_EVENT),
	__stringify(GPIO72_A_EVENT),
	__stringify(GPIO73_A_EVENT),
	__stringify(GPIO74_A_EVENT),
	__stringify(GPIO95_A_EVENT),
	__stringify(GPIO96_A_EVENT),
	__stringify(GPIO99_A_EVENT),
	__stringify(GPIO100_A_EVENT),
	__stringify(GPIO111_A_EVENT),
	__stringify(GPIO18_A_EVENT),
	__stringify(GPIO19_A_EVENT),
	__stringify(GPIO20_A_EVENT),
	__stringify(GPIO89_A_EVENT),
	__stringify(GPIO90_A_EVENT),
	__stringify(GPIO91_A_EVENT),
	__stringify(GPIO92_A_EVENT),
	__stringify(GPIO93_A_EVENT),
	__stringify(GPIO18_B_EVENT),
	__stringify(GPIO19_B_EVENT),
	__stringify(GPIO20_B_EVENT),
	__stringify(GPIO89_B_EVENT),
	__stringify(GPIO90_B_EVENT),
	__stringify(GPIO91_B_EVENT),
	__stringify(GPIO92_B_EVENT),
	__stringify(GPIO93_B_EVENT),
	__stringify(GPIO29_B_EVENT),
	__stringify(GPIO32_B_EVENT),
	__stringify(GPIO33_B_EVENT),
	__stringify(GPIO43_B_EVENT),
	__stringify(GPIO44_B_EVENT),
	__stringify(GPIO45_B_EVENT),
	__stringify(GPIO46_B_EVENT),
	__stringify(GPIO47_B_EVENT),
	__stringify(GPIO48_B_EVENT),
	__stringify(GPIO71_B_EVENT),
	__stringify(GPIO72_B_EVENT),
	__stringify(GPIO73_B_EVENT),
	__stringify(GPIO74_B_EVENT),
	__stringify(GPIO95_B_EVENT),
	__stringify(GPIO96_B_EVENT),
	__stringify(GPIO99_B_EVENT),
	__stringify(GPIO100_B_EVENT),
	__stringify(GPIO111_B_EVENT),
	__stringify(COMMON_TIMER_0_EVENT),
	__stringify(COMMON_TIMER_1_EVENT),
	__stringify(COMMON_TIMER_2_EVENT),
	__stringify(COMMON_TIMER_3_EVENT),
	__stringify(COMMON_TIMER_4_EVENT),
	__stringify(COMMON_INT_TO_AC_EVENT),
	__stringify(TZCFG_INT_TO_AC_EVENT),
	__stringify(DMA_REQUEST_EVENT),
	__stringify(MODEM1_EVENT),
	__stringify(MODEM2_EVENT),
	__stringify(SD1DAT1DAT3_EVENT),
	__stringify(BRIDGE_TO_AC_EVENT),
	__stringify(BRIDGE_TO_MODEM_EVENT),
	__stringify(VREQ_NONZERO_PI_MODEM_EVENT),
	NULL,
	__stringify(USBOTG_EVENT),
	__stringify(GPIO_EXP_IRQ_EVENT),
	__stringify(DBR_IRQ_EVENT),
	__stringify(ACI_EVENT),
	__stringify(PHY_RESUME_EVENT),
	__stringify(MODEMBUS_ACTIVE_EVENT),
	__stringify(SOFTWARE_0_EVENT),
	__stringify(SOFTWARE_1_EVENT),
	__stringify(SOFTWARE_2_EVENT),
};

#endif

struct rhea_event_table {
	u32 event_id;
	u32 trig_type;
	u32 policy_modem;
	u32 policy_arm_core;
	u32 policy_arm_sub;
	u32 policy_hub_aon;
	u32 policy_hub_switchable;
	u32 policy_mm;
};
static const struct rhea_event_table event_table[] = {
	/*event_id                              trig_type                       modem           arm_core        arm_sub         aon             hub             mm */
	{SOFTWARE_0_EVENT, PM_TRIG_BOTH_EDGE, PM_RET, PM_RET, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{SOFTWARE_1_EVENT, PM_TRIG_NONE, PM_RET, PM_RET, PM_RET, PM_RET, PM_RET,
	 PM_OFF,},
	/*JIRA HWRHEA-2093 : change HUB policy to 5 for all active events */
	{SOFTWARE_2_EVENT, PM_TRIG_BOTH_EDGE, PM_RET, PM_DFS, PM_ECO, PM_ECO,
	 PM_DFS, PM_OFF,},
	// This is a SW workaround for A0. Configure MODEMBUS_ACTIVE_EVENT to wake up AP at ECONOMY so that
	// AP stays awake long enough until all CP activities that could trigger MODEMBUS_ACTIVE_EVENT have completed.
	// For A0 chip, MODEMBUS_ACTIVE_EVENT is enabled to work around the JIRA that VREQ_NONZERO_PI_MODEM_EVENT is not auto-cleared.
	// JIRA HWRHEA-1253 : Remove MODEMBUS_ACTIVE_EVENT for B0
	{VREQ_NONZERO_PI_MODEM_EVENT, PM_TRIG_POS_EDGE, PM_DFS, PM_RET, PM_RET,
	 PM_DFS, PM_DFS, PM_OFF,},
	{COMMON_INT_TO_AC_EVENT, PM_TRIG_POS_EDGE, PM_RET, PM_DFS, PM_DFS,
	 PM_DFS, PM_DFS, PM_OFF,},
	{COMMON_TIMER_1_EVENT, PM_TRIG_POS_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{UBRX_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{UB2RX_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{SIMDET_EVENT, PM_TRIG_BOTH_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{SIM2DET_EVENT, PM_TRIG_BOTH_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{KEY_R0_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R1_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R2_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R3_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R4_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R5_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R6_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{KEY_R7_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{BATRM_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{GPIO29_A_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{GPIO71_A_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{MMC1D1_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,},
	{GPIO74_A_EVENT, PM_TRIG_BOTH_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{GPIO111_A_EVENT, PM_TRIG_POS_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{DBR_IRQ_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS,
	 PM_DFS, PM_OFF,},
	{ACI_EVENT, PM_TRIG_NEG_EDGE, PM_RET, PM_DFS, PM_DFS, PM_DFS, PM_DFS,
	 PM_OFF,}

};

static struct i2c_cmd i2c_dummy_seq_cmd[] = {
	{REG_ADDR, 0},		//0 - NOP
	{JUMP_VOLTAGE, 0},	//1 - jump to address based on current voltage request
	{REG_ADDR, 0},		//other:2 - NOP
	{SET_PC_PINS, 0xC0},	//3 - Set PC3/4 pins to 0 to begin transaction (HW has semaphore)
	{REG_ADDR, 0},		//4 - NOP
	{REG_ADDR, 0},		//5 - Set Address for i2cmmhs BSC_CS register
	{REG_ADDR, 0},		//6 - Set Start condition - write 3 to CS register
	{REG_ADDR, 0},		//7 - Wait..
	{REG_ADDR, 0},		//8 - Clear Start Condition - write 1 to CS
	{REG_ADDR, 0},		//9 - Wait..
	{REG_ADDR, 0},		//10 - PMU client addr  - 8 ..write to FIFO
	{REG_ADDR, 0},		//11 - Wait..
	{REG_ADDR, 0},		//12 - PMU register addr C0 (CSRSTRL1)
	{REG_ADDR, 0},		//13 - Wait..
	{REG_ADDR, 0},		//14 - Data - Write the requested voltage
	{REG_ADDR, 0},		//15 - Wait..
	{SET_PC_PINS, 0xCC},	//16 - Set PC3/4 to 1 to signal End of transaction
	{END, 0},		//17 - End
	{REG_ADDR, 0},		//18 - NOP
	{REG_ADDR, 0},		//19 - NOP
	{REG_ADDR, 0},		//20 - NOP
	{REG_ADDR, 0},		//21 - NOP
	{REG_ADDR, 0},		//22 - NOP
	{REG_ADDR, 0},		//23 - NOP
	{REG_ADDR, 0},		//24 - NOP
	{REG_ADDR, 0},		//25 - NOP
	{REG_ADDR, 0},		//26 - NOP
	{REG_ADDR, 0},		//27 - NOP
	{REG_ADDR, 0},		//28 - NOP
	{REG_ADDR, 0},		//29 - NOP
	{REG_ADDR, 0},		//30 - NOP
	{REG_ADDR, 0},		//31 - NOP
	{REG_ADDR, 0},		//32 - NOP
	{REG_ADDR, 0},		//33 - NOP
	{REG_ADDR, 0},		//34 - NOP
	{REG_ADDR, 0},		//35 - NOP
	{REG_ADDR, 0},		//36 - NOP
	{REG_ADDR, 0},		//37 - NOP
	{REG_ADDR, 0},		//38 - NOP
	{REG_ADDR, 0},		//39 - NOP
	{REG_ADDR, 0},		//40 - NOP
	{REG_ADDR, 0},		//41 - NOP
	{REG_ADDR, 0},		//42 - NOP
	{REG_ADDR, 0},		//43 - NOP
	{REG_ADDR, 0},		//44 - NOP
	{REG_ADDR, 0},		//45 - NOP
	{REG_ADDR, 0},		//46 - NOP
	{REG_ADDR, 0},		//47 - NOP
	{REG_ADDR, 0},		//48 - NOP
	{REG_ADDR, 0},		//49 - NOP
	{REG_ADDR, 0},		//50 - NOP
	{REG_ADDR, 0},		//51 - NOP
	{REG_ADDR, 0},		//set2/zero:52 - NOP
	{SET_PC_PINS, 0x30},	//53 - Set PC1 pins to 0
	{REG_ADDR, 0},		//54 -NOP
	{END, 0},		//55 -END
	{REG_ADDR, 0},		//set1:56 -NOP
	{SET_PC_PINS, 0x31},	//57 - Set PC1 pins to 1
	{REG_ADDR, 0},		//58 -NOP
	{END, 0},		//59 -END
	{REG_ADDR, 0},		//60 - NOP
	{REG_ADDR, 0},		//61 - NOP
	{REG_ADDR, 0},		//62 - NOP
	{REG_ADDR, 0},		//63 - NOP

};

#define RHEA_EVENT_POLICY_OFFSET	{ \
	[LCDTE_EVENT]		= 0x0, \
	[SSP2SYN_EVENT]		= 0x4, \
	[SSP2DI_EVENT]		= 0x4, \
	[SSP2CK_EVENT]		= 0x4, \
	[SSP1SYN_EVENT]		= 0x10, \
	[SSP1DI_EVENT]		= 0x10, \
	[SSP1CK_EVENT]		= 0x10, \
	[SSP0SYN_EVENT]		= 0x1C, \
	[SSP0DI_EVENT]		= 0x1C, \
	[SSP0CK_EVENT]		= 0x1C, \
	[DIGCLKREQ_EVENT]	= 0x28, \
	[ANA_SYS_REQ_EVENT]	= 0x28, \
	[SYSCLKREQ_EVENT]	= 0x28, \
	[UBRX_EVENT]		= 0x34, \
	[UBCTSN_EVENT]		= 0x34, \
	[UB2RX_EVENT]		= 0x3C, \
	[UB2CTSN_EVENT]		= 0x3C, \
	[SIMDET_EVENT]		= 0x44, \
	[SIM2DET_EVENT]		= 0x44, \
	[MMC0D3_EVENT]		= 0x4C, \
	[MMC0D1_EVENT]		= 0x4C, \
	[MMC1D3_EVENT]		= 0x4C, \
	[MMC1D1_EVENT]		= 0x4C, \
	[SDDAT3_EVENT]		= 0x5C, \
	[SDDAT1_EVENT]		= 0x5C, \
	[SLB1CLK_EVENT]		= 0x64, \
	[SLB1DAT_EVENT]		= 0x64, \
	[SWCLKTCK_EVENT]	= 0x6C, \
	[SWDIOTMS_EVENT]	= 0x6C, \
	[KEY_R0_EVENT]		= 0x74, \
	[KEY_R1_EVENT]		= 0x74, \
	[KEY_R2_EVENT]		= 0x74, \
	[KEY_R3_EVENT]		= 0x74, \
	[KEY_R4_EVENT]		= 0x74, \
	[KEY_R5_EVENT]		= 0x74, \
	[KEY_R6_EVENT]		= 0x74, \
	[KEY_R7_EVENT]		= 0x74, \
	[CAWAKE_EVENT]		= 0x94, \
	[CAREADY_EVENT]		= 0x94, \
	[CAFLAG_EVENT]		= 0x94, \
	[BATRM_EVENT]		= 0xA0, \
	[USBDP_EVENT]		= 0xA4, \
	[USBDN_EVENT]		= 0xA4, \
	[RXD_EVENT]		= 0xAC, \
	[GPIO29_A_EVENT]	= 0xB0, \
	[GPIO32_A_EVENT]	= 0xB0, \
	[GPIO33_A_EVENT]	= 0xB0, \
	[GPIO43_A_EVENT]	= 0xB0, \
	[GPIO44_A_EVENT]	= 0xB0, \
	[GPIO45_A_EVENT]	= 0xB0, \
	[GPIO46_A_EVENT]	= 0xB0, \
	[GPIO47_A_EVENT]	= 0xB0, \
	[GPIO48_A_EVENT]	= 0xB0, \
	[GPIO71_A_EVENT]	= 0xB0, \
	[GPIO72_A_EVENT]	= 0xB0, \
	[GPIO73_A_EVENT]	= 0xB0, \
	[GPIO74_A_EVENT]	= 0xB0, \
	[GPIO95_A_EVENT]	= 0xB0, \
	[GPIO96_A_EVENT]	= 0xB0, \
	[GPIO99_A_EVENT]	= 0xB0, \
	[GPIO100_A_EVENT]	= 0xB0, \
	[GPIO111_A_EVENT]	= 0xB0, \
	[GPIO18_A_EVENT]	= 0xB0, \
	[GPIO19_A_EVENT]	= 0xB0, \
	[GPIO20_A_EVENT]	= 0xB0, \
	[GPIO89_A_EVENT]	= 0xB0, \
	[GPIO90_A_EVENT]	= 0xB0, \
	[GPIO91_A_EVENT]	= 0xB0, \
	[GPIO92_A_EVENT]	= 0xB0, \
	[GPIO93_A_EVENT]	= 0xB0, \
	[GPIO18_B_EVENT]	= 0x118, \
	[GPIO19_B_EVENT]	= 0x118, \
	[GPIO20_B_EVENT]	= 0x118, \
	[GPIO89_B_EVENT]	= 0x118, \
	[GPIO90_B_EVENT]	= 0x118, \
	[GPIO91_B_EVENT]	= 0x118, \
	[GPIO92_B_EVENT]	= 0x118, \
	[GPIO93_B_EVENT]	= 0x118, \
	[GPIO29_B_EVENT]	= 0x118, \
	[GPIO32_B_EVENT]	= 0x118, \
	[GPIO33_B_EVENT]	= 0x118, \
	[GPIO43_B_EVENT]	= 0x118, \
	[GPIO44_B_EVENT]	= 0x118, \
	[GPIO45_B_EVENT]	= 0x118, \
	[GPIO46_B_EVENT]	= 0x118, \
	[GPIO47_B_EVENT]	= 0x118, \
	[GPIO48_B_EVENT]	= 0x118, \
	[GPIO71_B_EVENT]	= 0x118, \
	[GPIO72_B_EVENT]	= 0x118, \
	[GPIO73_B_EVENT]	= 0x118, \
	[GPIO74_B_EVENT]	= 0x118, \
	[GPIO95_B_EVENT]	= 0x118, \
	[GPIO96_B_EVENT]	= 0x118, \
	[GPIO99_B_EVENT]	= 0x118, \
	[GPIO100_B_EVENT]	= 0x118, \
	[GPIO111_B_EVENT]	= 0x118, \
	[COMMON_TIMER_0_EVENT]	= 0x180, \
	[COMMON_TIMER_1_EVENT]	= 0x184, \
	[COMMON_TIMER_2_EVENT]	= 0x188, \
	[COMMON_TIMER_3_EVENT]	= 0x18C, \
	[COMMON_TIMER_4_EVENT]	= 0x190, \
	[COMMON_INT_TO_AC_EVENT] = 0x194, \
	[TZCFG_INT_TO_AC_EVENT]	= 0x198, \
	[DMA_REQUEST_EVENT]	= 0x19C, \
	[MODEM1_EVENT]		= 0x1A0, \
	[MODEM2_EVENT]		= 0x1A4, \
	[SD1DAT1DAT3_EVENT]	= 0x1A8, \
	[BRIDGE_TO_AC_EVENT]	= 0x1AC, \
	[BRIDGE_TO_MODEM_EVENT]	= 0x1B0, \
	[VREQ_NONZERO_PI_MODEM_EVENT]	= 0x1B4, \
	[DUMMY_EVENT]		= INVALID_EVENT_OFFSET, \
	[USBOTG_EVENT]		= 0x1BC, \
	[GPIO_EXP_IRQ_EVENT]	= 0x1C0, \
	[DBR_IRQ_EVENT]		= 0x1C4, \
	[ACI_EVENT]		= 0x1C8, \
	[PHY_RESUME_EVENT]	= 0x1CC, \
	[MODEMBUS_ACTIVE_EVENT]	= 0x1D0, \
	[SOFTWARE_0_EVENT]	= 0x1D4, \
	[SOFTWARE_1_EVENT]	= 0x1D8, \
	[SOFTWARE_2_EVENT]	= 0x1DC, \
}

struct pwr_mgr_info rhea_pwr_mgr_info = {
	.num_pi = PI_MGR_PI_ID_MAX,
	.base_addr = KONA_PWRMGR_VA,
	.flags = PM_PMU_I2C | I2C_SIMULATE_BURST_MODE,
	.pwrmgr_intr = BCM_INT_ID_PWR_MGR,
	.event_policy_offset = RHEA_EVENT_POLICY_OFFSET,
};

int __init rhea_pwr_mgr_init()
{
	struct pm_policy_cfg cfg;

	int i;
	struct pi *pi;
	struct v0x_spec_i2c_cmd_ptr dummy_seq_v0_ptr = {
		.other_ptr = 2,
		.set2_val = 1,	/*Retention voltage inx */
		.set2_ptr = 52,
		.set1_val = 2,	/*wakeup from retention voltage inx */
		.set1_ptr = 56,
		.zerov_ptr = 52,	/*Not used for Rhea */
	};

	cfg.ac = 1;
	cfg.atl = 0;
	rhea_pm_params_init();
	/**
	 * Load the dummy sequencer during power manager initialization
	 * Actual sequencer will be loaded during pmu i2c driver
	 * initialisation
	 */
	rhea_pwr_mgr_info.i2c_cmds = i2c_dummy_seq_cmd;
	rhea_pwr_mgr_info.num_i2c_cmds = ARRAY_SIZE(i2c_dummy_seq_cmd);
	rhea_pwr_mgr_info.i2c_cmd_ptr[VOLT0] = &dummy_seq_v0_ptr;

	rhea_pwr_mgr_info.i2c_var_data = pwrmgr_init_param.def_vlt_tbl;
	rhea_pwr_mgr_info.num_i2c_var_data = pwrmgr_init_param.vlt_tbl_size;

	pwr_mgr_init(&rhea_pwr_mgr_info);
	rhea_pi_mgr_init();
#ifdef CONFIG_RHEA_WA_HWJIRA_2272
/*For B0, it was observed that if MM CCU is switched to and
	from shutdown state, it would break the DDR self refresh.
	Recommended workaround is to set the POWER_OK_MASK bit to 0 */
	if (JIRA_WA_ENABLED(2272))
		pwr_mgr_ignore_power_ok_signal(false);
#endif

	/*MM override is not set by default */
	pwr_mgr_pi_set_wakeup_override(PI_MGR_PI_ID_MM, false /*clear */ );

	/*Done in two steps to skip DUMMY_EVENT */
	pwr_mgr_event_clear_events(LCDTE_EVENT, VREQ_NONZERO_PI_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT, EVENT_ID_ALL);

	pwr_mgr_event_set(SOFTWARE_2_EVENT, 1);
	pwr_mgr_event_set(SOFTWARE_0_EVENT, 1);

	pwr_mgr_set_pc_clkreq_override(PC1, true, 1);
	pwr_mgr_pm_i2c_enable(true);
	/*Init event table */
	for (i = 0; i < ARRAY_SIZE(event_table); i++) {
		pwr_mgr_event_trg_enable(event_table[i].event_id,
					 event_table[i].trig_type);

		cfg.policy = event_table[i].policy_modem;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,
					    PI_MGR_PI_ID_MODEM, &cfg);

		cfg.policy = event_table[i].policy_arm_core;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,
					    PI_MGR_PI_ID_ARM_CORE, &cfg);

		cfg.policy = event_table[i].policy_arm_sub;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,
					    PI_MGR_PI_ID_ARM_SUB_SYSTEM, &cfg);

		cfg.policy = event_table[i].policy_hub_aon;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,
					    PI_MGR_PI_ID_HUB_AON, &cfg);

		cfg.policy = event_table[i].policy_hub_switchable;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,
					    PI_MGR_PI_ID_HUB_SWITCHABLE, &cfg);

		cfg.policy = event_table[i].policy_mm;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,
					    PI_MGR_PI_ID_MM, &cfg);
	}
	/*Init all PIs */

	for (i = 0; i < PI_MGR_PI_ID_MODEM; i++) {
		pi = pi_mgr_get(i);
		BUG_ON(pi == NULL);
		pi_init(pi);
	}

	/*init clks */
	rhea_clock_init();

	return 0;
}

early_initcall(rhea_pwr_mgr_init);

#ifdef CONFIG_DEBUG_FS

void pwr_mgr_mach_debug_fs_init(int type, int db_mux, int mux_param,
		u32 dbg_bit_sel)
{
	if (db_mux == 0)	/*GPIO ? */
		mux_param = (type == 0) ? DBG_BUS_PM_DBG_BUS_SEL
				: DBG_BUS_BMDB_DBG_BUS_SEL;

	debug_bus_mux_sel(db_mux, mux_param, dbg_bit_sel);
}

#endif /*CONFIG_DEBUG_FS */


int rhea_pwr_mgr_delayed_init(void)
{
	int i;
	struct pi *pi;

	/*All the initializations are done. Clear override bit here so that
	 * appropriate policies take effect*/
	for (i = 0; i < PI_MGR_PI_ID_MODEM; i++) {
		pi = pi_mgr_get(i);
		BUG_ON(pi == NULL);
		pi_init_state(pi);
	}

	/* Enable PI counters */
	for (i = 0; i < PI_MGR_PI_ID_MAX; i++)
		pwr_mgr_pi_counter_enable(i, 1);
	pm_mgr_pi_count_clear(1);
	pm_mgr_pi_count_clear(0);

#ifdef CONFIG_RHEA_IGNORE_DAP_POWERUP_REQ
	pwr_mgr_ignore_dap_powerup_request(true);
	pwr_mgr_ignore_mdm_dap_powerup_req(true);
#endif
	delayed_init_complete = 1;
#ifdef CONFIG_DEBUG_FS
	return pwr_mgr_debug_init(KONA_BMDM_PWRMGR_VA);
#else
	return 0;
#endif
}

#ifdef CONFIG_RHEA_DELAYED_PM_INIT
static int param_set_pm_late_init(const char *val,
			const struct kernel_param *kp)
{
	int ret = -1;
	int pm_delayed_init = 0;

	pr_info("%s\n", __func__);
	if (delayed_init_complete)
		return 0;
	if (!val)
		return -EINVAL;

	ret = sscanf(val, "%d", &pm_delayed_init);
	pr_info("%s, pm_delayed_init:%d\n", __func__, pm_delayed_init);
	if (pm_delayed_init == 1)
		rhea_pwr_mgr_delayed_init();

	if (delay_arm_lpm.valid)
		pi_mgr_qos_request_remove(&delay_arm_lpm);

	return 0;
}
#endif

int __init rhea_pwr_mgr_late_init(void)
{
#ifdef CONFIG_RHEA_DELAYED_PM_INIT
	int ret;
	if (is_charging_state()) {
		pr_info("%s: power off charging, complete int here\n",
						__func__);
		rhea_pwr_mgr_delayed_init();
	} else {
		ret = pi_mgr_qos_add_request(&delay_arm_lpm, "delay_arm_lpm",
				PI_MGR_PI_ID_ARM_CORE, 0);
	}
#else
	rhea_pwr_mgr_delayed_init();
#endif
	return 0;
}

late_initcall(rhea_pwr_mgr_late_init);

/**
 * Initialize the real sequencer
 * On return PWRMGR I2C block will be
 * enabled
 */

int __init mach_init_sequencer(void)
{
	pr_info("%s\n", __func__);

	rhea_pwr_mgr_info.i2c_rd_off = pwrmgr_init_param.i2c_rd_off;
	rhea_pwr_mgr_info.i2c_rd_slv_id_off1 =
	    pwrmgr_init_param.i2c_rd_slv_id_off1;
	rhea_pwr_mgr_info.i2c_rd_slv_id_off2 =
	    pwrmgr_init_param.i2c_rd_slv_id_off2;
	rhea_pwr_mgr_info.i2c_rd_reg_addr_off =
	    pwrmgr_init_param.i2c_rd_reg_addr_off;
	rhea_pwr_mgr_info.i2c_rd_fifo_off = pwrmgr_init_param.i2c_rd_fifo_off;
	rhea_pwr_mgr_info.i2c_wr_off = pwrmgr_init_param.i2c_wr_off;
	rhea_pwr_mgr_info.i2c_wr_slv_id_off =
	    pwrmgr_init_param.i2c_wr_slv_id_off;
	rhea_pwr_mgr_info.i2c_wr_reg_addr_off =
	    pwrmgr_init_param.i2c_wr_reg_addr_off;
	rhea_pwr_mgr_info.i2c_wr_val_addr_off =
	    pwrmgr_init_param.i2c_wr_val_addr_off;
	rhea_pwr_mgr_info.i2c_seq_timeout = pwrmgr_init_param.i2c_seq_timeout;
#ifdef CONFIG_KONA_PWRMGR_SWSEQ_FAKE_TRG_ERRATUM
	rhea_pwr_mgr_info.pc_toggle_off = pwrmgr_init_param.pc_toggle_off;
#endif

	rhea_pwr_mgr_info.i2c_cmds = pwrmgr_init_param.cmd_buf;
	rhea_pwr_mgr_info.num_i2c_cmds = pwrmgr_init_param.cmb_buf_size;
	rhea_pwr_mgr_info.i2c_cmd_ptr[VOLT0] = pwrmgr_init_param.v0ptr;

	pwr_mgr_init_sequencer(&rhea_pwr_mgr_info);

	/* pwr_mgr_init_sequencer function will disable the sequencer
	   re-enable the power manager i2c sequencer */
	pwr_mgr_pm_i2c_enable(true);
	return 0;
}

EXPORT_SYMBOL(mach_init_sequencer);

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
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <mach/io_map.h>
#include <linux/io.h>

#include<mach/clock.h>
#include<plat/pi_mgr.h>
#include<mach/pi_mgr.h>
#include<mach/pwr_mgr.h>
#include<plat/pwr_mgr.h>
#ifdef CONFIG_DEBUG_FS
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#endif

#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#endif

#define VLT_LUT_SIZE	16
/*PM policy definitions for Rhea */
#define PM_OFF		0
#define PM_RET		1
#define	PM_ECO		4
#define	PM_DFS		5


#ifdef CONFIG_DEBUG_FS
const char* _rhea__event2str[] =
{
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
	__stringify(MODEM_UART_EVENT),
	__stringify(BRIDGE_TO_AC_EVENT),
	__stringify(BRIDGE_TO_MODEM_EVENT),
	__stringify(VREQ_NONZERO_PI_MODEM_EVENT),
	__stringify(DUMMY_EVENT),
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

struct pwr_mgr_info rhea_pwr_mgr_info = {
	.num_pi = PI_MGR_PI_ID_MAX,
	.base_addr = KONA_PWRMGR_VA,
	.flags = PM_PMU_I2C,
};

struct rhea_event_table
{
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
		/*event_id				trig_type			modem		arm_core 	arm_sub		aon		hub		mm*/
	{	SOFTWARE_0_EVENT,			PM_TRIG_BOTH_EDGE,		PM_RET,		PM_RET, 	PM_DFS,		PM_DFS,		PM_DFS,		PM_RET, },
	{	SOFTWARE_1_EVENT,			PM_TRIG_NONE,			PM_RET,		PM_RET,		PM_RET,		PM_RET,		PM_RET,		PM_RET,	},
	/*JIRA HWRHEA-2093 : change HUB policy to 5 for all active events */
	{	SOFTWARE_2_EVENT,			PM_TRIG_BOTH_EDGE,		PM_RET,		PM_DFS,		PM_ECO,		PM_ECO,		PM_DFS,		PM_RET,	},
	// This is a SW workaround for A0. Configure MODEMBUS_ACTIVE_EVENT to wake up AP at ECONOMY so that
	// AP stays awake long enough until all CP activities that could trigger MODEMBUS_ACTIVE_EVENT have completed.
	// For A0 chip, MODEMBUS_ACTIVE_EVENT is enabled to work around the JIRA that VREQ_NONZERO_PI_MODEM_EVENT is not auto-cleared.
	// JIRA HWRHEA-1253 : Remove MODEMBUS_ACTIVE_EVENT for B0
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	{	MODEMBUS_ACTIVE_EVENT, 			PM_TRIG_POS_EDGE,		PM_RET,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
#endif
	{	VREQ_NONZERO_PI_MODEM_EVENT,		PM_TRIG_POS_EDGE,		PM_DFS,		PM_RET,		PM_RET,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	COMMON_INT_TO_AC_EVENT,			PM_TRIG_POS_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	COMMON_TIMER_1_EVENT,			PM_TRIG_POS_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	UBRX_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	UB2RX_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	SIMDET_EVENT,				PM_TRIG_BOTH_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	SIM2DET_EVENT,				PM_TRIG_BOTH_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R0_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R1_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R2_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R3_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R4_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R5_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R6_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	KEY_R7_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	BATRM_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	GPIO29_A_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	GPIO71_A_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	MMC1D1_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	GPIO74_A_EVENT,				PM_TRIG_BOTH_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	GPIO111_A_EVENT,			PM_TRIG_POS_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	DBR_IRQ_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	},
	{	ACI_EVENT,				PM_TRIG_NEG_EDGE,		PM_RET,		PM_DFS,		PM_DFS,		PM_DFS,		PM_DFS,		PM_RET,	}

};

static const struct i2c_cmd i2c_cmd[] = {
							{REG_ADDR,0},		//0 - NOP
							{JUMP_VOLTAGE,0},		//1 - jump to address based on current voltage request
							{REG_ADDR,0},		//other:2 - NOP
							{SET_PC_PINS,0xC0},	//3 - Set PC3/4 pins to 0 to begin transaction (HW has semaphore)
							{REG_ADDR,0},		//4 - NOP
							{REG_ADDR,0x20},	//5 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x03},	//6 - Set Start condition - write 3 to CS register
							{WAIT_TIMER,0x10},	//7 - Wait..
							{REG_DATA,1},  		//8 - Clear Start Condition - write 1 to CS
							{WAIT_TIMER,0x08},	//9 - Wait..
							{I2C_DATA,0x10},	//10 - PMU client addr  - 8 ..write to FIFO
							{WAIT_TIMER,0x80},	//11 - Wait..
							{I2C_DATA,0xC0},	//12 - PMU register addr C0 (CSRSTRL1)
							{WAIT_TIMER,0x80},	//13 - Wait..
							{I2C_VAR,0},		//14 - Data - Write the requested voltage
							{WAIT_TIMER,0x80},	//15 - Wait..
							{SET_PC_PINS,0xCC},	//16 - Set PC3/4 to 1 to signal End of transaction
							{END,0},			//17 - End
							{REG_ADDR,0},		//18 - NOP
							{REG_ADDR,0},		//19 - NOP
							{REG_ADDR,0},		//20 - NOP
							{REG_ADDR,0},		//21 - NOP
							{REG_ADDR,0},		//22 - NOP
							{REG_ADDR,0},		//23 - NOP
							{REG_ADDR,0},		//24 - NOP
							{REG_ADDR,0},		//25 - NOP
							{REG_ADDR,0},		//26 - NOP
							{REG_ADDR,0},		//27 - NOP
							{REG_ADDR,0},		//28 - NOP
							{REG_ADDR,0},		//29 - NOP
							{REG_ADDR,0},		//30 - NOP
							{REG_ADDR,0},		//31 - NOP
							{REG_ADDR,0},		//32 - NOP
							{REG_ADDR,0},		//33 - NOP
							{REG_ADDR,0},		//34 - NOP
							{REG_ADDR,0},		//35 - NOP
							{REG_ADDR,0},		//36 - NOP
							{REG_ADDR,0},		//37 - NOP
							{REG_ADDR,0},		//38 - NOP
							{REG_ADDR,0},		//39 - NOP
							{REG_ADDR,0},		//40 - NOP
							{REG_ADDR,0},		//41 - NOP
							{REG_ADDR,0},		//42 - NOP
							{REG_ADDR,0},		//43 - NOP
							{REG_ADDR,0},		//44 - NOP
							{REG_ADDR,0},		//set2/zero:45 - NOP
							{SET_PC_PINS,0x30},	//46 - Set PC1 pins to 0
							{REG_ADDR,0},		//47 -NOP
							{END,0},			//48 -END
							{REG_ADDR,0},		//set1:49 -NOP
							{SET_PC_PINS,0x31},	//50 - Set PC1 pins to 1
							{REG_ADDR,0},		//51 -NOP
							{END,0},			//52 -END
							{REG_ADDR,0},		//53 - NOP
							{REG_ADDR,0},		//54 - NOP
							{REG_ADDR,0},		//55 - NOP
							{REG_ADDR,0},		//56 - NOP
							{REG_ADDR,0},		//57 - NOP
							{REG_ADDR,0},		//58 - NOP
							{REG_ADDR,0},		//59 - NOP
							{REG_ADDR,0},		//60 - NOP
							{REG_ADDR,0},		//61 - NOP
							{REG_ADDR,0},		//62 - NOP
							{REG_ADDR,0},		//63 - NOP

						  };

/*Default voltage lookup table
Need to move this to board-file
*/
static u8 pwrmgr_default_volt_lut[] =
								{
									0x03,
									0x03,
									0x04,
									0x04,
									0x04,
									0x0e,
									0x0e,
									0x0e,
									0x0e,
									0x0e,
									0x0e,
									0x0e,
									0x13,
									0x13,
									0x13,
									0x13
								};


int __init rhea_pwr_mgr_init()
{
	struct v0x_spec_i2c_cmd_ptr v_ptr;
	int i;
	struct pi* pi;
	u32 reg_val = 0;
	struct pm_policy_cfg cfg;
	cfg.ac = 1;
	cfg.atl = 0;

	v_ptr.other_ptr = 2;
	v_ptr.set2_val = 1; /*Retention voltage inx*/
	v_ptr.set2_ptr = 45;
	v_ptr.set1_val = 2;/*wakeup from retention voltage inx*/
	v_ptr.set1_ptr = 49;
	v_ptr.zerov_ptr = 45; /*Not used for Rhea*/

	pwr_mgr_init(&rhea_pwr_mgr_info);
	rhea_pi_mgr_init();

#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	// JIRA HWRHEA-1689, HWRHEA-1739 we confirmed that there is a bug in Rhea A0 where wrong control signal
	// is used to turn on mm power switches which results in mm clamps getting released before mm subsystem
	// has powered up. This results in glitches on mm outputs which in some parts causes fake write
	// transaction to memc with random ID. Next real write transfer to memc from mm creates write
	// interleaving error in memc and hangs mm. This is the root cause of MM block test failures observed
	// in BLTS MobC00164066: SW workaround is to reduce inrush current setting on mm power switch control
	// from default 14.5mA (0x3) to 1.5mA (0x0) in bits 1:0 of CHIPREG:mm_powerswitch_control_status register.
	reg_val = readl(KONA_CHIPREG_VA + CHIPREG_MM_POWERSWITCH_CONTROL_STATUS_OFFSET);
	/* 1.5mA per switch */
	reg_val &= ~CHIPREG_MM_POWERSWITCH_CONTROL_STATUS_POWER_SWITCH_CTRL_MASK;
	writel(reg_val, (KONA_CHIPREG_VA + CHIPREG_MM_POWERSWITCH_CONTROL_STATUS_OFFSET));
#endif
	/*MM override is not set by default*/
	pwr_mgr_pi_set_wakeup_override(PI_MGR_PI_ID_MM,false/*clear*/);
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	/* 14.5mA per switch */
	reg_val |= 3;
	writel(reg_val, (KONA_CHIPREG_VA +CHIPREG_MM_POWERSWITCH_CONTROL_STATUS_OFFSET));
#endif
	/*Done in two steps to skip DUMMY_EVENT*/
	pwr_mgr_event_clear_events(LCDTE_EVENT,VREQ_NONZERO_PI_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT,EVENT_ID_ALL);

	pwr_mgr_event_set(SOFTWARE_2_EVENT,1);
	pwr_mgr_event_set(SOFTWARE_0_EVENT,1);

		/*Init I2c seq*/
	pwr_mgr_pm_i2c_enable(false);
	/*Program I2C sequencer*/
	pwr_mgr_pm_i2c_cmd_write(i2c_cmd,ARRAY_SIZE(i2c_cmd));
	/*Program voltage lookup table
	AVS driver may chnage this later*/
	pwr_mgr_pm_i2c_var_data_write(pwrmgr_default_volt_lut,VLT_LUT_SIZE);
	/*populate the jump voltage table */
	pwr_mgr_set_v0x_specific_i2c_cmd_ptr(VOLT0,&v_ptr);
	pwr_mgr_pm_i2c_enable(true);

	/*Init event table*/
	for(i = 0; i < ARRAY_SIZE(event_table);i++)
	{
		pwr_mgr_event_trg_enable(event_table[i].event_id,event_table[i].trig_type);

		cfg.policy = event_table[i].policy_modem;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,PI_MGR_PI_ID_MODEM,&cfg);

		cfg.policy = event_table[i].policy_arm_core;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,PI_MGR_PI_ID_ARM_CORE,&cfg);

		cfg.policy = event_table[i].policy_arm_sub;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,PI_MGR_PI_ID_ARM_SUB_SYSTEM,&cfg);

		cfg.policy = event_table[i].policy_hub_aon;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,PI_MGR_PI_ID_HUB_AON,&cfg);

		cfg.policy = event_table[i].policy_hub_switchable;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,PI_MGR_PI_ID_HUB_SWITCHABLE,&cfg);

		cfg.policy = event_table[i].policy_mm;
		pwr_mgr_event_set_pi_policy(event_table[i].event_id,PI_MGR_PI_ID_MM,&cfg);
	}
	/*Init all PIs*/


	for(i = 0; i < PI_MGR_PI_ID_MODEM;i++)
	{
		pi = pi_mgr_get(i);
		BUG_ON(pi == NULL);
		pi_init(pi);
	}

	/*init clks*/
	rhea_clock_init();

	/*All the initializations are done. Clear override bit here so that
	 * appropriate policies take effect*/
	for (i = 0; i < PI_MGR_PI_ID_MODEM;i++) {
	    pi = pi_mgr_get(i);
	    BUG_ON(pi == NULL);
	    pi_init_state(pi);
	}

return 0;
}
early_initcall(rhea_pwr_mgr_init);

#ifdef CONFIG_DEBUG_FS

void pwr_mgr_mach_debug_fs_init(int type)
{
	u32 reg_val;

	set_gpio_mux_for_debug_bus();
	reg_val = readl(KONA_CHIPREG_VA+CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);
	reg_val &= ~CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_MASK;
	if(type == 0)
		reg_val |= 0x2 << CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_SHIFT;
	else if(type == 1)
		reg_val |= 0xD << CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_SHIFT;
	else
		BUG();
	writel(reg_val,KONA_CHIPREG_VA+CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);
}

int __init rhea_pwr_mgr_late_init(void)
{
	u32 bmdm_pwr_mgr_base = (u32)ioremap_nocache(BMDM_PWRMGR_BASE_ADDR,SZ_1K);
	return pwr_mgr_debug_init(bmdm_pwr_mgr_base);
}

late_initcall(rhea_pwr_mgr_late_init);

#endif

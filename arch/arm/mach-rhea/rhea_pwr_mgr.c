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
			/*event_id				trig_type		modem		arm_core 	arm_sub		aon		hub		mm*/
	{	SOFTWARE_0_EVENT,			PM_TRIG_BOTH_EDGE,		1,		1, 		5,		5,		5,		1, },
	{	SOFTWARE_1_EVENT,			PM_TRIG_NONE,			1,		1,		1,		1,		1,		1,	},
	{	SOFTWARE_2_EVENT,			PM_TRIG_BOTH_EDGE,		1,		5,		4,		4,		4,		1,	},
	{	MODEMBUS_ACTIVE_EVENT, 			PM_TRIG_POS_EDGE,		1,		1,		5,		5,		5,		1,	},
	{	VREQ_NONZERO_PI_MODEM_EVENT,		PM_TRIG_POS_EDGE,		5,		1,		1,		5,		5,		1,	},
	{	COMMON_INT_TO_AC_EVENT,			PM_TRIG_POS_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	COMMON_TIMER_1_EVENT,			PM_TRIG_POS_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	UBRX_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	UB2RX_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	SIMDET_EVENT,				PM_TRIG_BOTH_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	SIM2DET_EVENT,				PM_TRIG_BOTH_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R0_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R1_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R2_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R3_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R4_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R5_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R6_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	KEY_R7_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	BATRM_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	GPIO29_A_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	GPIO71_A_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	MMC1D1_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	GPIO74_A_EVENT,				PM_TRIG_BOTH_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	GPIO111_A_EVENT,			PM_TRIG_POS_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	DBR_IRQ_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	},
	{	ACI_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,		1,	}

};

static const struct i2c_cmd i2c_cmd[] = {
							{REG_ADDR,0},
							{JUMP_VOLTAGE,0},
							{REG_ADDR,0},		//0 - NOP
							{SET_PC_PINS,0xC0},	//other: 1 - Set PC3/4 pins to 0 to begin transaction (HW has semaphore)
							{REG_ADDR,0},		//2 - NOP
							{REG_ADDR,0x20},	//3 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x0B},	//4 - Set Start condition - write 3 to CS register
							{WAIT_TIMER,0x80},	//5 - Wait..
							{REG_DATA,1},  		//6 - Clear Start Condition - write 1 to CS
							{WAIT_TIMER,0x80},	//7 - Wait..
							{I2C_DATA,0x10},	//8 - PMU client addr  - 8 ..write to FIFO
							{WAIT_TIMER,0x80},	//9 - Wait..
							{I2C_DATA,0xC0},	//10 - PMU register addr C0 (CSRSTRL1)
							{WAIT_TIMER,0x80},	//11 - Wait..
							{I2C_VAR,0},		//12 - Data - Write the requested voltage
							{WAIT_TIMER,0x80},	//13 - Wait..
							{SET_PC_PINS,0xCC},	//14 - Set PC3/4 to 1 to signal End of transaction
							{END,0},			//15 - End
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
	struct pm_policy_cfg cfg;
	cfg.ac = 1;
	cfg.atl = 0;

	v_ptr.other_ptr = 2;
	v_ptr.set2_val = 1; /*Retention voltage inx*/
	v_ptr.set2_ptr = 45;
	v_ptr.set1_val = 2;/*Should be 8 ????Wakeup override*/
	v_ptr.set1_ptr = 49;
	v_ptr.zerov_ptr = 45; /*Not used for Rhea*/

	pwr_mgr_init(&rhea_pwr_mgr_info);
	rhea_pi_mgr_init();

	/*MM override is not set by default*/
	pwr_mgr_pi_set_wakeup_override(PI_MGR_PI_ID_MM,false/*clear*/);

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

	rhea_clock_init();


	for(i = 0; i < PI_MGR_PI_ID_MODEM;i++)
	{
		pi = pi_mgr_get(i);
		BUG_ON(pi == NULL);
		pi_init(pi);
	}
	/*All the initializations are done. Clear override bit here so that
	 * appropriate policies take effect*/
	for (i = 0; i < PI_MGR_PI_ID_MODEM;i++) {
	    pi = pi_mgr_get(i);
	    BUG_ON(pi == NULL);
	    pwr_mgr_pi_set_wakeup_override(pi->id,true/*clear*/);
	}

return 0;
}
early_initcall(rhea_pwr_mgr_init);

#ifdef CONFIG_DEBUG_FS

void pwr_mgr_mach_debug_fs_init(int type)
{
	static bool mux_init = false;
	u32 reg_val;

	if(!mux_init)
	{
		mux_init = true;
		 /*Get pad control write access by rwiting password */
		writel(0xa5a501, KONA_PAD_CTRL + PADCTRLREG_WR_ACCESS_OFFSET);
		/* unlock first 32 pad control registers */
		writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK0_OFFSET);

		/* Configure GPIO_XX to TESTPORT_XX  */
		/* writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO00_OFFSET); */
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO00_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO01_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO02_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO03_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO04_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO05_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO06_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO07_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO08_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO09_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO10_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO11_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO12_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO13_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO14_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO15_OFFSET);
		writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO16_OFFSET);
	}
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

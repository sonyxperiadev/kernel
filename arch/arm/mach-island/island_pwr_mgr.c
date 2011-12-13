/****************************************************************************
*									      
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

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
#endif

#include <linux/i2c.h>
#include <linux/i2c-kona.h>

#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#endif

#define VLT_LUT_SIZE	16

#ifdef CONFIG_DEBUG_FS
const char* _island__event2str[] =
{
    __stringify(LCDTE_EVENT),
    __stringify(SSP3SYN_EVENT),	
    __stringify(SSP3DI_EVENT),   
    __stringify(SSP3CK_EVENT),
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
    __stringify(UBRX_EVENT),		//0X10
    __stringify(UBCTSN_EVENT),
    __stringify(UB2RX_EVENT),
    __stringify(UB2CTSN_EVENT),
    __stringify(UB3RX_EVENT),
    __stringify(UB3CTSN_EVENT),
    __stringify(UB4RX_EVENT),
    __stringify(UB4CTSN_EVENT),
    __stringify(SIMDET_EVENT),
    __stringify(SIM2DET_EVENT),
    __stringify(MMC0D3_EVENT),
    __stringify(MMC0D1_EVENT),
    __stringify(MMC1D3_EVENT),
    __stringify(MMC1D1_EVENT),
    __stringify(SDDAT3_EVENT),
    __stringify(SDDAT1_EVENT),	
    __stringify(SLB1CLK_EVENT),   //0X20
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
    __stringify(USBDP_EVENT),  //0X30
    __stringify(USBDN_EVENT),
    __stringify(RXD0_EVENT),
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
    __stringify(GPIO95_A_EVENT), //0X40
    __stringify(GPIO96_A_EVENT),
    __stringify(GPIO99_A_EVENT),
    __stringify(GPIO100_A_EVENT),	
    __stringify(GPIO111_A_EVENT),
    __stringify(GPIO49_A_EVENT),
    __stringify(GPIO141_A_EVENT),
    __stringify(GPIO142_A_EVENT),
    __stringify(GPIO143_A_EVENT),
    __stringify(GPIO144_A_EVENT),
    __stringify(GPIO156_A_EVENT),
    __stringify(SPARE1_A_EVENT),
    __stringify(SPARE2_A_EVENT),
    __stringify(SPARE3_A_EVENT),
    __stringify(SPARE4_A_EVENT),
    __stringify(SPARE5_A_EVENT),
    __stringify(SPARE6_A_EVENT),  //0X50
    __stringify(SPARE7_A_EVENT),
    __stringify(SPARE8_A_EVENT),
    __stringify(SPARE9_A_EVENT),
    __stringify(SPARE10_A_EVENT),
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
    __stringify(GPIO73_B_EVENT),  //0X60
    __stringify(GPIO74_B_EVENT),
    __stringify(GPIO95_B_EVENT),
    __stringify(GPIO96_B_EVENT),
    __stringify(GPIO99_B_EVENT),
    __stringify(GPIO100_B_EVENT),
    __stringify(GPIO111_B_EVENT),
    __stringify(GPIO49_B_EVENT),
    __stringify(GPIO141_B_EVENT),
    __stringify(GPIO142_B_EVENT),
    __stringify(GPIO143_B_EVENT),
    __stringify(GPIO144_B_EVENT),
    __stringify(GPIO156_B_EVENT),
    __stringify(SPARE1_B_EVENT),
    __stringify(SPARE2_B_EVENT),
    __stringify(SPARE3_B_EVENT),
    __stringify(SPARE4_B_EVENT),  //0X70
    __stringify(SPARE5_B_EVENT),
    __stringify(SPARE6_B_EVENT),  
    __stringify(SPARE7_B_EVENT),
    __stringify(SPARE8_B_EVENT),
    __stringify(SPARE9_B_EVENT),
    __stringify(SPARE10_B_EVENT),
    __stringify(COMMON_TIMER_0_EVENT),
    __stringify(COMMON_TIMER_1_EVENT),
    __stringify(COMMON_TIMER_2_EVENT),
    __stringify(COMMON_TIMER_3_EVENT),
    __stringify(COMMON_TIMER_4_EVENT),
    __stringify(MM_TIMER_EVENT),
    __stringify(COMMON_INT_TO_AC_EVENT),
    __stringify(COMMON_INT_TO_MM_EVENT),
    __stringify(TZCFG_INT_TO_AC_EVENT),
    __stringify(DMA_REQUEST_EVENT), //0X80
    __stringify(MODEM1_EVENT),
    __stringify(MODEM2_EVENT),
    __stringify(MODEM_UART_EVENT),
    __stringify(BRIDGE_TO_AC_EVENT),
    __stringify(BRIDGE_TO_MODEM_EVENT),
    __stringify(VREQ_NONZERO_PI_MODEM_EVENT),
    __stringify(USBOTG_EVENT),
    __stringify(GPIO_EXP_IRQ_EVENT),
    __stringify(DBR_IRQ_EVENT),
    __stringify(ACI_EVENT),
    __stringify(SOFTWARE_0_EVENT),
    __stringify(SOFTWARE_1_EVENT),
    __stringify(SOFTWARE_2_EVENT),
    __stringify(VPM_WAKEUP_EVENT),
    __stringify(ESW_WAKEUP_EVENT),
    __stringify(HDMI_WAKEUP_EVENT),  //0X90
    __stringify(SDIO2_CDN_EVENT),
    __stringify(SDIO3_CDN_EVENT),
    __stringify(ULPI1_EVENT),
    __stringify(ULPI2_EVENT),   //0X94 
};

#endif

struct pm_special_event_range island_special_event_list[] = {
	{GPIO29_A_EVENT, GPIO156_A_EVENT},
	{GPIO29_B_EVENT, GPIO156_B_EVENT},
	{KEY_R0_EVENT, KEY_R7_EVENT}
};

struct pwr_mgr_info island_pwr_mgr_info = {
	.num_pi = PI_MGR_PI_ID_MAX,
	.base_addr = KONA_PWRMGR_VA,
	.flags = PM_PMU_I2C,
	.special_event_list = &island_special_event_list,
	.num_special_event_range = ARRAY_SIZE(island_special_event_list),
};

struct island_event_table
{
	u32 event_id;
	u32 trig_type;
	u32 policy_modem;
	u32 policy_arm_core;
	u32 policy_arm_sub;
    u32 policy_hub_aon;
    u32 policy_hub_switchable;
   /* u32 policy_mm;*/
};

static const struct island_event_table event_table[] = {
			/*event_id				trig_type			modem	arm_core arm_sub	aon		hub		mm*/
	{	SOFTWARE_0_EVENT,			PM_TRIG_BOTH_EDGE,		1,		1, 		5,		5,		5,		 },
	{	SOFTWARE_1_EVENT,			PM_TRIG_NONE,			1,		1,		1,		1,		1,			},
	{	SOFTWARE_2_EVENT,			PM_TRIG_BOTH_EDGE,		1,		4,		4,		4,		4,			},
	{	VREQ_NONZERO_PI_MODEM_EVENT,PM_TRIG_POS_EDGE,		5,		1,		1,		5,		5,			},
	{	COMMON_INT_TO_AC_EVENT,		PM_TRIG_POS_EDGE,		1,		5,		5,		5,		5,			},
	{	COMMON_TIMER_1_EVENT,		PM_TRIG_POS_EDGE,		1,		4,		4,		4,		4,			},
	{	UBRX_EVENT,					PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	UB2RX_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	SIMDET_EVENT,				PM_TRIG_BOTH_EDGE,		1,		5,		5,		5,		5,			},
	{	SIM2DET_EVENT,				PM_TRIG_BOTH_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R0_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R1_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R2_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R3_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R4_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R5_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R6_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	KEY_R7_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	BATRM_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	GPIO29_A_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	GPIO71_A_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	MMC1D1_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	GPIO74_A_EVENT,				PM_TRIG_BOTH_EDGE,		1,		5,		5,		5,		5,			},
	{	GPIO111_A_EVENT,			PM_TRIG_POS_EDGE,		1,		5,		5,		5,		5,			},
	{	DBR_IRQ_EVENT,				PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			},
	{	ACI_EVENT,					PM_TRIG_NEG_EDGE,		1,		5,		5,		5,		5,			}

};

static const struct i2c_cmd i2c_cmd[] = {
							{REG_ADDR,0},		//0 - NOP
							{SET_PC_PINS,0xC0},	//other: 1 - Set PC3/4 pins to 0 to begin transaction (HW has semaphore)
							{REG_ADDR,0},		//2 - NOP
							{REG_ADDR,0x20},	//3 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x0B},	//4 - Set Start condition - write 3 to CS register
							{WAIT_TIMER,0x08},	//5 - Wait..
							{REG_DATA,1},  		//6 - Clear Start Condition - write 1 to CS
							{WAIT_TIMER,0x08},	//7 - Wait..
							{I2C_DATA,0x10},	//8 - PMU client addr  - 8 ..write to FIFO
							{WAIT_TIMER,0x08},	//9 - Wait..
							{I2C_DATA,0xC0},	//10 - PMU register addr C0 (CSRSTRL1)
							{WAIT_TIMER,0x08},	//11 - Wait..
							{I2C_VAR,0},		//12 - Data - Write the requested voltage
							{WAIT_TIMER,0x50},	//13 - Wait..
							{SET_PC_PINS,0xCC},	//14 - Set PC3/4 to 1 to signal End of transaction
							{END,0},			//15 - End
							{REG_ADDR,0},		//16 - NOP
							{REG_ADDR,0},		//17 - NOP
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



int island_pm_i2c_cmd_init(void)
{
  //  pwr_dbg("%s\n", __func__);
#if defined(CONFIG_MACH_ISLAND_BU) || defined(CONFIG_MACH_ISLAND_FF)
    //vi to vo
   writel(0x0000003f, KONA_PWRMGR_VA + PWRMGR_VI_TO_VO0_MAP_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_VI_TO_VO1_MAP_OFFSET);

   /* set the power manager I2C microcode for the FIRECHILD REGULATOR */
#ifdef CONFIG_MACH_ISLAND_BU
   writel(0x008f0000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET);
   writel(0x0010003c, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_02_OFFSET);
   writel(0x0019304c, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_03_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_04_OFFSET);
   writel(0x001f2024, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_05_OFFSET);
   writel(0x00020606, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_06_OFFSET);
   writel(0x00610103, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_07_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_08_OFFSET);
   writel(0x00601610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_09_OFFSET);
   writel(0x00000601, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_10_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_11_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_12_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_13_OFFSET);
   writel(0x006a2294, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_14_OFFSET);
   writel(0x006a2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_15_OFFSET);
   writel(0x006a2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_16_OFFSET);
   writel(0x000008f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_17_OFFSET);
   writel(0x00020000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_18_OFFSET);
   writel(0x00610105, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_19_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_20_OFFSET);
   writel(0x008f9610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_21_OFFSET);
   writel(0x00000700, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_22_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_23_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_24_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_25_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_26_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_27_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_28_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_29_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_30_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_31_OFFSET);
   writel(0x00700000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_32_OFFSET);

   // This is the voltage lookup table for FAIRCHILD
   writel(0x000B0C0E, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_OFFSET);
   writel(0x1315001B, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_02_OFFSET);
   writel(0x1C1E23E4, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_03_OFFSET);
   writel(0x2600292B, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_04_OFFSET);

   // This is the voltage lookup table for SET1/SET2 testing
   writel(0x01020304, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_05_OFFSET);
   writel(0x05060708, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_06_OFFSET);

   // This populates the voltage jump table
   writel(0x2685a69a, KONA_PWRMGR_VA + PWRMGR_VO0_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO0 cmd ptr
   writel(0x288628a2, KONA_PWRMGR_VA + PWRMGR_VO1_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO1 cmd ptr
   writel(0x6B16cb2c, KONA_PWRMGR_VA + PWRMGR_VO2_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO2 cmd ptr
#endif

#ifdef CONFIG_MACH_ISLAND_FF

   writel(0x008f0000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET);
   writel(0x0010003c, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_02_OFFSET);
   writel(0x0019304c, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_03_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_04_OFFSET);
   writel(0x001f2024, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_05_OFFSET);
   writel(0x00020606, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_06_OFFSET);
   writel(0x00610103, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_07_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_08_OFFSET);
   writel(0x00601610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_09_OFFSET);
   writel(0x00000601, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_10_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_11_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_12_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_13_OFFSET);
   writel(0x006a22C0, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_14_OFFSET);
   writel(0x006a2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_15_OFFSET);
   writel(0x006a2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_16_OFFSET);
   writel(0x000008f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_17_OFFSET);
   writel(0x00020000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_18_OFFSET);
   writel(0x00610105, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_19_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_20_OFFSET);
   writel(0x008f9610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_21_OFFSET);
   writel(0x00000700, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_22_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_23_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_24_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_25_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_26_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_27_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_28_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_29_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_30_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_31_OFFSET);
   writel(0x00700000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_32_OFFSET);
#if 0
   writel(0x008F0000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET);
   writel(0x0010003C, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_02_OFFSET);
   writel(0x0019304C, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_03_OFFSET);
   writel(0X00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_04_OFFSET);
   writel(0x001F2024, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_05_OFFSET);
   writel(0x00020606, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_06_OFFSET);
   writel(0x00610103, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_07_OFFSET);
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_08_OFFSET);
   writel(0x00601610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_09_OFFSET);
   writel(0x00E00601, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_10_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_11_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_12_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_13_OFFSET);
   writel(0x006A2294, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_14_OFFSET);
   writel(0X006A2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_15_OFFSET);
   writel(0x006a2300,  KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_16_OFFSET);
   writel(0x00f388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_17_OFFSET);
   writel(0x006a2290, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_18_OFFSET);
   writel(0x006a2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_19_OFFSET);
   writel(0x006a2300,  KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_20_OFFSET);
   writel(0x00f388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_21_OFFSET);
   writel(0x006a2290, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_22_OFFSET);
   writel(0x006a2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_23_OFFSET);
   writel(0x00f388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_24_OFFSET);
   writel(0x006a2290, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_25_OFFSET);
   writel(0X006A2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_26_OFFSET);
   writel(0x00f388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_27_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_28_OFFSET);
   writel(0x00105020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_29_OFFSET);
   writel(0x00020610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_30_OFFSET);
   writel(0x00610101, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_31_OFFSET);
   writel(0x007008f9, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_32_OFFSET);
#endif
      // This is the voltage lookup table for MAX8649
   writel(0x002f2f2f, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_OFFSET);
   writel(0x2f2f2f2f, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_02_OFFSET);
   writel(0x2f2f2f2f, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_03_OFFSET);
   writel(0x2F2f2f2f, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_04_OFFSET);

   // This is the voltage lookup table for SET1/SET2 testing
   writel(0x01020304, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_05_OFFSET);
   writel(0x05060708, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_06_OFFSET);

   // This populates the voltage jump table
   writel(0x26860514, KONA_PWRMGR_VA + PWRMGR_VO0_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO0 cmd ptr
   writel(0x298669A6, KONA_PWRMGR_VA + PWRMGR_VO1_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO1 cmd ptr
   writel(0x6B16CB2C, KONA_PWRMGR_VA + PWRMGR_VO2_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO2 cmd ptr
#endif   

#else
   writel(0x0000003f, KONA_PWRMGR_VA + PWRMGR_VI_TO_VO0_MAP_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_VI_TO_VO1_MAP_OFFSET);

   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET);
   writel(0x00700602, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_02_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_03_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_04_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_05_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_06_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_07_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_08_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_09_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_10_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_11_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_12_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_13_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_14_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_15_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_16_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_17_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_18_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_19_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_20_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_21_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_22_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_23_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_24_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_25_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_26_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_27_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_28_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_29_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_30_OFFSET);
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_31_OFFSET);
   writel(0x00700000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_32_OFFSET);

#endif
#if 0
   /* set the power manager I2C microcode for the MAX8649 PMU */

   /* NOTE: The code below is auto-generated */
   // This is the I2C sequencer code
   writel(0x008f0000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET);  // NOP THEN Set PC pins to 0000 to show start of sequence
   writel(0x0010003c, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_02_OFFSET);  // Select FIFO control THEN Disable TX FIFO
   writel(0x0019304c, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_03_OFFSET);  // Enable clocks in i2c mm hs (address) THEN Enable clocks in i2c mm hs (data)
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_04_OFFSET);  // Enable BSC Hardware (address) THEN Enable BSC Hardware (data)
   writel(0x001f2024, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_05_OFFSET);  // Set Address for i2cmmhs control register THEN Enable transfer
   writel(0x00020606, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_06_OFFSET);  // Wait THEN Set Address for i2cmmhs control register
   writel(0x00610103, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_07_OFFSET);  // Set Start condition THEN Wait 10uS
   writel(0x00101020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_08_OFFSET);  // Set Address for i2cmmhs control register THEN Clear Start Condition
   writel(0x002C0610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_09_OFFSET);  // Wait 10uS THEN PMU ID = C0, Write Command
   writel(0x00E006a2, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_10_OFFSET);  // Wait 100uS THEN Jump based on voltage request
   writel(0x006a2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_11_OFFSET);  // Write Address for CSR voltage control reg (MODE0) THEN Wait 100uS
   writel(0x006a2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_12_OFFSET);  // Data - Write the requested voltage THEN Wait 100uS
   writel(0x00F388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_13_OFFSET);  // Set PC pins to 0001 to show write finished THEN Jump - set stop and exit
   writel(0x006a2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_14_OFFSET);  // Write Address for CSR voltage control reg (MODE0) THEN Wait 100uS
   writel(0x006a2306, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_15_OFFSET);  // Data - Write the requested voltage (index 16 - VO1_SET2) THEN Wait 100uS
   writel(0x00F388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_16_OFFSET);  // Set PC pins to 0001 to show write finished THEN Jump - set stop and exit
   writel(0x006a2200, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_17_OFFSET);  // Write Address for CSR voltage control reg (MODE0) THEN Wait 100uS
   writel(0x006a230C, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_18_OFFSET);  // Data - Write the requested voltage (index 17 - VO1_SET1) THEN Wait 100uS
   writel(0x00F388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_19_OFFSET);  // Set PC pins to 0001 to show write finished THEN Jump - set stop and exit
   writel(0x006a2201, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_20_OFFSET);  // Write Address for CSR voltage control reg (MODE1) THEN Wait 100uS
   writel(0x006a2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_21_OFFSET);  // Data - Write the requested voltage THEN Wait 100uS
   writel(0x00F388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_22_OFFSET);  // Set PC pins to 0001 to show write finished THEN Jump - set stop and exit
   writel(0x006a2203, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_23_OFFSET);  // Write Address for CSR voltage control reg (MODE3) THEN Wait 100uS
   writel(0x006a2300, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_24_OFFSET);  // Data - Write the requested voltage THEN Wait 100uS
   writel(0x00F388f1, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_25_OFFSET);  // Set PC pins to 0001 to show write finished THEN Jump - set stop and exit
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_26_OFFSET);  // NOP THEN NOP
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_27_OFFSET);  // NOP THEN NOP
   writel(0x00000000, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_28_OFFSET);  // NOP THEN NOP
   writel(0x00105020, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_29_OFFSET);  // Set Address for i2cmmhs control register THEN Set Stop condition
   writel(0x00020610, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_30_OFFSET);  // Wait 10uS THEN Set Address for i2cmmhs control register
   writel(0x00610101, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_31_OFFSET);  // Clear Stop condition THEN Wait 10uS for STOP clear
   writel(0x007008f9, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_32_OFFSET);  // Set PC pins to 1001 to show end of sequence THEN END - end of standard transaction

   // This is the voltage lookup table for MAX8649
   writel(0x000D0F11, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_OFFSET);
   writel(0x17191B21, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_02_OFFSET);
   writel(0x23252B2D, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_03_OFFSET);
   writel(0x2F313335, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_04_OFFSET);

   // This is the voltage lookup table for SET1/SET2 testing
   writel(0x01020304, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_05_OFFSET);
   writel(0x05060708, KONA_PWRMGR_VA + PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_06_OFFSET);

   // This populates the voltage jump table
   writel(0x26860514, KONA_PWRMGR_VA + PWRMGR_VO0_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO0 cmd ptr
   writel(0x298669A6, KONA_PWRMGR_VA + PWRMGR_VO1_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO1 cmd ptr
   writel(0x6B16CB2C, KONA_PWRMGR_VA + PWRMGR_VO2_SPECIFIC_I2C_COMMAND_POINTER_OFFSET); // VO2 cmd ptr
#endif

  return 0;

} 

int __init island_pwr_mgr_init()
{

	struct v0x_spec_i2c_cmd_ptr v_ptr;
	int i;
	struct pi* pi;
	struct pm_policy_cfg cfg;
	cfg.ac = 1;
	cfg.atl = 0;

	v_ptr.other_ptr = 1;
	v_ptr.set2_val = 1; /*Retention voltage inx*/
	v_ptr.set2_ptr = 45;
	v_ptr.set1_val = 2;/*Should be 8 ????Wakeup override*/
	v_ptr.set1_ptr = 49;
	v_ptr.zerov_ptr = 45; /*Not used for island*/
#if 0
	/*disable jtag clock instrusive during debug*/
	pwr_mgr_ignore_dap_powerup_request(true);
#endif
	pwr_mgr_init(&island_pwr_mgr_info);
	island_pi_mgr_init();

	/*MM override is not set by default*/
	//pwr_mgr_pi_set_wakeup_override(PI_MGR_PI_ID_MM,false/*clear*/);

		/*clear all the event */
	pwr_mgr_event_clear_events(LCDTE_EVENT, EVENT_ID_ALL);

	pwr_mgr_event_set(SOFTWARE_2_EVENT,1);
	pwr_mgr_event_set(SOFTWARE_0_EVENT,1);

		/*Init I2c seq*/
	pwr_mgr_pm_i2c_enable(false);
	/*Program I2C sequencer*/
	island_pm_i2c_cmd_init();
	//pwr_mgr_pm_i2c_cmd_write(i2c_cmd,ARRAY_SIZE(i2c_cmd));
	/*Program voltage lookup table
	AVS driver may chnage this later*/
	//pwr_mgr_pm_i2c_var_data_write(pwrmgr_default_volt_lut,VLT_LUT_SIZE);
	/*populate the jump voltage table */
	//pwr_mgr_set_v0x_specific_i2c_cmd_ptr(VOLT0,&v_ptr);

	pwr_mgr_pm_i2c_enable(true);

	/*Init event table*/
	for(i = 0; i < ARRAY_SIZE(event_table);i++)
	{
		u32 event_id;

		event_id = event_table[i].event_id;

		if (event_id >= GPIO29_A_EVENT && event_id <= SPARE10_A_EVENT)
			event_id = GPIO29_A_EVENT;

		if (event_id >= GPIO29_B_EVENT && event_id <= SPARE10_B_EVENT)
			event_id = GPIO29_B_EVENT;


		pwr_mgr_event_trg_enable(event_table[i].event_id,event_table[i].trig_type);

		cfg.policy = event_table[i].policy_modem;
		pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_MODEM, &cfg);

		cfg.policy = event_table[i].policy_arm_core;
		pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_ARM_CORE, &cfg);

		cfg.policy = event_table[i].policy_arm_sub;
		pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_ARM_SUB_SYSTEM, &cfg);

		cfg.policy = event_table[i].policy_hub_aon;
		pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_HUB_AON, &cfg);

		cfg.policy = event_table[i].policy_hub_switchable;
		pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_HUB_SWITCHABLE, &cfg);

	}
	/*Init all PIs*/
/*Init all PIs*/
	for(i = 0; i < PI_MGR_PI_ID_MODEM;i++)
	{
		pi = pi_mgr_get(i);
		BUG_ON(pi == NULL);
		pi_init(pi);
	}

	island_clock_init();

	/*All the initializations are done. Clear override bit here so that
	 * appropriate policies take effect*/
#if 1
	for (i = 0; i < PI_MGR_PI_ID_MODEM;i++) {
	    pi = pi_mgr_get(i);
	    BUG_ON(pi == NULL);
	    pi_init_state(pi);
	}
#endif
return 0;
}
early_initcall(island_pwr_mgr_init);
#ifdef CONFIG_DEBUG_FS

void pwr_mgr_mach_debug_fs_init(int type)
{
	static bool mux_init = false;
	u32 reg_val;

	if(!mux_init)
	{

		/* Configure GPIO_XX to TESTPORT_XX  */
   /* writel(0x503, KONA_PAD_CTRL + PADCTRLREG_GPIO00_OFFSET); */
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_CEN_0_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_CEN_1_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_RDY_0_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_RDY_1_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_CLE_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_ALE_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_OEN_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_WEN_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_7_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_6_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_5_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_4_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_3_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_2_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_1_OFFSET);
    writel(0x503, KONA_PAD_CTRL + CHIPREG_NAND_AD_0_OFFSET);
	}	
	reg_val = readl(KONA_CHIPREG_VA+CHIPREG_PERIPH_SPARE_REG1_OFFSET);
	reg_val &= ~0x00F0000;
	if(type == 0)
		reg_val |= 0x1 << 16;
	else if(type == 1)
		reg_val |= 0xD << 16;
	else
		BUG();
	writel(reg_val,KONA_CHIPREG_VA+CHIPREG_PERIPH_SPARE_REG1_OFFSET);
}

int __init island_pwr_mgr_late_init(void)
{
	u32 bmdm_pwr_mgr_base = (u32)ioremap_nocache(BMDM_PWRMGR_BASE_ADDR,SZ_1K);
	
	return pwr_mgr_debug_init(bmdm_pwr_mgr_base);
}

late_initcall(island_pwr_mgr_late_init);

#endif
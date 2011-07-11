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

#include<mach/pi_mgr.h>
#include<mach/pwr_mgr.h>
#include<plat/pwr_mgr.h>

struct pwr_mgr_info rhea_pwr_mgr_info = {
	.num_pi = PI_MGR_PI_ID_MAX,
	.base_addr = KONA_PWRMGR_VA,
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
							{SET_PC_PINS,0xC0},	//14 - Set PC3/4 to 1 to signal End of transaction 
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

/*Look up takbe for 59055 PMU*/
static const u8 voltage_lookup[] = {
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
												  

void rhea_pwr_mgr_init()
{
	struct v0x_spec_i2c_cmd_ptr v_ptr;
	v_ptr.other_ptr = 1;
	v_ptr.set2_val = 1; /*Retention voltage inx*/
	v_ptr.set2_ptr = 45;
	v_ptr.set1_val = 2;/*Should be 8 ????Wakeup override*/
	v_ptr.set1_ptr = 49;
	v_ptr.zerov_ptr = 45; /*Not used for Rhea*/
	
	pwr_mgr_init(&rhea_pwr_mgr_info);
	/*Done in two steps to skip DUMMY_EVENT*/
	pwr_mgr_event_clear_events(LCDTE_EVENT,VREQ_NONZERO_PI_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT,EVENT_ID_ALL);
	
	/*Init I2c seq*/
	pwr_mgr_pm_i2c_enable(false);
	/*Program I2C sequencer*/
	pwr_mgr_pm_i2c_cmd_write(i2c_cmd,ARRAY_SIZE(i2c_cmd));
	/*Program voltage lookup table*/
	pwr_mgr_pm_i2c_var_data_write(voltage_lookup,ARRAY_SIZE(voltage_lookup));
	/*populate the jump voltage table */
	pwr_mgr_set_v0x_specific_i2c_cmd_ptr(VOLT0,&v_ptr);
	pwr_mgr_pm_i2c_enable(true);
	
}
EXPORT_SYMBOL(rhea_pwr_mgr_init);

/*****************************************************************************
*
* Power Manager config parameters for Rhea platform
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <linux/io.h>
#include<plat/pi_mgr.h>
#include<mach/pi_mgr.h>
#include<mach/pwr_mgr.h>
#include<plat/pwr_mgr.h>
#include "pm_params.h"

#ifdef CONFIG_KONA_POWER_MGR

#ifdef CONFIG_KONA_PWRMGR_REV2

#if 0 /*B0 Seq with PMU read/write support - not in use as of now*/
static struct i2c_cmd i2c_cmd[] =
					{
							{REG_ADDR,0},		//0 - NOP
							{JUMP_VOLTAGE,0},	//1 - jump to address based on current voltage request
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
							{REG_ADDR,0},		//18 - NOP   - SW SEQ read start
							{REG_ADDR,0x20},	//19 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x03},	//20 - Set Start condition - write 3 to CS register
							{WAIT_TIMER,0x10},	//21 - Wait..
							{REG_DATA,1},  		//22 - Clear Start Condition - write 1 to CS
							{WAIT_TIMER,0x08},	//23 - Wait..
							{I2C_DATA,0x10},	//24 - PMU Read client addr offset
							{WAIT_TIMER,0x80},	//25 - Wait..
							{I2C_DATA,0x00},	//26 - PMU Read register addr offset
							{WAIT_TIMER,0x80},	//27 - Wait..
							{REG_ADDR,0x20},	//28 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x07},	//29 - Send Read command
							{WAIT_TIMER,0x10},	//30 - Wait..
							{REG_DATA,0},  		//31 - NOACTION - write 1 to CS
							{WAIT_TIMER,0x08},	//32 - Wait..
							{I2C_DATA,0x11},	//33 - PMU Read client addr offset
							{WAIT_TIMER,0x80},	//34 - Wait..
							{READ_FIFO,0},		//35 -  read FIFO
							{SET_READ_DATA,0},	//36 - Copy read byte to PWRMGR register
							{END,0},			//37 - NOP  - Read seq end
							{REG_ADDR,0},		//38 - NOP - write seq start
							{REG_ADDR,0x20},	//39 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x03},	//40 - Set Start condition - write 3 to CS register
							{WAIT_TIMER,0x10},	//41 - Wait..
							{REG_DATA,1},  		//42 - Clear Start Condition - write 1 to CS
							{WAIT_TIMER,0x08},	//43 - Wait..
							{I2C_DATA,0x10},	//44 - PMU write client addr offset
							{WAIT_TIMER,0x80},	//45 - Wait..
							{I2C_DATA,0x00},	//46 - PMU write register addr offset
							{WAIT_TIMER,0x80},	//47 - Wait..
							{I2C_DATA,0xC0},	//48 - PMU write register data offset
							{WAIT_TIMER,0x80},	//49 - Wait..
							{END,0},			//50 - NOP  - write seq end
							{REG_ADDR,0},		//51 - NOP
							{REG_ADDR,0},		//set2/zero:52 - NOP
							{SET_PC_PINS,0x30},	//53 - Set PC1 pins to 0
							{REG_ADDR,0},		//54 -NOP
							{END,0},			//55 -END
							{REG_ADDR,0},		//set1:56 -NOP
							{SET_PC_PINS,0x31},	//57 - Set PC1 pins to 1
							{REG_ADDR,0},		//58 -NOP
							{END,0},			//59 -END
							{REG_ADDR,0},		//60 - NOP
							{REG_ADDR,0},		//61 - NOP
							{REG_ADDR,0},		//62 - NOP
							{REG_ADDR,0},		//63 - NOP

						  };
#endif
#endif

#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
#ifdef CONFIG_KONA_PWRMGR_REV2
	#define RESTART_DELAY 		1
	#define WRITE_DELAY	  		3
	#define VLT_CHANGE_DELAY	0x20
#else
	#define RESTART_DELAY 		6
	#define WRITE_DELAY	  		6
	#define VLT_CHANGE_DELAY	0x28
#endif /*CONFIG_KONA_PWRMGR_REV2*/
#else
	#define RESTART_DELAY 		0x10
	#define WRITE_DELAY	  		0x80
	#define VLT_CHANGE_DELAY	0x80
#endif /*CONFIG_KONA_PMU_BSC_HS_MODE*/

static struct i2c_cmd i2c_cmd[] =
					{
							{REG_ADDR,0},			//0 - NOP
							{JUMP_VOLTAGE,0},		//1 - jump to address based on current voltage request
							{REG_ADDR,0},			//other:2 - NOP
							{SET_PC_PINS,0xC0},		//3 - Set PC3/4 pins to 0 to begin transaction (HW has semaphore)
							{REG_ADDR,0},			//4 - {REG_ADDR,0x5C},change the oeb of BSC pins
							{REG_ADDR,0},			//5 - {REG_DATA,0x80},BSC CK pin enabled
							{REG_ADDR,0x20},		//6 - Set Address for i2cmmhs BSC_CS register
							{REG_DATA,0x0b},		//7 - Set Start condition - write 3 to CS register
							{WAIT_TIMER,RESTART_DELAY},	//8 - Wait..
							{REG_DATA,1},  			//9 - Clear Start Condition - write 1 to CS
							{WAIT_TIMER,WRITE_DELAY},	//10 - Wait..
							{I2C_DATA,0x10},		//11 - PMU client addr  - 8 ..write to FIFO
							{WAIT_TIMER,WRITE_DELAY},	//12 - Wait..
							{I2C_DATA,0xC0},		//13 - PMU register addr C0 (CSRSTRL1)
							{WAIT_TIMER,WRITE_DELAY},	//14 - Wait..
							{I2C_VAR,0},			//15 - Data - Write the requested voltage
							{WAIT_TIMER,VLT_CHANGE_DELAY},	//16 - Wait..
							{REG_ADDR,0},			//17 - {REG_ADDR,0x5C},change the oeb of BSC pins
							{REG_ADDR,0},			//18- {REG_DATA,0x0C},BSC CK pin disabled
							{SET_PC_PINS,0xCC},		//19 - Set PC3/4 to 1 to signal End of transaction
							{END,0},			//20 - End
							{REG_ADDR,0},			//21 - NOP
							{REG_ADDR,0},			//22 - NOP
							{REG_ADDR,0},			//23 - NOP
							{REG_ADDR,0},			//24 - NOP
							{REG_ADDR,0},			//25 - NOP
							{REG_ADDR,0},			//26 - NOP
							{REG_ADDR,0},			//27 - NOP
							{REG_ADDR,0},			//28 - NOP
							{REG_ADDR,0},			//29 - NOP
							{REG_ADDR,0},			//30 - NOP
							{REG_ADDR,0},			//31 - NOP
							{REG_ADDR,0},			//32 - NOP
							{REG_ADDR,0},			//33 - NOP
							{REG_ADDR,0},			//34 - NOP
							{REG_ADDR,0},			//35 - NOP
							{REG_ADDR,0},			//36 - NOP
							{REG_ADDR,0},			//37 - NOP
							{REG_ADDR,0},			//38 - NOP
							{REG_ADDR,0},			//39 - NOP
							{REG_ADDR,0},			//40 - NOP
							{REG_ADDR,0},			//41 - NOP
							{REG_ADDR,0},			//42 - NOP
							{REG_ADDR,0},			//43 - NOP
							{REG_ADDR,0},			//44 - NOP
							{REG_ADDR,0},			//45 - NOP
							{REG_ADDR,0},			//46 - NOP
							{REG_ADDR,0},			//47 - NOP
							{REG_ADDR,0},			//48 - NOP
							{REG_ADDR,0},			//49 - NOP
							{REG_ADDR,0},			//50 - NOP
							{REG_ADDR,0},			//51 - NOP
							{REG_ADDR,0},			//set2/zero:52 - NOP
							{SET_PC_PINS,0x30},		//53 - Set PC1 pins to 0
							{REG_ADDR,0},			//54 - {REG_ADDR,0x5C},change the oeb of BSC pins
							{REG_ADDR,0},			//55 - {REG_DATA,0x04},BSC CK pin disabled
							{END,0},			//56 -END
							{REG_ADDR,0},			//set1:57 -NOP
							{SET_PC_PINS,0x31},		//58 - Set PC1 pins to 1
							{REG_ADDR,0},			//59 - {REG_ADDR,0x5C},change the oeb of BSC pins
							{REG_ADDR,0},			//60 - {REG_DATA,0x00},BSC CK pin enabled
							{END,0},			//61 -END
							{REG_ADDR,0},			//62 - NOP
							{REG_ADDR,0},			//63 - NOP

						  };
#if 0 /*Dummy seq*/
static struct i2c_cmd i2c_cmd[] = {
							{REG_ADDR,0},		//0 - NOP
							{JUMP_VOLTAGE,0},	//1 - jump to address based on current voltage request
							{REG_ADDR,0},		//other:2 - NOP
							{SET_PC_PINS,0xC0},	//3 - Set PC3/4 pins to 0 to begin transaction (HW has semaphore)
							{REG_ADDR,0},		//4 - NOP
							{REG_ADDR,0},	//5 - Set Address for i2cmmhs BSC_CS register
							{REG_ADDR,0},	//6 - Set Start condition - write 3 to CS register
							{REG_ADDR,0},	//7 - Wait..
							{REG_ADDR,0},  		//8 - Clear Start Condition - write 1 to CS
							{REG_ADDR,0},	//9 - Wait..
							{REG_ADDR,0},	//10 - PMU client addr  - 8 ..write to FIFO
							{REG_ADDR,0},	//11 - Wait..
							{REG_ADDR,0},	//12 - PMU register addr C0 (CSRSTRL1)
							{REG_ADDR,0},	//13 - Wait..
							{REG_ADDR,0},		//14 - Data - Write the requested voltage
							{REG_ADDR,0},	//15 - Wait..
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
							{REG_ADDR,0},		//45 - NOP
							{REG_ADDR,0},		//46 - NOP
							{REG_ADDR,0},		//47 - NOP
							{REG_ADDR,0},		//48 - NOP
							{REG_ADDR,0},		//49 - NOP
							{REG_ADDR,0},		//50 - NOP
							{REG_ADDR,0},		//51 - NOP
							{REG_ADDR,0},		//set2/zero:52 - NOP
							{SET_PC_PINS,0x30},	//53 - Set PC1 pins to 0
							{REG_ADDR,0},		//54 -NOP
							{END,0},			//55 -END
							{REG_ADDR,0},		//set1:56 -NOP
							{SET_PC_PINS,0x31},	//57 - Set PC1 pins to 1
							{REG_ADDR,0},		//58 -NOP
							{END,0},			//59 -END
							{REG_ADDR,0},		//60 - NOP
							{REG_ADDR,0},		//61 - NOP
							{REG_ADDR,0},		//62 - NOP
							{REG_ADDR,0},		//63 - NOP

						  };

#endif
/*Default voltage lookup table
Need to move this to board-file
*/
static u8 pwrmgr_default_volt_lut[] =
									{
										PMU_SCR_VLT_TBL_SS
									};


static struct v0x_spec_i2c_cmd_ptr v0_ptr =
		{
			.other_ptr = 2,
			.set2_val = 1, /*Retention voltage inx*/
			.set2_ptr = 52,
			.set1_val = 2,/*wakeup from retention voltage inx*/
			.set1_ptr = 57,
			.zerov_ptr = 52, /*Not used for Rhea*/
		};

struct pwrmgr_init_param pwrmgr_init_param =
	{
		.cmd_buf = i2c_cmd,
		.cmb_buf_size = ARRAY_SIZE(i2c_cmd),
		.v0ptr = &v0_ptr,
		.def_vlt_tbl = pwrmgr_default_volt_lut,
		.vlt_tbl_size = ARRAY_SIZE(pwrmgr_default_volt_lut),
#if defined(CONFIG_KONA_PWRMGR_REV2)
#if 0 /*Need to enable only if SW seq is enabled*/
		.i2c_rd_off = 18,
		.i2c_rd_slv_addr_off = 24,
		.i2c_rd_reg_addr_off = 26,
		.i2c_wr_off = 38,
		.i2c_wr_slv_addr_off = 44,
		.i2c_wr_reg_addr_off = 46,
		.i2c_wr_val_addr_off = 48,
		.i2c_seq_timeout = 100,
#endif
#endif
	};

#endif /*CONFIG_KONA_POWER_MGR*/


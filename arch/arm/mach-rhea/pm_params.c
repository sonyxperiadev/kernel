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


/*JIRA workaround flag and sysfs definitions
These flags can be used to enable/disable JIRA workaround at runtime
*/
#ifdef CONFIG_RHEA_WA_HWJIRA_2531
DEFINE_JIRA_WA_RO_FLG(2531, 1);
#endif

#ifdef CONFIG_RHEA_WA_CRMEMC_919
DEFINE_JIRA_WA_FLG(919, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
DEFINE_JIRA_WA_FLG(2221, 0); /*Disable by default*/
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2490
DEFINE_JIRA_WA_RO_FLG(2490, 1);
#endif


#ifdef CONFIG_KONA_POWER_MGR

#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
	#define START_CMD			0xb
	#define START_DELAY			6
	#define WRITE_DELAY	  		6
	#define VLT_CHANGE_DELAY		0x25
#else
	#define START_CMD			0x3
	#define START_DELAY			0x10
	#define WRITE_DELAY	  		0x80
	#define VLT_CHANGE_DELAY		0x80
#endif /*CONFIG_KONA_PMU_BSC_HS_MODE*/
#define PMU_SLAVE_ID				0x8
#define PMU_CSR_REG_ADDR			0xC0
#define READ_DELAY				0x20
/**
 * PMU BSC Registers and masks used
 * by the the sequencer code
 */
#define PMU_BSC_INT_STATUS_REG			(0x48)
#define PMU_BSC_INT_STATUS_MASK			(0xFF)

static struct i2c_cmd i2c_cmd[] = {
	{REG_ADDR, 0},		/* 0:NOP */
	{JUMP_VOLTAGE, 0},	/* 1:jump to address based on voltage req */
	{REG_ADDR, 0},		/* 2:NOP */
	{SET_PC_PINS, 0xC0},	/* 3:Set PC3/4 pins to 0 to begin transaction*/
	{REG_ADDR, 0},		/* 4:NOP */
	{REG_ADDR, 0x20},	/* 5:Set Address for BSC_CS register */
	{REG_DATA, START_CMD},	/* 6:Start condition:write 3 to CS register */
	{WAIT_TIMER, START_DELAY},	/* 7:Wait.. */
	{REG_DATA, 1},			/* 8:Clear Start Condition */
	{WAIT_TIMER, WRITE_DELAY},	/* 9:Wait.. */
	{I2C_DATA, (PMU_SLAVE_ID << 1)},/* 10:Write PMU slave addr to I2C */
	{WAIT_TIMER, WRITE_DELAY},	/* 11:Wait.. */
	{I2C_DATA, PMU_CSR_REG_ADDR},	/* 12:PMU register addr CSRSTRL1 */
	{WAIT_TIMER, WRITE_DELAY},	/* 13:Wait.. */
	{I2C_VAR, 0},		/* 14:Data:Write the requested voltage */
	{WAIT_TIMER, VLT_CHANGE_DELAY},	/* 15:Wait.. */
	{SET_PC_PINS, 0xCC},/* 16:Set PC3/4 to 1 to signal end of transaction */
	{END, 0},		/* 17:End */
	{REG_ADDR, 0},		/* 18:NOP:SW SEQ read start - oeb - TBD */
	{REG_ADDR, 0},		/* 19:NOP:SW SEQ read start - oeb - TBD */
	{REG_ADDR, 0x20},	/* 20:Set Address for BSC_CS register */
	{REG_DATA, START_CMD},	/* 21:Set Start condition */
	{WAIT_TIMER, START_DELAY},	/* 22:Wait.. */
	{REG_DATA, 1},		/* 23:Clear Start Condition */
	{WAIT_TIMER, WRITE_DELAY},	/* 24:Wait.. */
	{I2C_DATA, 0x10},		/* 25:PMU Read client addr offset */
	{WAIT_TIMER, WRITE_DELAY},	/* 26:Wait.. */
	{I2C_DATA, 0x00},		/* 27:PMU Read register addr offset */
	{WAIT_TIMER, WRITE_DELAY},	/* 28:Wait.. */
	{REG_ADDR, 0x20},	/* 29:Set Address for PMU_BSC CS register */
	{REG_DATA, START_CMD},	/* 30:restart command */
	{WAIT_TIMER, START_DELAY},	/* 31:Wait.. */
	{REG_DATA, 1},		/* 32:NOACTION - write 1 to CS */
	{WAIT_TIMER, WRITE_DELAY},	/* 33:Wait.. */
	{I2C_DATA, 0x11},		/* 34:PMU Read client addr offset */
	{WAIT_TIMER, WRITE_DELAY},	/* 35:Wait.. */
	{REG_ADDR, 0x20 },	/* 36:Set Address for PMU_BSC CS register */
	{REG_DATA, 0xF},		/* 37:READ command */
	{WAIT_TIMER, READ_DELAY},	/* 38:WAIT command */
	{REG_DATA, 1},			/* 39:NOACTION:write 1 to CS */
#ifdef CONFIG_KONA_PWRMGR_REV2
	{READ_FIFO, 0},			/* 40:read FIFO */
#else
	{REG_ADDR, 0},			/* 40:NOP */
#endif
	{END, 0},			/* 41:NOP:Read seq end */
	{REG_ADDR, 0x20}, /* 42:Set Address for i2cmmhs BSC_CS register */
	{REG_DATA, START_CMD},	/* 43:Set Start condition:write 3 to cs */
	{WAIT_TIMER, START_DELAY},	/* 44:Wait.. */
	{REG_DATA, 1},		/* 45:Clear Start Condition*/
	{WAIT_TIMER, WRITE_DELAY},	/* 46:Wait.. */
	{I2C_DATA, 0x10},		/* 47:PMU write client addr offset */
	{WAIT_TIMER, WRITE_DELAY},	/* 48:Wait.. */
	{I2C_DATA, 0x00},		/* 49:PMU write register addr offset */
	{WAIT_TIMER, WRITE_DELAY},	/* 50:Wait.. */
	{I2C_DATA, 0xC0},		/* 51:PMU write register data offset */
	{WAIT_TIMER, WRITE_DELAY},	/* 52:Wait.. */
#ifdef CONFIG_KONA_PWRMGR_REV2
	{SET_READ_DATA, 0x48}, /* 53:copy NACK bit to PWRMGR I2C_READ_DATA[0]*/
#else
	{REG_ADDR, 0}, /* 53: NOP */
#endif
	{REG_ADDR, PMU_BSC_INT_STATUS_REG}, /* 54:PMU BSC INT status reg */
	{REG_DATA, PMU_BSC_INT_STATUS_MASK},/* 55:Clear INT status register */
	{END, 0},			/* 56:NOP- write seq end */
	{REG_ADDR, 0},			/* 57:set2/zero - NOP */
	{SET_PC_PINS, 0x30},		/* 58:Set PC1 pins to 0 */
	{REG_ADDR, 0},			/* 59:NOP */
	{END, 0},			/* 60:END */
	{REG_ADDR, 0},			/* 61:set1 -NOP */
	{SET_PC_PINS, 0x31},		/* 62:Set PC1 pins to 1 */
	{END, 0},			/* End:NOP */
};



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
			.set2_ptr = 57,
			.set1_val = 2,/*wakeup from retention voltage inx*/
			.set1_ptr = 61,
			.zerov_ptr = 57, /*Not used for Rhea*/
		};

struct pwrmgr_init_param pwrmgr_init_param =
	{
		.cmd_buf = i2c_cmd,
		.cmb_buf_size = ARRAY_SIZE(i2c_cmd),
		.v0ptr = &v0_ptr,
		.def_vlt_tbl = pwrmgr_default_volt_lut,
		.vlt_tbl_size = ARRAY_SIZE(pwrmgr_default_volt_lut),
#if defined(CONFIG_KONA_PWRMGR_REV2)
		.i2c_rd_off = 18,
		.i2c_rd_slv_id_off1 = 25,
		.i2c_rd_reg_addr_off = 27,
		.i2c_rd_slv_id_off2 = 34,
		.i2c_wr_off = 42,
		.i2c_wr_slv_id_off = 47,
		.i2c_wr_reg_addr_off = 49,
		.i2c_wr_val_addr_off = 51,
		.i2c_seq_timeout = 100,
#endif
	};

#endif /*CONFIG_KONA_POWER_MGR*/


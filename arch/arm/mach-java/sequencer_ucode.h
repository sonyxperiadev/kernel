/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#define SET_PC_PIN_CMD(pc_pin)			\
	(SET_PC_PIN_CMD_##pc_pin##_PIN_VALUE_MASK|\
	 SET_PC_PIN_CMD_##pc_pin##_PIN_OVERRIDE_MASK)

#define CLEAR_PC_PIN_CMD(pc_pin)			\
	 SET_PC_PIN_CMD_##pc_pin##_PIN_OVERRIDE_MASK

/**
 * Rampupm time for CSR/MSR from off state to Vout
 */
#define SR_VLT_SOFT_START_DELAY		200

/**
 * Offsets in the Sequencer code
 */
#define VO0_HW_SEQ_START_OFF		64
#define VO0_SET1_OFFSET			95
#define VO0_SET2_OFFSET			97

#define VO1_HW_SEQ_START_OFF		80
#define VO1_SET1_OFFSET			80
#define VO1_ZERO_PTR_OFFSET		99
#define VO1_SET2_OFFSET			101


#define SW_SEQ_RD_START_OFF		2
#define SW_SEQ_RD_SLAVE_ID_1_OFF	8
#define SW_SEQ_RD_REG_ADDR_OFF		10
#define SW_SEQ_RD_SLAVE_ID_2_OFF	16

#define SW_SEQ_WR_START_OFF		25
#define SW_SEQ_WR_SLAVE_ID_OFF		31
#define SW_SEQ_WR_REG_ADDR_OFF		33
#define SW_SEQ_WR_VALUE_OFF		35

#define RDWR_CLR_BSC_ISR_JUMP_OFF		37
#define FAKE_TRG_ERRATUM_PC_PIN_TOGGLE_OFF	42



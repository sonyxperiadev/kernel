/* Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/msm_kgsl.h>

#include "adreno.h"
#include "kgsl_sharedmem.h"
#include "a4xx_reg.h"
#include "adreno_a3xx.h"
#include "adreno_a4xx.h"
#include "adreno_cp_parser.h"
#include "adreno_trace.h"
#include "adreno_pm4types.h"
#include "adreno_perfcounter.h"

#define SP_TP_PWR_ON BIT(20)

/*
 * Define registers for a4xx that contain addresses used by the
 * cp parser logic
 */
const unsigned int a4xx_cp_addr_regs[ADRENO_CP_ADDR_MAX] = {
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_0,
				A4XX_VSC_PIPE_DATA_ADDRESS_0),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_0,
				A4XX_VSC_PIPE_DATA_LENGTH_0),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_1,
				A4XX_VSC_PIPE_DATA_ADDRESS_1),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_1,
				A4XX_VSC_PIPE_DATA_LENGTH_1),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_2,
				A4XX_VSC_PIPE_DATA_ADDRESS_2),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_2,
				A4XX_VSC_PIPE_DATA_LENGTH_2),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_3,
				A4XX_VSC_PIPE_DATA_ADDRESS_3),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_3,
				A4XX_VSC_PIPE_DATA_LENGTH_3),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_4,
				A4XX_VSC_PIPE_DATA_ADDRESS_4),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_4,
				A4XX_VSC_PIPE_DATA_LENGTH_4),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_5,
				A4XX_VSC_PIPE_DATA_ADDRESS_5),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_5,
				A4XX_VSC_PIPE_DATA_LENGTH_5),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_6,
				A4XX_VSC_PIPE_DATA_ADDRESS_6),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_6,
				A4XX_VSC_PIPE_DATA_LENGTH_6),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_ADDRESS_7,
				A4XX_VSC_PIPE_DATA_ADDRESS_7),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_PIPE_DATA_LENGTH_7,
				A4XX_VSC_PIPE_DATA_LENGTH_7),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_0,
				A4XX_VFD_FETCH_INSTR_1_0),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_1,
				A4XX_VFD_FETCH_INSTR_1_1),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_2,
				A4XX_VFD_FETCH_INSTR_1_2),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_3,
				A4XX_VFD_FETCH_INSTR_1_3),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_4,
				A4XX_VFD_FETCH_INSTR_1_4),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_5,
				A4XX_VFD_FETCH_INSTR_1_5),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_6,
				A4XX_VFD_FETCH_INSTR_1_6),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_7,
				A4XX_VFD_FETCH_INSTR_1_7),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_8,
				A4XX_VFD_FETCH_INSTR_1_8),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_9,
				A4XX_VFD_FETCH_INSTR_1_9),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_10,
				A4XX_VFD_FETCH_INSTR_1_10),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_11,
				A4XX_VFD_FETCH_INSTR_1_11),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_12,
				A4XX_VFD_FETCH_INSTR_1_12),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_13,
				A4XX_VFD_FETCH_INSTR_1_13),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_14,
				A4XX_VFD_FETCH_INSTR_1_14),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_15,
				A4XX_VFD_FETCH_INSTR_1_15),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_16,
				A4XX_VFD_FETCH_INSTR_1_16),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_17,
				A4XX_VFD_FETCH_INSTR_1_17),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_18,
				A4XX_VFD_FETCH_INSTR_1_18),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_19,
				A4XX_VFD_FETCH_INSTR_1_19),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_20,
				A4XX_VFD_FETCH_INSTR_1_20),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_21,
				A4XX_VFD_FETCH_INSTR_1_21),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_22,
				A4XX_VFD_FETCH_INSTR_1_22),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_23,
				A4XX_VFD_FETCH_INSTR_1_23),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_24,
				A4XX_VFD_FETCH_INSTR_1_24),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_25,
				A4XX_VFD_FETCH_INSTR_1_25),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_26,
				A4XX_VFD_FETCH_INSTR_1_26),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_27,
				A4XX_VFD_FETCH_INSTR_1_27),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_28,
				A4XX_VFD_FETCH_INSTR_1_28),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_29,
				A4XX_VFD_FETCH_INSTR_1_29),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_30,
				A4XX_VFD_FETCH_INSTR_1_30),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VFD_FETCH_INSTR_1_31,
				A4XX_VFD_FETCH_INSTR_1_31),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_VSC_SIZE_ADDRESS,
				A4XX_VSC_SIZE_ADDRESS),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_SP_VS_PVT_MEM_ADDR,
				A4XX_SP_VS_PVT_MEM_ADDR),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_SP_FS_PVT_MEM_ADDR,
				A4XX_SP_FS_PVT_MEM_ADDR),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_SP_VS_OBJ_START_REG,
				A4XX_SP_VS_OBJ_START),
	ADRENO_REG_DEFINE(ADRENO_CP_ADDR_SP_FS_OBJ_START_REG,
				A4XX_SP_FS_OBJ_START),
	ADRENO_REG_DEFINE(ADRENO_CP_UCHE_INVALIDATE0,
				A4XX_UCHE_INVALIDATE0),
	ADRENO_REG_DEFINE(ADRENO_CP_UCHE_INVALIDATE1,
				A4XX_UCHE_INVALIDATE1),
};

static const struct adreno_vbif_data a405_vbif[] = {
	{ A4XX_VBIF_ROUND_ROBIN_QOS_ARB, 0x00000003 },
	{0, 0},
};

static const struct adreno_vbif_data a420_vbif[] = {
	{ A4XX_VBIF_ABIT_SORT, 0x0001001F },
	{ A4XX_VBIF_ABIT_SORT_CONF, 0x000000A4 },
	{ A4XX_VBIF_GATE_OFF_WRREQ_EN, 0x00000001 },
	{ A4XX_VBIF_IN_RD_LIM_CONF0, 0x18181818 },
	{ A4XX_VBIF_IN_RD_LIM_CONF1, 0x00000018 },
	{ A4XX_VBIF_IN_WR_LIM_CONF0, 0x18181818 },
	{ A4XX_VBIF_IN_WR_LIM_CONF1, 0x00000018 },
	{ A4XX_VBIF_ROUND_ROBIN_QOS_ARB, 0x00000003 },
	{0, 0},
};

static const struct adreno_vbif_data a430_vbif[] = {
	{ A4XX_VBIF_GATE_OFF_WRREQ_EN, 0x00000001 },
	{ A4XX_VBIF_IN_RD_LIM_CONF0, 0x18181818 },
	{ A4XX_VBIF_IN_RD_LIM_CONF1, 0x00000018 },
	{ A4XX_VBIF_IN_WR_LIM_CONF0, 0x18181818 },
	{ A4XX_VBIF_IN_WR_LIM_CONF1, 0x00000018 },
	{ A4XX_VBIF_ROUND_ROBIN_QOS_ARB, 0x00000003 },
	{0, 0},
};

static const struct adreno_vbif_platform a4xx_vbif_platforms[] = {
	{ adreno_is_a405, a405_vbif },
	{ adreno_is_a420, a420_vbif },
	{ adreno_is_a430, a430_vbif },
	{ adreno_is_a418, a430_vbif },
};

/*
 * a4xx_is_sptp_idle() - A430 SP/TP should be off to be considered idle
 * @adreno_dev: The adreno device pointer
 */
static bool a4xx_is_sptp_idle(struct adreno_device *adreno_dev)
{
	unsigned int reg;
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);

	if (!ADRENO_FEATURE(adreno_dev, ADRENO_SPTP_PC))
		return true;

	/* If SP/TP pc isn't enabled, don't worry about power */
	kgsl_regread(device, A4XX_CP_POWER_COLLAPSE_CNTL, &reg);
	if (!(reg & 0x10))
		return true;

	/* Check that SP/TP is off */
	kgsl_regread(device, A4XX_RBBM_POWER_STATUS, &reg);
	return !(reg & SP_TP_PWR_ON);
}

/*
 * a4xx_regulator_enable() - Enable any necessary HW regulators
 * @adreno_dev: The adreno device pointer
 *
 * Some HW blocks may need their regulators explicitly enabled
 * on a restart.  Clocks must be on during this call.
 */
static int a4xx_regulator_enable(struct adreno_device *adreno_dev)
{
	unsigned int reg;
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);

	if (!(adreno_is_a430(adreno_dev) || adreno_is_a418(adreno_dev)))
		return 0;

	/* Set the default register values; set SW_COLLAPSE to 0 */
	kgsl_regwrite(device, A4XX_RBBM_POWER_CNTL_IP, 0x778000);
	do {
		udelay(5);
		kgsl_regread(device, A4XX_RBBM_POWER_STATUS, &reg);
	} while (!(reg & SP_TP_PWR_ON));
	return 0;
}

/*
 * a4xx_regulator_disable() - Disable any necessary HW regulators
 * @adreno_dev: The adreno device pointer
 *
 * Some HW blocks may need their regulators explicitly disabled
 * on a power down to prevent current spikes.  Clocks must be on
 * during this call.
 */
static void a4xx_regulator_disable(struct adreno_device *adreno_dev)
{
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);

	if (!(adreno_is_a430(adreno_dev) || adreno_is_a418(adreno_dev)))
		return;

	/* Set the default register values; set SW_COLLAPSE to 1 */
	kgsl_regwrite(device, A4XX_RBBM_POWER_CNTL_IP, 0x778001);
}

/*
 * a4xx_enable_pc() - Enable the SP/TP block power collapse
 * @adreno_dev: The adreno device pointer
 */
static void a4xx_enable_pc(struct adreno_device *adreno_dev)
{
	if (!ADRENO_FEATURE(adreno_dev, ADRENO_SPTP_PC) ||
		!test_bit(ADRENO_SPTP_PC_CTRL, &adreno_dev->pwrctrl_flag))
		return;

	kgsl_regwrite(KGSL_DEVICE(adreno_dev), A4XX_CP_POWER_COLLAPSE_CNTL,
		0x00400010);
	trace_adreno_sp_tp((unsigned long) __builtin_return_address(0));
};

/*
 * a4xx_enable_ppd() - Enable the Peak power detect logic in the h/w
 * @adreno_dev: The adreno device pointer
 *
 * A430 can detect peak current conditions inside h/w and throttle
 * the workload to ALUs to mitigate it.
 */
static void a4xx_enable_ppd(struct adreno_device *adreno_dev)
{
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);

	if (!ADRENO_FEATURE(adreno_dev, ADRENO_PPD) ||
		!test_bit(ADRENO_PPD_CTRL, &adreno_dev->pwrctrl_flag) ||
		!adreno_is_a430v2(adreno_dev))
		return;

	/* Program thresholds */
	kgsl_regwrite(device, A4XX_RBBM_PPD_EPOCH_INTER_TH_HIGH_CLEAR_THR,
								0x003F0101);
	kgsl_regwrite(device, A4XX_RBBM_PPD_EPOCH_INTER_TH_LOW, 0x00000101);
	kgsl_regwrite(device, A4XX_RBBM_PPD_V2_SP_PWR_WEIGHTS, 0x00085014);
	kgsl_regwrite(device, A4XX_RBBM_PPD_V2_SP_RB_EPOCH_TH, 0x00000B46);
	kgsl_regwrite(device, A4XX_RBBM_PPD_V2_TP_CONFIG, 0xE4525111);
	kgsl_regwrite(device, A4XX_RBBM_PPD_RAMP_V2_CONTROL, 0x0000000B);

	/* Enable PPD*/
	kgsl_regwrite(device, A4XX_RBBM_PPD_CTRL, 0x1002E40C);
};

/*
 * a4xx_pwrlevel_change_settings() - Program the hardware during power level
 * transitions
 * @adreno_dev: The adreno device pointer
 * @prelevel: The previous power level
 * @postlevel: The new power level
 * @post: True if called after the clock change has taken effect
 */
static void a4xx_pwrlevel_change_settings(struct adreno_device *adreno_dev,
				unsigned int prelevel, unsigned int postlevel,
				bool post)
{
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);
	static int pre;

	/* PPD programming only for A430v2 */
	if (!ADRENO_FEATURE(adreno_dev, ADRENO_PPD) ||
		!test_bit(ADRENO_PPD_CTRL, &adreno_dev->pwrctrl_flag) ||
		!adreno_is_a430v2(adreno_dev))
		return;

	/* if this is a real pre, or a post without a previous pre, set pre */
	if ((post == 0) || (pre == 0 && post == 1))
		pre = 1;
	else if (post == 1)
		pre = 0;

	if ((prelevel == 0) && pre) {
		/* Going to Non-Turbo mode - mask the throttle and reset */
		kgsl_regwrite(device, A4XX_RBBM_PPD_CTRL, 0x1002E40E);
		kgsl_regwrite(device, A4XX_RBBM_PPD_CTRL, 0x1002E40C);
	} else if ((postlevel == 0) && post) {
		/* Going to Turbo mode - unmask the throttle and reset */
		kgsl_regwrite(device, A4XX_RBBM_PPD_CTRL, 0x1002E40A);
		kgsl_regwrite(device, A4XX_RBBM_PPD_CTRL, 0x1002E408);
	}

	if (post)
		pre = 0;
}

/*
 * a4xx_enable_hwcg() - Program the clock control registers
 * @device: The adreno device pointer
 */
static void a4xx_enable_hwcg(struct kgsl_device *device)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_TP0, 0x02222202);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_TP1, 0x02222202);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_TP2, 0x02222202);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_TP3, 0x02222202);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_TP0, 0x00002222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_TP1, 0x00002222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_TP2, 0x00002222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_TP3, 0x00002222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_TP0, 0x0E739CE7);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_TP1, 0x0E739CE7);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_TP2, 0x0E739CE7);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_TP3, 0x0E739CE7);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_TP0, 0x00111111);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_TP1, 0x00111111);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_TP2, 0x00111111);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_TP3, 0x00111111);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_SP0, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_SP1, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_SP2, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_SP3, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_SP0, 0x00222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_SP1, 0x00222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_SP2, 0x00222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_SP3, 0x00222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_SP0, 0x00000104);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_SP1, 0x00000104);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_SP2, 0x00000104);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_SP3, 0x00000104);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_SP0, 0x00000081);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_SP1, 0x00000081);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_SP2, 0x00000081);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_SP3, 0x00000081);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_UCHE, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_UCHE, 0x02222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL3_UCHE, 0x00000000);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL4_UCHE, 0x00000000);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_UCHE, 0x00004444);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_UCHE, 0x00001112);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_RB0, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_RB1, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_RB2, 0x22222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_RB3, 0x22222222);
	/* Disable L1 clocking in A420 due to CCU issues with it */
	if (adreno_is_a420(adreno_dev)) {
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB0, 0x00002020);
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB1, 0x00002020);
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB2, 0x00002020);
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB3, 0x00002020);
	} else {
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB0, 0x00022020);
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB1, 0x00022020);
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB2, 0x00022020);
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2_RB3, 0x00022020);
	}
	/* No CCU for A405 */
	if (!adreno_is_a405(adreno_dev)) {
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_CTL_MARB_CCU0, 0x00000922);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_CTL_MARB_CCU1, 0x00000922);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_CTL_MARB_CCU2, 0x00000922);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_CTL_MARB_CCU3, 0x00000922);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_HYST_RB_MARB_CCU0, 0x00000000);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_HYST_RB_MARB_CCU1, 0x00000000);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_HYST_RB_MARB_CCU2, 0x00000000);
		kgsl_regwrite(device,
			A4XX_RBBM_CLOCK_HYST_RB_MARB_CCU3, 0x00000000);
		kgsl_regwrite(device,
				A4XX_RBBM_CLOCK_DELAY_RB_MARB_CCU_L1_0,
				0x00000001);
		kgsl_regwrite(device,
				A4XX_RBBM_CLOCK_DELAY_RB_MARB_CCU_L1_1,
				0x00000001);
		kgsl_regwrite(device,
				A4XX_RBBM_CLOCK_DELAY_RB_MARB_CCU_L1_2,
				0x00000001);
		kgsl_regwrite(device,
				A4XX_RBBM_CLOCK_DELAY_RB_MARB_CCU_L1_3,
				0x00000001);
	}
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_MODE_GPC, 0x02222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_GPC, 0x04100104);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_GPC, 0x00022222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_COM_DCOM, 0x00000022);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_COM_DCOM, 0x0000010F);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_COM_DCOM, 0x00000022);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_TSE_RAS_RBBM, 0x00222222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_TSE_RAS_RBBM, 0x00004104);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_TSE_RAS_RBBM, 0x00000222);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL_HLSQ , 0x00000000);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_HYST_HLSQ, 0x00000000);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_HLSQ, 0x00220000);
	/*
	 * Due to a HW timing issue, top level HW clock gating is causing
	 * register read/writes to be dropped in adreno a430.
	 * This timing issue started happening because of SP/TP power collapse.
	 * On targets that do not have SP/TP PC there is no timing issue.
	 * The HW timing issue could be fixed by
	 * a) disabling SP/TP power collapse
	 * b) or disabling HW clock gating.
	 * Disabling HW clock gating + NAP enabled combination has
	 * minimal power impact. So this option is chosen over disabling
	 * SP/TP power collapse.
	 * Revisions of A430 which chipid 2 and above do not have the issue.
	 */
	if (adreno_is_a430(adreno_dev) &&
		(ADRENO_CHIPID_PATCH(adreno_dev->chipid) < 2))
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL, 0);
	else
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL, 0xAAAAAAAA);
	kgsl_regwrite(device, A4XX_RBBM_CLOCK_CTL2, 0);
}

/**
 * a4xx_protect_init() - Initializes register protection on a4xx
 * @adreno_dev: Pointer to the device structure
 * Performs register writes to enable protected access to sensitive
 * registers
 */
static void a4xx_protect_init(struct adreno_device *adreno_dev)
{
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);
	int index = 0;
	struct kgsl_protected_registers *iommu_regs;

	/* enable access protection to privileged registers */
	kgsl_regwrite(device, A4XX_CP_PROTECT_CTRL, 0x00000007);
	/* RBBM registers */
	adreno_set_protected_registers(adreno_dev, &index, 0x4, 2);
	adreno_set_protected_registers(adreno_dev, &index, 0x8, 3);
	adreno_set_protected_registers(adreno_dev, &index, 0x10, 4);
	adreno_set_protected_registers(adreno_dev, &index, 0x20, 5);
	adreno_set_protected_registers(adreno_dev, &index, 0x40, 6);
	adreno_set_protected_registers(adreno_dev, &index, 0x80, 4);

	/* Content protection registers */
	if (kgsl_mmu_is_secured(&device->mmu)) {
		adreno_set_protected_registers(adreno_dev, &index,
			   A4XX_RBBM_SECVID_TSB_TRUSTED_BASE, 3);
		adreno_set_protected_registers(adreno_dev, &index,
			   A4XX_RBBM_SECVID_TRUST_CONTROL, 1);
	}

	/* CP registers */
	adreno_set_protected_registers(adreno_dev, &index, 0x200, 7);
	adreno_set_protected_registers(adreno_dev, &index, 0x580, 4);
	adreno_set_protected_registers(adreno_dev, &index, A4XX_CP_PREEMPT, 1);
	/* RB registers */
	adreno_set_protected_registers(adreno_dev, &index, 0xCC0, 0);

	/* HLSQ registers */
	adreno_set_protected_registers(adreno_dev, &index, 0xE00, 0);

	/* VPC registers */
	adreno_set_protected_registers(adreno_dev, &index, 0xE60, 1);

	if (adreno_is_a430(adreno_dev) || adreno_is_a420(adreno_dev) ||
			adreno_is_a418(adreno_dev)) {
		/*
		 * Protect registers that might cause XPU violation if
		 * accessed by GPU
		 */
		adreno_set_protected_registers(adreno_dev, &index, 0x2c00, 10);
		adreno_set_protected_registers(adreno_dev, &index, 0x3300, 8);
		adreno_set_protected_registers(adreno_dev, &index, 0x3400, 10);
	}

	/* SMMU registers */
	iommu_regs = kgsl_mmu_get_prot_regs(&device->mmu);
	if (iommu_regs)
		adreno_set_protected_registers(adreno_dev, &index,
				iommu_regs->base, iommu_regs->range);
}

static struct adreno_snapshot_sizes a4xx_snap_sizes = {
	.cp_pfp = 0x14,
	.vpc_mem = 2048,
	.cp_meq = 64,
	.shader_mem = 0x4000,
	.cp_merciu = 64,
	.roq = 512,
};


static void a4xx_start(struct adreno_device *adreno_dev)
{
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);
	struct adreno_gpudev *gpudev = ADRENO_GPU_DEVICE(adreno_dev);
	unsigned int cp_debug = A4XX_CP_DEBUG_DEFAULT;

	adreno_vbif_start(adreno_dev, a4xx_vbif_platforms,
			ARRAY_SIZE(a4xx_vbif_platforms));
	/* Make all blocks contribute to the GPU BUSY perf counter */
	kgsl_regwrite(device, A4XX_RBBM_GPU_BUSY_MASKED, 0xFFFFFFFF);

	/* Tune the hystersis counters for SP and CP idle detection */
	kgsl_regwrite(device, A4XX_RBBM_SP_HYST_CNT, 0x10);
	kgsl_regwrite(device, A4XX_RBBM_WAIT_IDLE_CLOCKS_CTL, 0x10);
	if (adreno_is_a430(adreno_dev))
		kgsl_regwrite(device, A4XX_RBBM_WAIT_IDLE_CLOCKS_CTL2, 0x30);

	/*
	 * Enable the RBBM error reporting bits.  This lets us get
	 * useful information on failure
	 */

	kgsl_regwrite(device, A4XX_RBBM_AHB_CTL0, 0x00000001);

	/* Enable AHB error reporting */
	kgsl_regwrite(device, A4XX_RBBM_AHB_CTL1, 0xA6FFFFFF);

	/* Turn on the power counters */
	kgsl_regwrite(device, A4XX_RBBM_RBBM_CTL, 0x00000030);

	/*
	 * Turn on hang detection - this spews a lot of useful information
	 * into the RBBM registers on a hang
	 */
	set_bit(ADRENO_DEVICE_HANG_INTR, &adreno_dev->priv);
	gpudev->irq->mask |= (1 << A4XX_INT_MISC_HANG_DETECT);
	kgsl_regwrite(device, A4XX_RBBM_INTERFACE_HANG_INT_CTL,
			(1 << 30) | 0xFFFF);

	/* Set the GMEM/OCMEM base address for A4XX */
	kgsl_regwrite(device, A4XX_RB_GMEM_BASE_ADDR,
			(unsigned int)(adreno_dev->gmem_base >> 14));

	/* Turn on performance counters */
	kgsl_regwrite(device, A4XX_RBBM_PERFCTR_CTL, 0x01);

	/* Enable VFD to access most of the UCHE (7 ways out of 8) */
	kgsl_regwrite(device, A4XX_UCHE_CACHE_WAYS_VFD, 0x07);

	/* Disable L2 bypass to avoid UCHE out of bounds errors */
	kgsl_regwrite(device, UCHE_TRAP_BASE_LO, 0xffff0000);
	kgsl_regwrite(device, UCHE_TRAP_BASE_HI, 0xffff0000);

	/* On A420 cores turn on SKIP_IB2_DISABLE in addition to the default */
	if (adreno_is_a420(adreno_dev))
		cp_debug |= (1 << 29);
	/*
	 * Set chicken bit to disable the speed up of bootstrap on A430
	 * and its derivatives
	 */
	else
		cp_debug |= (1 << 14);

	kgsl_regwrite(device, A4XX_CP_DEBUG, cp_debug);

	/* On A430 enable SP regfile sleep for power savings */
	if (!adreno_is_a420(adreno_dev)) {
		kgsl_regwrite(device, A4XX_RBBM_SP_REGFILE_SLEEP_CNTL_0,
				0x00000441);
		kgsl_regwrite(device, A4XX_RBBM_SP_REGFILE_SLEEP_CNTL_1,
				0x00000441);
	}

	a4xx_enable_hwcg(device);
	/*
	 * For A420 set RBBM_CLOCK_DELAY_HLSQ.CGC_HLSQ_TP_EARLY_CYC >= 2
	 * due to timing issue with HLSQ_TP_CLK_EN
	 */
	if (adreno_is_a420(adreno_dev)) {
		unsigned int val;
		kgsl_regread(device, A4XX_RBBM_CLOCK_DELAY_HLSQ, &val);
		val &= ~A4XX_CGC_HLSQ_TP_EARLY_CYC_MASK;
		val |= 2 << A4XX_CGC_HLSQ_TP_EARLY_CYC_SHIFT;
		kgsl_regwrite(device, A4XX_RBBM_CLOCK_DELAY_HLSQ, val);
	}

	/* A430 and derivatives offers bigger chunk of CP_STATE_DEBUG regs */
	if (!adreno_is_a420(adreno_dev))
		a4xx_snap_sizes.cp_pfp = 0x34;

	if (adreno_is_a405(adreno_dev))
		gpudev->vbif_xin_halt_ctrl0_mask =
			A405_VBIF_XIN_HALT_CTRL0_MASK;

	adreno_set_preempt_state(adreno_dev, ADRENO_PREEMPT_NONE);

	a4xx_protect_init(adreno_dev);
}

/*
 * a4xx_err_callback() - Callback for a4xx error interrupts
 * @adreno_dev: Pointer to device
 * @bit: Interrupt bit
 */
static void a4xx_err_callback(struct adreno_device *adreno_dev, int bit)
{
	struct kgsl_device *device = KGSL_DEVICE(adreno_dev);
	unsigned int reg;

	switch (bit) {
	case A4XX_INT_RBBM_AHB_ERROR: {
		kgsl_regread(device, A4XX_RBBM_AHB_ERROR_STATUS, &reg);

		/*
		 * Return the word address of the erroring register so that it
		 * matches the register specification
		 */
		KGSL_DRV_CRIT_RATELIMIT(device,
			"RBBM | AHB bus error | %s | addr=%x | ports=%x:%x\n",
			reg & (1 << 28) ? "WRITE" : "READ",
			(reg & 0xFFFFF) >> 2, (reg >> 20) & 0x3,
			(reg >> 24) & 0xF);

		/* Clear the error */
		kgsl_regwrite(device, A4XX_RBBM_AHB_CMD, (1 << 4));
		break;
	}
	case A4XX_INT_RBBM_REG_TIMEOUT:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: AHB register timeout\n");
		break;
	case A4XX_INT_RBBM_ME_MS_TIMEOUT:
		kgsl_regread(device, A4XX_RBBM_AHB_ME_SPLIT_STATUS, &reg);
		KGSL_DRV_CRIT_RATELIMIT(device,
			"RBBM | ME master split timeout | status=%x\n", reg);
		break;
	case A4XX_INT_RBBM_PFP_MS_TIMEOUT:
		kgsl_regread(device, A4XX_RBBM_AHB_PFP_SPLIT_STATUS, &reg);
		KGSL_DRV_CRIT_RATELIMIT(device,
			"RBBM | PFP master split timeout | status=%x\n", reg);
		break;
	case A4XX_INT_RBBM_ETS_MS_TIMEOUT:
		KGSL_DRV_CRIT_RATELIMIT(device,
			"RBBM: ME master split timeout\n");
		break;
	case A4XX_INT_RBBM_ASYNC_OVERFLOW:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: ASYNC overflow\n");
		break;
	case A4XX_INT_CP_OPCODE_ERROR:
		KGSL_DRV_CRIT_RATELIMIT(device,
			"ringbuffer opcode error interrupt\n");
		break;
	case A4XX_INT_CP_RESERVED_BIT_ERROR:
		KGSL_DRV_CRIT_RATELIMIT(device,
			"ringbuffer reserved bit error interrupt\n");
		break;
	case A4XX_INT_CP_HW_FAULT:
	{
		struct adreno_gpudev *gpudev = ADRENO_GPU_DEVICE(adreno_dev);
		kgsl_regread(device, A4XX_CP_HW_FAULT, &reg);
		KGSL_DRV_CRIT_RATELIMIT(device,
			"CP | Ringbuffer HW fault | status=%x\n", reg);
		/*
		 * mask off this interrupt since it can spam, it will be
		 * turned on again when device resets
		 */
		adreno_writereg(adreno_dev, ADRENO_REG_RBBM_INT_0_MASK,
			gpudev->irq->mask & ~(1 << A4XX_INT_CP_HW_FAULT));
		break;
	}
	case A4XX_INT_CP_REG_PROTECT_FAULT:
		kgsl_regread(device, A4XX_CP_PROTECT_STATUS, &reg);
		KGSL_DRV_CRIT_RATELIMIT(device,
			"CP | Protected mode error| %s | addr=%x\n",
			reg & (1 << 24) ? "WRITE" : "READ",
			(reg & 0xFFFFF) >> 2);
		break;
	case A4XX_INT_CP_AHB_ERROR_HALT:
		KGSL_DRV_CRIT_RATELIMIT(device,
			"ringbuffer AHB error interrupt\n");
		break;
	case A4XX_INT_RBBM_ATB_BUS_OVERFLOW:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: ATB bus overflow\n");
		break;
	case A4XX_INT_UCHE_OOB_ACCESS:
		KGSL_DRV_CRIT_RATELIMIT(device, "UCHE: Out of bounds access\n");
		break;
	case A4XX_INT_RBBM_DPM_CALC_ERR:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: dpm calc error\n");
		break;
	case A4XX_INT_RBBM_DPM_EPOCH_ERR:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: dpm epoch error\n");
		break;
	case A4XX_INT_RBBM_DPM_THERMAL_YELLOW_ERR:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: dpm thermal yellow\n");
		break;
	case A4XX_INT_RBBM_DPM_THERMAL_RED_ERR:
		KGSL_DRV_CRIT_RATELIMIT(device, "RBBM: dpm thermal red\n");
		break;
	default:
		KGSL_DRV_CRIT_RATELIMIT(device, "Unknown interrupt\n");
	}
}

/* Register offset defines for A4XX, in order of enum adreno_regs */
static unsigned int a4xx_register_offsets[ADRENO_REG_REGISTER_MAX] = {
	ADRENO_REG_DEFINE(ADRENO_REG_CP_ME_RAM_WADDR, A4XX_CP_ME_RAM_WADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_ME_RAM_DATA, A4XX_CP_ME_RAM_DATA),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PFP_UCODE_DATA, A4XX_CP_PFP_UCODE_DATA),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PFP_UCODE_ADDR, A4XX_CP_PFP_UCODE_ADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_WFI_PEND_CTR, A4XX_CP_WFI_PEND_CTR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_RB_BASE, A4XX_CP_RB_BASE),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_RB_BASE_HI, ADRENO_REG_SKIP),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_RB_RPTR_ADDR_LO, A4XX_CP_RB_RPTR_ADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_RB_RPTR, A4XX_CP_RB_RPTR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_RB_WPTR, A4XX_CP_RB_WPTR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_CNTL, A4XX_CP_CNTL),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_ME_CNTL, A4XX_CP_ME_CNTL),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_RB_CNTL, A4XX_CP_RB_CNTL),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_IB1_BASE, A4XX_CP_IB1_BASE),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_IB1_BASE_HI, ADRENO_REG_SKIP),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_IB1_BUFSZ, A4XX_CP_IB1_BUFSZ),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_IB2_BASE, A4XX_CP_IB2_BASE),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_IB2_BASE_HI, ADRENO_REG_SKIP),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_IB2_BUFSZ, A4XX_CP_IB2_BUFSZ),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_ME_RAM_RADDR, A4XX_CP_ME_RAM_RADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_ROQ_ADDR, A4XX_CP_ROQ_ADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_ROQ_DATA, A4XX_CP_ROQ_DATA),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_MERCIU_ADDR, A4XX_CP_MERCIU_ADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_MERCIU_DATA, A4XX_CP_MERCIU_DATA),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_MERCIU_DATA2, A4XX_CP_MERCIU_DATA2),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_MEQ_ADDR, A4XX_CP_MEQ_ADDR),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_MEQ_DATA, A4XX_CP_MEQ_DATA),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_HW_FAULT, A4XX_CP_HW_FAULT),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PROTECT_STATUS, A4XX_CP_PROTECT_STATUS),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_SCRATCH_REG6, A4XX_CP_SCRATCH_REG6),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_SCRATCH_REG7, A4XX_CP_SCRATCH_REG7),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PREEMPT, A4XX_CP_PREEMPT),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PREEMPT_DEBUG, A4XX_CP_PREEMPT_DEBUG),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PREEMPT_DISABLE,
						A4XX_CP_PREEMPT_DISABLE),
	ADRENO_REG_DEFINE(ADRENO_REG_CP_PROTECT_REG_0, A4XX_CP_PROTECT_REG_0),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_STATUS, A4XX_RBBM_STATUS),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_CTL, A4XX_RBBM_PERFCTR_CTL),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_LOAD_CMD0,
					A4XX_RBBM_PERFCTR_LOAD_CMD0),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_LOAD_CMD1,
					A4XX_RBBM_PERFCTR_LOAD_CMD1),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_LOAD_CMD2,
				A4XX_RBBM_PERFCTR_LOAD_CMD2),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_LOAD_CMD3,
				ADRENO_REG_SKIP),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_PWR_1_LO,
					A4XX_RBBM_PERFCTR_PWR_1_LO),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_INT_0_MASK, A4XX_RBBM_INT_0_MASK),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_INT_0_STATUS, A4XX_RBBM_INT_0_STATUS),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_CLOCK_CTL, A4XX_RBBM_CLOCK_CTL),
	ADRENO_REG_DEFINE(ADRENO_REG_VPC_DEBUG_RAM_SEL,
					A4XX_VPC_DEBUG_RAM_SEL),
	ADRENO_REG_DEFINE(ADRENO_REG_VPC_DEBUG_RAM_READ,
					A4XX_VPC_DEBUG_RAM_READ),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_INT_CLEAR_CMD,
				A4XX_RBBM_INT_CLEAR_CMD),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_RBBM_CTL, A4XX_RBBM_RBBM_CTL),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_SW_RESET_CMD, A4XX_RBBM_SW_RESET_CMD),
	ADRENO_REG_DEFINE(ADRENO_REG_UCHE_INVALIDATE0, A4XX_UCHE_INVALIDATE0),
	ADRENO_REG_DEFINE(ADRENO_REG_UCHE_INVALIDATE1, A4XX_UCHE_INVALIDATE1),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_LOAD_VALUE_LO,
				A4XX_RBBM_PERFCTR_LOAD_VALUE_LO),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_PERFCTR_LOAD_VALUE_HI,
				A4XX_RBBM_PERFCTR_LOAD_VALUE_HI),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_SECVID_TRUST_CONTROL,
				A4XX_RBBM_SECVID_TRUST_CONTROL),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_ALWAYSON_COUNTER_LO,
				A4XX_RBBM_ALWAYSON_COUNTER_LO),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_ALWAYSON_COUNTER_HI,
				A4XX_RBBM_ALWAYSON_COUNTER_HI),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_SECVID_TRUST_CONFIG,
				A4XX_RBBM_SECVID_TRUST_CONFIG),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_SECVID_TSB_CONTROL,
				A4XX_RBBM_SECVID_TSB_CONTROL),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_SECVID_TSB_TRUSTED_BASE,
				A4XX_RBBM_SECVID_TSB_TRUSTED_BASE),
	ADRENO_REG_DEFINE(ADRENO_REG_RBBM_SECVID_TSB_TRUSTED_SIZE,
				A4XX_RBBM_SECVID_TSB_TRUSTED_SIZE),
	ADRENO_REG_DEFINE(ADRENO_REG_VBIF_XIN_HALT_CTRL0,
				A4XX_VBIF_XIN_HALT_CTRL0),
	ADRENO_REG_DEFINE(ADRENO_REG_VBIF_XIN_HALT_CTRL1,
				A4XX_VBIF_XIN_HALT_CTRL1),
	ADRENO_REG_DEFINE(ADRENO_REG_VBIF_VERSION,
				A4XX_VBIF_VERSION),
};

static const struct adreno_reg_offsets a4xx_reg_offsets = {
	.offsets = a4xx_register_offsets,
	.offset_0 = ADRENO_REG_REGISTER_MAX,
};

static struct adreno_perfcount_register a4xx_perfcounters_cp[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_0_LO,
		A4XX_RBBM_PERFCTR_CP_0_HI, 0, A4XX_CP_PERFCTR_CP_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_1_LO,
		A4XX_RBBM_PERFCTR_CP_1_HI, 1, A4XX_CP_PERFCTR_CP_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_2_LO,
		A4XX_RBBM_PERFCTR_CP_2_HI, 2, A4XX_CP_PERFCTR_CP_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_3_LO,
		A4XX_RBBM_PERFCTR_CP_3_HI, 3, A4XX_CP_PERFCTR_CP_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_4_LO,
		A4XX_RBBM_PERFCTR_CP_4_HI, 4, A4XX_CP_PERFCTR_CP_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_5_LO,
		A4XX_RBBM_PERFCTR_CP_5_HI, 5, A4XX_CP_PERFCTR_CP_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_6_LO,
		A4XX_RBBM_PERFCTR_CP_6_HI, 6, A4XX_CP_PERFCTR_CP_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_7_LO,
		A4XX_RBBM_PERFCTR_CP_7_HI, 7, A4XX_CP_PERFCTR_CP_SEL_7 },
};

/*
 * Special list of CP registers for 420 to account for flaws.  This array is
 * inserted into the tables during perfcounter init
 */
static struct adreno_perfcount_register a420_perfcounters_cp[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_0_LO,
		A4XX_RBBM_PERFCTR_CP_0_HI, 0, A4XX_CP_PERFCTR_CP_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_1_LO,
		A4XX_RBBM_PERFCTR_CP_1_HI, 1, A4XX_CP_PERFCTR_CP_SEL_1 },
	/*
	 * The selector registers for 3, 5, and 7 are swizzled on the hardware.
	 * CP_4 and CP_6 are duped to SEL_2 and SEL_3 so we don't enable them
	 * here
	 */
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_3_LO,
		A4XX_RBBM_PERFCTR_CP_3_HI, 3, A4XX_CP_PERFCTR_CP_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_5_LO,
		A4XX_RBBM_PERFCTR_CP_5_HI, 5, A4XX_CP_PERFCTR_CP_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CP_7_LO,
		A4XX_RBBM_PERFCTR_CP_7_HI, 7, A4XX_CP_PERFCTR_CP_SEL_4 },
};

static struct adreno_perfcount_register a4xx_perfcounters_rbbm[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RBBM_0_LO,
		A4XX_RBBM_PERFCTR_RBBM_0_HI, 8, A4XX_RBBM_PERFCTR_RBBM_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RBBM_1_LO,
		A4XX_RBBM_PERFCTR_RBBM_1_HI, 9, A4XX_RBBM_PERFCTR_RBBM_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RBBM_2_LO,
		A4XX_RBBM_PERFCTR_RBBM_2_HI, 10, A4XX_RBBM_PERFCTR_RBBM_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RBBM_3_LO,
		A4XX_RBBM_PERFCTR_RBBM_3_HI, 11, A4XX_RBBM_PERFCTR_RBBM_SEL_3 },
};

static struct adreno_perfcount_register a4xx_perfcounters_pc[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_0_LO,
		A4XX_RBBM_PERFCTR_PC_0_HI, 12, A4XX_PC_PERFCTR_PC_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_1_LO,
		A4XX_RBBM_PERFCTR_PC_1_HI, 13, A4XX_PC_PERFCTR_PC_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_2_LO,
		A4XX_RBBM_PERFCTR_PC_2_HI, 14, A4XX_PC_PERFCTR_PC_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_3_LO,
		A4XX_RBBM_PERFCTR_PC_3_HI, 15, A4XX_PC_PERFCTR_PC_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_4_LO,
		A4XX_RBBM_PERFCTR_PC_4_HI, 16, A4XX_PC_PERFCTR_PC_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_5_LO,
		A4XX_RBBM_PERFCTR_PC_5_HI, 17, A4XX_PC_PERFCTR_PC_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_6_LO,
		A4XX_RBBM_PERFCTR_PC_6_HI, 18, A4XX_PC_PERFCTR_PC_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PC_7_LO,
		A4XX_RBBM_PERFCTR_PC_7_HI, 19, A4XX_PC_PERFCTR_PC_SEL_7 },
};

static struct adreno_perfcount_register a4xx_perfcounters_vfd[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_0_LO,
		A4XX_RBBM_PERFCTR_VFD_0_HI, 20, A4XX_VFD_PERFCTR_VFD_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_1_LO,
		A4XX_RBBM_PERFCTR_VFD_1_HI, 21, A4XX_VFD_PERFCTR_VFD_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_2_LO,
		A4XX_RBBM_PERFCTR_VFD_2_HI, 22, A4XX_VFD_PERFCTR_VFD_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_3_LO,
		A4XX_RBBM_PERFCTR_VFD_3_HI, 23, A4XX_VFD_PERFCTR_VFD_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_4_LO,
		A4XX_RBBM_PERFCTR_VFD_4_HI, 24, A4XX_VFD_PERFCTR_VFD_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_5_LO,
		A4XX_RBBM_PERFCTR_VFD_5_HI, 25, A4XX_VFD_PERFCTR_VFD_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_6_LO,
		A4XX_RBBM_PERFCTR_VFD_6_HI, 26, A4XX_VFD_PERFCTR_VFD_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VFD_7_LO,
		A4XX_RBBM_PERFCTR_VFD_7_HI, 27, A4XX_VFD_PERFCTR_VFD_SEL_7 },
};

static struct adreno_perfcount_register a4xx_perfcounters_hlsq[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_0_LO,
		A4XX_RBBM_PERFCTR_HLSQ_0_HI, 28, A4XX_HLSQ_PERFCTR_HLSQ_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_1_LO,
		A4XX_RBBM_PERFCTR_HLSQ_1_HI, 29, A4XX_HLSQ_PERFCTR_HLSQ_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_2_LO,
		A4XX_RBBM_PERFCTR_HLSQ_2_HI, 30, A4XX_HLSQ_PERFCTR_HLSQ_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_3_LO,
		A4XX_RBBM_PERFCTR_HLSQ_3_HI, 31, A4XX_HLSQ_PERFCTR_HLSQ_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_4_LO,
		A4XX_RBBM_PERFCTR_HLSQ_4_HI, 32, A4XX_HLSQ_PERFCTR_HLSQ_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_5_LO,
		A4XX_RBBM_PERFCTR_HLSQ_5_HI, 33, A4XX_HLSQ_PERFCTR_HLSQ_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_6_LO,
		A4XX_RBBM_PERFCTR_HLSQ_6_HI, 34, A4XX_HLSQ_PERFCTR_HLSQ_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_HLSQ_7_LO,
		A4XX_RBBM_PERFCTR_HLSQ_7_HI, 35, A4XX_HLSQ_PERFCTR_HLSQ_SEL_7 },
};

static struct adreno_perfcount_register a4xx_perfcounters_vpc[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VPC_0_LO,
		A4XX_RBBM_PERFCTR_VPC_0_HI, 36, A4XX_VPC_PERFCTR_VPC_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VPC_1_LO,
		A4XX_RBBM_PERFCTR_VPC_1_HI, 37, A4XX_VPC_PERFCTR_VPC_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VPC_2_LO,
		A4XX_RBBM_PERFCTR_VPC_2_HI, 38, A4XX_VPC_PERFCTR_VPC_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VPC_3_LO,
		A4XX_RBBM_PERFCTR_VPC_3_HI, 39, A4XX_VPC_PERFCTR_VPC_SEL_3 },
};

static struct adreno_perfcount_register a4xx_perfcounters_ccu[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CCU_0_LO,
		A4XX_RBBM_PERFCTR_CCU_0_HI, 40, A4XX_RB_PERFCTR_CCU_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CCU_1_LO,
		A4XX_RBBM_PERFCTR_CCU_1_HI, 41, A4XX_RB_PERFCTR_CCU_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CCU_2_LO,
		A4XX_RBBM_PERFCTR_CCU_2_HI, 42, A4XX_RB_PERFCTR_CCU_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_CCU_3_LO,
		A4XX_RBBM_PERFCTR_CCU_3_HI, 43, A4XX_RB_PERFCTR_CCU_SEL_3 },
};

static struct adreno_perfcount_register a4xx_perfcounters_tse[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TSE_0_LO,
		A4XX_RBBM_PERFCTR_TSE_0_HI, 44, A4XX_GRAS_PERFCTR_TSE_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TSE_1_LO,
		A4XX_RBBM_PERFCTR_TSE_1_HI, 45, A4XX_GRAS_PERFCTR_TSE_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TSE_2_LO,
		A4XX_RBBM_PERFCTR_TSE_2_HI, 46, A4XX_GRAS_PERFCTR_TSE_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TSE_3_LO,
		A4XX_RBBM_PERFCTR_TSE_3_HI, 47, A4XX_GRAS_PERFCTR_TSE_SEL_3 },
};


static struct adreno_perfcount_register a4xx_perfcounters_ras[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RAS_0_LO,
		A4XX_RBBM_PERFCTR_RAS_0_HI, 48, A4XX_GRAS_PERFCTR_RAS_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RAS_1_LO,
		A4XX_RBBM_PERFCTR_RAS_1_HI, 49, A4XX_GRAS_PERFCTR_RAS_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RAS_2_LO,
		A4XX_RBBM_PERFCTR_RAS_2_HI, 50, A4XX_GRAS_PERFCTR_RAS_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RAS_3_LO,
		A4XX_RBBM_PERFCTR_RAS_3_HI, 51, A4XX_GRAS_PERFCTR_RAS_SEL_3 },
};

static struct adreno_perfcount_register a4xx_perfcounters_uche[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_0_LO,
		A4XX_RBBM_PERFCTR_UCHE_0_HI, 52, A4XX_UCHE_PERFCTR_UCHE_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_1_LO,
		A4XX_RBBM_PERFCTR_UCHE_1_HI, 53, A4XX_UCHE_PERFCTR_UCHE_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_2_LO,
		A4XX_RBBM_PERFCTR_UCHE_2_HI, 54, A4XX_UCHE_PERFCTR_UCHE_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_3_LO,
		A4XX_RBBM_PERFCTR_UCHE_3_HI, 55, A4XX_UCHE_PERFCTR_UCHE_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_4_LO,
		A4XX_RBBM_PERFCTR_UCHE_4_HI, 56, A4XX_UCHE_PERFCTR_UCHE_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_5_LO,
		A4XX_RBBM_PERFCTR_UCHE_5_HI, 57, A4XX_UCHE_PERFCTR_UCHE_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_6_LO,
		A4XX_RBBM_PERFCTR_UCHE_6_HI, 58, A4XX_UCHE_PERFCTR_UCHE_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_UCHE_7_LO,
		A4XX_RBBM_PERFCTR_UCHE_7_HI, 59, A4XX_UCHE_PERFCTR_UCHE_SEL_7 },
};

static struct adreno_perfcount_register a4xx_perfcounters_tp[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_0_LO,
		A4XX_RBBM_PERFCTR_TP_0_HI, 60, A4XX_TPL1_PERFCTR_TP_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_1_LO,
		A4XX_RBBM_PERFCTR_TP_1_HI, 61, A4XX_TPL1_PERFCTR_TP_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_2_LO,
		A4XX_RBBM_PERFCTR_TP_2_HI, 62, A4XX_TPL1_PERFCTR_TP_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_3_LO,
		A4XX_RBBM_PERFCTR_TP_3_HI, 63, A4XX_TPL1_PERFCTR_TP_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_4_LO,
		A4XX_RBBM_PERFCTR_TP_4_HI, 64, A4XX_TPL1_PERFCTR_TP_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_5_LO,
		A4XX_RBBM_PERFCTR_TP_5_HI, 65, A4XX_TPL1_PERFCTR_TP_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_6_LO,
		A4XX_RBBM_PERFCTR_TP_6_HI, 66, A4XX_TPL1_PERFCTR_TP_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_TP_7_LO,
		A4XX_RBBM_PERFCTR_TP_7_HI, 67, A4XX_TPL1_PERFCTR_TP_SEL_7 },
};

static struct adreno_perfcount_register a4xx_perfcounters_sp[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_0_LO,
		A4XX_RBBM_PERFCTR_SP_0_HI, 68, A4XX_SP_PERFCTR_SP_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_1_LO,
		A4XX_RBBM_PERFCTR_SP_1_HI, 69, A4XX_SP_PERFCTR_SP_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_2_LO,
		A4XX_RBBM_PERFCTR_SP_2_HI, 70, A4XX_SP_PERFCTR_SP_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_3_LO,
		A4XX_RBBM_PERFCTR_SP_3_HI, 71, A4XX_SP_PERFCTR_SP_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_4_LO,
		A4XX_RBBM_PERFCTR_SP_4_HI, 72, A4XX_SP_PERFCTR_SP_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_5_LO,
		A4XX_RBBM_PERFCTR_SP_5_HI, 73, A4XX_SP_PERFCTR_SP_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_6_LO,
		A4XX_RBBM_PERFCTR_SP_6_HI, 74, A4XX_SP_PERFCTR_SP_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_7_LO,
		A4XX_RBBM_PERFCTR_SP_7_HI, 75, A4XX_SP_PERFCTR_SP_SEL_7 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_8_LO,
		A4XX_RBBM_PERFCTR_SP_8_HI, 76, A4XX_SP_PERFCTR_SP_SEL_8 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_9_LO,
		A4XX_RBBM_PERFCTR_SP_9_HI, 77, A4XX_SP_PERFCTR_SP_SEL_9 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_10_LO,
		A4XX_RBBM_PERFCTR_SP_10_HI, 78, A4XX_SP_PERFCTR_SP_SEL_10 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_SP_11_LO,
		A4XX_RBBM_PERFCTR_SP_11_HI, 79, A4XX_SP_PERFCTR_SP_SEL_11 },
};

static struct adreno_perfcount_register a4xx_perfcounters_rb[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_0_LO,
		A4XX_RBBM_PERFCTR_RB_0_HI, 80, A4XX_RB_PERFCTR_RB_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_1_LO,
		A4XX_RBBM_PERFCTR_RB_1_HI, 81, A4XX_RB_PERFCTR_RB_SEL_1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_2_LO,
		A4XX_RBBM_PERFCTR_RB_2_HI, 82, A4XX_RB_PERFCTR_RB_SEL_2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_3_LO,
		A4XX_RBBM_PERFCTR_RB_3_HI, 83, A4XX_RB_PERFCTR_RB_SEL_3 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_4_LO,
		A4XX_RBBM_PERFCTR_RB_4_HI, 84, A4XX_RB_PERFCTR_RB_SEL_4 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_5_LO,
		A4XX_RBBM_PERFCTR_RB_5_HI, 85, A4XX_RB_PERFCTR_RB_SEL_5 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_6_LO,
		A4XX_RBBM_PERFCTR_RB_6_HI, 86, A4XX_RB_PERFCTR_RB_SEL_6 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_RB_7_LO,
		A4XX_RBBM_PERFCTR_RB_7_HI, 87, A4XX_RB_PERFCTR_RB_SEL_7 },
};

static struct adreno_perfcount_register a4xx_perfcounters_vsc[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VSC_0_LO,
		A4XX_RBBM_PERFCTR_VSC_0_HI, 88, A4XX_VSC_PERFCTR_VSC_SEL_0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_VSC_1_LO,
		A4XX_RBBM_PERFCTR_VSC_1_HI, 89, A4XX_VSC_PERFCTR_VSC_SEL_1 },
};

static struct adreno_perfcount_register a4xx_perfcounters_pwr[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PWR_0_LO,
		A4XX_RBBM_PERFCTR_PWR_0_HI, -1, 0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_PERFCTR_PWR_1_LO,
		A4XX_RBBM_PERFCTR_PWR_1_HI, -1, 0},
};

static struct adreno_perfcount_register a4xx_perfcounters_vbif[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_CNT_LOW0,
		A4XX_VBIF_PERF_CNT_HIGH0, -1, A4XX_VBIF_PERF_CNT_SEL0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_CNT_LOW1,
		A4XX_VBIF_PERF_CNT_HIGH1, -1, A4XX_VBIF_PERF_CNT_SEL1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_CNT_LOW2,
		A4XX_VBIF_PERF_CNT_HIGH2, -1, A4XX_VBIF_PERF_CNT_SEL2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_CNT_LOW3,
		A4XX_VBIF_PERF_CNT_HIGH3, -1, A4XX_VBIF_PERF_CNT_SEL3 },
};

static struct adreno_perfcount_register a4xx_perfcounters_vbif_pwr[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_PWR_CNT_LOW0,
		A4XX_VBIF_PERF_PWR_CNT_HIGH0, -1, A4XX_VBIF_PERF_PWR_CNT_EN0 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_PWR_CNT_LOW1,
		A4XX_VBIF_PERF_PWR_CNT_HIGH1, -1, A4XX_VBIF_PERF_PWR_CNT_EN1 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_PWR_CNT_LOW2,
		A4XX_VBIF_PERF_PWR_CNT_HIGH2, -1, A4XX_VBIF_PERF_PWR_CNT_EN2 },
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_VBIF_PERF_PWR_CNT_LOW3,
		A4XX_VBIF_PERF_PWR_CNT_HIGH3, -1, A4XX_VBIF_PERF_PWR_CNT_EN3 },
};

static struct adreno_perfcount_register a4xx_perfcounters_alwayson[] = {
	{ KGSL_PERFCOUNTER_NOT_USED, 0, 0, A4XX_RBBM_ALWAYSON_COUNTER_LO,
		A4XX_RBBM_ALWAYSON_COUNTER_HI, -1 },
};

#define A4XX_PERFCOUNTER_GROUP(offset, name) \
	ADRENO_PERFCOUNTER_GROUP(a4xx, offset, name)

#define A4XX_PERFCOUNTER_GROUP_FLAGS(offset, name, flags) \
	ADRENO_PERFCOUNTER_GROUP_FLAGS(a4xx, offset, name, flags)

static struct adreno_perfcount_group a4xx_perfcounter_groups
				[KGSL_PERFCOUNTER_GROUP_MAX] = {
	A4XX_PERFCOUNTER_GROUP(CP, cp),
	A4XX_PERFCOUNTER_GROUP(RBBM, rbbm),
	A4XX_PERFCOUNTER_GROUP(PC, pc),
	A4XX_PERFCOUNTER_GROUP(VFD, vfd),
	A4XX_PERFCOUNTER_GROUP(HLSQ, hlsq),
	A4XX_PERFCOUNTER_GROUP(VPC, vpc),
	A4XX_PERFCOUNTER_GROUP(CCU, ccu),
	A4XX_PERFCOUNTER_GROUP(TSE, tse),
	A4XX_PERFCOUNTER_GROUP(RAS, ras),
	A4XX_PERFCOUNTER_GROUP(UCHE, uche),
	A4XX_PERFCOUNTER_GROUP(TP, tp),
	A4XX_PERFCOUNTER_GROUP(SP, sp),
	A4XX_PERFCOUNTER_GROUP(RB, rb),
	A4XX_PERFCOUNTER_GROUP(VSC, vsc),
	A4XX_PERFCOUNTER_GROUP_FLAGS(PWR, pwr,
		ADRENO_PERFCOUNTER_GROUP_FIXED),
	A4XX_PERFCOUNTER_GROUP(VBIF, vbif),
	A4XX_PERFCOUNTER_GROUP_FLAGS(VBIF_PWR, vbif_pwr,
		ADRENO_PERFCOUNTER_GROUP_FIXED),
	A4XX_PERFCOUNTER_GROUP_FLAGS(ALWAYSON, alwayson,
		ADRENO_PERFCOUNTER_GROUP_FIXED),
};

static struct adreno_perfcounters a4xx_perfcounters = {
	a4xx_perfcounter_groups,
	ARRAY_SIZE(a4xx_perfcounter_groups),
};

static struct adreno_ft_perf_counters a4xx_ft_perf_counters[] = {
	{KGSL_PERFCOUNTER_GROUP_SP, A4XX_SP_ALU_ACTIVE_CYCLES},
	{KGSL_PERFCOUNTER_GROUP_SP, A4XX_SP0_ICL1_MISSES},
	{KGSL_PERFCOUNTER_GROUP_SP, A4XX_SP_FS_CFLOW_INSTRUCTIONS},
	{KGSL_PERFCOUNTER_GROUP_TSE, A4XX_TSE_INPUT_PRIM_NUM},
};

/*
 * On A420 a number of perfcounters are un-usable. The following defines the
 * array of countables that do not work and should not be used
 */
static const unsigned int a420_pc_invalid_countables[] = {
	PC_INSTANCES, PC_VERTEX_HITS, PC_GENERATED_FIBERS, PC_GENERATED_WAVES,
};

static const unsigned int a420_vfd_invalid_countables[] = {
	VFD_VPC_BYPASS_TRANS, VFD_UPPER_SHADER_FIBERS, VFD_LOWER_SHADER_FIBERS,
};

static const unsigned int a420_hlsq_invalid_countables[] = {
	HLSQ_SP_VS_STAGE_CONSTANT, HLSQ_SP_VS_STAGE_INSTRUCTIONS,
	HLSQ_SP_FS_STAGE_CONSTANT, HLSQ_SP_FS_STAGE_INSTRUCTIONS,
	HLSQ_FS_STAGE_16_WAVES, HLSQ_FS_STAGE_32_WAVES, HLSQ_FS_STAGE_64_WAVES,
	HLSQ_VS_STAGE_16_WAVES, HLSQ_VS_STAGE_32_WAVES,
};

static const unsigned int a420_uche_invalid_countables[] = {
	UCHE_READ_REQUESTS_MARB, UCHE_READ_REQUESTS_SP,
	UCHE_WRITE_REQUESTS_MARB, UCHE_WRITE_REQUESTS_SP,
	UCHE_WRITE_REQUESTS_VPC
};

static const unsigned int a420_tp_invalid_countables[] = {
	TP_OUTPUT_TEXELS_POINT, TP_OUTPUT_TEXELS_BILINEAR, TP_OUTPUT_TEXELS_MIP,
	TP_OUTPUT_TEXELS_ANISO, TP_OUTPUT_TEXELS_OPS16, TP_OUTPUT_TEXELS_OPS32,
	TP_ZERO_LOD, TP_LATENCY, TP_LATENCY_TRANS,
};

static const unsigned int a420_sp_invalid_countables[] = {
	SP_FS_STAGE_BARY_INSTRUCTIONS,
};

static const unsigned int a420_rb_invalid_countables[] = {
	RB_VALID_SAMPLES, RB_Z_FAIL, RB_S_FAIL,
};

static const unsigned int a420_ccu_invalid_countables[] = {
	CCU_VBIF_STALL, CCU_VBIF_LATENCY_CYCLES, CCU_VBIF_LATENCY_SAMPLES,
	CCU_Z_READ, CCU_Z_WRITE, CCU_C_READ, CCU_C_WRITE,
};

static const struct adreno_invalid_countables
	a420_perfctr_invalid_countables[KGSL_PERFCOUNTER_GROUP_MAX] = {
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_pc, PC),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_vfd, VFD),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_hlsq, HLSQ),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_tp, TP),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_sp, SP),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_rb, RB),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_ccu, CCU),
	ADRENO_PERFCOUNTER_INVALID_COUNTABLE(a420_uche, UCHE),
};

static struct adreno_coresight_register a4xx_coresight_registers[] = {
	{ A4XX_RBBM_CFG_DEBBUS_CTLT },
	{ A4XX_RBBM_CFG_DEBBUS_SEL_A },
	{ A4XX_RBBM_CFG_DEBBUS_SEL_B },
	{ A4XX_RBBM_CFG_DEBBUS_SEL_C },
	{ A4XX_RBBM_CFG_DEBBUS_SEL_D },
	{ A4XX_RBBM_CFG_DEBBUS_OPL },
	{ A4XX_RBBM_CFG_DEBBUS_OPE },
	{ A4XX_RBBM_CFG_DEBBUS_IVTL_0 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTL_1 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTL_2 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTL_3 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKL_0 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKL_1 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKL_2 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKL_3 },
	{ A4XX_RBBM_CFG_DEBBUS_BYTEL_0 },
	{ A4XX_RBBM_CFG_DEBBUS_BYTEL_1 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTE_0 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTE_1 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTE_2 },
	{ A4XX_RBBM_CFG_DEBBUS_IVTE_3 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKE_0 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKE_1 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKE_2 },
	{ A4XX_RBBM_CFG_DEBBUS_MASKE_3 },
	{ A4XX_RBBM_CFG_DEBBUS_NIBBLEE },
	{ A4XX_RBBM_CFG_DEBBUS_PTRC0 },
	{ A4XX_RBBM_CFG_DEBBUS_PTRC1 },
	{ A4XX_RBBM_CFG_DEBBUS_CLRC },
	{ A4XX_RBBM_CFG_DEBBUS_LOADIVT },
	{ A4XX_RBBM_CFG_DEBBUS_IDX },
	{ A4XX_RBBM_CFG_DEBBUS_LOADREG },
	{ A4XX_RBBM_EXT_TRACE_BUS_CTL },
	{ A4XX_RBBM_CFG_DEBBUS_CTLM },
};

static void a4xx_perfcounter_init(struct adreno_device *adreno_dev)
{
	if (adreno_is_a420(adreno_dev)) {
		struct adreno_gpudev *gpudev = ADRENO_GPU_DEVICE(adreno_dev);
		struct adreno_perfcounters *counters = gpudev->perfcounters;

		/*
		 * The CP counters on A420 are... special.  Some of the counters
		 * are swizzled so only a subset of them are usable
		 */

		if (counters != NULL) {
			counters->groups[KGSL_PERFCOUNTER_GROUP_CP].regs =
				a420_perfcounters_cp;
			counters->groups[KGSL_PERFCOUNTER_GROUP_CP].reg_count =
				ARRAY_SIZE(a420_perfcounters_cp);
		}

		/*
		 * Also on A420 a number of the countables are not functional so
		 * we maintain a blacklist of countables to protect the user
		 */

		gpudev->invalid_countables = a420_perfctr_invalid_countables;
	}

	/*
	 * Enable the GPU busy count counter. This is a fixed counter on
	 * A4XX so we don't need to bother checking the return value
	 */

	adreno_perfcounter_get(adreno_dev, KGSL_PERFCOUNTER_GROUP_PWR, 1,
		NULL, NULL, PERFCOUNTER_FLAG_KERNEL);
}

static void a4xx_perfcounter_close(struct adreno_device *adreno_dev)
{
	adreno_perfcounter_put(adreno_dev, KGSL_PERFCOUNTER_GROUP_PWR, 1,
		PERFCOUNTER_FLAG_KERNEL);
}

static const unsigned int _a4xx_pwron_fixup_fs_instructions[] = {
	0x00000000, 0x304CC300, 0x00000000, 0x304CC304,
	0x00000000, 0x304CC308, 0x00000000, 0x304CC30C,
	0x00000000, 0x304CC310, 0x00000000, 0x304CC314,
	0x00000000, 0x304CC318, 0x00000000, 0x304CC31C,
	0x00000000, 0x304CC320, 0x00000000, 0x304CC324,
	0x00000000, 0x304CC328, 0x00000000, 0x304CC32C,
	0x00000000, 0x304CC330, 0x00000000, 0x304CC334,
	0x00000000, 0x304CC338, 0x00000000, 0x304CC33C,
	0x00000000, 0x00000400, 0x00020000, 0x63808003,
	0x00060004, 0x63828007, 0x000A0008, 0x6384800B,
	0x000E000C, 0x6386800F, 0x00120010, 0x63888013,
	0x00160014, 0x638A8017, 0x001A0018, 0x638C801B,
	0x001E001C, 0x638E801F, 0x00220020, 0x63908023,
	0x00260024, 0x63928027, 0x002A0028, 0x6394802B,
	0x002E002C, 0x6396802F, 0x00320030, 0x63988033,
	0x00360034, 0x639A8037, 0x003A0038, 0x639C803B,
	0x003E003C, 0x639E803F, 0x00000000, 0x00000400,
	0x00000003, 0x80D00003, 0x00000007, 0x80D00007,
	0x0000000B, 0x80D0000B, 0x0000000F, 0x80D0000F,
	0x00000013, 0x80D00013, 0x00000017, 0x80D00017,
	0x0000001B, 0x80D0001B, 0x0000001F, 0x80D0001F,
	0x00000023, 0x80D00023, 0x00000027, 0x80D00027,
	0x0000002B, 0x80D0002B, 0x0000002F, 0x80D0002F,
	0x00000033, 0x80D00033, 0x00000037, 0x80D00037,
	0x0000003B, 0x80D0003B, 0x0000003F, 0x80D0003F,
	0x00000000, 0x00000400, 0xFFFFFFFF, 0x304CC300,
	0xFFFFFFFF, 0x304CC304, 0xFFFFFFFF, 0x304CC308,
	0xFFFFFFFF, 0x304CC30C, 0xFFFFFFFF, 0x304CC310,
	0xFFFFFFFF, 0x304CC314, 0xFFFFFFFF, 0x304CC318,
	0xFFFFFFFF, 0x304CC31C, 0xFFFFFFFF, 0x304CC320,
	0xFFFFFFFF, 0x304CC324, 0xFFFFFFFF, 0x304CC328,
	0xFFFFFFFF, 0x304CC32C, 0xFFFFFFFF, 0x304CC330,
	0xFFFFFFFF, 0x304CC334, 0xFFFFFFFF, 0x304CC338,
	0xFFFFFFFF, 0x304CC33C, 0x00000000, 0x00000400,
	0x00020000, 0x63808003, 0x00060004, 0x63828007,
	0x000A0008, 0x6384800B, 0x000E000C, 0x6386800F,
	0x00120010, 0x63888013, 0x00160014, 0x638A8017,
	0x001A0018, 0x638C801B, 0x001E001C, 0x638E801F,
	0x00220020, 0x63908023, 0x00260024, 0x63928027,
	0x002A0028, 0x6394802B, 0x002E002C, 0x6396802F,
	0x00320030, 0x63988033, 0x00360034, 0x639A8037,
	0x003A0038, 0x639C803B, 0x003E003C, 0x639E803F,
	0x00000000, 0x00000400, 0x00000003, 0x80D00003,
	0x00000007, 0x80D00007, 0x0000000B, 0x80D0000B,
	0x0000000F, 0x80D0000F, 0x00000013, 0x80D00013,
	0x00000017, 0x80D00017, 0x0000001B, 0x80D0001B,
	0x0000001F, 0x80D0001F, 0x00000023, 0x80D00023,
	0x00000027, 0x80D00027, 0x0000002B, 0x80D0002B,
	0x0000002F, 0x80D0002F, 0x00000033, 0x80D00033,
	0x00000037, 0x80D00037, 0x0000003B, 0x80D0003B,
	0x0000003F, 0x80D0003F, 0x00000000, 0x03000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/**
 * _a4xx_pwron_fixup() - Initialize a special command buffer to run a
 * post-power collapse shader workaround
 * @adreno_dev: Pointer to a adreno_device struct
 *
 * Some targets require a special workaround shader to be executed after
 * power-collapse.  Construct the IB once at init time and keep it
 * handy
 *
 * Returns: 0 on success or negative on error
 */
static int _a4xx_pwron_fixup(struct adreno_device *adreno_dev)
{
	unsigned int *cmds;
	unsigned int count = ARRAY_SIZE(_a4xx_pwron_fixup_fs_instructions);
	unsigned int num_units = count >> 5;
	int ret;

	/* Return if the fixup is already in place */
	if (test_bit(ADRENO_DEVICE_PWRON_FIXUP, &adreno_dev->priv))
			return 0;

	ret = kgsl_allocate_global(KGSL_DEVICE(adreno_dev),
		&adreno_dev->pwron_fixup, PAGE_SIZE,
		KGSL_MEMFLAGS_GPUREADONLY, 0);

	if (ret)
		return ret;

	cmds = adreno_dev->pwron_fixup.hostptr;

	*cmds++ = cp_type3_packet(CP_WAIT_FOR_IDLE, 1);
	*cmds++ = 0x00000000;
	*cmds++ = cp_type0_packet(A4XX_SP_MODE_CONTROL, 1);
	*cmds++ = 0x00000018;
	*cmds++ = cp_type0_packet(A4XX_TPL1_TP_MODE_CONTROL, 1);
	*cmds++ = 0x00000002;
	*cmds++ = cp_type3_packet(CP_WAIT_FOR_IDLE, 1);
	*cmds++ = 0x00000000;
	*cmds++ = cp_type0_packet(A4xx_HLSQ_CONTROL_0, 5);
	*cmds++ = 0x800001a0;
	*cmds++ = 0xfcfc0000;
	*cmds++ = 0xcff3f3f0;
	*cmds++ = 0xfcfcfcfc;
	*cmds++ = 0xccfcfcfc;
	*cmds++ = cp_type0_packet(A4XX_SP_FS_CTRL_1, 1);
	*cmds++ = 0x80000000;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_UPDATE_CONTROL, 1);
	*cmds++ = 0x00000038;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_MODE_CONTROL, 1);
	*cmds++ = 0x00000003;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_UPDATE_CONTROL, 1);
	*cmds++ = 0x00000000;
	*cmds++ = cp_type0_packet(A4XX_TPL1_TP_TEX_TSIZE_1, 1);
	*cmds++ = 0x00008000;
	*cmds++ = cp_type0_packet(A4xx_HLSQ_CONTROL_0, 2);
	*cmds++ = 0x800001a0;
	*cmds++ = 0xfcfc0000;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_CS_CONTROL, 1);
	*cmds++ = 0x00018030 | (num_units << 24);
	*cmds++ = cp_type0_packet(A4XX_HLSQ_CL_NDRANGE_0, 7);
	*cmds++ = 0x000000fd;
	*cmds++ = 0x00000040;
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000001;
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000001;
	*cmds++ = 0x00000000;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_CL_CONTROL_0, 2);
	*cmds++ = 0x0001201f;
	*cmds++ = 0x0000f003;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_CL_KERNEL_CONST, 1);
	*cmds++ = 0x0001800b;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_CL_KERNEL_GROUP_X, 3);
	*cmds++ = 0x00000001;
	*cmds++ = 0x00000001;
	*cmds++ = 0x00000001;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_CL_WG_OFFSET, 1);
	*cmds++ = 0x00000022;
	*cmds++ = cp_type0_packet(A4XX_UCHE_INVALIDATE0, 2);
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000012;
	*cmds++ = cp_type0_packet(A4XX_HLSQ_MODE_CONTROL, 1);
	*cmds++ = 0x00000003;
	*cmds++ = cp_type0_packet(A4XX_SP_SP_CTRL, 1);
	*cmds++ = 0x00920000;
	*cmds++ = cp_type0_packet(A4XX_SP_INSTR_CACHE_CTRL, 1);
	*cmds++ = 0x00000260;
	*cmds++ = cp_type0_packet(A4XX_SP_CS_CTRL_0, 1);
	*cmds++ = 0x00200400;
	*cmds++ = cp_type0_packet(A4XX_SP_CS_OBJ_OFFSET, 1);
	*cmds++ = 0x00000000;
	*cmds++ = cp_type0_packet(A4XX_SP_CS_OBJ_START, 1);
	*cmds++ = 0x00000000;
	*cmds++ = cp_type0_packet(A4XX_SP_CS_LENGTH, 1);
	*cmds++ =  num_units;
	*cmds++ = cp_type0_packet(A4XX_SP_MODE_CONTROL, 1);
	*cmds++ = 0x00000018;
	*cmds++ = cp_type3_packet(CP_LOAD_STATE, 2 + count);
	*cmds++ = 0x00340000 | (num_units << CP_LOADSTATE_NUMOFUNITS_SHIFT);
	*cmds++ = 0x00000000;

	memcpy(cmds, _a4xx_pwron_fixup_fs_instructions, count << 2);
	cmds += count;

	*cmds++ = cp_type3_packet(CP_EXEC_CL, 1);
	*cmds++ = 0x00000000;
	*cmds++ = cp_type3_packet(CP_WAIT_FOR_IDLE, 1);
	*cmds++ = 0x00000000;

	/*
	 * Remember the number of dwords in the command buffer for when we
	 * program the indirect buffer call in the ringbuffer
	 */
	adreno_dev->pwron_fixup_dwords =
		(cmds - (unsigned int *) adreno_dev->pwron_fixup.hostptr);

	/* Mark the flag in ->priv to show that we have the fix */
	set_bit(ADRENO_DEVICE_PWRON_FIXUP, &adreno_dev->priv);
	return 0;
}

/*
 * a4xx_init() - Initialize gpu specific data
 * @adreno_dev: Pointer to adreno device
 */
static void a4xx_init(struct adreno_device *adreno_dev)
{
	if ((adreno_is_a405(adreno_dev)) || (adreno_is_a420(adreno_dev)))
		_a4xx_pwron_fixup(adreno_dev);
}

static int a4xx_send_me_init(struct adreno_device *adreno_dev,
			 struct adreno_ringbuffer *rb)
{
	unsigned int *cmds;
	int ret;

	cmds = adreno_ringbuffer_allocspace(rb, 20);
	if (IS_ERR(cmds))
		return PTR_ERR(cmds);
	if (cmds == NULL)
		return -ENOSPC;

	*cmds++ = cp_type3_packet(CP_ME_INIT, 17);

	/*
	 * Ordinal 2 of ME_INIT packet, the bits being set are:
	 * Ordinal 3, 4, 5-12, 14, 15, 16, 17, 18 are present
	 * Microcode Default Reset Control = 3
	 */
	*cmds++ = 0x000003f7;
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000080;
	*cmds++ = 0x00000100;
	*cmds++ = 0x00000180;
	*cmds++ = 0x00006600;
	*cmds++ = 0x00000150;
	*cmds++ = 0x0000014e;
	*cmds++ = 0x00000154;
	/* MAX Context */
	*cmds++ = 0x00000001;
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000000;

	/* Enable protected mode registers for A3XX/A4XX */
	*cmds++ = 0x20000000;

	*cmds++ = 0x00000000;
	*cmds++ = 0x00000000;

	*cmds++ = cp_type3_packet(CP_PREEMPT_ENABLE, 1);
	*cmds++ = 1;

	ret = adreno_ringbuffer_submit_spin(rb, NULL, 2000);
	if (ret) {
		struct kgsl_device *device = KGSL_DEVICE(adreno_dev);

		dev_err(device->dev, "CP initialization failed to idle\n");
		kgsl_device_snapshot(device, NULL);
	}

	return ret;
}

/*
 * a4xx_rb_start() - Start the ringbuffer
 * @adreno_dev: Pointer to adreno device
 * @start_type: Warm or cold start
 */
static int a4xx_rb_start(struct adreno_device *adreno_dev,
			 unsigned int start_type)
{
	struct adreno_ringbuffer *rb = ADRENO_CURRENT_RINGBUFFER(adreno_dev);
	struct kgsl_device *device = &adreno_dev->dev;
	uint64_t addr;
	int ret;

	addr = SCRATCH_RPTR_GPU_ADDR(device, rb->id);

	adreno_writereg64(adreno_dev, ADRENO_REG_CP_RB_RPTR_ADDR_LO,
			ADRENO_REG_CP_RB_RPTR_ADDR_HI, addr);

	/*
	 * The size of the ringbuffer in the hardware is the log2
	 * representation of the size in quadwords (sizedwords / 2).
	 * Also disable the host RPTR shadow register as it might be unreliable
	 * in certain circumstances.
	 */

	adreno_writereg(adreno_dev, ADRENO_REG_CP_RB_CNTL,
			((ilog2(4) << 8) & 0x1F00) |
			(ilog2(KGSL_RB_DWORDS >> 1) & 0x3F));

	adreno_writereg(adreno_dev, ADRENO_REG_CP_RB_BASE,
			  rb->buffer_desc.gpuaddr);

	ret = a3xx_microcode_load(adreno_dev, start_type);
	if (ret)
		return ret;

	/* clear ME_HALT to start micro engine */
	adreno_writereg(adreno_dev, ADRENO_REG_CP_ME_CNTL, 0);

	ret = a4xx_send_me_init(adreno_dev, rb);
	if (ret == 0) {
		a4xx_enable_pc(adreno_dev);
		a4xx_enable_ppd(adreno_dev);
	}

	return ret;
}

static ADRENO_CORESIGHT_ATTR(cfg_debbus_ctrlt, &a4xx_coresight_registers[0]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_sela, &a4xx_coresight_registers[1]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_selb, &a4xx_coresight_registers[2]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_selc, &a4xx_coresight_registers[3]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_seld, &a4xx_coresight_registers[4]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_opl, &a4xx_coresight_registers[5]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ope, &a4xx_coresight_registers[6]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivtl0, &a4xx_coresight_registers[7]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivtl1, &a4xx_coresight_registers[8]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivtl2, &a4xx_coresight_registers[9]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivtl3, &a4xx_coresight_registers[10]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maskl0, &a4xx_coresight_registers[11]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maskl1, &a4xx_coresight_registers[12]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maskl2, &a4xx_coresight_registers[13]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maskl3, &a4xx_coresight_registers[14]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_bytel0, &a4xx_coresight_registers[15]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_bytel1, &a4xx_coresight_registers[16]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivte0, &a4xx_coresight_registers[17]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivte1, &a4xx_coresight_registers[18]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivte2, &a4xx_coresight_registers[19]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ivte3, &a4xx_coresight_registers[20]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maske0, &a4xx_coresight_registers[21]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maske1, &a4xx_coresight_registers[22]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maske2, &a4xx_coresight_registers[23]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_maske3, &a4xx_coresight_registers[24]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_nibblee, &a4xx_coresight_registers[25]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ptrc0, &a4xx_coresight_registers[26]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ptrc1, &a4xx_coresight_registers[27]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_clrc, &a4xx_coresight_registers[28]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_loadivt, &a4xx_coresight_registers[29]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_idx, &a4xx_coresight_registers[30]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_loadreg, &a4xx_coresight_registers[31]);
static ADRENO_CORESIGHT_ATTR(ext_tracebus_ctl, &a4xx_coresight_registers[32]);
static ADRENO_CORESIGHT_ATTR(cfg_debbus_ctrlm, &a4xx_coresight_registers[33]);


static struct attribute *a4xx_coresight_attrs[] = {
	&coresight_attr_cfg_debbus_ctrlt.attr.attr,
	&coresight_attr_cfg_debbus_sela.attr.attr,
	&coresight_attr_cfg_debbus_selb.attr.attr,
	&coresight_attr_cfg_debbus_selc.attr.attr,
	&coresight_attr_cfg_debbus_seld.attr.attr,
	&coresight_attr_cfg_debbus_opl.attr.attr,
	&coresight_attr_cfg_debbus_ope.attr.attr,
	&coresight_attr_cfg_debbus_ivtl0.attr.attr,
	&coresight_attr_cfg_debbus_ivtl1.attr.attr,
	&coresight_attr_cfg_debbus_ivtl2.attr.attr,
	&coresight_attr_cfg_debbus_ivtl3.attr.attr,
	&coresight_attr_cfg_debbus_maskl0.attr.attr,
	&coresight_attr_cfg_debbus_maskl1.attr.attr,
	&coresight_attr_cfg_debbus_maskl2.attr.attr,
	&coresight_attr_cfg_debbus_maskl3.attr.attr,
	&coresight_attr_cfg_debbus_bytel0.attr.attr,
	&coresight_attr_cfg_debbus_bytel1.attr.attr,
	&coresight_attr_cfg_debbus_ivte0.attr.attr,
	&coresight_attr_cfg_debbus_ivte1.attr.attr,
	&coresight_attr_cfg_debbus_ivte2.attr.attr,
	&coresight_attr_cfg_debbus_ivte3.attr.attr,
	&coresight_attr_cfg_debbus_maske0.attr.attr,
	&coresight_attr_cfg_debbus_maske1.attr.attr,
	&coresight_attr_cfg_debbus_maske2.attr.attr,
	&coresight_attr_cfg_debbus_maske3.attr.attr,
	&coresight_attr_cfg_debbus_nibblee.attr.attr,
	&coresight_attr_cfg_debbus_ptrc0.attr.attr,
	&coresight_attr_cfg_debbus_ptrc1.attr.attr,
	&coresight_attr_cfg_debbus_clrc.attr.attr,
	&coresight_attr_cfg_debbus_loadivt.attr.attr,
	&coresight_attr_cfg_debbus_idx.attr.attr,
	&coresight_attr_cfg_debbus_loadreg.attr.attr,
	&coresight_attr_ext_tracebus_ctl.attr.attr,
	&coresight_attr_cfg_debbus_ctrlm.attr.attr,
	NULL,
};

static const struct attribute_group a4xx_coresight_group = {
	.attrs = a4xx_coresight_attrs,
};

static const struct attribute_group *a4xx_coresight_groups[] = {
	&a4xx_coresight_group,
	NULL,
};

static struct adreno_coresight a4xx_coresight = {
	.registers = a4xx_coresight_registers,
	.count = ARRAY_SIZE(a4xx_coresight_registers),
	.groups = a4xx_coresight_groups,
};

static void a4xx_preempt_callback(struct adreno_device *adreno_dev, int bit)
{
	if (atomic_read(&adreno_dev->preempt.state) != ADRENO_PREEMPT_TRIGGERED)
		return;

	trace_adreno_hw_preempt_trig_to_comp_int(adreno_dev->cur_rb,
			      adreno_dev->next_rb,
			      adreno_get_rptr(adreno_dev->cur_rb),
			      adreno_get_rptr(adreno_dev->next_rb));

	adreno_dispatcher_schedule(KGSL_DEVICE(adreno_dev));
}

#define A4XX_INT_MASK \
	((1 << A4XX_INT_RBBM_AHB_ERROR) |		\
	 (1 << A4XX_INT_RBBM_REG_TIMEOUT) |		\
	 (1 << A4XX_INT_RBBM_ME_MS_TIMEOUT) |		\
	 (1 << A4XX_INT_RBBM_PFP_MS_TIMEOUT) |		\
	 (1 << A4XX_INT_RBBM_ETS_MS_TIMEOUT) |		\
	 (1 << A4XX_INT_RBBM_ASYNC_OVERFLOW) |		\
	 (1 << A4XX_INT_CP_SW) |			\
	 (1 << A4XX_INT_CP_OPCODE_ERROR) |		\
	 (1 << A4XX_INT_CP_RESERVED_BIT_ERROR) |	\
	 (1 << A4XX_INT_CP_HW_FAULT) |			\
	 (1 << A4XX_INT_CP_IB1_INT) |			\
	 (1 << A4XX_INT_CP_IB2_INT) |			\
	 (1 << A4XX_INT_CP_RB_INT) |			\
	 (1 << A4XX_INT_CACHE_FLUSH_TS) |		\
	 (1 << A4XX_INT_CP_REG_PROTECT_FAULT) |		\
	 (1 << A4XX_INT_CP_AHB_ERROR_HALT) |		\
	 (1 << A4XX_INT_RBBM_ATB_BUS_OVERFLOW) |	\
	 (1 << A4XX_INT_UCHE_OOB_ACCESS) |		\
	 (1 << A4XX_INT_RBBM_DPM_CALC_ERR) |		\
	 (1 << A4XX_INT_RBBM_DPM_EPOCH_ERR) |		\
	 (1 << A4XX_INT_RBBM_DPM_THERMAL_YELLOW_ERR) |\
	 (1 << A4XX_INT_RBBM_DPM_THERMAL_RED_ERR))


static struct adreno_irq_funcs a4xx_irq_funcs[32] = {
	ADRENO_IRQ_CALLBACK(NULL),                   /* 0 - RBBM_GPU_IDLE */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 1 - RBBM_AHB_ERROR */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 2 - RBBM_REG_TIMEOUT */
	/* 3 - RBBM_ME_MS_TIMEOUT */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	/* 4 - RBBM_PFP_MS_TIMEOUT */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 5 - RBBM_ETS_MS_TIMEOUT */
	/* 6 - RBBM_ATB_ASYNC_OVERFLOW */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	ADRENO_IRQ_CALLBACK(NULL), /* 7 - RBBM_GPC_ERR */
	ADRENO_IRQ_CALLBACK(a4xx_preempt_callback), /* 8 - CP_SW */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 9 - CP_OPCODE_ERROR */
	/* 10 - CP_RESERVED_BIT_ERROR */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 11 - CP_HW_FAULT */
	ADRENO_IRQ_CALLBACK(NULL), /* 12 - CP_DMA */
	ADRENO_IRQ_CALLBACK(adreno_cp_callback), /* 13 - CP_IB2_INT */
	ADRENO_IRQ_CALLBACK(adreno_cp_callback), /* 14 - CP_IB1_INT */
	ADRENO_IRQ_CALLBACK(adreno_cp_callback), /* 15 - CP_RB_INT */
	/* 16 - CP_REG_PROTECT_FAULT */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	ADRENO_IRQ_CALLBACK(NULL), /* 17 - CP_RB_DONE_TS */
	ADRENO_IRQ_CALLBACK(NULL), /* 18 - CP_VS_DONE_TS */
	ADRENO_IRQ_CALLBACK(NULL), /* 19 - CP_PS_DONE_TS */
	ADRENO_IRQ_CALLBACK(adreno_cp_callback), /* 20 - CP_CACHE_FLUSH_TS */
	/* 21 - CP_AHB_ERROR_FAULT */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 22 - RBBM_ATB_BUS_OVERFLOW */
	ADRENO_IRQ_CALLBACK(NULL), /* 23 - Unused */
	/* 24 - MISC_HANG_DETECT */
	ADRENO_IRQ_CALLBACK(adreno_hang_int_callback),
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 25 - UCHE_OOB_ACCESS */
	ADRENO_IRQ_CALLBACK(NULL), /* 26 - Unused */
	ADRENO_IRQ_CALLBACK(NULL), /* 27 - RBBM_TRACE_MISR */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 28 - RBBM_DPM_CALC_ERR */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback), /* 29 - RBBM_DPM_EPOCH_ERR */
	/* 30 - RBBM_DPM_THERMAL_YELLOW_ERR */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
	/* 31 - RBBM_DPM_THERMAL_RED_ERR */
	ADRENO_IRQ_CALLBACK(a4xx_err_callback),
};

static struct adreno_irq a4xx_irq = {
	.funcs = a4xx_irq_funcs,
	.mask = A4XX_INT_MASK,
};

static struct adreno_snapshot_data a4xx_snapshot_data = {
	.sect_sizes = &a4xx_snap_sizes,
};

struct adreno_gpudev adreno_a4xx_gpudev = {
	.reg_offsets = &a4xx_reg_offsets,
	.ft_perf_counters = a4xx_ft_perf_counters,
	.ft_perf_counters_count = ARRAY_SIZE(a4xx_ft_perf_counters),
	.perfcounters = &a4xx_perfcounters,
	.irq = &a4xx_irq,
	.irq_trace = trace_kgsl_a4xx_irq_status,
	.snapshot_data = &a4xx_snapshot_data,
	.num_prio_levels = KGSL_PRIORITY_MAX_RB_LEVELS,
	.vbif_xin_halt_ctrl0_mask = A4XX_VBIF_XIN_HALT_CTRL0_MASK,

	.perfcounter_init = a4xx_perfcounter_init,
	.perfcounter_close = a4xx_perfcounter_close,
	.rb_start = a4xx_rb_start,
	.init = a4xx_init,
	.microcode_read = a3xx_microcode_read,
	.coresight = &a4xx_coresight,
	.start = a4xx_start,
	.snapshot = a4xx_snapshot,
	.is_sptp_idle = a4xx_is_sptp_idle,
	.pwrlevel_change_settings = a4xx_pwrlevel_change_settings,
	.regulator_enable = a4xx_regulator_enable,
	.regulator_disable = a4xx_regulator_disable,
	.preemption_pre_ibsubmit = a4xx_preemption_pre_ibsubmit,
	.preemption_schedule = a4xx_preemption_schedule,
	.preemption_init = a4xx_preemption_init,
};

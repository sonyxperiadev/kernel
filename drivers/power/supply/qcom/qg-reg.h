/* Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __QG_REG_H__
#define __QG_REG_H__

#define PERPH_TYPE_REG				0x04
#define QG_TYPE					0x0D

#define QG_STATUS1_REG				0x08
#define QG_OK_BIT				BIT(7)
#define BATTERY_PRESENT_BIT			BIT(0)
#define ESR_MEAS_DONE_BIT			BIT(4)

#define QG_STATUS2_REG				0x09
#define GOOD_OCV_BIT				BIT(1)

#define QG_STATUS3_REG				0x0A
#define COUNT_FIFO_RT_MASK			GENMASK(3, 0)

#define QG_STATUS4_REG				0x0B
#define ESR_MEAS_IN_PROGRESS_BIT		BIT(4)

#define QG_INT_RT_STS_REG			0x10
#define FIFO_UPDATE_DONE_RT_STS_BIT		BIT(3)
#define VBAT_LOW_INT_RT_STS_BIT			BIT(1)
#define BATTERY_MISSING_INT_RT_STS_BIT		BIT(0)

#define QG_INT_LATCHED_STS_REG			0x18
#define FIFO_UPDATE_DONE_INT_LAT_STS_BIT	BIT(3)

#define QG_DATA_CTL1_REG			0x41
#define MASTER_HOLD_OR_CLR_BIT			BIT(0)

#define QG_DATA_CTL2_REG			0x42
#define BURST_AVG_HOLD_FOR_READ_BIT		BIT(0)

#define QG_MODE_CTL1_REG			0x43
#define PARALLEL_IBAT_SENSE_EN_BIT		BIT(7)

#define QG_VBAT_EMPTY_THRESHOLD_REG		0x4B
#define QG_VBAT_LOW_THRESHOLD_REG		0x4C

#define QG_S2_NORMAL_MEAS_CTL2_REG		0x51
#define FIFO_LENGTH_MASK			GENMASK(5, 3)
#define FIFO_LENGTH_SHIFT			3
#define NUM_OF_ACCUM_MASK			GENMASK(2, 0)

#define QG_S2_NORMAL_MEAS_CTL3_REG		0x52

#define QG_S3_SLEEP_OCV_MEAS_CTL4_REG		0x59
#define S3_SLEEP_OCV_TIMER_MASK			GENMASK(2, 0)

#define QG_S3_SLEEP_OCV_TREND_CTL2_REG		0x5C
#define TREND_TOL_MASK				GENMASK(5, 0)

#define QG_S3_SLEEP_OCV_IBAT_CTL1_REG		0x5D
#define SLEEP_IBAT_QUALIFIED_LENGTH_MASK	GENMASK(2, 0)

#define QG_S3_ENTRY_IBAT_THRESHOLD_REG		0x5E
#define QG_S3_EXIT_IBAT_THRESHOLD_REG		0x5F

#define QG_S5_OCV_VALIDATE_MEAS_CTL1_REG	0x60
#define ALLOW_S5_BIT				BIT(7)

#define QG_S7_PON_OCV_MEAS_CTL1_REG		0x64
#define ADC_CONV_DLY_MASK			GENMASK(3, 0)

#define QG_ESR_MEAS_TRIG_REG			0x68
#define HW_ESR_MEAS_START_BIT			BIT(0)

#define QG_S7_PON_OCV_V_DATA0_REG		0x70
#define QG_S7_PON_OCV_I_DATA0_REG		0x72
#define QG_S3_GOOD_OCV_V_DATA0_REG		0x74
#define QG_S3_GOOD_OCV_I_DATA0_REG		0x76

#define QG_PRE_ESR_V_DATA0_REG			0x78
#define QG_PRE_ESR_I_DATA0_REG			0x7A
#define QG_POST_ESR_V_DATA0_REG			0x7C
#define QG_POST_ESR_I_DATA0_REG			0x7E

#define QG_V_ACCUM_DATA0_RT_REG			0x88
#define QG_I_ACCUM_DATA0_RT_REG			0x8B
#define QG_ACCUM_CNT_RT_REG			0x8E

#define QG_V_FIFO0_DATA0_REG			0x90
#define QG_I_FIFO0_DATA0_REG			0xA0

#define QG_SOC_MONOTONIC_REG			0xBF

#define QG_LAST_ADC_V_DATA0_REG			0xC0
#define QG_LAST_ADC_I_DATA0_REG			0xC2

#define QG_LAST_BURST_AVG_I_DATA0_REG		0xC6

#define QG_LAST_S3_SLEEP_V_DATA0_REG		0xCC

/* SDAM offsets */
#define QG_SDAM_VALID_OFFSET			0x46 /* 1-byte 0x46 */
#define QG_SDAM_SOC_OFFSET			0x47 /* 1-byte 0x47 */
#define QG_SDAM_TEMP_OFFSET			0x48 /* 2-byte 0x48-0x49 */
#define QG_SDAM_RBAT_OFFSET			0x4A /* 2-byte 0x4A-0x4B */
#define QG_SDAM_OCV_OFFSET			0x4C /* 4-byte 0x4C-0x4F */
#define QG_SDAM_IBAT_OFFSET			0x50 /* 4-byte 0x50-0x53 */
#define QG_SDAM_TIME_OFFSET			0x54 /* 4-byte 0x54-0x57 */
#define QG_SDAM_CYCLE_COUNT_OFFSET		0x58 /* 16-byte 0x58-0x67 */
#define QG_SDAM_LEARNED_CAPACITY_OFFSET		0x68 /* 2-byte 0x68-0x69 */
#define QG_SDAM_ESR_CHARGE_DELTA_OFFSET		0x6A /* 4-byte 0x6A-0x6D */
#define QG_SDAM_ESR_DISCHARGE_DELTA_OFFSET	0x6E /* 4-byte 0x6E-0x71 */
#define QG_SDAM_ESR_CHARGE_SF_OFFSET		0x72 /* 2-byte 0x72-0x73 */
#define QG_SDAM_ESR_DISCHARGE_SF_OFFSET		0x74 /* 2-byte 0x74-0x75 */
#define QG_SDAM_MAX_OFFSET			0xA4

/* Below offset is used by PBS */
#define QG_SDAM_PON_OCV_OFFSET			0xBC /* 2-byte 0xBC-0xBD */

#endif

/*
 * Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2019, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

/* Misc definitions */
#define ADC_TM3_625_UV				625000
#define ADC_TM3_RATIOMETRIC_RANGE		1800
#define ADC_TM3_RATIOMETRIC_RANGE_8998		1875

/* Addresses */
#define ADC_TM_M0_LOW_THR_LSB			0x5c
#define ADC_TM_M0_LOW_THR_MSB			0x5d
#define ADC_TM_M0_HIGH_THR_LSB			0x5e
#define ADC_TM_M0_HIGH_THR_MSB			0x5f
#define ADC_TM_M1_ADC_CH_SEL_CTL		0x68
#define ADC_TM_M1_LOW_THR_LSB			0x69
#define ADC_TM_M1_LOW_THR_MSB			0x6a
#define ADC_TM_M1_HIGH_THR_LSB			0x6b
#define ADC_TM_M1_HIGH_THR_MSB			0x6c
#define ADC_TM_M2_ADC_CH_SEL_CTL		0x70
#define ADC_TM_M2_LOW_THR_LSB			0x71
#define ADC_TM_M2_LOW_THR_MSB			0x72
#define ADC_TM_M2_HIGH_THR_LSB			0x73
#define ADC_TM_M2_HIGH_THR_MSB			0x74
#define ADC_TM_M3_ADC_CH_SEL_CTL		0x78
#define ADC_TM_M3_LOW_THR_LSB			0x79
#define ADC_TM_M3_LOW_THR_MSB			0x7a
#define ADC_TM_M3_HIGH_THR_LSB			0x7b
#define ADC_TM_M3_HIGH_THR_MSB			0x7c
#define ADC_TM_M4_ADC_CH_SEL_CTL		0x80
#define ADC_TM_M4_LOW_THR_LSB			0x81
#define ADC_TM_M4_LOW_THR_MSB			0x82
#define ADC_TM_M4_HIGH_THR_LSB			0x83
#define ADC_TM_M4_HIGH_THR_MSB			0x84
#define ADC_TM_M5_ADC_CH_SEL_CTL		0x88
#define ADC_TM_M5_LOW_THR_LSB			0x89
#define ADC_TM_M5_LOW_THR_MSB			0x8a
#define ADC_TM_M5_HIGH_THR_LSB			0x8b
#define ADC_TM_M5_HIGH_THR_MSB			0x8c
#define ADC_TM_M6_ADC_CH_SEL_CTL		0x90
#define ADC_TM_M6_LOW_THR_LSB			0x91
#define ADC_TM_M6_LOW_THR_MSB			0x92
#define ADC_TM_M6_HIGH_THR_LSB			0x93
#define ADC_TM_M6_HIGH_THR_MSB			0x94
#define ADC_TM_M7_ADC_CH_SEL_CTL		0x98
#define ADC_TM_M7_LOW_THR_LSB			0x99
#define ADC_TM_M7_LOW_THR_MSB			0x9a
#define ADC_TM_M7_HIGH_THR_LSB			0x9b
#define ADC_TM_M7_HIGH_THR_MSB			0x9c

#define ADC_TM_M0_MEAS_INTERVAL_CTL		0x59
#define ADC_TM_M1_MEAS_INTERVAL_CTL		0x6d
#define ADC_TM_M2_MEAS_INTERVAL_CTL		0x75
#define ADC_TM_M3_MEAS_INTERVAL_CTL		0x7d
#define ADC_TM_M4_MEAS_INTERVAL_CTL		0x85
#define ADC_TM_M5_MEAS_INTERVAL_CTL		0x8d
#define ADC_TM_M6_MEAS_INTERVAL_CTL		0x95
#define ADC_TM_M7_MEAS_INTERVAL_CTL		0x9d

/* Commodity ugly macro */
#define DEFINE_FUNC_RETRIEVE_PARAM(__retrieve_param) 			\
	static inline int ADC_TM_Mn_##__retrieve_param##_DEF(int _n)	\
	{								\
		switch (_n) {						\
			case 0:						\
				return ADC_TM_M0_##__retrieve_param;	\
			case 1:						\
				return ADC_TM_M1_##__retrieve_param;	\
			case 2:						\
				return ADC_TM_M2_##__retrieve_param;	\
			case 3:						\
				return ADC_TM_M3_##__retrieve_param;	\
			case 4:						\
				return ADC_TM_M4_##__retrieve_param;	\
			case 5:						\
				return ADC_TM_M5_##__retrieve_param;	\
			case 6:						\
				return ADC_TM_M6_##__retrieve_param;	\
			case 7:						\
				return ADC_TM_M7_##__retrieve_param;	\
			default:					\
				return -EINVAL;				\
		}							\
	}

DEFINE_FUNC_RETRIEVE_PARAM(LOW_THR_MSB);
DEFINE_FUNC_RETRIEVE_PARAM(LOW_THR_LSB);
DEFINE_FUNC_RETRIEVE_PARAM(HIGH_THR_MSB);
DEFINE_FUNC_RETRIEVE_PARAM(HIGH_THR_LSB);
DEFINE_FUNC_RETRIEVE_PARAM(ADC_CH_SEL_CTL);
DEFINE_FUNC_RETRIEVE_PARAM(MEAS_INTERVAL_CTL);

#define ADC_TM_Mn_MULTI_MEAS_EN_DEF(_n)		BIT(_n)
#define ADC_TM_Mn_LOW_THR_INT_EN_DEF(_n)	BIT(_n)
#define ADC_TM_Mn_HIGH_THR_INT_EN_DEF(_n)	BIT(_n)

enum tm4_vadc_mode_sel {
	ADC_TM_OP_NORMAL_MODE = 0,
	ADC_TM_OP_CONVERSION_SEQUENCER,
	ADC_TM_OP_MEASUREMENT_INTERVAL,
	ADC_TM_OP_MODE_NONE,
};

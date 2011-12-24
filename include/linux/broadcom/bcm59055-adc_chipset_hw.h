/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/*
	VENDOR_ADC_PRODUCT_CONFIGURATION
	Description: Used to determine if this configuration is here and it is to be used
	Default value: Defined.
*/
#define VENDOR_ADC_PRODUCT_CONFIGURATION

#define VENDOR_ADC_CAL_FUNCTION hal_adc_cal_calc_dalton

/*
	VENDOR_ADC_NUM_CHANNELS
	Description: Number of physical channels in ADC
 	Default value for Voyager: 9
 	In dalton we store data for 17 channels - the 16 in the normal ADC and one for IBAT fast channel
*/
#define VENDOR_ADC_NUM_CHANNELS 17

/*
	VENDOR_ADC_ADCIN_BASE_CHANNEL
	Description: The first channel of the ADCIN (4 channel 10-bit ADC)
	Default value: 5
*/
#define VENDOR_ADC_BASE_CHANNEL 0

/*
	VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
	Description: 	The physical channel of the Temperature compensation circuit
	Default value: 	B2 and onwards: default VENDOR_ADC_ADCIN_BASE_CHANNEL
 					Prior to this it should not be defined.
 	Dalton:			This is used differently in dalton, so no single temperature channel is defined.
*/
//#define VENDOR_ADC_TEMPERATURE_COMP_CHANNEL VENDOR_ADC_ADCIN_BASE_CHANNEL


/*
	VENDOR_ADC_BOM_CHANNEL
	Description: 	The physical channel of the BOM detection circuit
	Default value: 	B2 and onwards: default VENDOR_ADC_AUXADC_BASE_CHANNEL + 1
					Prior to this VENDOR_ADC_ADCIN_BASE_CHANNEL
*/
#define VENDOR_ADC_BOM_CHANNEL_1 ADC_BOM_CHANNEL

/*
	VENDOR_ADC_BOM_CHANNEL
	Description: 	The physical channel of the BOM detection circuit
	Default value: 	B2 and onwards: default VENDOR_ADC_AUXADC_BASE_CHANNEL + 1
					Prior to this it should not be defined.
*/
//#define VENDOR_ADC_BOM_CHANNEL_2 VENDOR_ADC_AUXADC_BASE_CHANNEL + 4

/*
	VENDOR_ADC_BOMS
	Description: 	Number of BOMS per channel
	Default value: 	10 for B1.2 and previous, 6 for B2 and onwards
*/
#define VENDOR_ADC_BOMS 6

/*
	VENDOR_ADC_BOM_VALUES
	Description: 	The values of the limits for BOM detections
	Default value:
*/
#define VENDOR_ADC_BOM_VALUES {{0, 7},{8, 38},{39, 93},{94, 166},{167, 250},{251, 255}}


/*
	VENDOR_ADC_REFERENCE_VOLTAGE
	Description: 	The reference voltage for the ADC. This is shared between all channels
	Default value: 	1200 (mV)
*/
#define VENDOR_ADC_REFERENCE_VOLTAGE 1200

/*
	VENDOR_ADC_VBAT_SCALE_CHANNEL
	Description: 	The physical channel for the VBAT circuit
	Default value: 	HAL_ADC_ADCIN_BASE_CHANNEL + 1 (ADCIN2)
*/
#define VENDOR_ADC_VBAT_SCALE_CHANNEL ADC_VMBAT_CHANNEL
/*
	VENDOR_ADC_VBAT_SCALE_PULLUP
	Description: 	Total resistance from VBAT+ to ADC (upper part of voltage divider)
 	Default value: 	540000 for B1.2 and onwards, 680000 for previous
				 	In dalton, no external components are used for VBAT channel
*/

//#define VENDOR_ADC_VBAT_SCALE_PULLUP 540000
/*
	VENDOR_ADC_VBAT_SCALE_PULLDOWN
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
 	Default value: 	180000 for B1.2 and onwards, 220000 for previous
					In dalton, no external components are used for VBAT channel
*/
//#define VENDOR_ADC_VBAT_SCALE_PULLDOWN 180000

/*
	VENDOR_ADC_VBAT_SCALE_UVPERBITS
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
 	Default value: 	180000 for B1.2 and onwards, 220000 for previous
					In dalton, no external components are used for VBAT channel
*/
#define VENDOR_ADC_VBAT_SCALE_UVPERBITS 4687

#define VENDOR_ADC_VBBAT_CHANNEL ADC_VBBAT_CHANNEL
/*
	VENDOR_ADC_VBAT_SCALE_PULLUP
	Description: 	Total resistance from VBAT+ to ADC (upper part of voltage divider)
 	Default value: 	540000 for B1.2 and onwards, 680000 for previous
				 	In dalton, no external components are used for VBAT channel
*/

//#define VENDOR_ADC_VBBAT_PULLUP 540000
/*
	VENDOR_ADC_VBAT_SCALE_PULLDOWN
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
 	Default value: 	180000 for B1.2 and onwards, 220000 for previous
					In dalton, no external components are used for VBAT channel
*/
//#define VENDOR_ADC_VBBAT_PULLDOWN 180000

/*
	VENDOR_ADC_VBAT_SCALE_UVPERBITS
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
 	Default value: 	180000 for B1.2 and onwards, 220000 for previous
					In dalton, no external components are used for VBAT channel
*/
#define VENDOR_ADC_VBBAT_UVPERBITS 4687

/*
	VENDOR_ADC_BSI_CHANNEL
	Description: 	The physical channel for the BSI circuit
	Default value: 	HAL_ADC_ADCIN_BASE_CHANNEL + 3 (ADCIN4)
*/
#define VENDOR_ADC_BSI_CHANNEL ADC_BSI_CHANNEL
/*
	VENDOR_ADC_BSI_VMAX
	Description: 	The voltage on the circuit for BSI evaluation
	Default value: 	2500 (mV)
*/
#define VENDOR_ADC_BSI_VMAX 1800

/*
	VENDOR_ADC_BSI_RPULLUP
	Description: 	Pullup resistor in BSI evaluation circuit
	Default value: 	180000 for Voyager, 120000 for Dalton
*/
#define VENDOR_ADC_BSI_RPULLUP 162000

/*
	VENDOR_ADC_BTEMP_CHANNEL
	Description: 	The physical channel for the BTEMP circuit
	Default value: 	5
*/
#define VENDOR_ADC_BTEMP_CHANNEL ADC_NTC_CHANNEL

/*
	VENDOR_ADC_BTEMP_VMAX
	Description: 	The voltage on the circuit for BTEMP measurement
	Default value: 	2500 (mV)
*/
#define VENDOR_ADC_BTEMP_VMAX 1200
/*
	VENDOR_ADC_BTEMP_RPULLUP
	Description: 	The pullup resistor in the BTEMP measurement circuit
	Default value: 	180000 for B1.2 and onwards, 174000 for prior
*/

#define VENDOR_ADC_BTEMP_RPULLUP 162000

/*
	VENDOR_ADC_BTEMP_RSERIES
	Description: 	Series resitor in BTEMP circuit. Legacy from voyager 1, where the BTEMP circuit was shared
					between JUNO and PMU.
	Default value:  0
*/
#define VENDOR_ADC_BTEMP_RSERIES 0

/*
	VENDOR_ADC_BTEMP_RPAR
	Description: 	Parallel resistor to NTC in BTEMP circuit. Used to limit the voltage on the ADC.
	Default value: 	180000 for B1.2 and onwards, 150000 for prior
*/
#define VENDOR_ADC_BTEMP_RPAR 0

/*
	VENDOR_ADC_BTEMP_R0
	Description: 	R0 of the NTC in the BTEMP circuit
	Default value: 	47000
*/
#define VENDOR_ADC_BTEMP_R0 47000

/*
	VENDOR_ADC_BTEMP_B
	Description: 	Temperature coeficient of the NTC
	Default value: 	4050
*/
#define VENDOR_ADC_BTEMP_B 4050

/*
	VENDOR_ADC_PATEMP_CHANNEL
	Description: 	The physical channel for the PATEMP circuit
	Default value: 	HAL_ADC_AUXADC_BASE_CHANNEL + 0
*/
#define VENDOR_ADC_PATEMP_CHANNEL ADC_PATEMP_CHANNEL
/*
	VENDOR_ADC_PATEMP_VMAX
	Description: 	The voltage on the circuit for BTEMP measurement
	Default value: 	2500 (mV)
*/
#define VENDOR_ADC_PATEMP_VMAX 1200

/*
	VENDOR_ADC_PATEMP_RPULLUP
	Description: 	The pullup resistor in the PATEMP measurement circuit
	Default value: 	174000
*/

#define VENDOR_ADC_PATEMP_RPULLUP 162000

/*
	VENDOR_ADC_PATEMP_RSERIES
	Description: 	Series resitor in PATEMP circuit. Legacy from voyager 1, where the PATEMP circuit was implemented same way BTEMP
					between JUNO and PMU.
	Default value:  0
*/
#define VENDOR_ADC_PATEMP_RSERIES 0

/*
	VENDOR_ADC_PATEMP_RPAR
	Description: 	Parallel resistor to NTC in PATEMP circuit. Used to limit the voltage on the ADC.
	Default value: 	150000
*/
#define VENDOR_ADC_PATEMP_RPAR 0

/*
	VENDOR_ADC_PATEMP_R0
	Description: 	R0 of the NTC in the PATEMP circuit
	Default value: 	47000
*/
#define VENDOR_ADC_PATEMP_R0 47000

/*
	VENDOR_ADC_PATEMP_B
	Description: 	Temperature coeficient of the NTC
	Default value: 	4050
*/
#define VENDOR_ADC_PATEMP_B 4050

#define VENDOR_ADC_X32TEMP_CHANNEL ADC_32KTEMP_CHANNEL
/*
	VENDOR_ADC_X32TEMP_VMAX
	Description: 	The voltage on the circuit for X32TEMP measurement
	Default value: 	2500 (mV)
*/
#define VENDOR_ADC_X32TEMP_VMAX 1200

/*
	VENDOR_ADC_PATEMP_RPULLUP
	Description: 	The pullup resistor in the PATEMP measurement circuit
	Default value: 	174000
*/

#define VENDOR_ADC_X32TEMP_RPULLUP 162000

/*
	VENDOR_ADC_X32TEMP_RSERIES
	Description: 	Series resitor in PATEMP circuit. Legacy from voyager 1, where the PATEMP circuit was implemented same way BTEMP
					between JUNO and PMU.
	Default value:  0
*/
#define VENDOR_ADC_X32TEMP_RSERIES 0

/*
	VENDOR_ADC_X32TEMP_RPAR
	Description: 	Parallel resistor to NTC in PATEMP circuit. Used to limit the voltage on the ADC.
	Default value: 	150000
*/
#define VENDOR_ADC_X32TEMP_RPAR 0

/*
	VENDOR_ADC_X32TEMP_R0
	Description: 	R0 of the NTC in the PATEMP circuit
	Default value: 	47000
*/
#define VENDOR_ADC_X32TEMP_R0 47000

/*
	VENDOR_ADC_PATEMP_B
	Description: 	Temperature coeficient of the NTC
	Default value: 	4050
*/
#define VENDOR_ADC_X32TEMP_B 4050


/*
    VENDOR_ADC_VCHAR_CHANNEL
    Description: 	The physical channel for the VCHAR circuit
	Default value: 	HAL_ADC_AUXADC_BASE_CHANNEL + 3 (Calama B2)
*/
#define VENDOR_ADC_VCHAR_CHANNEL ADC_VWALL_CHANNEL

/*
	VENDOR_ADC_VCHAR_PULLUP
	Description: 	Total resistance from ADC to CHARGER (upper part of voltage divider)
 	Default value: 	1500000 (Calama B2)
				 	Not used in Dalton
*/
//#define VENDOR_ADC_VCHAR_RPULLUP 1500000

/*
	VENDOR_ADC_VCHAR_PULLDOWN
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
	Default value: 	174000 (Calama B2)
*/
//#define VENDOR_ADC_VCHAR_RPULLDOWN 174000

/*
	VENDOR_ADC_VCHAR_SCALE_UVPERBITS
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
 	Default value:	In dalton, no external components are used for VCHAR channel.
*/
#define VENDOR_ADC_VCHAR_UVPERBITS 9766

#define VENDOR_ADC_VBUS_CHANNEL ADC_VBUS_CHANNEL

/*
	VENDOR_ADC_VBUS_UVPERBITS
	Description: 	Total resistance from ADC to battery ground (lower part of voltage divider)
 	Default value:	In dalton, no external components are used for VCHAR channel.
*/
#define VENDOR_ADC_VBUS_UVPERBITS 7813

#define VENDOR_ADC_ID_CHANNEL ADC_ID_CHANNEL

#define VENDOR_ADC_BSI_COMP_CHANNELS
#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
#define VENDOR_ADC_BSI_CAL_L_CHANNEL ADC_BSI_CAL_L_CHANNEL
#define VENDOR_ADC_NTC_CAL_L_CHANNEL ADC_NTC_CAL_L_CHANNEL
#define VENDOR_ADC_NTC_CAL_H_CHANNEL ADC_NTC_CAL_H_CHANNEL
#define VENDOR_ADC_BSI_CAL_H_CHANNEL ADC_BSI_CAL_H_CHANNEL

#define VENDOR_ADC_BSI_COMP_LOW_VOLTAGE		72
#define VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE	1128
#define VENDOR_ADC_COMP_SAMPLES 8
#define VENDOR_ADC_COMP_FREQUENCY 60
#endif

#define VENDOR_ADC_IBAT_CHANNEL VENDOR_ADC_BASE_CHANNEL + 16


#define VENDOR_ADC_KELVIN_DOWN_TABLE_BEGIN (273-35)

/*
	VENDOR_ADC_KELVIN_DOWN_TABLE
	Description: 	Table of Temperature and matching NTC resistance.
                    Each index in the table corresponds to one specific degree in kelvin. Index 0 corresponds to
                    VENDOR_ADC_KELVIN_DOWN_TABLE_BEGIN degrees in kelvin, index 1 corresponds to
                    VENDOR_ADC_KELVIN_DOWN_TABLE_BEGIN + 1 degrees in kelvin. The table must be in (non - strict)
                    descending order, as it is used by a binary search algorithm.
*/
#define VENDOR_ADC_KELVIN_DOWN_TABLE       \
{                                               \
            /* -35 C == (VENDOR_ADC_KELVIN_DOWN_TABLE_BEGIN) K */\
    1191390,                                    \
    1126266,                                    \
    1061142,                                    \
    996018,                                     \
    930894,                                     \
            /* -30 C */                         \
    865770,                                     \
    819832,                                     \
    773894,                                     \
    727956,                                     \
    682018,                                     \
    636080,                                     \
    603286,                                     \
    570492,                                     \
    537698,                                     \
    504904,                                     \
            /* -20 C */                         \
    472110,                                     \
    448464,                                     \
    424818,                                     \
    401172,                                     \
    377526,                                     \
    353880,                                     \
    336642,                                     \
    319404,                                     \
    302166,                                     \
    284928,                                     \
            /* -10 C */                         \
    267690,                                     \
    254996,                                     \
    242302,                                     \
    229608,                                     \
    216914,                                     \
    204220,                                     \
    194780,                                     \
    185340,                                     \
    175900,                                     \
    166460,                                     \
          /*   0 C */                           \
    157020,                                     \
    149942,                                     \
    142864,                                     \
    135786,                                     \
    128708,                                     \
    121630,                                     \
    116290,                                     \
    110950,                                     \
    105610,                                     \
    100270,                                     \
           /*  10 C */                          \
    94930,                                      \
    90864,                                      \
    86798,                                      \
    82732,                                      \
    78666,                                      \
    74600,                                      \
    71484,                                      \
    68368,                                      \
    65252,                                      \
    62136,                                      \
           /*  20 C */                          \
    59020,                                      \
    56616,                                      \
    54212,                                      \
    51808,                                      \
    49404,                                      \
    47000,                                      \
    45132,                                      \
    43264,                                      \
    41396,                                      \
    39528,                                      \
           /*  30 C */                          \
    37660,                                      \
    36198,                                      \
    34736,                                      \
    33274,                                      \
    31812,                                      \
    30350,                                      \
    29200,                                      \
    28050,                                      \
    26900,                                      \
    25750,                                      \
           /*  40 C */                          \
    24600,                                      \
    23690,                                      \
    22780,                                      \
    21870,                                      \
    20960,                                      \
    20050,                                      \
    19328,                                      \
    18606,                                      \
    17884,                                      \
    17162,                                      \
           /*  50 C */                          \
    16440,                                      \
    15860,                                      \
    15280,                                      \
    14700,                                      \
    14120,                                      \
    13540,                                      \
    13074,                                      \
    12608,                                      \
    12142,                                      \
    11676,                                      \
           /*  60 C */                          \
    11210,                                      \
    10834,                                      \
    10458,                                      \
    10082,                                      \
    9706,                                       \
    9330,                                       \
    9024,                                       \
    8718,                                       \
    8412,                                       \
    8106,                                       \
           /*  70 C */                          \
    7800,                                       \
    7550,                                       \
    7300,                                       \
    7050,                                       \
    6800,                                       \
    6550,                                       \
    6344,                                       \
    6138,                                       \
    5932,                                       \
    5726,                                       \
           /*  80 C */                          \
    5520,                                       \
    5350,                                       \
    5180,                                       \
    5010,                                       \
    4840,                                       \
    4670,                                       \
    4530,                                       \
    4390,                                       \
    4250,                                       \
    4110,                                       \
           /*  90 C */                          \
    3970,                                       \
    3854,                                       \
    3738,                                       \
    3622,                                       \
    3506,                                       \
    3390,                                       \
    3292,                                       \
    3194,                                       \
    3096,                                       \
    2998,                                       \
            /* 100 C */                         \
    2900,                                       \
    2820,                                       \
    2740,                                       \
    2660,                                       \
    2580,                                       \
    2500,                                       \
    2430,                                       \
    2360,                                       \
    2290,                                       \
    2220,                                       \
            /* 110 C */                         \
    2150,                                       \
    2094,                                       \
    2038,                                       \
    1982,                                       \
    1926,                                       \
    1870                                        \
}

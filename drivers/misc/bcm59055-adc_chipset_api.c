/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/misc/bcm59055-fuelgauge.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html(the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  bcm59055-adc_chipset_api.c
*
*  PURPOSE:
*
*     This implements the driver for the Fuel Gauge on BCM59055 PMU chip.
*
*  NOTES:
*
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/core.h>
/*#include <linux/workqueue.h>*/
#include <linux/semaphore.h>
#include <linux/stringify.h>
#include <linux/proc_fs.h>
#include <linux/csapi_adc.h>
#include <linux/broadcom/bcm59055-adc.h>
#include <linux/broadcom/bcm59055-adc_chipset_hw.h>
#include <linux/broadcom/bcm59055-fuelgauge.h>


#define GLUE_DBG(s)

#define FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS 9

#define THERMISTOR_T0	0.00335401		/* 1/298,15 - T0 (25 degrees C) */

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
int VENDOR_ADC_kelvin_down_table[] = VENDOR_ADC_KELVIN_DOWN_TABLE;
#endif

s32 hal_adc_cal_calc_dalton(struct csapi_cal_req *Data);

typedef s32(*CSAPI_ADC_HW_CAL_CALC_FUNC)(struct csapi_cal_req *Data);
#ifdef VENDOR_ADC_CAL_FUNCTION
CSAPI_ADC_HW_CAL_CALC_FUNC hal_adc_hw_cal_calc_func = VENDOR_ADC_CAL_FUNCTION;
#else
CSAPI_ADC_HW_CAL_CALC_FUNC hal_adc_hw_cal_calc_func = NULL;
#endif

#define CSAPI_ADC_ERR_SUCCESS 0

static int bsearch_descending_int_table(int *table, int sz, int value);

typedef enum {
	adc_unit_unknown = 0,
	adc_unit_volts,
	adc_unit_amps,
	adc_unit_ohms,
	adc_unit_kelvin_down,	/* NTC resistor is connected to ground */
	adc_unit_kelvin_down_table,	/* NTC resistor is connected to ground(table lookup) */
	adc_unit_kelvin_up,		/* NTC resistor is connected to supply */
	adc_unit_bom,			/* BOM detection */
	adc_unit_ibat_k,
	adc_unit_raw
} adc_channel_unit_t;

struct adc_unit_t {
	adc_channel_unit_t unit;
	u32 cal_id;				/* calibration identifier... */
	u32 uvperlsb;
	u32 vmax;
	int offset;
	union {
		struct {
			int dummy;
		} volts;
		struct {
			u32 rshunt;
		} amps;
		struct {
			u32 rpullup;
			u32 relerror;
			u32 reference_reading;
		} ohms;
		struct {
			u32 rpullup;
			u32 rseries;
			u32 rpar;
			u32 r0;
			u32 b;
		} kelvin;
		struct {
			u32 k;
		} ibat_k;
	} data;
};


struct adc_channels_t {
	int select;
	u8 bits;
	u8 locked;
	u32 lockvalue;
	struct adc_unit_t unit;
};


struct bcm59055_adc_chipset_api {
	struct bcm590xx *bcm59055;

	struct adc_channels_t adc_channels[VENDOR_ADC_NUM_CHANNELS];
	struct semaphore fg_read_sem;
	struct semaphore rtm_read_sem;
};

static struct bcm59055_adc_chipset_api *bcm59055_adc_chipset_api=NULL;

static int columb_counter = 0, ibat_avg = 0;

void hal_adc_init_channels(u8 use_cal_data)
{
	struct adc_channels_t local_adc_channels[VENDOR_ADC_NUM_CHANNELS];
	struct adc_channels_t *adc_cal_channels = NULL;
	int channel;

	memset(local_adc_channels, 0, sizeof(local_adc_channels));

	for (channel = 0; channel < VENDOR_ADC_NUM_CHANNELS; channel++) {
		local_adc_channels[channel].bits = 10;
		local_adc_channels[channel].select = channel;
		local_adc_channels[channel].locked = 0;
		local_adc_channels[channel].lockvalue = 0;
		local_adc_channels[channel].unit.cal_id = 0;
		GLUE_DBG(("hal_adc_init_channel: Initializing channel %x(%x, %d)", channel, local_adc_channels[channel].select, local_adc_channels[channel].bits));
	}

#ifdef VENDOR_ADC_VBAT_SCALE_CHANNEL
	local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.unit = adc_unit_volts;
	if (adc_cal_channels) {
		local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = adc_cal_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb;				/* VREF *(540k + 180 k)/180 k / steps */
		local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset;
		local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.cal_id = adc_cal_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.cal_id;
	} else {
#ifdef VENDOR_ADC_VBAT_SCALE_UVPERBITS
		local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = VENDOR_ADC_VBAT_SCALE_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VBAT_SCALE_PULLUP + VENDOR_ADC_VBAT_SCALE_PULLDOWN)*1000) / VENDOR_ADC_VBAT_SCALE_PULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].bits);				/* VREF *(540k + 180 k)/180 k / steps */
#endif
	}
#endif

#ifdef VENDOR_ADC_BSI_CHANNEL
	local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.unit = adc_unit_ohms;
	local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.rpullup = VENDOR_ADC_BSI_RPULLUP;			/* Rpullup in Ohm */
	if (adc_cal_channels) {
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.uvperlsb = adc_cal_channels[VENDOR_ADC_BSI_CHANNEL].unit.uvperlsb;
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_BSI_CHANNEL].unit.offset;		/* Relative reference value(used from B1_2) */
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.vmax = adc_cal_channels[VENDOR_ADC_BSI_CHANNEL].unit.vmax;		/* Relative reference value(used from B1_2) */
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.reference_reading = adc_cal_channels[VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.reference_reading;		/* Relative reference value(used from B1_2) */
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.cal_id = adc_cal_channels[VENDOR_ADC_BSI_CHANNEL].unit.cal_id;
	} else {
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.vmax = VENDOR_ADC_BSI_VMAX * 1000;
		local_adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels[VENDOR_ADC_BSI_CHANNEL].bits);	/* 1,2 v, 1024 steps, 10 bit ADC */
	}
#endif

#ifdef VENDOR_ADC_BTEMP_CHANNEL

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.unit = adc_unit_kelvin_down_table;
#else
	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.unit = adc_unit_kelvin_down;
#endif /* VENDOR_ADC_KELVIN_DOWN_TABLE */

	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.data.ohms.rpullup = VENDOR_ADC_BTEMP_RPULLUP;			/* Rpullup in Ohm */
	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.r0 = VENDOR_ADC_BTEMP_R0;			/* NTC R0 in Ohm */
	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.b = VENDOR_ADC_BTEMP_B;			/* Temperature coeficient */
	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.rpar = VENDOR_ADC_BTEMP_RPAR;			/* Parallel resistor in Ohm */
	local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.rseries = VENDOR_ADC_BTEMP_RSERIES;			/* No series resistor */
	if (adc_cal_channels) {
		local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.vmax = adc_cal_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.vmax;
		local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.uvperlsb = adc_cal_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.uvperlsb;
		local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.offset;		/* Relative reference value(used from B1_2) */
		local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.cal_id = adc_cal_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.cal_id;
	} else {
		local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.vmax = VENDOR_ADC_BTEMP_VMAX * 1000;
		local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels[VENDOR_ADC_BTEMP_CHANNEL].bits);	/* 1,2 v, 1024 steps, 10 bit ADC */
	}
#endif

#ifdef VENDOR_ADC_PATEMP_CHANNEL

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.unit = adc_unit_kelvin_down_table;
#else
	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.unit = adc_unit_kelvin_down;
#endif /* VENDOR_ADC_KELVIN_DOWN_TABLE */

	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.rpullup = VENDOR_ADC_PATEMP_RPULLUP;			/* Rpullup in Ohm */
	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.r0 = VENDOR_ADC_PATEMP_R0;			/* NTC R0 in Ohm */
	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.b = VENDOR_ADC_PATEMP_B;			/* Temperature coeficient */
	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.rpar = VENDOR_ADC_PATEMP_RPAR;			/* Parallel resistor in Ohm */
	local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.rseries = VENDOR_ADC_PATEMP_RSERIES;			/* No series resistor */


	if (adc_cal_channels) {
		local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.vmax = adc_cal_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.vmax;
		local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.uvperlsb = adc_cal_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.uvperlsb;
		local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.offset;		/* Relative reference value(used from B1_2) */
		local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.cal_id = adc_cal_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.cal_id;

	} else {
		local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.vmax = VENDOR_ADC_PATEMP_VMAX * 1000;
		local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].bits);	/* 1,2 v, 1024 steps, 10 bit ADC */
	}
#endif

#ifdef VENDOR_ADC_X32TEMP_CHANNEL

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.unit = adc_unit_kelvin_down_table;
#else
	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.unit = adc_unit_kelvin_down;
#endif /* VENDOR_ADC_KELVIN_DOWN_TABLE */

	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.rpullup = VENDOR_ADC_X32TEMP_RPULLUP;			/* Rpullup in Ohm */
	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.r0 = VENDOR_ADC_X32TEMP_R0;			/* NTC R0 in Ohm */
	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.b = VENDOR_ADC_X32TEMP_B;			/* Temperature coeficient */
	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.rpar = VENDOR_ADC_X32TEMP_RPAR;			/* Parallel resistor in Ohm */
	local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.rseries = VENDOR_ADC_X32TEMP_RSERIES;			/* No series resistor */


	if (adc_cal_channels && adc_cal_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax) {
		local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax = adc_cal_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax;
		local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.uvperlsb = adc_cal_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.uvperlsb;
		local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.offset;		/* Relative reference value(used from B1_2) */
		local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.cal_id = adc_cal_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.cal_id;

	} else {
		local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax = VENDOR_ADC_X32TEMP_VMAX * 1000;
		local_adc_channels[VENDOR_ADC_X32TEMP_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels[VENDOR_ADC_PATEMP_CHANNEL].bits);	/* 1,2 v, 1024 steps, 10 bit ADC */
	}
#endif

#ifdef VENDOR_ADC_VCHAR_CHANNEL

	local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.unit = adc_unit_volts;
	if (adc_cal_channels && adc_cal_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb) {
		local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = adc_cal_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb;
		local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.offset;
	} else {
#ifdef VENDOR_ADC_VCHAR_UVPERBITS
		local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = VENDOR_ADC_VCHAR_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VCHAR_RPULLUP + VENDOR_ADC_VCHAR_RPULLDOWN)*1000) / VENDOR_ADC_VCHAR_RPULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].bits);				/* VREF *(540k + 180 k)/180 k / steps */
#endif
	}
#endif

#ifdef VENDOR_ADC_VBUS_CHANNEL
	local_adc_channels[VENDOR_ADC_VBUS_CHANNEL].unit.unit = adc_unit_volts;
	{
#ifdef VENDOR_ADC_VBUS_UVPERBITS
		local_adc_channels[VENDOR_ADC_VBUS_CHANNEL].unit.uvperlsb = VENDOR_ADC_VBUS_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VBUS_RPULLUP + VENDOR_ADC_VBUS_RPULLDOWN)*1000) / VENDOR_ADC_VCHAR_RPULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels[VENDOR_ADC_VBUS_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].bits);				/* VREF *(540k + 180 k)/180 k / steps */
#endif
	}
#endif

#ifdef VENDOR_ADC_VBBAT_CHANNEL
	local_adc_channels[VENDOR_ADC_VBBAT_CHANNEL].unit.unit = adc_unit_volts;
	{
#ifdef VENDOR_ADC_VBBAT_UVPERBITS
		local_adc_channels[VENDOR_ADC_VBBAT_CHANNEL].unit.uvperlsb = VENDOR_ADC_VBBAT_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VBBAT_RPULLUP + VENDOR_ADC_VBBAT_RPULLDOWN)*1000) / VENDOR_ADC_VBBAT_RPULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels[VENDOR_ADC_VBBAT_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VBBAT_CHANNEL].bits);				/* VREF *(540k + 180 k)/180 k / steps */
#endif
	}
#endif

#ifdef VENDOR_ADC_IBAT_CHANNEL
	{
		u8 k;
		local_adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.unit = adc_unit_ibat_k;
		local_adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.uvperlsb = 1000;
		local_adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k = 0;
		if (adc_cal_channels) {
			local_adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k = adc_cal_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k;
			local_adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.offset = adc_cal_channels[VENDOR_ADC_IBAT_CHANNEL].unit.offset;
			/* Tell driver about the K value */
			k = local_adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k;
			if (k) {
				/* Call driver...*/
				bcm59055_fg_write_gain_trim (k);
			}
		}
	}
#endif

#ifdef VENDOR_ADC_ALS_CHANNEL
	local_adc_channels[VENDOR_ADC_ALS_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels[VENDOR_ADC_ID_CHANNEL].bits);	/* 1,2 v, 1024 steps, 10 bit ADC */
	local_adc_channels[VENDOR_ADC_ALS_CHANNEL].unit.unit = adc_unit_raw;
#endif

#ifdef VENDOR_ADC_ID_CHANNEL
	local_adc_channels[VENDOR_ADC_ID_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels[VENDOR_ADC_ID_CHANNEL].bits);	/* 1,2 v, 1024 steps, 10 bit ADC */
	local_adc_channels[VENDOR_ADC_ID_CHANNEL].unit.unit = adc_unit_volts;
#endif

#ifdef VENDOR_ADC_BOM_CHANNEL_1
	local_adc_channels[VENDOR_ADC_BOM_CHANNEL_1].unit.uvperlsb = 1;
	local_adc_channels[VENDOR_ADC_BOM_CHANNEL_1].unit.unit = adc_unit_bom;
#endif

#ifdef VENDOR_ADC_BOM_CHANNEL_2
	local_adc_channels[VENDOR_ADC_BOM_CHANNEL_2].unit.uvperlsb = 1;
	local_adc_channels[VENDOR_ADC_BOM_CHANNEL_2].unit.unit = adc_unit_bom;
#endif

	memcpy(bcm59055_adc_chipset_api->adc_channels, local_adc_channels, sizeof(local_adc_channels));
}

static struct adc_channels_t *hal_adc_map_channel(struct adc_channels_t *channel_array, u8 channel)
{
	struct adc_channels_t *pchan = NULL;

	switch (channel) {
#ifdef VENDOR_ADC_VBAT_SCALE_CHANNEL
	case CSAPI_ADC_VBAT:
		pchan = &channel_array[VENDOR_ADC_VBAT_SCALE_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_BSI_CHANNEL
		/* Main ADC(Used during phone calibration) */
	case CSAPI_ADC_MAIN_CAL:
		/* Fall through(this is also the BSI ADC) */
		/* Battery Size Identification */
	case CSAPI_ADC_BSI:
		pchan = &channel_array[VENDOR_ADC_BSI_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_BTEMP_CHANNEL
		/* Battery TEMPerature */
	case CSAPI_ADC_BTEMP:
		pchan = &channel_array[VENDOR_ADC_BTEMP_CHANNEL];
		break;
#endif
		/* Power Amplifier TEMPerature */
#ifdef VENDOR_ADC_PATEMP_CHANNEL
	case CSAPI_ADC_PATEMP:
		pchan = &channel_array[VENDOR_ADC_PATEMP_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_X32TEMP_CHANNEL
	case CSAPI_ADC_CTEMP:
		pchan = &channel_array[VENDOR_ADC_X32TEMP_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_VCHAR_CHANNEL
	case CSAPI_ADC_VCHAR:
		pchan = &channel_array[VENDOR_ADC_VCHAR_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_VBUS_CHANNEL
	case CSAPI_ADC_VBUS:
		pchan = &channel_array[VENDOR_ADC_VBUS_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_IBAT_CHANNEL
	case CSAPI_ADC_IBAT:
		pchan = &channel_array[VENDOR_ADC_IBAT_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_VBBAT_CHANNEL
	case CSAPI_ADC_VBACK:
		pchan = &channel_array[VENDOR_ADC_VBBAT_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_ID_CHANNEL
	case CSAPI_ADC_IDIN:
		pchan = &channel_array[VENDOR_ADC_ID_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_ALS_CHANNEL
	case CSAPI_ADC_IDIN:
		pchan = &channel_array[VENDOR_ADC_ALS_CHANNEL];
		break;
#endif

#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
	case CSAPI_ADC_VENDOR_CH_0:
		pchan = &channel_array[VENDOR_ADC_BSI_CAL_L_CHANNEL];
		pchan->unit.unit = adc_unit_bom;
		break;
	case CSAPI_ADC_VENDOR_CH_1:
		pchan = &channel_array[VENDOR_ADC_BSI_CAL_H_CHANNEL];
		pchan->unit.unit = adc_unit_bom;
		break;
	case CSAPI_ADC_VENDOR_CH_2:
		pchan = &channel_array[VENDOR_ADC_NTC_CAL_L_CHANNEL];
		pchan->unit.unit = adc_unit_bom;
		break;
	case CSAPI_ADC_VENDOR_CH_3:
		pchan = &channel_array[VENDOR_ADC_NTC_CAL_H_CHANNEL];
		pchan->unit.unit = adc_unit_bom;
		break;
#endif
#ifdef VENDOR_ADC_BOM_CHANNEL_1
	case CSAPI_ADC_VENDOR_CH_4:
		pchan = &channel_array[VENDOR_ADC_BOM_CHANNEL_1];
		pchan->unit.unit = adc_unit_bom;
		break;
#endif
	default:
		pchan = NULL;
		break;
	}
	pr_debug("%s, CSAPI channel %d, pChan 0x%x", __func__, channel, (u32)pchan);
	return pchan;
}


static u16 read_hk_adc(int physical_channel)
{
	int reading;
	if (physical_channel <= ADC_ALS_CHANNEL) {
		do {
			reading = bcm59055_saradc_read_data(physical_channel);
		} while (reading < 0);
		return reading;
	} else
		return 0;
}


static int read_rtm_adc(int physical_channel)
{
	int reading;
	do {
		reading = bcm59055_saradc_request_rtm(physical_channel);
		/* pr_debug(KERN_INFO "%s: reading %d", reading); */
		if (reading < 0) {
			mdelay(20);
		}
	} while (reading < 0);
	return reading;
}

/* Update_columb */
/* Returns average current consumption */
/* updates the static variable columb with the values from the fuel gauge */
static int update_columb(void)
{
	u32 fg_accm;
	u16 fg_cnt, fg_sleep_cnt;
	u8 signbit;
	int smpl_time, samples;
	int ret;

	/* Get the value from the fuel gauge */
	bcm59055_fg_init_read();
	ret = bcm59055_fg_read_soc(&fg_accm, &fg_cnt, &fg_sleep_cnt);

	pr_debug("%s: raw data: %x, %d, %d\n", __func__, fg_accm, fg_cnt, fg_sleep_cnt);

	if (ret < 0) { /* || !(fg_accm & BCM59055_REG_FGACCM_VALID)) { */
		return ibat_avg;
	}
	/* Calculate for how long the FG has been averaging */
	/* Time(ms) = FGCNT * 500 + FGSLEEPCNT * 32 */
	smpl_time = fg_cnt * 500 + fg_sleep_cnt * 32;
	/* if time == 0, return the previous value */
	if (smpl_time < 500) {
		return ibat_avg;
	}

	samples = smpl_time / 500;

	signbit = (fg_accm & BCM59055_REG_FGACCM_SIGNBIT) ? 1:0;
	/* Calculate the amount of columbs(almost mC) - or multiply it with 976 to get uC */
	/* Right now it is stored with a sign bit in bit 26 */
	if (signbit) {
		fg_accm = (fg_accm & BCM59055_REG_FGACCM_VALMASK) - 1;
		fg_accm = (~fg_accm) & BCM59055_REG_FGACCM_VALMASK;
	} else {
		fg_accm &= BCM59055_REG_FGACCM_VALMASK;
	}

	fg_accm *= 976;	/* To make it uC */
	/* Adjust fg_accm for sleep current */
	fg_accm /= 1000;
	/* Update the final columb counter */
	if (signbit) {
		ibat_avg = -(fg_accm / samples);
		columb_counter += (ibat_avg * smpl_time)/1000;
	} else {
		ibat_avg = fg_accm / samples;
		columb_counter += (ibat_avg * smpl_time)/1000;
	}
	/* Calculate the average current consumption - right now it is uC/ms = mA*/

	pr_debug("%s: fg_accm %d, ibat_avg %d, signbit %d, smpltime %d\n", __func__, fg_accm, ibat_avg, signbit, smpl_time);

	/* Return new average current */
	return ibat_avg;
}

int csapi_adc_raw_read(struct csapi_cli *cli,
					   u8 cha, u32 *val, csapi_cb cb, void *ptr)
{

	struct adc_channels_t *pchan;
	u32 reading, overflow, current_time;
	int status = CSAPI_ADC_ERR_SUCCESS;
	static u32 ibat_last_sample_time = 0;
	struct timespec ts_current_time;

	if (!bcm59055_adc_chipset_api)
		return -ENOMEM;

	/* Special CSAPI IBAT channels */
	if (cha == CSAPI_ADC_IBAT_AVG) {
		*val = update_columb();
		return CSAPI_ADC_ERR_SUCCESS;
	}
	if (cha == CSAPI_ADC_IBAT_CC) {
		update_columb();
		*val = columb_counter;
		return CSAPI_ADC_ERR_SUCCESS;
	}

	pchan = hal_adc_map_channel(bcm59055_adc_chipset_api->adc_channels, cha);
	GLUE_DBG(("hal_adc_raw_read: Reading CSAPI channel %x, pchan %x", cha, pchan));

	if (!pchan) {
		return -ENODEV;
	}

#ifdef VENDOR_ADC_IBAT_CHANNEL
		if (pchan->unit.unit == adc_unit_ibat_k) {
			/* Fuel gauge... */
			/* Get the current time */

			ts_current_time = CURRENT_TIME;
			current_time = (ts_current_time.tv_sec * 1000) + (ts_current_time.tv_nsec / 1000000); /* milliseconds */
			GLUE_DBG(("hal_adc_raw_read: ibat: last_sample_time %d, current_time %d", ibat_last_sample_time, current_time));
			if (ibat_last_sample_time) {
				if ((current_time - ibat_last_sample_time) < FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS) {
					udelay((FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS - (current_time - ibat_last_sample_time)) * 1000);
					GLUE_DBG(("hal_adc_raw_read: Waittime %d", FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS - (current_time - ibat_last_sample_time)));
					ts_current_time = CURRENT_TIME;
					current_time = (ts_current_time.tv_sec * 1000) + (ts_current_time.tv_nsec / 1000000); /* milliseconds */
					GLUE_DBG(("hal_adc_raw_read: Waiting for next sample to be available, new current_time %d", current_time));
				}
			}
			ibat_last_sample_time = current_time;
			/* Read sampleB register... */
			{
				s16 ibat;

				bcm59055_fg_set_fgfrzsmpl();
				bcm59055_fg_read_sample(fg_smpl_raw, &ibat);
				ibat += 2;	/* For rounding... */
				ibat >>= 2;
				*val = ibat;
				GLUE_DBG(("hal_adc_raw_read: IBAT reading returns %d", ibat));
				return CSAPI_ADC_ERR_SUCCESS;
			}
		}
#endif

	overflow = (1 << pchan->bits) - 1;

	switch (pchan->select) {
	case ADC_VMBAT_CHANNEL:
	case ADC_VBBAT_CHANNEL:
	case ADC_VWALL_CHANNEL:
	case ADC_VBUS_CHANNEL:
	case ADC_ID_CHANNEL:
	case ADC_NTC_CHANNEL:
	case ADC_BOM_CHANNEL:
	case ADC_32KTEMP_CHANNEL:
	case ADC_PATEMP_CHANNEL:
	case ADC_ALS_CHANNEL:
		reading = read_hk_adc(pchan->select);
		break;
	case ADC_BSI_CHANNEL:
	case ADC_BSI_CAL_L_CHANNEL:
	case ADC_NTC_CAL_L_CHANNEL:
	case ADC_NTC_CAL_H_CHANNEL:
	case ADC_BSI_CAL_H_CHANNEL:
	default:
		reading = read_rtm_adc(pchan->select);
		break;
	}

	if (reading == overflow) {
		status = -ERANGE;
	} else
		status = CSAPI_ADC_ERR_SUCCESS;
	*val = reading;

	if (cb) {
		cb(cha, *val, status, ptr);
		GLUE_DBG(("hal_adc_raw_read: returned from callback"));
	}
	GLUE_DBG(("hal_adc_raw_read: Returning value %x, status %d, callback %x, context %x", *val, status, adc_handler, ptr));
	return status;
}
EXPORT_SYMBOL(csapi_adc_raw_read);

int csapi_adc_unit_read(struct csapi_cli *cli,
						u8 cha, u32 *val, csapi_cb cb, void *ptr)
{
	int res;
	u32 raw;
	res = csapi_adc_raw_read(cli, cha, &raw, NULL, NULL);

	if (res >= 0) {
		*val = csapi_adc_unit_convert(cli, cha, raw);
		return 0;
	}
	return res;
}
EXPORT_SYMBOL(csapi_adc_unit_read);

static int bsearch_descending_int_table(int *table, int sz, int value)
{
	int low = 0;
	int high = sz;
	int res;
	while (low < high) {
		int mid = low + (high-low)/2;
		if (table[mid] > value) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	if (low < sz) {
		/* Find best approximation, not just the next lower value in table */
		if (low <= 0 || table[low]-value > value-table[low-1]) {
			res = low;
		} else {
			res = low-1;
		}
	} else {
		res = sz-1;
	}
	return res;
}

static int get_cal_vread (struct adc_channels_t *chan,  u32 raw)
{
	int vread;
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
	/* Voyager back compability - or for future use with only one referece channel */
	u32 stored_reference_reading = 0;
	stored_reference_reading = bcm59055_adc_chipset_api->adc_channels[VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.reference_reading;
	if (stored_reference_reading && value) {
		u16 reference_reading = 0;
		reference_reading += read_adc(VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);
		reference_reading += read_adc(VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);
		reference_reading += read_adc(VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);
		reference_reading += read_adc(VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);
		reference_reading += 2;			/* Ensure correct rounding */
		reference_reading >>= 2;			/* Divide by four */
		raw *= stored_reference_reading;
		raw /= reference_reading;
		vread = raw * chan->unit.uvperlsb + chan->unit.offset;
		pr_debug("get_cal_vread: Temperature compensated value %d, reference_reading = %d, vread = %d",
				  raw, chan->unit.uvperlsb, reference_reading, vread);
		return vread;
	}
#endif
#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
	if (chan->unit.unit == adc_unit_ohms || chan->unit.unit == adc_unit_kelvin_down_table) {
		/* Read the two ADC channels
		*  calculate gain and offset
		*  Both the reading and gain variables needs to be static - there are multiple channels using the
		*  same data.
		*/
		u16 read0, i;
		static u16 read1, read2;
		static int gain, offset;
		static u32 last_time;

		if ((!read1) || (last_time + VENDOR_ADC_COMP_FREQUENCY < get_seconds())) {
			last_time = get_seconds();
			read1 = 0;
			for (i=0; i<VENDOR_ADC_COMP_SAMPLES;) {
				read0 = read_rtm_adc(VENDOR_ADC_NTC_CAL_L_CHANNEL);
				if (read0 != 0x3ff) {
					read1 += read0;
					i++;
				}
			}
			read1 += VENDOR_ADC_COMP_SAMPLES/2;	/* add samples/2 for rounding */
			read1 /= VENDOR_ADC_COMP_SAMPLES;	/* Divide by the number of samples */

			read2 = 0;
			for (i=0; i<VENDOR_ADC_COMP_SAMPLES;) {
				read0 = read_rtm_adc(VENDOR_ADC_NTC_CAL_H_CHANNEL);
				if (read0 != 0x3ff) {
					read2 += read0;
					i++;
				}
			}
			read2 += VENDOR_ADC_COMP_SAMPLES/2;	/* For rounding */
			read2 /= VENDOR_ADC_COMP_SAMPLES;	/* Divide to get average */

			/* Calculate uvperbit and offset */
			if (read1 != read2) {
				gain = ((VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE-VENDOR_ADC_BSI_COMP_LOW_VOLTAGE)*1000) / (read2 - read1);
				offset = VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE*1000 - (read2*gain);
			}
			pr_debug("get_cal_vread: Value %d, read1 %d, read2 %d, gain %d, offset %d, vmax %d",
					  raw, read1, read2, gain, offset, chan->unit.vmax);
		}
		chan->unit.uvperlsb = gain;
		chan->unit.offset = offset;
	}
#endif
	vread = raw * chan->unit.uvperlsb;
	vread += chan->unit.offset;

	return vread;
}

/*int bcm59055_hal_adc_unit_convert(u8 cha, u32 val)*/
int csapi_adc_unit_convert(struct csapi_cli *cli, u8 cha, u32 raw)
{
	struct adc_channels_t *chan;
	int vread, temp_to_return=298, ibat_to_return, ibat, ibat_cal;
	u32 u_iread, u_rread;
	double vread_f, vpullup, iread, rread, rpullup;
#ifdef MATHEMATICAL_TEMPERATURE_CALC
	double rr0, lnrr0, temperature, ft;
#endif
#ifdef DEBUGOUT_ADC_CHIPSET_API
	int vread_32, intc, iread_32, rread_32, rr0_32, lnrr0_32;
#endif

	if (!bcm59055_adc_chipset_api) {
		return -ENOMEM;
	}

	/* Special CSAPI IBAT channels */
	if (cha == CSAPI_ADC_IBAT_AVG || cha == CSAPI_ADC_IBAT_CC) {
		return raw;
	}

	chan = hal_adc_map_channel(bcm59055_adc_chipset_api->adc_channels, cha);
	if (!chan) {
		return -ENODEV;
	}
	if (chan->locked) {
		return chan->lockvalue;
	}

	vread = get_cal_vread (chan, raw);

	GLUE_DBG(("hal_adc_unit_convert: value %d, unit %d, uv %d, vread = %d", value, chan->unit.unit, chan->unit.uvperlsb, vread));
	switch (chan->unit.unit) {
	case adc_unit_volts:
		/* should return in millivolts */
		if ((vread % 1000) > 500) {
			vread += 500;		/* round up... */
		}
		GLUE_DBG(("hal_adc_unit_convert: Millivolts %d", (vread/1000)));
		return (int)(vread/1000);

	case adc_unit_amps:
		iread = vread/chan->unit.data.amps.rshunt;	/* uV / ohms = uA */
		return (int)(iread/1000);

	case adc_unit_ohms:
		/* vread_f = vread; */
		if (cha == CSAPI_ADC_MAIN_CAL) {
			return vread;
		}
		/* Read current consumption */
		csapi_adc_raw_read(NULL, CSAPI_ADC_IBAT, (u32 *)&ibat, NULL, NULL);
		ibat_cal = csapi_adc_unit_convert(NULL, CSAPI_ADC_IBAT, ibat);

		vread += ((ibat_cal + 20)/40)*1000;	/* compensate reading with the 25 mOhm in the FG and battery terminal */
		GLUE_DBG(("hal_adc_unit_convert: res - ibat %d, new vread %d", ibat_cal, vread));
		/* vpullup = chan->unit.vmax - vread; */
		u_iread = ((chan->unit.vmax - vread)*100) / (chan->unit.data.ohms.rpullup);
		if (!u_iread) {
			/* No resistor is present. */
			return -1;
		}
		/* u_iread is now in 10nA */
		vread += u_iread/2;		/* For rounding... */
		u_rread = vread/u_iread;	/* Hecto ohm... */

		GLUE_DBG(("hal_adc_unit_convert: res - iread %d, rread %d", u_iread, u_rread));

		return u_rread;

	case adc_unit_kelvin_down:					/* Temperature where NTC is connected to ground */
#ifdef MATHEMATICAL_TEMPERATURE_CALC
		vread_f = 0;
		vread_f += vread;
		vpullup = chan->unit.vmax - vread;
		rpullup = chan->unit.data.kelvin.rpullup;
		iread = vpullup/rpullup;
		if (chan->unit.data.kelvin.rpar) {
			iread -= vread_f/chan->unit.data.kelvin.rpar;
		}

		if (!iread) {
			/* No resistor is present. */
			return -1;
		}
		rread = vread_f/iread;	/* Unit : Ohms */
		rread -= chan->unit.data.kelvin.rseries;

		ft = chan->unit.data.kelvin.r0;
		rr0 = rread/ft;
		lnrr0 = log(rr0);
		if (rread < chan->unit.data.kelvin.r0) {
			ft = chan->unit.data.kelvin.b;	/* Constant */
		} else {
			ft = chan->unit.data.kelvin.b - 170;	/* Constant */
		}
		lnrr0 /= ft;
		lnrr0 += THERMISTOR_T0;	/* 1/298,15 == default Temperature for NTC */
		temperature = 1/lnrr0;
		temp_to_return = temperature;

#ifdef DEBUGOUT_ADC_CHIPSET_API
		vread_32 = vread_f;
		iread_32 = (vpullup/rpullup)*1000;
		intc = iread * 1000;
		rread_32 = rread;
		rr0_32 = rr0 * 1000,
		lnrr0_32 = lnrr0 * 1000000;
		GLUE_DBG(("hal_adc_unit_convert: temperature: vread_32, iread_32, intc, rread_32, rr0_32, lnrr0_32"));
		GLUE_DBG(("hal_adc_unit_convert: temperature: %d, %d, %d, %d, %d, %d", vread_32, iread_32, intc, rread_32, rr0_32, lnrr0_32));

#endif
#else
		return 0;
#endif
		return temp_to_return;

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	case adc_unit_kelvin_down_table:					/* Temperature where NTC is connected to ground, using CSAPI supplied lookup table. */
		vread_f = 0;
		vread_f += vread;
		vpullup = chan->unit.vmax - vread;
		rpullup = chan->unit.data.kelvin.rpullup;
		iread = vpullup/rpullup;
		if (chan->unit.data.kelvin.rpar) {
			iread -= vread_f/chan->unit.data.kelvin.rpar;
		}

		if (!iread) {
			/* No resistor is present. */
			return -1;
		}
		rread = vread_f/iread;	/* Unit : Ohms */
		rread -= chan->unit.data.kelvin.rseries;

		temp_to_return =
		VENDOR_ADC_KELVIN_DOWN_TABLE_BEGIN +
		bsearch_descending_int_table(VENDOR_ADC_kelvin_down_table,
									 sizeof(VENDOR_ADC_kelvin_down_table) / sizeof(*VENDOR_ADC_kelvin_down_table),
									 rread);

#ifdef DEBUGOUT_ADC_CHIPSET_API
		vread_32 = vread_f;
		iread_32 = (vpullup/rpullup)*1000;
		intc = iread * 1000;
		rread_32 = rread;
		GLUE_DBG(("hal_adc_unit_convert: temperature: vread_32, iread_32, intc, rread_32"));
		GLUE_DBG(("hal_adc_unit_convert: temperature: %d, %d, %d, %d", vread_32, iread_32, intc, rread_32));

#endif

		return temp_to_return;
#endif /* VENDOR_ADC_KELVIN_DOWN_TABLE */

	case adc_unit_kelvin_up:					/* Temperature where NTC is connected to supply voltage */
#ifdef MATHEMATICAL_TEMPERATURE_CALC
		vread_f = vread;
		vpullup = bcm59055_adc_chipset_api->adc_channels[channel].unit.vmax - vread;
		iread = vpullup/chan->unit.data.kelvin.rpullup;
		if (!iread) {
			/* No resistor is present. */
			return -1;
		}
		rread = vread_f/iread;	/* Unit : Ohms */
		rread -= chan->unit.data.kelvin.rseries;

		rr0 = rread/chan->unit.data.kelvin.r0;
		lnrr0 = log(rr0);
		lnrr0 /= chan->unit.data.kelvin.b;	/* Constant */
		lnrr0 += THERMISTOR_T0;	/* 1/298,15 == default Temperature for NTC */
		temperature = 1/lnrr0;
		temp_to_return = temperature;
		GLUE_DBG(("hal_adc_unit_convert: temperature(vpullup, iread, rread, rr0, temperature)"));
		GLUE_DBG(("hal_adc_unit_convert: temperature(%d, %3.8f, %f, %1.3f, %3.2f)",
				  vpullup, iread, rread, rr0, temp_to_return));
#else
		return 0;
#endif
		return temp_to_return;
	case adc_unit_bom:
		return 0; /*hal_adc_get_bom_id();*/
	case adc_unit_ibat_k:
		{
			int modifier = 0, reading = raw;
			if (chan->unit.data.ibat_k.k) {
				modifier = (chan->unit.data.ibat_k.k > 128) ? 768 + chan->unit.data.ibat_k.k : 1024 + chan->unit.data.ibat_k.k;
				ibat_to_return = ((reading * modifier) / 1024) - chan->unit.offset;
				GLUE_DBG(("hal_adc_unit_convert: ibat_to_return before negation and 976 modification %d", ibat_to_return));
			} else
				ibat_to_return = raw;
			ibat_to_return = -((ibat_to_return*976)/1000);
			GLUE_DBG(("hal_adc_unit_convert: value %d, modifier %d Offset %d, ibat %d", value, modifier, chan->unit.offset, ibat_to_return));
			return ibat_to_return;
		}
	case adc_unit_raw:
		return raw;
	case adc_unit_unknown:
		return 0;
	}
	/* should be unreachable... */
	return 0;

}
EXPORT_SYMBOL(csapi_adc_unit_convert);

int csapi_cal_unit_convert(struct csapi_cli *cli, u8 cha, u32 raw)
{
	return csapi_adc_unit_convert(cli, cha, raw);
}
EXPORT_SYMBOL(csapi_cal_unit_convert);

int csapi_cal_unit_convert_lock(struct csapi_cli *cli, u8 cha, int val)
{
	struct adc_channels_t *chan;

	chan = hal_adc_map_channel(bcm59055_adc_chipset_api->adc_channels, cha);

	if (!chan) {
		GLUE_DBG(("%s returns Not supported for channel %d", __func__, cha));
		return -ENODEV;
	}
	pr_debug("%s: pchan 0x%x", __func__, (u32)chan);

	chan->locked = true;
	chan->lockvalue = val;
	return 0;
}
EXPORT_SYMBOL(csapi_cal_unit_convert_lock);

int csapi_cal_unit_convert_unlock(struct csapi_cli *cli, u8 cha)
{
	struct adc_channels_t *chan;

	chan = hal_adc_map_channel(bcm59055_adc_chipset_api->adc_channels, cha);

	if (!chan) {
		GLUE_DBG(("%s returns Not supported for channel %d", __func__, cha));
		return -ENODEV;
	}
	pr_debug(KERN_INFO "%s: pchan 0x%x", __func__, (u32)chan);

	chan->locked = false;
	chan->lockvalue = 0;
	return 0;

}
EXPORT_SYMBOL(csapi_cal_unit_convert_unlock);

int csapi_cal_data_get(struct csapi_cli *cli,
					   u8 cha, u32 *id, u32 *p1, u32 *p2, u32 *p3)
{
	struct adc_channels_t *pchan;

	pchan = hal_adc_map_channel(bcm59055_adc_chipset_api->adc_channels, cha);

	if (!pchan) {
		GLUE_DBG(("hal_adc_cal_get returns Not supported for channel %d", cha));
		return -ENODEV;
	}
	pr_debug("%s: pchan 0x%x", __func__, (u32)pchan);
	*p1 = 0;
	*p2 = 0;
	*p3 = 0;
	*id = pchan->unit.cal_id;

	switch (pchan->unit.unit) {
	case adc_unit_volts:
		*p1 = pchan->unit.uvperlsb;	/* uV per step */
		*p2 = pchan->unit.offset; /* Offset in uV */
		break;
	case adc_unit_kelvin_down:
	case adc_unit_kelvin_down_table:
	case adc_unit_kelvin_up:
		*p1 = pchan->unit.uvperlsb;	/* uV per step */
		*p2 = pchan->unit.vmax;		/* reference voltage, uV */
		break;
	case adc_unit_ohms:
		*p1 = pchan->unit.uvperlsb;				/* uV per step */
		if (cha == CSAPI_ADC_MAIN_CAL) {
			*p2=pchan->unit.offset;
		} else {
			*p2 = pchan->unit.offset;
			*p3 = pchan->unit.vmax;					/* VNTC */
#if !defined(VENDOR_ADC_TEMPERATURE_COMP_CHANNEL)
			*p3 = pchan->unit.vmax;
#else
			*p3 = pchan->unit.data.ohms.reference_reading;
#endif
		}
		break;
	case adc_unit_ibat_k:
		{
			s16 off;
			*p1 = pchan->unit.data.ibat_k.k;
			bcm59055_fg_read_offset(&off);
			pchan->unit.offset = off;
			*p2 = pchan->unit.offset;
		}
		break;
	case adc_unit_amps:
	case adc_unit_bom:
	case adc_unit_raw:
	case adc_unit_unknown:
		break;
	}
	GLUE_DBG(("hal_adc_cal_get for channel %d: Id %d, values %d, %d, %d",
			  cha, *identification, *p1, *p2, *p3));
	return CSAPI_ADC_ERR_SUCCESS;
}


int csapi_cal_data_set(struct csapi_cli *cli,
					   u8 cha, u32 id, u32 p1, u32 p2, u32 p3)
{
	struct adc_channels_t *pchan;
	/* Do we have a copy of the structure? If not, use primary structure as source. */
	pchan = hal_adc_map_channel(bcm59055_adc_chipset_api->adc_channels, cha);

	if (!pchan) {
		GLUE_DBG(("hal_adc_cal_get returns Not supported for channel %d", cha));
		return -ENODEV;
	}

	pchan->unit.cal_id = id;

	switch (pchan->unit.unit) {
	case adc_unit_volts:
		pchan->unit.uvperlsb = p1;	/* uV per step */
		pchan->unit.offset = p2;	/* Offset in uV */
		break;
	case adc_unit_kelvin_down:
	case adc_unit_kelvin_down_table:
	case adc_unit_kelvin_up:
		pchan->unit.uvperlsb = p1;	/* uV per step */
		pchan->unit.vmax = p2;		/* reference voltage, uV */
		break;
	case adc_unit_ohms:
		pchan->unit.uvperlsb = p1;				/* uV per step */
		if (cha == CSAPI_ADC_MAIN_CAL) {
			pchan->unit.offset = p2;
		} else {
			pchan->unit.offset = p2;
#if !defined(VENDOR_ADC_TEMPERATURE_COMP_CHANNEL)
			pchan->unit.vmax = p3;					/* VNTC */
#else
			pchan->unit.data.ohms.reference_reading = p3;
#endif
		}
		break;
	case adc_unit_ibat_k:
		pchan->unit.data.ibat_k.k = p1;
		bcm59055_fg_write_gain_trim(pchan->unit.data.ibat_k.k);
		pchan->unit.offset = p2;
		/* shouldn't we get it from the PMU it self? */
		break;
	case adc_unit_amps:
	case adc_unit_bom:
	case adc_unit_raw:
	case adc_unit_unknown:
		break;

	}
	GLUE_DBG(("hal_adc_cal_get for channel %d: Id %d, values %d, %d, %d",
			  cha, identification, p1, p2, p3));
	return CSAPI_ADC_ERR_SUCCESS;
}




s32 hal_adc_cal_calc_dalton(struct csapi_cal_req *Data)
{
	struct adc_channels_t *pchan = NULL;
	struct adc_channels_t *adc_channels = bcm59055_adc_chipset_api->adc_channels;
	u32 vapplied_uv;
	u8 element;

#ifdef VENDOR_ADC_BSI_CHANNEL
	/*uint32 vbsi_val1, vbsi_val2; */
	u32 vbsi_pullup;
#endif
#ifdef VENDOR_ADC_VBAT_SCALE_CHANNEL
	u32 vbat_val1 = 0, vbat_read1 = 0;
	s32 vbat_val_diff, vbat_read_diff;
#endif
#ifdef VENDOR_ADC_VCHAR_CHANNEL
	u32 vchar_val1 = 0, vchar_read1 = 0;
	s32 vchar_val_diff, vchar_read_diff;
#endif


	GLUE_DBG(("hal_adc_cal_calc_dalton: NumberOfElements %d", Data->NumberOfElements));

	if (!Data->size) {
		return -EINVAL;

	}
	for (element=0; element < Data->size; element++) {
		pchan = hal_adc_map_channel(adc_channels, Data->data[element].cha);
		if (!pchan) {
			return -ENODEV;
		}
		pchan->unit.cal_id = Data->data[element].id;
		switch (Data->data[element].cha) {

#ifdef VENDOR_ADC_BSI_CHANNEL
		case CSAPI_ADC_BSI:
			/* Get the two reference values from the ADC */
			{
#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
				u16 read1, read2, i;

				/* Read the two ADC channels */
				read1 = 0;
				for (i = 0; i < VENDOR_ADC_COMP_SAMPLES; i++) {
					read1 += read_rtm_adc(VENDOR_ADC_BSI_CAL_L_CHANNEL);
				}
				read1 += VENDOR_ADC_COMP_SAMPLES/2;	/* add samples/2 for rounding */
				read1 /= VENDOR_ADC_COMP_SAMPLES;	/* Divide by the number of samples */

				read2 = 0;
				for (i = 0; i < VENDOR_ADC_COMP_SAMPLES; i++) {
					read2 += read_rtm_adc(VENDOR_ADC_BSI_CAL_H_CHANNEL);
				}
				read2 += VENDOR_ADC_COMP_SAMPLES/2;	/* For rounding */
				read2 /= VENDOR_ADC_COMP_SAMPLES;	/* Divide to get average */

				/* Calculate uvperbit and offset */
				if (read1 != read2) {
					pchan->unit.uvperlsb = ((VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE-VENDOR_ADC_BSI_COMP_LOW_VOLTAGE)*1000) / (read2 - read1);
					pchan->unit.offset = VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE*1000 - (read2*pchan->unit.uvperlsb);
					GLUE_DBG(("adc_cal_calc_dalton: read1 %d, read2 %d, gain %d, offset %d",
							  read1, read2, pchan->unit.uvperlsb, pchan->unit.offset));
				} else {
					GLUE_DBG(("adc_cal_calc_dalton: Unable to get reference channel readings"));
				}

#endif

				/* Calculate the BSI_VREF based on the three readings */
				vbsi_pullup = (Data->data[element].raw * pchan->unit.uvperlsb + pchan->unit.offset) *(pchan->unit.data.ohms.rpullup / 1000);
				vbsi_pullup /= Data->data[element].ref;
				vbsi_pullup *= 1000;	/* Overflow 'danger' has passed, go back to uV */
				vbsi_pullup += Data->data[element].raw * pchan->unit.uvperlsb + pchan->unit.offset;
				pchan->unit.vmax = vbsi_pullup;
			}
			break;
#endif

#ifdef VENDOR_ADC_VBAT_SCALE_CHANNEL
		case CSAPI_ADC_VBAT:
			/* Even though we're not supposed to implement it in Dalton, then the possibility doesn't harm */
			if (!vbat_read1) {
				/* Store values for later reference... */
				vbat_read1 = Data->data[element].raw;
				vbat_val1 = Data->data[element].ref;

				vapplied_uv = Data->data[element].ref;
				vapplied_uv *= 1000;		/* Make it microvolts */
				adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = vapplied_uv / Data->data[element].raw;
				adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset = 0;
				GLUE_DBG(("hal_adc_cal_calc_dalton: VBAT one point: ref. %d, reading %d, Applied %d, uvperlsb %d",
						  Data->data[element].ref, Data->data[element].raw, vapplied_uv, adc_cal_channels[6].unit.uvperlsb));
			} else {
				/* Two point calibration */
				vapplied_uv = Data->data[element].ref * 1000;		/* Make it microvolts */
				vbat_val1 *= 1000;	/* also microvolts */
				vbat_val_diff = vapplied_uv - vbat_val1;
				vbat_read_diff = Data->data[element].raw - vbat_read1;
				adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = vbat_val_diff / vbat_read_diff;

				adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset = vbat_val1 - (adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb * vbat_read1);
				GLUE_DBG(("hal_adc_cal_calc_dalton: VBAT two point: voltages(%d, %d), references(%d, %d)",
						  vapplied_uv, vbat_val1, Data->data[element].raw, vbat_read1));
				GLUE_DBG(("hal_adc_cal_calc_dalton: VBAT two point: uvperlsb %d, offset %d",
						  adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb, adc_cal_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset));
			}
			break;
#endif

#ifdef VENDOR_ADC_BTEMP_CHANNEL
		case CSAPI_ADC_BTEMP:
			break;
#endif
#ifdef VENDOR_ADC_VCHAR_CHANNEL
		case CSAPI_ADC_VCHAR:
			if (!vbat_read1) {
				/* Store values for later reference... */
				vchar_read1 = Data->data[element].raw;
				vchar_val1 = Data->data[element].ref;

				vapplied_uv = Data->data[element].ref;
				vapplied_uv *= 1000;		/* Make it microvolts */
				adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = vapplied_uv / Data->data[element].raw;
				adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.offset = 0;
				GLUE_DBG(("hal_adc_cal_calc_dalton: VCHAR one point: ref. %d, reading %d, Applied %d, uvperlsb %d",
						  Data->data[element].ref, Data->data[element].raw, vapplied_uv, adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb));
			} else {
				/* Two point calibration */
				vapplied_uv = Data->data[element].ref * 1000;		/* Make it microvolts */
				vchar_val1 *= 1000;	/* also microvolts */
				vchar_val_diff = vapplied_uv - vchar_val1;
				vchar_read_diff = Data->data[element].raw - vchar_read1;
				adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = vchar_val_diff / vchar_read_diff;

				adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.offset = vchar_val1 - (adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb * vbat_read1);
				GLUE_DBG(("hal_adc_cal_calc_dalton: VCHAR two point: voltages(%d, %d), references(%d, %d)",
						  vapplied_uv, vbat_val1, Data->data[element].raw, vbat_read1));
				GLUE_DBG(("hal_adc_cal_calc_dalton: VCHAR two point: uvperlsb %d, offset %d",
						  adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb, adc_channels[VENDOR_ADC_VCHAR_CHANNEL].unit.offset));
			}
			break;
#endif
#ifdef VENDOR_ADC_IBAT_CHANNEL
		case CSAPI_ADC_IBAT:
			if (pchan->unit.unit == adc_unit_amps) {
				/* classical RSHUNT calibration - not implemented */
				break;
			}
			if (pchan->unit.unit == adc_unit_ibat_k) {
				s16 fgoffset;
				bcm59055_fg_read_offset(&fgoffset);
				GLUE_DBG(("hal_Adc_cal_calc_dalton: IBAT K calibration: FGOFFSET read from the PMU: 0x%x", fgoffset));
				fgoffset >>= 2;
				GLUE_DBG(("hal_Adc_cal_calc_dalton: IBAT K calibration: Converted FGOFFSET: 0x%x", fgoffset));
				/* k is defined as IBAT(measured)*1024/IBAT(reading-offset) % 256 */
				adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k = ((Data->data[element].ref * 1024) / ((Data->data[element].raw - fgoffset)*976/1000)) % 256;
				adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.offset = fgoffset;
				GLUE_DBG(("hal_Adc_cal_calc_dalton: IBAT K calibration: Reading %d, reference value %d, k %d, offset %d",
						  Data->data[element].raw, Data->data[element].ref, adc_cal_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k,
						  adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.offset));
				bcm59055_fg_write_gain_trim(adc_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k);

			}
			break;
#endif
		default:
			return -EINVAL;
		}
	}
	return CSAPI_ADC_ERR_SUCCESS;
}

/*s32 bcm59055_adc_cal_calc(struct CSAPI_ADC_CAL_DATA_TYPE * Data)*/
int csapi_cal_calc(struct csapi_cal_req *req)
{
	/* Allocate copy of structure */
	if (hal_adc_hw_cal_calc_func) {
		return hal_adc_hw_cal_calc_func(req);
	} else {
		return -EINVAL;
	}
}
EXPORT_SYMBOL(csapi_cal_calc);
/* Debug interface */

static int bcm59055_adc_chipset_open(struct inode *inode, struct file *file)
{
	pr_debug("%s\n", __func__);
	file->private_data = PDE(inode)->data;

	return 0;
}

int bcm59055_adc_chipset_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static long bcm59055_adc_chipset_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	/* TODO: */
	return 0;
}

#define MAX_USER_INPUT_LEN      256
#define MAX_ARGS 25

static ssize_t bcm59055_adc_chipset_write(struct file *file, const char __user *buffer,
										  size_t len, loff_t *offset)
{
	/*struct bcm59055_adc_chipset_api *data = file->private_data;*/
	/*struct adc_debug dbg;*/
	char cmd[MAX_USER_INPUT_LEN], * pcmd = cmd;
	char **ap, arg_cnt;
	char *argv[MAX_ARGS];
	u8 channel;
	int i;

	/*int ret, i;*/
	int adc_raw, adc_unit;

	pr_info("%s\n", __func__);

	if (!bcm59055_adc_chipset_api) {
		pr_err("%s: driver not initialized\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(cmd, buffer, len)) {
		pr_err("%s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';

	arg_cnt = 0;
	for (ap = argv;(*ap = strsep(&pcmd, " \t")) != NULL; arg_cnt++) {
		if (**ap != '\0') {
			if (++ap >= &argv[MAX_ARGS]) {
				break;
			}
		}
	}

	printk(KERN_INFO "%s: Command %s, total number of args(including first command) %d", __func__, argv[0], arg_cnt);
	i = 0;
	while (i < arg_cnt) {
		if (strcmp(argv[i], "VBAT_UNIT") == 0) {
			csapi_adc_raw_read(NULL, CSAPI_ADC_VBAT, &adc_raw, NULL, NULL);
			adc_unit = csapi_adc_unit_convert(NULL, CSAPI_ADC_VBAT, adc_raw);
			printk(KERN_INFO "%s: VBAT: Raw %d, unit %d mV\n", __func__, adc_raw, adc_unit);

		} else if (strcmp(argv[i], "BSI_UNIT") == 0) {
			csapi_adc_raw_read(NULL, CSAPI_ADC_BSI, &adc_raw, NULL, NULL);
			adc_unit = csapi_adc_unit_convert(NULL, CSAPI_ADC_BSI, adc_raw);
			printk(KERN_INFO "%s: BSI: Raw %d, unit %d HOhm\n", __func__, adc_raw, adc_unit);
		} else if (strcmp(argv[i], "BTEMP_UNIT") == 0) {
			csapi_adc_raw_read(NULL, CSAPI_ADC_BTEMP, &adc_raw, NULL, NULL);
			adc_unit = csapi_adc_unit_convert(NULL, CSAPI_ADC_BTEMP, adc_raw);
			printk(KERN_INFO "%s: BTEMP: Raw %d, unit %d K\n", __func__, adc_raw, adc_unit);
		}

		else if (strcmp(argv[i], "csapi_adc_raw_read") == 0) {
			/* Only(CSAPI) channel can be supplied here, rest is given */
			if (argv[i+1]) {
				csapi_adc_raw_read(NULL, simple_strtol(argv[i+1], NULL, 0), &adc_raw, NULL, NULL);
				printk(KERN_INFO "%s: Raw %d\n", __func__, adc_raw);
				i++; /* since we are using two arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_adc_unit_read") == 0) {
			/* Only(CSAPI) channel can be supplied here, rest is given */
			if (argv[i+1]) {
				csapi_adc_unit_read(NULL, simple_strtol(argv[i+1], NULL, 0), &adc_raw, NULL, NULL);
				printk(KERN_INFO "%s: Channel %s, %d\n", __func__,argv[i+1], adc_raw);
				i++; /* since we are using two arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}

		} else if (strcmp(argv[i], "csapi_adc_unit_convert") == 0) {
			/* CSAPI channel and raw reading can be supplied here, rest is given */
			if (argv[i+2]) {
				channel = simple_strtol(argv[i+1], NULL, 0);
				adc_raw = simple_strtol(argv[i+2], NULL, 0);
				adc_unit = csapi_adc_unit_convert(NULL, channel, adc_raw);
				printk(KERN_INFO "%s: Channel %d, Raw %d, unit %d\n", __func__, channel, adc_raw, adc_unit);
				i += 2;	/* since we are using two arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_unit_convert_lock") == 0) {
			/* CSAPI channel and raw reading can be supplied here, rest is given */
			if (argv[i+2]) {
				channel = simple_strtol(argv[i+1], NULL, 0);
				adc_unit = simple_strtol(argv[i+2], NULL, 0);
				csapi_cal_unit_convert_lock(NULL, channel, adc_unit);
				printk(KERN_INFO "%s: Channel %d, locked to %d\n", __func__, channel, adc_unit);
				i += 2;	/* since we are using two arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_unit_convert_unlock") == 0) {
			/* CSAPI channel and raw reading can be supplied here, rest is given */
			if (argv[i+1]) {
				channel = simple_strtol(argv[i+1], NULL, 0);
				csapi_cal_unit_convert_unlock(NULL, channel);
				printk(KERN_INFO "%s: Channel %d, unlocked\n", __func__, channel);
				i++; /* since we are using two arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_data_get") == 0) {
			if (argv[i+1]) {
				u32 id, p1, p2, p3;
				channel = simple_strtol(argv[i+1], NULL, 0);
				csapi_cal_data_get(NULL, channel, &id, &p1, &p2, &p3);
				printk(KERN_INFO "%s(%s): chl %d, id %d, p1 %d, p2 %d, p3 %d\n", __func__, argv[i], channel, id, p1, p2, p3);
				i++; /* since we are using two arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_data_set") == 0) {
			if (argv[i+5]) {
				u32 id, p1, p2, p3;
				channel = simple_strtol(argv[i+1], NULL, 0);
				id = simple_strtol(argv[i+2], NULL, 0);
				p1 = simple_strtol(argv[i+3], NULL, 0);
				p2 = simple_strtol(argv[i+4], NULL, 0);
				p3 = simple_strtol(argv[i+5], NULL, 0);
				csapi_cal_data_set(NULL, channel, id, p1, p2, p3);
				printk(KERN_INFO "%s(%s): chl %d, id %d, p1 %d, p2 %d, p3 %d\n", __func__, argv[i], channel, id, p1, p2, p3);
				i += 5;	/* since we are using five arguments here, add one extra to i */
			} else {
				printk(KERN_INFO "%s: Not enough parameters for %s\n", __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_data_calc") == 0) {
			/* only one command in this case */
			if (i == 0) {
				struct csapi_cal_req * cal;
				arg_cnt--; /* skip command */
				cal = kzalloc(sizeof(struct csapi_cal_req) + (sizeof(struct csapi_cal_data)*(arg_cnt/4)), GFP_KERNEL);
				cal->size = arg_cnt / 4;
				i = 0;
				while (i < cal->size) {
					cal->data[i].cha = simple_strtol(argv[i*4+1], NULL, 0);
					cal->data[i].type = simple_strtol(argv[i*4+2], NULL, 0);
					cal->data[i].raw = simple_strtol(argv[i*4+3], NULL, 0);
					cal->data[i].ref = simple_strtol(argv[i*4+4], NULL, 0);
					cal->data[i].id = 0xfedeabe;
					i++;
				}
				if (cal->size) {
					csapi_cal_calc(cal);
				}
				kfree(cal);
			} else {
				printk(KERN_INFO "%s: Calc is only allowed alone", __func__);
			}
			i = arg_cnt;

		}
		i++;
	}
	*offset += len;
	return len;
}

static const struct file_operations bcm59055_adc_chipset_ops = {
	.open = bcm59055_adc_chipset_open,
	.unlocked_ioctl = bcm59055_adc_chipset_ioctl,
	.write = bcm59055_adc_chipset_write,
	.release = bcm59055_adc_chipset_release,
	.owner = THIS_MODULE,
};

/* Framework inteface */

static int __devinit bcm59055_adc_chipset_api_probe(struct platform_device *pdev)
{
	struct bcm590xx *bcm59055 = dev_get_drvdata(pdev->dev.parent);
	struct bcm59055_adc_chipset_api *priv_data;

	pr_info("BCM59055 ADC CHIPSET API Driver\n");
	priv_data = kzalloc(sizeof(struct bcm59055_adc_chipset_api), GFP_KERNEL);
	if (!priv_data) {
		pr_info("%s: Memory can not be allocated!!\n",
				__func__);
		return -ENOMEM;
	}
	priv_data->bcm59055 = bcm59055;
	bcm59055_adc_chipset_api = priv_data;

	hal_adc_init_channels(false); /* False for now - true when data can be retrieved */


	sema_init(&priv_data->fg_read_sem, 0);
	sema_init(&priv_data->rtm_read_sem, 0);

	/* Make sure fuel gauge is active. So far we don't care about return value */
	bcm59055_fg_enable();
	bcm59055_fg_offset_cal(false);
	/* When we get calibraion data, call driver to set the value */

	proc_create_data("adc_chipset_api", S_IRWXUGO, NULL, &bcm59055_adc_chipset_ops, bcm59055_adc_chipset_api);

	return 0;
}

static int __devexit bcm59055_adc_chipset_api_remove(struct platform_device *pdev)
{
	struct bcm59055_adc_chipset_api_driver *priv_data = platform_get_drvdata(pdev);
	bcm59055_fg_disable();

	kfree(priv_data);
	return 0;
}

struct platform_driver bcm59055_adc_chipset_api_driver = {
	.probe = bcm59055_adc_chipset_api_probe,
	.remove = __devexit_p(bcm59055_adc_chipset_api_remove),
			  .driver = {
		.name = "bcm59055-adc_chipset_api",
	}
};

static int __init bcm59055_adc_chipset_api_init(void)
{
	platform_driver_register(&bcm59055_adc_chipset_api_driver);
	/* initialize semaphore for ADC access control */
	return 0;
}				/* bcm59055_fg_init */

/****************************************************************************
*
*  bcm59055_adc_chipset_api_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit bcm59055_adc_chipset_api_exit(void)
{
	platform_driver_unregister(&bcm59055_adc_chipset_api_driver);

}				/* bcm59055_fg_exit */

subsys_initcall(bcm59055_adc_chipset_api_init);
module_exit(bcm59055_adc_chipset_api_exit);

MODULE_AUTHOR("HJEHG");
MODULE_DESCRIPTION("BCM59055 ADC CHIPSET API Driver");




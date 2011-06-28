/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/misc/bcm59055-fuelgauge.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
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
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/core.h>
//#include <linux/workqueue.h>
#include <linux/semaphore.h>
//#include <mach/irqs.h>
#include <linux/stringify.h>
#include <linux/proc_fs.h>
#include <linux/broadcom/bcm59055-adc.h>
#include <linux/broadcom/bcm59055-adc_chipset_api.h>
#include <linux/broadcom/bcm59055-adc_chipset_hw.h>
#include <linux/broadcom/bcm59055-fuelgauge.h>


#define GLUE_DBG(s)

#define FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS 9

#define THERMISTOR_T0	0.00335401		// 1/298,15 - T0 (25 degrees C)

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
    int VENDOR_ADC_kelvin_down_table[] = VENDOR_ADC_KELVIN_DOWN_TABLE;
#endif

static int bsearch_descending_int_table(int* table, int sz, int value);

typedef enum {
	adc_unit_unknown = 0,
	adc_unit_volts,
	adc_unit_amps,
	adc_unit_ohms,
	adc_unit_kelvin_down,	/* NTC resistor is connected to ground */
	adc_unit_kelvin_down_table,	/* NTC resistor is connected to ground (table lookup) */
	adc_unit_kelvin_up,		/* NTC resistor is connected to supply */
	adc_unit_bom,			/* BOM detection */
	adc_unit_ibat_k
} adc_channel_unit_t;

struct adc_unit_t {
	adc_channel_unit_t unit;
	u32 cal_id; 			// calibration identifier...
	u32 uvperlsb;
	u32 vmax;
	int offset;
	union {
		struct {
			int dummy;
		} volts;
		struct {
			u32 rshunt;		//
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

void hal_adc_init_channels (u8 use_cal_data)
{
	struct adc_channels_t local_adc_channels [VENDOR_ADC_NUM_CHANNELS];
	struct adc_channels_t *adc_cal_channels = NULL;
	int channel;
#if 0
	u8 status;
	u32 size;
	int EM_PMM_index;
#endif

	memset (local_adc_channels, 0, sizeof (local_adc_channels));

	for (channel = 0; channel < VENDOR_ADC_NUM_CHANNELS; channel++) {				
		local_adc_channels [channel].bits = 10;
		local_adc_channels [channel].select = channel;
		local_adc_channels [channel].locked = 0;
		local_adc_channels [channel].lockvalue = 0;
		local_adc_channels [channel].unit.cal_id = 0;
		GLUE_DBG (("hal_adc_init_channel: Initializing channel %x (%x, %d)", channel, local_adc_channels [channel].select, local_adc_channels [channel].bits));
	}

#if 0 /* TBD how this is done in Clipper */
	status = vendor_secure_storage_data_size_get (&size, VENDOR_AREA_EM_CAL);
	/*backward compatibility*/
	/*check if data has the right size. If not then attempt to read from the old index*/
	if ((status == VENDOR_SECURE_STORAGE_OK) && (size == sizeof (adc_channels_t) * VENDOR_ADC_NUM_CHANNELS))
	{
		EM_PMM_index = VENDOR_AREA_EM_CAL;
	}
	else
	{
		EM_PMM_index = VENDOR_AREA_EM_CAL_OLD;
		status = vendor_secure_storage_data_size_get (&size, EM_PMM_index);	
	}
	GLUE_DBG (("hal_adc_init: Status %d, Reading EM cal from PMM index %d, size %d", status, EM_PMM_index, size));
	if ((status == VENDOR_SECURE_STORAGE_OK) && (size == sizeof (adc_channels_t) * VENDOR_ADC_NUM_CHANNELS)) {
		/*OK, let us get the data. */
		adc_cal_channels = OSHEAP_Alloc (size);
		assert (adc_cal_channels);
		status = vendor_secure_storage_data_read (adc_cal_channels, size, 0, EM_PMM_index);
		/* Verify structure */
		if (status == VENDOR_SECURE_STORAGE_OK) {
			GLUE_DBG (("hal_adc_init: Verifying ADC data integrity"));

			for (channel=0; channel<VENDOR_ADC_NUM_CHANNELS; channel++) {
				if (adc_cal_channels[channel].select != local_adc_channels[channel].select) {
					channel = 0;
					break;
				}
			}
			if (!channel) {
				GLUE_DBG (("hal_adc_init: ADC data integrity test failed"));
				OSHEAP_Delete (adc_cal_channels);
				adc_cal_channels = NULL;
			}
		}
	}
#endif

#ifdef VENDOR_ADC_VBAT_SCALE_CHANNEL
	local_adc_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.unit = adc_unit_volts;
	if (adc_cal_channels) {
		local_adc_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = adc_cal_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb;				// VREF * (540k + 180 k)/180 k / steps
		local_adc_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset = adc_cal_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.offset;		
		local_adc_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.cal_id = adc_cal_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.cal_id;
	}
	else {
#ifdef VENDOR_ADC_VBAT_SCALE_UVPERBITS
		local_adc_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = VENDOR_ADC_VBAT_SCALE_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VBAT_SCALE_PULLUP + VENDOR_ADC_VBAT_SCALE_PULLDOWN)*1000) / VENDOR_ADC_VBAT_SCALE_PULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels [VENDOR_ADC_VBAT_SCALE_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VBAT_SCALE_CHANNEL].bits);				// VREF * (540k + 180 k)/180 k / steps
#endif
	}
#endif

#ifdef VENDOR_ADC_BSI_CHANNEL
	local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.unit = adc_unit_ohms;
	local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.rpullup = VENDOR_ADC_BSI_RPULLUP;			// Rpullup in Ohm
	if (adc_cal_channels) {
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.uvperlsb = adc_cal_channels [VENDOR_ADC_BSI_CHANNEL].unit.uvperlsb;
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.offset = adc_cal_channels [VENDOR_ADC_BSI_CHANNEL].unit.offset;		// Relative reference value (used from B1_2)
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.vmax = adc_cal_channels [VENDOR_ADC_BSI_CHANNEL].unit.vmax;		// Relative reference value (used from B1_2)
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.reference_reading = adc_cal_channels [VENDOR_ADC_BSI_CHANNEL].unit.data.ohms.reference_reading;		// Relative reference value (used from B1_2)
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.cal_id = adc_cal_channels [VENDOR_ADC_BSI_CHANNEL].unit.cal_id;
	}
	else {
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.vmax = VENDOR_ADC_BSI_VMAX * 1000;
		local_adc_channels [VENDOR_ADC_BSI_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels [VENDOR_ADC_BSI_CHANNEL].bits);	// 1,2 v, 1024 steps, 10 bit ADC
	}
#endif

#ifdef VENDOR_ADC_BTEMP_CHANNEL

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.unit = adc_unit_kelvin_down_table;
#else
	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.unit = adc_unit_kelvin_down;
#endif // VENDOR_ADC_KELVIN_DOWN_TABLE

	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.data.ohms.rpullup = VENDOR_ADC_BTEMP_RPULLUP;			// Rpullup in Ohm
	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.r0 = VENDOR_ADC_BTEMP_R0;			// NTC R0 in Ohm
	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.b = VENDOR_ADC_BTEMP_B;			// Temperature coeficient
	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.rpar = VENDOR_ADC_BTEMP_RPAR;			// Parallel resistor in Ohm
	local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.data.kelvin.rseries = VENDOR_ADC_BTEMP_RSERIES;			// No series resistor
	if (adc_cal_channels) {
		local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.vmax = adc_cal_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.vmax;
		local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.uvperlsb = adc_cal_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.uvperlsb;
		local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.offset = adc_cal_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.offset;		// Relative reference value (used from B1_2)
		local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.cal_id = adc_cal_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.cal_id;
	}
	else {
		local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.vmax = VENDOR_ADC_BTEMP_VMAX * 1000;
		local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels [VENDOR_ADC_BTEMP_CHANNEL].bits);	// 1,2 v, 1024 steps, 10 bit ADC
	}
#endif

#ifdef VENDOR_ADC_PATEMP_CHANNEL

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.unit = adc_unit_kelvin_down_table;
#else
	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.unit = adc_unit_kelvin_down;
#endif // VENDOR_ADC_KELVIN_DOWN_TABLE

	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.rpullup = VENDOR_ADC_PATEMP_RPULLUP;			// Rpullup in Ohm
	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.r0 = VENDOR_ADC_PATEMP_R0;			// NTC R0 in Ohm
	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.b = VENDOR_ADC_PATEMP_B;			// Temperature coeficient
	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.rpar = VENDOR_ADC_PATEMP_RPAR;			// Parallel resistor in Ohm
	local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.data.kelvin.rseries = VENDOR_ADC_PATEMP_RSERIES;			// No series resistor


	if (adc_cal_channels) {
		local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.vmax = adc_cal_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.vmax;
		local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.uvperlsb = adc_cal_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.uvperlsb;
		local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.offset = adc_cal_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.offset;		// Relative reference value (used from B1_2)
		local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.cal_id = adc_cal_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.cal_id;

	}
	else {
		local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.vmax = VENDOR_ADC_PATEMP_VMAX * 1000;
		local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].bits);	// 1,2 v, 1024 steps, 10 bit ADC
	}
#endif

#ifdef VENDOR_ADC_X32TEMP_CHANNEL

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.unit = adc_unit_kelvin_down_table;
#else
	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.unit = adc_unit_kelvin_down;
#endif // VENDOR_ADC_KELVIN_DOWN_TABLE

	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.rpullup = VENDOR_ADC_X32TEMP_RPULLUP;			// Rpullup in Ohm
	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.r0 = VENDOR_ADC_X32TEMP_R0;			// NTC R0 in Ohm
	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.b = VENDOR_ADC_X32TEMP_B;			// Temperature coeficient
	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.rpar = VENDOR_ADC_X32TEMP_RPAR;			// Parallel resistor in Ohm
	local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.data.kelvin.rseries = VENDOR_ADC_X32TEMP_RSERIES;			// No series resistor


	if (adc_cal_channels && adc_cal_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax) {
		local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax = adc_cal_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax;
		local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.uvperlsb = adc_cal_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.uvperlsb;
		local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.offset = adc_cal_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.offset;		// Relative reference value (used from B1_2)
		local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.cal_id = adc_cal_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.cal_id;

	}
	else {
		local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.vmax = VENDOR_ADC_X32TEMP_VMAX * 1000;
		local_adc_channels [VENDOR_ADC_X32TEMP_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels [VENDOR_ADC_PATEMP_CHANNEL].bits);	// 1,2 v, 1024 steps, 10 bit ADC
	}
#endif

#ifdef VENDOR_ADC_VCHAR_CHANNEL

	local_adc_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.unit = adc_unit_volts;
	if (adc_cal_channels && adc_cal_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb) {
		local_adc_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = adc_cal_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb;
		local_adc_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.offset = adc_cal_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.offset;	
	}
	else {
#ifdef VENDOR_ADC_VCHAR_UVPERBITS
		local_adc_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = VENDOR_ADC_VCHAR_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VCHAR_RPULLUP + VENDOR_ADC_VCHAR_RPULLDOWN)*1000) / VENDOR_ADC_VCHAR_RPULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels [VENDOR_ADC_VCHAR_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].bits);				// VREF * (540k + 180 k)/180 k / steps
#endif
	}
#endif

#ifdef VENDOR_ADC_VBUS_CHANNEL
	local_adc_channels [VENDOR_ADC_VBUS_CHANNEL].unit.unit = adc_unit_volts;
	{
#ifdef VENDOR_ADC_VBUS_UVPERBITS
		local_adc_channels [VENDOR_ADC_VBUS_CHANNEL].unit.uvperlsb = VENDOR_ADC_VBUS_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VBUS_RPULLUP + VENDOR_ADC_VBUS_RPULLDOWN)*1000) / VENDOR_ADC_VCHAR_RPULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels [VENDOR_ADC_VBUS_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VCHAR_CHANNEL].bits);				// VREF * (540k + 180 k)/180 k / steps
#endif
	}
#endif

#ifdef VENDOR_ADC_VBBAT_CHANNEL
	local_adc_channels [VENDOR_ADC_VBBAT_CHANNEL].unit.unit = adc_unit_volts;
	{
#ifdef VENDOR_ADC_VBBAT_UVPERBITS
		local_adc_channels [VENDOR_ADC_VBBAT_CHANNEL].unit.uvperlsb = VENDOR_ADC_VBBAT_UVPERBITS;
#else
		u32 vmax;
		vmax = ((VENDOR_ADC_VBBAT_RPULLUP + VENDOR_ADC_VBBAT_RPULLDOWN)*1000) / VENDOR_ADC_VBBAT_RPULLDOWN;
		vmax *= VENDOR_ADC_REFERENCE_VOLTAGE;
		local_adc_channels [VENDOR_ADC_VBBAT_CHANNEL].unit.uvperlsb = vmax / (1<<local_adc_channels[VENDOR_ADC_VBBAT_CHANNEL].bits);				// VREF * (540k + 180 k)/180 k / steps
#endif
	}
#endif

#ifdef VENDOR_ADC_IBAT_CHANNEL
	{
		u8 k;
		local_adc_channels [VENDOR_ADC_IBAT_CHANNEL].unit.unit = adc_unit_ibat_k;
		local_adc_channels [VENDOR_ADC_IBAT_CHANNEL].unit.uvperlsb = 1000;
		local_adc_channels [VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k = 0;
		if (adc_cal_channels) {
			local_adc_channels [VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k = adc_cal_channels[VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k;
			local_adc_channels [VENDOR_ADC_IBAT_CHANNEL].unit.offset= adc_cal_channels[VENDOR_ADC_IBAT_CHANNEL].unit.offset;
			// Tell driver about the K value
			k = local_adc_channels [VENDOR_ADC_IBAT_CHANNEL].unit.data.ibat_k.k;
			if (k) {
				/* Call driver...*/
				// adcchipsetapi_pmu_write (1/*PAGE1*/, 0xc5 /*FGGAIN*/, 1, &k); 
			}
		}
	}
#endif

#ifdef VENDOR_ADC_ID_CHANNEL
	local_adc_channels [VENDOR_ADC_ID_CHANNEL].unit.uvperlsb = VENDOR_ADC_REFERENCE_VOLTAGE * 1000 / (1<<local_adc_channels [VENDOR_ADC_ID_CHANNEL].bits);	// 1,2 v, 1024 steps, 10 bit ADC
	local_adc_channels [VENDOR_ADC_ID_CHANNEL].unit.unit = adc_unit_volts;
#endif

#ifdef VENDOR_ADC_BOM_CHANNEL_1
	local_adc_channels [VENDOR_ADC_BOM_CHANNEL_1].unit.uvperlsb = 1;
	local_adc_channels [VENDOR_ADC_BOM_CHANNEL_1].unit.unit = adc_unit_bom;
#endif

#ifdef VENDOR_ADC_BOM_CHANNEL_2
	local_adc_channels [VENDOR_ADC_BOM_CHANNEL_2].unit.uvperlsb = 1;
	local_adc_channels [VENDOR_ADC_BOM_CHANNEL_2].unit.unit = adc_unit_bom;
#endif

#if 0
	if (adc_cal_channels) {
		OSHEAP_Delete (adc_cal_channels);
		adc_cal_channels = NULL;
	}
#endif
	memcpy (bcm59055_adc_chipset_api->adc_channels, local_adc_channels, sizeof (local_adc_channels));
}

static struct adc_channels_t * hal_adc_map_channel (struct adc_channels_t * channel_array, u8 channel)
{
	struct adc_channels_t * pchan = NULL;

	switch (channel) {
#ifdef VENDOR_ADC_VBAT_SCALE_CHANNEL
	case 0:	// temporary hack
	case HAL_ADC_CHIPSET_VBAT:
		pchan = &channel_array [VENDOR_ADC_VBAT_SCALE_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_BSI_CHANNEL
	// Main ADC (Used during phone calibration)
	case HAL_ADC_MAIN_CAL:
	// Fall through (this is also the BSI ADC)
	// Battery Size Identification 
	case HAL_ADC_CHIPSET_BSI:
		pchan = &channel_array [VENDOR_ADC_BSI_CHANNEL];
		break;
#endif
#ifdef VENDOR_ADC_BTEMP_CHANNEL
	// Battery TEMPerature
	case HAL_ADC_CHIPSET_BTEMP:
		pchan = &channel_array [VENDOR_ADC_BTEMP_CHANNEL];	
		break;
#endif
	// Power Amplifier TEMPerature
#ifdef VENDOR_ADC_PATEMP_CHANNEL
	case HAL_ADC_CHIPSET_PATEMP:
		pchan = &channel_array [VENDOR_ADC_PATEMP_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_X32TEMP_CHANNEL
	case HAL_ADC_CHIPSET_VCXOTEMP:
		pchan = &channel_array [VENDOR_ADC_X32TEMP_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_VCHAR_CHANNEL
	case HAL_ADC_CHIPSET_VCHAR:
		pchan = &channel_array [VENDOR_ADC_VCHAR_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_VBUS_CHANNEL
	case HAL_ADC_CHIPSET_VBUS:
		pchan = &channel_array [VENDOR_ADC_VBUS_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_IBAT_CHANNEL
	case HAL_ADC_CHIPSET_IBAT_FAST:
		pchan = &channel_array [VENDOR_ADC_IBAT_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_VBBAT_CHANNEL
	case HAL_ADC_CHIPSET_VBACK:
		pchan = &channel_array [VENDOR_ADC_VBBAT_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_ID_CHANNEL
	case HAL_ADC_CHIPSET_IDDET:
		pchan = &channel_array [VENDOR_ADC_ID_CHANNEL]; 
		break;
#endif
#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
	case HAL_ADC_CHIPSET_VENDOR_CH_0:
		pchan = &channel_array [VENDOR_ADC_BSI_COMP_CHANNEL_1];	
		pchan->unit.unit = adc_unit_bom;
		break;
	case HAL_ADC_CHIPSET_VENDOR_CH_1:
		pchan = &channel_array [VENDOR_ADC_BSI_COMP_CHANNEL_2];	
		pchan->unit.unit = adc_unit_bom;
		break;
	case HAL_ADC_CHIPSET_VENDOR_CH_2:
		pchan = &channel_array [VENDOR_ADC_ADC_COMP_CHANNEL_1];	
		pchan->unit.unit = adc_unit_bom;
		break;
	case HAL_ADC_CHIPSET_VENDOR_CH_3:
		pchan = &channel_array [VENDOR_ADC_ADC_COMP_CHANNEL_2];	
		pchan->unit.unit = adc_unit_bom;
		break;
#endif
	default:
		pchan = &channel_array [VENDOR_ADC_BOM_CHANNEL_1];	
		pchan->unit.unit = adc_unit_bom;
		break;
	}
	return pchan;
}


static u16 read_hk_adc (int physical_channel)
{
	int reading;
	if (physical_channel <= ADC_ALS_CHANNEL) {
		do {
			reading = bcm59055_saradc_read_data (physical_channel);
			if (reading < 0) {
			
			}
		} while (reading < 0);
		return reading;
	}
	else 
		return 0;
}

#if 0
static u16 read_rtm_adc (int physical_channel)
{
	int reading;
	if (physical_channel <= ADC_ALS_CHANNEL) {
		do {
			reading = bcm59055_saradc_read_data (physical_channel);
			if (reading < 0) {
			
			}
		} while (reading < 0);
		return reading;
	}
	else 
		return 0;
}
#endif


int bcm59055_hal_adc_raw_read (u8 channel, u32* value, HAL_ADC_CALLBACK adc_handler, void* context)
{
#if 0
 if ((channel >= 50) && (channel < 100) ) 
 {
	*value = glue_selftest_debug_executer(channel);
	return HAL_ADC_ERR_SUCCESS;
 }
 else 
#endif
 {
	struct adc_channels_t * pchan;
	u32 reading, overflow;
	int status = HAL_ADC_ERR_SUCCESS;
#if 0
	static u32 ibat_last_sample_time = 0;
	u32 current_time;
#endif

	if (!bcm59055_adc_chipset_api)
		return -ENOMEM;

	pchan = hal_adc_map_channel (bcm59055_adc_chipset_api->adc_channels, channel);
	GLUE_DBG (("hal_adc_raw_read: Reading Nokia channel %x, pchan %x", channel, pchan));

	if (!pchan) {
		return HAL_ADC_ERR_NOTSUP;
	}
	if (pchan->locked) {
		*value = pchan->lockvalue;
		return HAL_ADC_ERR_SUCCESS;
	}
#ifdef VENDOR_ADC_IBAT_CHANNEL	
	if (pchan->unit.unit == adc_unit_ibat_k) {
	    // Fuel gauge...
	    // Get the current time
#if 0
	    current_time = TIMER_GetValue ();
    	    GLUE_DBG (("hal_adc_raw_read: ibat: last_sample_time %d, current_time %d", ibat_last_sample_time, current_time));
	    if (ibat_last_sample_time) {
	        if ((current_time - ibat_last_sample_time) < FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS) {
		    OSTASK_Sleep (FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS - (current_time - ibat_last_sample_time));
			GLUE_DBG (("hal_adc_raw_read: Waittime %d", FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS - (current_time - ibat_last_sample_time) ));
		    current_time = TIMER_GetValue ();
		    GLUE_DBG (("hal_adc_raw_read: Waiting for next sample to be available, new current_time %d", current_time));
	        }
	    }
	    ibat_last_sample_time = current_time; 
	    // Read sampleB register...
	    {
	        i16 ibat;
		PMU_FGSMPL_data_st_t data;
		
		data.type = PMU_FG_TYPE_SMPLB;
		data.smpl = &ibat;
	        fg_reading_in_progress = true;

		/*HAL_EM_PMU_Ctrl (EM_PMU_ACTION_READ_FGSMPL, &data, NULL);*/
    	        down_interruptible (bcm59055_adc_chipset_api->fg_read_sem);
	    
	        ibat += 2;	/* For rounding... */
	        ibat >>= 2;
	        *value = ibat;
	        fg_reading_in_progress = false;
		GLUE_DBG (("hal_adc_raw_read: IBAT reading returns %d", ibat));
                return HAL_ADC_ERR_SUCCESS;
	    }
#endif	    
	}
#endif
	
	overflow = (1 << pchan->bits) - 1;

	reading = read_hk_adc (pchan->select);
	if (reading == overflow) {
		status = HAL_ADC_ERR_OVERFLOW;
	}
	else
		status = HAL_ADC_ERR_SUCCESS;
	*value = reading;

	if (adc_handler) {
		adc_handler ((u32)status, *value, context);
		GLUE_DBG (("hal_adc_raw_read: returned from callback"));
	}	
	GLUE_DBG (("hal_adc_raw_read: Returning value %x, status %d, callback %x, context %x", *value, status, adc_handler, context));
	return status;
 }
}
EXPORT_SYMBOL(bcm59055_hal_adc_raw_read);

static int bsearch_descending_int_table(int* table, int sz, int value)
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
		// Find best approximation, not just the next lower value in table
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

int bcm59055_hal_adc_unit_convert (u8 channel, u32 value)
{
	struct adc_channels_t *chan;
	int vread, temp_to_return=298, ibat_to_return, ibat, ibat_cal;
	u32 u_iread, u_rread;
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
	u32 stored_reference_reading = 0;
#endif
	double vread_f, vpullup, iread, rread, rpullup;
#if 0
	double rr0, lnrr0, temperature, ft;
#endif
#ifdef DEBUGOUT_ADC_CHIPSET_API
	int vread_32, intc, iread_32, rread_32, rr0_32, lnrr0_32;
#endif

	if (!bcm59055_adc_chipset_api) {
		return -ENOMEM;
	}
#if 0
	if (adc_cal_channels) {
		// use cal data
		chan = hal_adc_map_channel (adc_cal_channels, channel);
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
		stored_reference_reading = adc_cal_channels [8].unit.data.ohms.reference_reading;
#endif
	}
	else
#endif
	{
		// Use normal data
		chan = hal_adc_map_channel (bcm59055_adc_chipset_api->adc_channels, channel);
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
		stored_reference_reading = bcm59055_adc_chipset_api->adc_channels [8].unit.data.ohms.reference_reading;
#endif
	}
	if (!chan) {
		return value;
	}
	if (chan->locked) {
		return chan->lockvalue;
	}

	vread = value * chan->unit.uvperlsb;
	vread += chan->unit.offset;

	GLUE_DBG (("hal_adc_unit_convert: value %d, unit %d, uv %d, vread = %d", value, chan->unit.unit, chan->unit.uvperlsb, vread));
	switch (chan->unit.unit) {
	case adc_unit_volts:
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
		{
			if (stored_reference_reading && value) {
				u16 reference_reading = 0;
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += 2;			// Ensure correct rounding
				reference_reading >>=2;			// Divide by four
				value *= stored_reference_reading;
				value /= reference_reading;
				vread = value * chan->unit.uvperlsb + chan->unit.offset;	
				GLUE_DBG (("hal_adc_unit_convert: Temperature compensated value %d, reference_reading = %d, vread = %d", 
						value, chan->unit.uvperlsb, reference_reading, vread));
			}	
		}		
#endif
		// should return in millivolts
		if ((vread % 1000) > 500) {
			vread += 500;		// round up... 
		}
		GLUE_DBG (("hal_adc_unit_convert: Millivolts %d", (vread/1000)));
		return (int) (vread/1000);

	case adc_unit_amps:
		iread = vread/chan->unit.data.amps.rshunt; 	// uV / ohms = uA
		return (int) (iread/1000);

	case adc_unit_ohms:
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
		{
			if (chan->unit.data.ohms.reference_reading) {
				u16 reference_reading = 0;
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += 2;			// Ensure correct rounding
				reference_reading >>=2;			// Divide by four
				value *= chan->unit.data.ohms.reference_reading;
				value /= reference_reading;
				vread = value * chan->unit.uvperlsb + chan->unit.offset;	
				GLUE_DBG (("hal_adc_unit_convert: Temperature compensated value %d, reference_reading = %d, vread = %d", 
						value, chan->unit.uvperlsb, reference_reading, vread));
			}	
		}		
#endif
#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
		{
		    	// Read the two ADC channels
		    	// calculate gain and offset
		    	//
		    	u16 read0, read1, read2, i;
		    	int gain;
		    	int offset;

			gain = chan->unit.uvperlsb;
			offset = chan->unit.offset;
			read1 = 0;
			for (i=0; i<BSI_COMP_SAMPLES;) {
				read0 = read_adc (VENDOR_ADC_BSI_COMP_CHANNEL_1);
				if (read0 != 0x3ff) {
					read1 += read0;
					i++;
				}
			}
			read1 += BSI_COMP_SAMPLES/2;	// add samples/2 for rounding
			read1 /= BSI_COMP_SAMPLES;	// Divide by the number of samples

		    	read2 = 0;
			for (i=0; i<BSI_COMP_SAMPLES;) {
			    	read0 = read_adc (VENDOR_ADC_BSI_COMP_CHANNEL_2);
				if (read0 != 0x3ff) {
					read2 += read0;
					i++;
				}
			}
			read2 +=BSI_COMP_SAMPLES/2;	// For rounding
			read2 /=BSI_COMP_SAMPLES;	// Divide to get average
		   
		    	// Calculate uvperbit and offset
		    	if (read1 != read2) {
		            gain = ((VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE-VENDOR_ADC_BSI_COMP_LOW_VOLTAGE)*1000) / (read2 - read1);
		            offset = VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE*1000 - (read2*gain);
		    	}
			vread = value * gain + offset;
			GLUE_DBG (("hal_adc_unit_convert: Value %d, read1 %d, read2 %d, gain %d, offset %d, new vread %d, vmax %d",
		    		value, read1, read2, gain, offset, vread,chan->unit.vmax));
 
		}
#endif
		//vread_f = vread;
		if(channel == 0xfe) {
			return vread;
		}
		// Read current consumption
		bcm59055_hal_adc_raw_read (HAL_ADC_CHIPSET_IBAT_FAST, (u32*)&ibat, NULL, NULL);
		ibat_cal = bcm59055_hal_adc_unit_convert (HAL_ADC_CHIPSET_IBAT_FAST, ibat);
		
		vread += ((ibat_cal + 20)/40)*1000;	// compensate reading with the 25 mOhm in the FG and battery terminal
		GLUE_DBG (("hal_adc_unit_convert: res - ibat %d, new vread %d", ibat_cal, vread));
		//vpullup = chan->unit.vmax - vread;
		u_iread = ((chan->unit.vmax - vread)*100) / (chan->unit.data.ohms.rpullup);
		if (!u_iread) {
			// No resistor is present. 
			return -1;
		}
		// u_iread is now in 10nA
		vread += u_iread/2;		// For rounding...
		u_rread = vread/u_iread;	// Hecto ohm...
		
		/*rread = vread_f/iread;					// Unit : Ohms
		GLUE_DBG (("adc_unit_convert: res - (vpullup, iread, rread)"));
		GLUE_DBG (("adc_unit_convert: res - (%d, %f, %f)", vpullup, iread, rread));
		rread/= 100;			// Unit : hecto-ohms.
		if (fmod (rread, 1.0) >= 0.5) {
			rread += 1;
		}*/
		GLUE_DBG (("hal_adc_unit_convert: res - iread %d, rread %d", u_iread, u_rread));
		
		return u_rread; 				

	case adc_unit_kelvin_down:					// Temperature where NTC is connected to ground
#if 0
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
		{
			if (stored_reference_reading) {
				u16 reference_reading = 0;
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += 2;			// Ensure correct rounding
				reference_reading >>=2;			// Divide by four
				value *= stored_reference_reading;
				value /= reference_reading;
				vread = value * chan->unit.uvperlsb + chan->unit.offset;	
				GLUE_DBG (("hal_adc_unit_convert: Temperature compensated value %d, reference_reading = %d, vread = %d", 
						value, chan->unit.uvperlsb, reference_reading, vread));
			}	
		}		
#endif
		vread_f = 0;
		vread_f += vread;
		vpullup = chan->unit.vmax - vread;
		rpullup = chan->unit.data.kelvin.rpullup;
		iread = vpullup/rpullup;
		if (chan->unit.data.kelvin.rpar) {
			iread -= vread_f/chan->unit.data.kelvin.rpar;
		}

		if (!iread) {
			// No resistor is present. 
			return -1;
		}
		rread = vread_f/iread;	// Unit : Ohms
		rread -= chan->unit.data.kelvin.rseries;

		ft = chan->unit.data.kelvin.r0;
		rr0 = rread/ft;
		lnrr0 = log (rr0);
		if (rread < chan->unit.data.kelvin.r0) {
			ft = chan->unit.data.kelvin.b;	// Constant
		}
		else {
			ft = chan->unit.data.kelvin.b - 170;	// Constant
		}
		lnrr0 /= ft;
		lnrr0 += THERMISTOR_T0;	// 1/298,15 == default Temperature for NTC
		temperature = 1/lnrr0;
		temp_to_return = temperature;

#ifdef DEBUGOUT_ADC_CHIPSET_API
		vread_32 = vread_f;
		iread_32 = (vpullup/rpullup)*1000;
		intc = iread * 1000;
		rread_32 = rread;
		rr0_32 = rr0 * 1000, 
		lnrr0_32 = lnrr0 * 1000000;
		GLUE_DBG (("hal_adc_unit_convert: temperature: vread_32, iread_32, intc, rread_32, rr0_32, lnrr0_32"));
		GLUE_DBG (("hal_adc_unit_convert: temperature: %d, %d, %d, %d, %d, %d", vread_32, iread_32, intc, rread_32, rr0_32, lnrr0_32));

#endif
#endif
		return temp_to_return;

#ifdef VENDOR_ADC_KELVIN_DOWN_TABLE
	case adc_unit_kelvin_down_table:					// Temperature where NTC is connected to ground, using Nokia supplied lookup table.
#ifdef VENDOR_ADC_TEMPERATURE_COMP_CHANNEL
		{
			if (stored_reference_reading) {
				u16 reference_reading = 0;
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += read_adc (VENDOR_ADC_TEMPERATURE_COMP_CHANNEL);	
				reference_reading += 2;			// Ensure correct rounding
				reference_reading >>=2;			// Divide by four
				value *= stored_reference_reading;
				value /= reference_reading;
				vread = value * chan->unit.uvperlsb + chan->unit.offset;	
				GLUE_DBG (("hal_adc_unit_convert: Temperature compensated value %d, reference_reading = %d, vread = %d", 
						value, chan->unit.uvperlsb, reference_reading, vread));
			}	
		}		
#endif
#ifdef VENDOR_ADC_BSI_COMP_CHANNELS
		{
		    	// Read the two ADC channels
		    	// calculate gain and offset
		    	//
		 	u16 read0, read1, read2, i;
		    	int gain;
		    	int offset;

			gain = chan->unit.uvperlsb;
			offset = chan->unit.offset;
			read1 = 0;
			for (i=0; i<BSI_COMP_SAMPLES;) {
				read0 = read_adc (VENDOR_ADC_BSI_COMP_CHANNEL_1);
				if (read0 != 0x3ff) {
					read1 += read0;
					i++;
				}
			}
			read1 += BSI_COMP_SAMPLES/2;	// add samples/2 for rounding
			read1 /= BSI_COMP_SAMPLES;	// Divide by the number of samples

		    	read2 = 0;
			for (i=0; i<BSI_COMP_SAMPLES;) {
			    	read0 = read_adc (VENDOR_ADC_BSI_COMP_CHANNEL_2);
				if (read0 != 0x3ff) {
					read2 += read0;
					i++;
				}
			}
			read2 +=BSI_COMP_SAMPLES/2;	// For rounding
			read2 /=BSI_COMP_SAMPLES;	// Divide to get average
		   
		    	// Calculate uvperbit and offset
		    	if (read1 != read2) {
		            gain = ((VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE-VENDOR_ADC_BSI_COMP_LOW_VOLTAGE)*1000) / (read2 - read1);
		            offset = VENDOR_ADC_BSI_COMP_HIGH_VOLTAGE*1000 - (read2*gain);
		    	}
			vread = value * gain + offset;
			GLUE_DBG (("hal_adc_unit_convert: Value %d, read1 %d, read2 %d, gain %d, offset %d, new vread %d, vmax %d",
		    		value, read1, read2, gain, offset, vread,chan->unit.vmax));
 
		}
#endif

		vread_f = 0;
		vread_f += vread;
		vpullup = chan->unit.vmax - vread;
		rpullup = chan->unit.data.kelvin.rpullup;
		iread = vpullup/rpullup;
		if (chan->unit.data.kelvin.rpar) {
			iread -= vread_f/chan->unit.data.kelvin.rpar;
		}

		if (!iread) {
			// No resistor is present. 
			return -1;
		}
		rread = vread_f/iread;	// Unit : Ohms
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
		GLUE_DBG (("hal_adc_unit_convert: temperature: vread_32, iread_32, intc, rread_32"));
		GLUE_DBG (("hal_adc_unit_convert: temperature: %d, %d, %d, %d", vread_32, iread_32, intc, rread_32));

#endif

		return temp_to_return;
#endif // VENDOR_ADC_KELVIN_DOWN_TABLE

	case adc_unit_kelvin_up:					// Temperature where NTC is connected to supply voltage
#if 0
		vread_f = vread;
		vpullup = bcm59055_adc_chipset_api->adc_channels [channel].unit.vmax - vread;
		iread = vpullup/chan->unit.data.kelvin.rpullup;
		if (!iread) {
			// No resistor is present. 
			return -1;
		}
		rread = vread_f/iread;	// Unit : Ohms
		rread -= chan->unit.data.kelvin.rseries;

		rr0 = rread/chan->unit.data.kelvin.r0;
		lnrr0 = log (rr0);
		lnrr0 /= chan->unit.data.kelvin.b;	// Constant
		lnrr0 += THERMISTOR_T0;	// 1/298,15 == default Temperature for NTC
		temperature = 1/lnrr0;
		temp_to_return = temperature;
		GLUE_DBG (("hal_adc_unit_convert: temperature (vpullup, iread, rread, rr0, temperature)"));
		GLUE_DBG (("hal_adc_unit_convert: temperature (%d, %3.8f, %f, %1.3f, %3.2f)",
					 vpullup, iread, rread, rr0, temp_to_return));
#endif
        	return temp_to_return;
	case adc_unit_bom:
        	return 0; /*hal_adc_get_bom_id ();*/
	case adc_unit_ibat_k:
        {
		int modifier = 0, reading = value;
		if (chan->unit.data.ibat_k.k) {
			modifier = (chan->unit.data.ibat_k.k > 128) ? 768 + chan->unit.data.ibat_k.k : 1024 + chan->unit.data.ibat_k.k;
			ibat_to_return = ((reading * modifier) / 1024) - chan->unit.offset;
			GLUE_DBG (("hal_adc_unit_convert: ibat_to_return before negation and 976 modification %d", ibat_to_return));
		}
		else
			ibat_to_return = value;
		ibat_to_return = -((ibat_to_return*976)/1000);
		GLUE_DBG (("hal_adc_unit_convert: value %d, modifier %d Offset %d, ibat %d", value, modifier, chan->unit.offset, ibat_to_return));
	        return ibat_to_return;
	}
	case adc_unit_unknown:
		return 0;
    }
    // should be unreachable...
    return 0;

}
EXPORT_SYMBOL(bcm59055_hal_adc_unit_convert);

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

static int bcm59055_adc_chipset_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	/* TODO: */
	return 0;
}

#define MAX_USER_INPUT_LEN      100

static ssize_t bcm59055_adc_chipset_write(struct file *file, const char __user *buffer,
	size_t len, loff_t *offset)
{
	/*struct bcm59055_adc_chipset_api *data = file->private_data;*/
	/*struct adc_debug dbg;*/
	char cmd[MAX_USER_INPUT_LEN];
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

	if (strcmp (cmd, "VBAT_UNIT") == 0) {
		bcm59055_hal_adc_raw_read (HAL_ADC_CHIPSET_VBAT, &adc_raw, NULL, NULL);
		adc_unit = bcm59055_hal_adc_unit_convert (HAL_ADC_CHIPSET_VBAT, adc_raw);
		printk(KERN_INFO "%s: VBAT: Raw %d, unit %d mV\n", __func__, adc_raw, adc_unit);
	
	}
	else if (strcmp(cmd, "BSI_UNIT") == 0) {
		bcm59055_hal_adc_raw_read (HAL_ADC_CHIPSET_BSI, &adc_raw, NULL, NULL);
		adc_unit = bcm59055_hal_adc_unit_convert (HAL_ADC_CHIPSET_BSI, adc_raw);
		printk(KERN_INFO "%s: BSI: Raw %d, unit %d HOhm\n", __func__, adc_raw, adc_unit);
	}
	else if (strcmp(cmd, "BTEMP_UNIT") == 0) {
		bcm59055_hal_adc_raw_read (HAL_ADC_CHIPSET_BTEMP, &adc_raw, NULL, NULL);
		adc_unit = bcm59055_hal_adc_unit_convert (HAL_ADC_CHIPSET_BTEMP, adc_raw);
		printk(KERN_INFO "%s: BTEMP: Raw %d, unit %d K\n", __func__, adc_raw, adc_unit);
	}


	*offset += len;
	return len;
}

static const struct file_operations bcm59055_adc_chipset_ops = {
	.open = bcm59055_adc_chipset_open,
	.ioctl = bcm59055_adc_chipset_ioctl,
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
	priv_data = kzalloc(sizeof(struct bcm59055_adc_chipset_api ), GFP_KERNEL);
	if (!priv_data) {
		pr_info("%s: Memory can not be allocated!!\n",
			__func__);
		return -ENOMEM;
	}
	priv_data->bcm59055 = bcm59055;
	bcm59055_adc_chipset_api = priv_data;

	hal_adc_init_channels (false); /* False for now - true when data can be retrieved */


	sema_init (&priv_data->fg_read_sem, 0);
	sema_init (&priv_data->rtm_read_sem, 0);

	/* Make sure fuel gauge is active. So far we don't care about return value */
	bcm59055_fg_enable ();
	bcm59055_fg_offset_cal (false);
	/* When we get calibraion data, call driver to set the value */

	proc_create_data("adc_chipset_api", S_IRWXUGO, NULL, &bcm59055_adc_chipset_ops, bcm59055_adc_chipset_api);

	return 0;
}

static int __devexit bcm59055_adc_chipset_api_remove(struct platform_device *pdev)
{
	struct bcm59055_adc_chipset_api_driver *priv_data = platform_get_drvdata(pdev);
	bcm59055_fg_disable ();
	
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




/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/misc/bcmpu-fuelgauge.c
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
*  bcmpu-adc_chipset_api.c
*
*  PURPOSE:
*
*     This implements the driver for the Fuel Gauge on bcmpu PMU chip.
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
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/stringify.h>
#include <linux/proc_fs.h>
#include <linux/csapi_adc.h>
#include <linux/mfd/bcmpmu.h>

/*#define pr_debug(text, ...)  printk(KERN_INFO text"\n", ## __VA_ARGS__)*/
/*#define pr_info(text, ...)  printk(KERN_INFO text"\n", ## __VA_ARGS__)*/

/*#define CSADCAPI_DEBUG_ENABLED*/
#ifdef CSADCAPI_DEBUG_ENABLED
 #define csapi_pr_debug(text, ...)  printk(KERN_INFO text"\n", ## __VA_ARGS__)
 #define csapi_pr_info(text, ...)  printk(KERN_INFO text"\n", ## __VA_ARGS__)
 #define GLUE_DBG(text, ...)  printk(KERN_INFO text"\n", ## __VA_ARGS__)
#else
 #define csapi_pr_info pr_info
 #define csapi_pr_debug pr_debug
 #define GLUE_DBG(text, ...)
#endif

#define FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS 9

typedef s32(*CSAPI_ADC_HW_CAL_CALC_FUNC) (struct csapi_cal_req *Data);

#ifdef VENDOR_ADC_CAL_FUNCTION
CSAPI_ADC_HW_CAL_CALC_FUNC hal_adc_hw_cal_calc_func = VENDOR_ADC_CAL_FUNCTION;
#else
CSAPI_ADC_HW_CAL_CALC_FUNC hal_adc_hw_cal_calc_func /*= NULL*/ ;
#endif

#define CSAPI_ADC_ERR_SUCCESS 0

struct adc_channels_t {
	enum bcmpmu_adc_sig sig;
	u8 bits;
	u8 locked;
	u32 lockvalue;
	u32 cal_id;
};

static struct adc_channels_t adc_channels[CSAPI_ADC_VENDOR_CH_15 + 1] = {
	[CSAPI_ADC_VBAT] = {.sig = PMU_ADC_VMBATT, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VCHAR] = {.sig = PMU_ADC_VWALL, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_ICHAR] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_BSI] = {.sig = PMU_ADC_BSI, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_BTEMP] = {.sig = PMU_ADC_NTC, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_PATEMP] = {.sig = PMU_ADC_PATEMP, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_IBAT_AVG] = {.sig = PMU_ADC_FG_CURRSMPL, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_IBAT] = {.sig = PMU_ADC_FG_RAW, .bits = 16,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_IBAT_CC] = {.sig = PMU_ADC_FG_RAW, .bits = 16,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VBACK] = {.sig = PMU_ADC_VBBATT, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VBUS] = {.sig = PMU_ADC_VBUS, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_IDIN] = {.sig = PMU_ADC_ID, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_CTEMP] = {.sig = PMU_ADC_32KTEMP, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_ALS] = {.sig = PMU_ADC_ALS, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_MAIN_CAL] = {.sig = PMU_ADC_BSI, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_0] = {.sig = PMU_ADC_BSI_CAL_LO, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_1] = {.sig = PMU_ADC_BSI_CAL_HI, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_2] = {.sig = PMU_ADC_NTC_CAL_LO, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_3] = {.sig = PMU_ADC_NTC_CAL_HI, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_4] = {.sig = PMU_ADC_BOM, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_5] = {.sig = PMU_ADC_FG_VMBATT, .bits = 10,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_6] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_7] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_8] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_9] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_10] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_11] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_12] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_13] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_14] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
	[CSAPI_ADC_VENDOR_CH_15] = {.sig = PMU_ADC_MAX, .bits = 0,
		.locked = 0, .lockvalue = 0, .cal_id = 0},
};

struct bcmpmu_adc_chipset_api {
	struct bcmpmu *bcmpmu;
	struct bcmpmu_adc_setting *adc_setting;
	struct adc_channels_t *adc_channels;
};

static struct bcmpmu_adc_chipset_api *bcmpmu_adc_chipset_api;	/* = NULL; */

static int columb_counter;	/* = 0 */

static u16 read_hk_adc(int physical_channel)
{
	int status;
	struct bcmpmu_adc_req ar;

	if (physical_channel <= PMU_ADC_ALS) {
		memset(&ar, 0, sizeof(ar));
		ar.sig = physical_channel;
		ar.tm = PMU_ADC_TM_HK;
		ar.flags = PMU_ADC_RAW_ONLY;
		do {
			status =
			    bcmpmu_adc_chipset_api->bcmpmu->
			    adc_req(bcmpmu_adc_chipset_api->bcmpmu, &ar);
		} while (status < 0);
		csapi_pr_debug("%s: raw %d, cal %d, cnv %d", __func__,
			       ar.raw, ar.cal,
			 ar.cnv);
		return ar.raw;
	} else
		return 0;
}

static int read_rtm_adc(int physical_channel)
{
	int status;
	struct bcmpmu_adc_req ar;

	memset(&ar, 0, sizeof(ar));
	ar.sig = physical_channel;
	ar.tm = PMU_ADC_TM_RTM_SW;
	ar.flags = PMU_ADC_RAW_ONLY;

	do {
		status =
		    bcmpmu_adc_chipset_api->bcmpmu->
		    adc_req(bcmpmu_adc_chipset_api->bcmpmu, &ar);
		csapi_pr_debug("%s: Status %d, reading %d", __func__, status,
			       ar.raw);
		if (status < 0)
			msleep(20);
	} while (status < 0);

	return ar.raw;
}

#define SIGNBIT 0x04000000
#define VALMASK 0x03ffffff

/* Update_columb */
/* Returns average current consumption */
/* updates the static variable columb with the values from the fuel gauge */
static int update_columb(void)
{
	int ret, dontcaredata;
	struct bcmpmu_fg *pfg =
	    (struct bcmpmu_fg *)bcmpmu_adc_chipset_api->bcmpmu->fginfo;

	/* Get the value from the fuel gauge */
	ret =
	    bcmpmu_adc_chipset_api->bcmpmu->fg_acc_mas(bcmpmu_adc_chipset_api->
						       bcmpmu, &dontcaredata);

	columb_counter = pfg->fg_columb_cnt;

	csapi_pr_debug
	    ("%s: columb_counter %d, pfg->fg_columb_cnt %d, pfg->ibat_avg %d",
	     __func__, columb_counter, pfg->fg_columb_cnt, pfg->fg_ibat_avg);

	/* Return new average current */

	return pfg->fg_ibat_avg;
}

int csapi_adc_raw_read(struct csapi_cli *cli,
		       u8 cha, u32 *val, csapi_cb cb, void *ptr)
{
	struct adc_channels_t *pchan;
	u32 reading, overflow, current_time;
	int status = CSAPI_ADC_ERR_SUCCESS;
	static u32 ibat_last_sample_time /* = 0 */ ;
	struct timespec ts_current_time;

	if (!bcmpmu_adc_chipset_api)
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

	pchan = &bcmpmu_adc_chipset_api->adc_channels[cha];
	GLUE_DBG("csapi_adc_raw_read: Reading CSAPI channel %x, pchan %p", cha,
		  pchan);

	if (pchan->sig == PMU_ADC_MAX)
		return -ENODEV;

	if (cha == CSAPI_ADC_IBAT) {
		/* Fuel gauge... */
		/* Get the current time */

		ts_current_time = CURRENT_TIME;
		current_time = (ts_current_time.tv_sec * 1000) +
			(ts_current_time.tv_nsec / 1000000); /* milliseconds */
		csapi_pr_debug
		    ("csapi_adc_raw_read: ibat: "
		     "last_sample_time %d, current_time %d",
		     ibat_last_sample_time, current_time);
		if (ibat_last_sample_time) {
			if ((current_time - ibat_last_sample_time) <
			    FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS) {
				udelay((FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS
					- (current_time -
					   ibat_last_sample_time)) * 1000);
				GLUE_DBG("hal_adc_raw_read: Waittime %d",
					FUELGAUGE_MINIMUM_TIME_BETWEEN_ADC_READS
					- (current_time -
					   ibat_last_sample_time));
				ts_current_time = CURRENT_TIME;
				/* current_time in milliseconds */
				current_time = (ts_current_time.tv_sec * 1000) +
					 (ts_current_time.tv_nsec / 1000000);
				GLUE_DBG("csapi_adc_raw_read: Waiting for next "
					  "sample to be available, "
					  "new current_time %d", current_time);
			}
		}
		ibat_last_sample_time = current_time;

		/* Read sampleB register... */
		{
			struct bcmpmu_adc_req ar;
			int ibat;

			ar.sig = PMU_ADC_FG_RAW;
			ar.tm = PMU_ADC_TM_HK;
			ar.flags = PMU_ADC_RAW_ONLY;

			status =
			    bcmpmu_adc_chipset_api->bcmpmu->
			    adc_req(bcmpmu_adc_chipset_api->bcmpmu, &ar);
			ibat = ar.raw;
			if (ar.raw & 0x8000)	/* negative offset */
				ibat |= 0xffff0000;
			*val = ibat;
			csapi_pr_debug("csapi_adc_raw_read: IBAT reading"
				       " returns %d",
				ibat);
			return CSAPI_ADC_ERR_SUCCESS;
		}
	}

	overflow = (1 << pchan->bits) - 1;

	switch (pchan->sig) {
	case PMU_ADC_VMBATT:
	case PMU_ADC_VBBATT:
	case PMU_ADC_VWALL:
	case PMU_ADC_VBUS:
	case PMU_ADC_ID:
	case PMU_ADC_NTC:
	case PMU_ADC_BOM:
	case PMU_ADC_32KTEMP:
	case PMU_ADC_PATEMP:
	case PMU_ADC_ALS:
		reading = read_hk_adc(pchan->sig);
		break;
	case PMU_ADC_BSI:
	case PMU_ADC_BSI_CAL_LO:
	case PMU_ADC_NTC_CAL_LO:
	case PMU_ADC_NTC_CAL_HI:
	case PMU_ADC_BSI_CAL_HI:
	default:
		reading = read_rtm_adc(pchan->sig);
		break;
	}

	if (reading == overflow)
		status = -ERANGE;
	else
		status = CSAPI_ADC_ERR_SUCCESS;
	*val = reading;

	if (cb) {
		cb(cha, *val, status, ptr);
		GLUE_DBG("csapi_adc_raw_read: returned from callback");
	}
	GLUE_DBG("csapi_adc_raw_read: Returning value %x, status %d, "
		  "context %p", *val, status, ptr);
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
		csapi_pr_debug("%s: raw %d, *val %d", __func__, raw, *val);
		if (cb) {
			int status = CSAPI_ADC_ERR_SUCCESS;
			cb(cha, *val, status, ptr);
			GLUE_DBG("csapi_adc_raw_read: returned from callback");
		}
		return 0;
	}
	return res;
}
EXPORT_SYMBOL(csapi_adc_unit_read);

int csapi_adc_unit_convert(struct csapi_cli *cli, u8 cha, u32 raw)
{
	struct adc_channels_t *chan;
	struct bcmpmu_adc_req req;

	csapi_pr_debug("%s: raw %d", __func__, raw);

	if (!bcmpmu_adc_chipset_api)
		return -ENOMEM;

	/* Special CSAPI IBAT channels */
	if (cha == CSAPI_ADC_IBAT_AVG || cha == CSAPI_ADC_IBAT_CC)
		return raw;

	chan = &bcmpmu_adc_chipset_api->adc_channels[cha];
	if (chan->sig == PMU_ADC_MAX)
		return -ENODEV;

	if (chan->locked)
		return chan->lockvalue;

	req.sig = chan->sig;
	req.raw = raw;
	req.cal = raw;
	req.flags = PMU_ADC_UNIT_ONLY;

	bcmpmu_adc_chipset_api->bcmpmu->adc_req(bcmpmu_adc_chipset_api->bcmpmu,
						&req);
	return req.cnv;
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

	chan = &bcmpmu_adc_chipset_api->adc_channels[cha];
	if (!chan) {
		GLUE_DBG("%s returns Not supported for channel %d", __func__,
			  cha);
		return -ENODEV;
	}
	csapi_pr_debug("%s: pchan 0x%x", __func__, (u32) chan);

	chan->locked = true;
	chan->lockvalue = val;
	return 0;
}
EXPORT_SYMBOL(csapi_cal_unit_convert_lock);

int csapi_cal_unit_convert_unlock(struct csapi_cli *cli, u8 cha)
{
	struct adc_channels_t *chan;

	chan = &bcmpmu_adc_chipset_api->adc_channels[cha];
	if (!chan) {
		GLUE_DBG("%s returns Not supported for channel %d", __func__,
			  cha);
		return -ENODEV;
	}
	csapi_pr_debug(KERN_INFO "%s: pchan 0x%x", __func__, (u32) chan);

	chan->locked = false;
	chan->lockvalue = 0;
	return 0;

}
EXPORT_SYMBOL(csapi_cal_unit_convert_unlock);

int csapi_cal_data_get(struct csapi_cli *cli,
		       u8 cha, u32 *id, u32 * p1, u32 * p2, u32 * p3)
{
	struct adc_channels_t *pchan;
	struct bcmpmu_adc_unit data;

	pchan = &bcmpmu_adc_chipset_api->adc_channels[cha];

	if (pchan->sig == PMU_ADC_MAX) {
		GLUE_DBG("csapi_adc_raw_read returns Not supported "
			  "for channel %d", cha);
		return -ENODEV;
	}
	csapi_pr_debug("%s: pchan 0x%x", __func__, (u32) pchan);
	*p1 = 0;
	*p2 = 0;
	*p3 = 0;
	*id = pchan->cal_id;

	bcmpmu_adc_chipset_api->bcmpmu->unit_get(bcmpmu_adc_chipset_api->bcmpmu,
						 pchan->sig, &data);

	switch (cha) {
	case CSAPI_ADC_VBAT:
	case CSAPI_ADC_VBACK:
	case CSAPI_ADC_VCHAR:
	case CSAPI_ADC_VBUS:
	case CSAPI_ADC_IDIN:
		*p1 = data.vstep;	/* uV per step */
		*p2 = data.voffset;	/* Offset in uV */
		break;
	case CSAPI_ADC_BTEMP:
	case CSAPI_ADC_PATEMP:
	case CSAPI_ADC_CTEMP:
		*p1 = data.vstep;	/* uV per step */
		*p2 = data.voffset;	/* Offset in uV */
		*p3 = data.vmax;	/* reference voltage, uV */
		break;
	case CSAPI_ADC_BSI:
		*p1 = data.vstep;	/* uV per step */
		*p2 = data.voffset;	/* Offset in uV */
		*p3 = data.vmax;	/* reference voltage, uV */
		break;
	case CSAPI_ADC_IBAT:
	case CSAPI_ADC_IBAT_AVG:
	case CSAPI_ADC_IBAT_CC:
		*p1 = data.fg_k;
		*p2 = data.voffset;
		break;
	default:
		break;
	}
	GLUE_DBG("csapi_adc_raw_read for channel %d: Id %d, values %d, %d, %d",
		  cha, *id, *p1, *p2, *p3);
	return CSAPI_ADC_ERR_SUCCESS;
}

int csapi_cal_data_set(struct csapi_cli *cli,
		       u8 cha, u32 id, u32 p1, u32 p2, u32 p3)
{
	struct adc_channels_t *pchan;
	struct bcmpmu_adc_unit data;
	/* Do we have a copy of the structure?
	   If not, use primary structure as source. */
	pchan = &bcmpmu_adc_chipset_api->adc_channels[cha];

	if (pchan->sig == PMU_ADC_MAX) {
		csapi_pr_info("csapi_cal_data_set returns Not supported for "
			      "channel %d", cha);
		return -ENODEV;
	}

	pchan->cal_id = id;

	bcmpmu_adc_chipset_api->bcmpmu->unit_get(bcmpmu_adc_chipset_api->bcmpmu,
						 pchan->sig, &data);

	switch (cha) {
	case CSAPI_ADC_VBAT:
	case CSAPI_ADC_VBACK:
	case CSAPI_ADC_VCHAR:
	case CSAPI_ADC_VBUS:
	case CSAPI_ADC_IDIN:
		data.vstep = p1;	/* uV per step */
		data.voffset = p2;	/* Offset in uV */
		break;
	case CSAPI_ADC_BTEMP:
	case CSAPI_ADC_PATEMP:
	case CSAPI_ADC_CTEMP:
		data.vstep = p1;	/* uV per step */
		data.voffset = p2;	/* Offset in uV */
		data.vmax = p3;	/* reference voltage, uV */
		break;
	case CSAPI_ADC_BSI:
		data.vstep = p1;	/* uV per step */
		data.voffset = p2;	/* Offset in uV */
		data.vmax = p3;	/* Reference voltage */
		break;
	case CSAPI_ADC_IBAT:
	case CSAPI_ADC_IBAT_AVG:
	case CSAPI_ADC_IBAT_CC:
		data.fg_k = p1;
		bcmpmu_adc_chipset_api->bcmpmu->
		    fg_trim_write(bcmpmu_adc_chipset_api->bcmpmu, data.fg_k);
		data.voffset = p2;
		/* shouldn't we get it from the PMU it self? */
		break;
	default:
		break;

	}
	bcmpmu_adc_chipset_api->bcmpmu->unit_set(bcmpmu_adc_chipset_api->bcmpmu,
						 pchan->sig, &data);
	csapi_pr_info("csapi_cal_data_set for channel %d: Id %d, "
		      "values %d, %d, %d", cha, id, p1, p2, p3);
	return CSAPI_ADC_ERR_SUCCESS;
}

s32 hal_adc_cal_calc_dalton(struct csapi_cal_req *Data)
{
	struct adc_channels_t *pchan = NULL;
	u32 vapplied_uv;
	u8 element;

	u32 vbat_val1 = 0, vbat_read1 = 0;
	s32 vbat_val_diff, vbat_read_diff;

	u32 vchar_val1 = 0, vchar_read1 = 0;
	s32 vchar_val_diff, vchar_read_diff;

	GLUE_DBG("hal_adc_cal_calc_dalton: NumberOfElements %d",
		  Data->size);

	if (!Data->size)
		return -EINVAL;

	for (element = 0; element < Data->size; element++) {
		pchan =
		    &bcmpmu_adc_chipset_api->adc_channels[Data->data[element].
							  cha];
		if (pchan->sig == PMU_ADC_MAX)
			return -ENODEV;

		pchan->cal_id = Data->data[element].id;
		csapi_pr_info("%s: Element %d, channel %d, raw %d, "
			      "ref %d, id %d",
			__func__, element, Data->data[element].cha,
			Data->data[element].raw, Data->data[element].ref,
			Data->data[element].id);
		switch (Data->data[element].cha) {
		case CSAPI_ADC_BSI:
			/* Get the two reference values from the ADC */
			{
				u16 read1, read2, i;
				u8 samples =
				    bcmpmu_adc_chipset_api->adc_setting->
				    compensation_samples;
				int gain, offset;
				struct bcmpmu_adc_unit bsi_data;
				u32 vbsi_pullup;

				/* Read the two ADC channels */
				read1 = 0;
				for (i = 0; i < samples; i++) {
					read1 +=
					    read_rtm_adc(PMU_ADC_BSI_CAL_LO);
				}
				/* add samples/2 for rounding */
				read1 += samples / 2;
				/* Divide by the number of samples */
				read1 /= samples;

				read2 = 0;
				for (i = 0; i < samples; i++) {
					read2 +=
					    read_rtm_adc(PMU_ADC_BSI_CAL_HI);
				}
				/* For rounding */
				read2 += samples / 2;
				/* Divide to get average */
				read2 /= samples;

				/* Calculate uvperbit and offset */
				if (read1 != read2) {
					s64 tmp =
					   div_s64(((bcmpmu_adc_chipset_api->
					   adc_setting->compensation_volt_hi -
					   bcmpmu_adc_chipset_api->adc_setting->
					   compensation_volt_lo) *
					   1000LL * 1000LL),
					   (read2 - read1));
					gain = div_s64(tmp, 1000);
					offset =
					    bcmpmu_adc_chipset_api->
					    adc_setting->compensation_volt_hi *
					    1000 - div_s64((read2 * tmp), 1000);
					csapi_pr_info("adc_cal_calc_dalton:"
						" read1 %d,"
						" read2 %d, gain %d, offset %d",
					    read1, read2, gain, offset);
				} else {
					csapi_pr_info
					    ("adc_cal_calc_dalton: Unable to "
					     "get reference channel readings");
					/*Get the gain/offset from normal data*/
					bcmpmu_adc_chipset_api->bcmpmu->
					    unit_get(bcmpmu_adc_chipset_api->
						     bcmpmu, pchan->sig,
						     &bsi_data);
					gain = bsi_data.vstep;
					offset = bsi_data.voffset;
				}

				/* Calculate the BSI_VREF based on the
				   three readings */
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_get(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &bsi_data);
				csapi_pr_info("Old: Gain %d, Offset %d, "
					      "Pull %d",
				    bsi_data.vstep, bsi_data.voffset,
					      bsi_data.vmax);
				bsi_data.voffset = offset;
				bsi_data.vstep = gain;
				vbsi_pullup =
				    (Data->data[element].raw * gain +
				     offset) * (bsi_data.rpullup / 1000);
				vbsi_pullup /= Data->data[element].ref;
				/* Overflow 'danger' has passed, go back to uV*/
				vbsi_pullup *= 1000;
				vbsi_pullup +=
				    Data->data[element].raw * gain + offset;
				bsi_data.vmax = vbsi_pullup;
				csapi_pr_info("%s: vbsi_pullup %d", __func__,
					vbsi_pullup);
				bcmpmu_adc_chipset_api->bcmpmu->
				    unit_set(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &bsi_data);
				csapi_pr_info("New: Gain %d, Offset %d, "
					      "Pull %d",
				    bsi_data.vstep, bsi_data.voffset,
					      bsi_data.vmax);
			}
			break;

		case CSAPI_ADC_VBAT:
			/* Even though we're not supposed to implement it in
			   Dalton, then the possibility doesn't harm */
			{
				int gain, offset;
				struct bcmpmu_adc_unit vbat_data;
				if (!vbat_read1) {
					/* Store values for later reference...*/
					vbat_read1 = Data->data[element].raw;
					vbat_val1 = Data->data[element].ref;

					vapplied_uv = Data->data[element].ref;
					/* Make it microvolts */
					vapplied_uv *= 1000;
					gain =
					    vapplied_uv /
					    Data->data[element].raw;
					offset = 0;
					GLUE_DBG("hal_adc_cal_calc_dalton: "
					  "VBAT one point: ref. %d, "
					  "reading %d, Applied %d, "
					  "uvperlsb %d",
					  Data->data[element].ref,
					  Data->data[element].raw,
					  vapplied_uv,
					  gain);
				} else {
					/* Two point calibration */
					/* Make it microvolts */
					vapplied_uv =
						Data->data[element].ref * 1000;
					vbat_val1 *= 1000; /* also microvolts */
					vbat_val_diff = vapplied_uv - vbat_val1;
					vbat_read_diff =
					    Data->data[element].raw -
					    vbat_read1;
					gain = vbat_val_diff / vbat_read_diff;

					offset =
					    vbat_val1 - (gain * vbat_read1);
					GLUE_DBG("hal_adc_cal_calc_dalton: "
						  "VBAT two point: "
						  "voltages(%d, %d), "
						  "references(%d, %d)",
						  vapplied_uv, vbat_val1,
						  Data->data[element].raw,
						  vbat_read1);
					GLUE_DBG("hal_adc_cal_calc_dalton: "
						  "VBAT two point: uvperlsb %d,"
						  " offset %d", gain, offset);
				}
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_get(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &vbat_data);
				vbat_data.vstep = gain;
				vbat_data.voffset = offset;
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_set(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &vbat_data);
			}
			break;

		case CSAPI_ADC_BTEMP:
			break;

		case CSAPI_ADC_VCHAR:
			{
				int gain, offset;
				struct bcmpmu_adc_unit vchar_data;
				if (!vbat_read1) {
					/* Store values for later reference...*/
					vchar_read1 = Data->data[element].raw;
					vchar_val1 = Data->data[element].ref;

					vapplied_uv = Data->data[element].ref;
					/* Make it microvolts */
					vapplied_uv *= 1000;
					gain =
					    vapplied_uv /
					    Data->data[element].raw;
					offset = 0;
					GLUE_DBG("hal_adc_cal_calc_dalton: "
						  "VCHAR one point: ref. %d, "
						  "reading %d, Applied %d, "
						  "uvperlsb %d",
						  Data->data[element].ref,
						  Data->data[element].raw,
						  vapplied_uv, gain);
				} else {
					/* Two point calibration */
					/* Make it microvolts */
					vapplied_uv =
						Data->data[element].ref * 1000;
					vchar_val1 *= 1000;/* also microvolts */
					vchar_val_diff =
					    vapplied_uv - vchar_val1;
					vchar_read_diff =
					    Data->data[element].raw -
					    vchar_read1;
					gain = vchar_val_diff / vchar_read_diff;

					offset =
					    vchar_val1 - (gain * vchar_read1);
					GLUE_DBG("hal_adc_cal_calc_dalton: "
						  "VCHAR two point: "
						  "voltages(%d, %d), "
						  "references(%d, %d)",
						  vapplied_uv,
						  vbat_val1,
						  Data->data[element].raw,
						  vchar_read1);
					GLUE_DBG("hal_adc_cal_calc_dalton: "
						  "VCHAR two point: "
						  "uvperlsb %d, offset %d",
						  gain, offset);
				}
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_get(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &vchar_data);
				vchar_data.vstep = gain;
				vchar_data.voffset = offset;
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_set(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &vchar_data);
			}
			break;
		case CSAPI_ADC_IBAT:
			{
				int fgoffset;
				u32 fg_k;
				struct bcmpmu_adc_unit fg_data;

				bcmpmu_adc_chipset_api->bcmpmu->
				    fg_offset_cal_read(bcmpmu_adc_chipset_api->
						       bcmpmu, &fgoffset);
				if (fgoffset & 0x8000)	/* negative offset */
					fgoffset |= 0xffff0000;
				csapi_pr_info
				    ("hal_Adc_cal_calc_dalton: "
				     "IBAT K calibration: "
				     "FGOFFSET read from the PMU: 0x%x",
				     fgoffset);
				fgoffset /= 4;
				csapi_pr_info
				    ("hal_Adc_cal_calc_dalton: "
				     "IBAT K calibration: "
				     "Converted FGOFFSET: 0x%x",
				     fgoffset);
				/* k is defined as
				IBAT(measured)*1024/IBAT(reading-offset)%256 */
				fg_k =
				    ((Data->data[element].ref * 1024) /
				     ((Data->data[element].raw -
				       fgoffset) * 976 / 1000)) % 256;
				csapi_pr_info
				    ("hal_Adc_cal_calc_dalton:"
				     " IBAT K calibration: Reading %d, "
				     "reference value %d, k %d, offset %d",
				     Data->data[element].raw,
				     Data->data[element].ref, fg_k, fgoffset);

				bcmpmu_adc_chipset_api->bcmpmu->
				   fg_trim_write(bcmpmu_adc_chipset_api->bcmpmu,
						fg_k);
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_get(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &fg_data);
				fg_data.fg_k = fg_k;
				fg_data.voffset = fgoffset;
				bcmpmu_adc_chipset_api->bcmpmu->
					unit_set(bcmpmu_adc_chipset_api->bcmpmu,
					     pchan->sig, &fg_data);
			}
			break;
		default:
			return -EINVAL;
		}
	}
	return CSAPI_ADC_ERR_SUCCESS;
}

/*s32 bcmpmu_adc_cal_calc(struct CSAPI_ADC_CAL_DATA_TYPE * Data)*/
int csapi_cal_calc(struct csapi_cal_req *req)
{
	if (hal_adc_hw_cal_calc_func)
		return hal_adc_hw_cal_calc_func(req);
	else
		return -EINVAL;
}
EXPORT_SYMBOL(csapi_cal_calc);
/* Debug interface */

static int bcmpmu_adc_chipset_open(struct inode *inode, struct file *file)
{
	csapi_pr_debug("%s\n", __func__);
	file->private_data = PDE(inode)->data;

	return 0;
}

int bcmpmu_adc_chipset_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

#define MAX_USER_INPUT_LEN      256
#define MAX_ARGS 25

#define adccsapi_kstrtol(arg)  (kstrtol(arg, 10, &val) < 0 ? 0 : val)

static ssize_t bcmpmu_adc_chipset_write(struct file *file,
					const char __user *buffer, size_t len,
					loff_t *offset)
{
	/*struct bcmpmu_adc_chipset_api *data = file->private_data; */
	/*struct adc_debug dbg; */
	char cmd[MAX_USER_INPUT_LEN], *pcmd = cmd;
	char **ap, arg_cnt;
	char *argv[MAX_ARGS];
	u8 channel;
	int i, status;
	long val; /* used in macro */

	/*int ret, i; */
	int adc_raw, adc_unit;

	csapi_pr_info("%s\n", __func__);

	if (!bcmpmu_adc_chipset_api) {
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
	for (ap = argv; (*ap = strsep(&pcmd, " \t")) != NULL; arg_cnt++) {
		if (**ap != '\0') {
			if (++ap >= &argv[MAX_ARGS])
				break;
		}
	}

/*	printk(KERN_INFO */
/*	 "%s: Command %s, total number of args(including first command) %d", */
/*	  __func__, argv[0], arg_cnt); */
	i = 0;
	while (i < arg_cnt) {
		if (strcmp(argv[i], "VBAT_UNIT") == 0) {
			csapi_adc_raw_read(NULL, CSAPI_ADC_VBAT, &adc_raw, NULL,
					   NULL);
			adc_unit =
			    csapi_adc_unit_convert(NULL, CSAPI_ADC_VBAT,
						   adc_raw);
			printk(KERN_INFO "%s: VBAT: Raw %d, unit %d mV\n",
			       __func__, adc_raw, adc_unit);

		} else if (strcmp(argv[i], "BSI_UNIT") == 0) {
			csapi_adc_raw_read(NULL, CSAPI_ADC_BSI, &adc_raw, NULL,
					   NULL);
			adc_unit =
			    csapi_adc_unit_convert(NULL, CSAPI_ADC_BSI,
						   adc_raw);
			printk(KERN_INFO "%s: BSI: Raw %d, unit %d HOhm\n",
			       __func__, adc_raw, adc_unit);
		} else if (strcmp(argv[i], "BTEMP_UNIT") == 0) {
			csapi_adc_raw_read(NULL, CSAPI_ADC_BTEMP, &adc_raw,
					   NULL, NULL);
			adc_unit =
			    csapi_adc_unit_convert(NULL, CSAPI_ADC_BTEMP,
						   adc_raw);
			printk(KERN_INFO "%s:%s: BTEMP: Raw %d, unit %d K\n",
			       __func__, argv[0], adc_raw, adc_unit);
		}

		else if (strcmp(argv[i], "csapi_adc_raw_read") == 0) {
			/* Only(CSAPI) channel can be supplied here,
			   rest is given */
			if (argv[i + 1]) {
				status = csapi_adc_raw_read(NULL,
						  adccsapi_kstrtol(argv
						  [i + 1]),
						  &adc_raw, NULL, NULL);
				printk(KERN_INFO "%s:%s: Status %d, Raw %d\n",
				       __func__, argv[0], status, adc_raw);
				/* since we are using two arguments here,
				   add one extra to i */
				i++;
			} else {
				printk(KERN_INFO
				       "%s:%s: Not enough parameters for %s\n",
				       __func__, argv[0], argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_adc_unit_read") == 0) {
			/* Only(CSAPI) channel can be supplied here,
			   rest is given */
			if (argv[i + 1]) {
				status = csapi_adc_unit_read(NULL,
						adccsapi_kstrtol(argv
						[i + 1]),
						&adc_raw, NULL, NULL);
				printk(KERN_INFO "%s:%s: Status %d, Unit %d\n",
				       __func__, argv[0], status, adc_raw);
				/* since we are using two arguments here,
				   add one extra to i */
				i++;
			} else {
				printk(KERN_INFO
				       "%s:%s: Not enough parameters for %s\n",
				       __func__, argv[0], argv[i]);
			}

		} else if (strcmp(argv[i], "csapi_adc_unit_convert") == 0) {
			/* CSAPI channel and raw reading can be supplied here,
			   rest is given */
			if (argv[i + 2]) {
				channel = adccsapi_kstrtol(argv[i + 1]);
				adc_raw = adccsapi_kstrtol(argv[i + 2]);
				adc_unit =
				    csapi_adc_unit_convert(NULL, channel,
							   adc_raw);
				printk(KERN_INFO
				       "%s: Channel %d, Raw %d, unit %d\n",
				       __func__, channel, adc_raw, adc_unit);
				/* since we are using two arguments here,
				   add one extra to i */
				i += 2;
			} else {
				printk(KERN_INFO
				       "%s: Not enough parameters for %s\n",
				       __func__, argv[i]);
			}
		} else if (strcmp(argv[i],
				   "csapi_cal_unit_convert_lock") == 0) {
			/* CSAPI channel and raw reading can be supplied here,
			   rest is given */
			if (argv[i + 2]) {
				channel = adccsapi_kstrtol(argv[i + 1]);
				adc_unit = adccsapi_kstrtol(argv[i + 2]);
				csapi_cal_unit_convert_lock(NULL, channel,
							    adc_unit);
				printk(KERN_INFO
				       "%s: Channel %d, locked to %d\n",
				       __func__, channel, adc_unit);
				/* since we are using two arguments here,
				   add one extra to i */
				i += 2;
			} else {
				printk(KERN_INFO
				       "%s: Not enough parameters for %s\n",
				       __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_unit_convert_unlock") ==
			   0) {
			/* CSAPI channel and raw reading can be supplied here,
			   rest is given */
			if (argv[i + 1]) {
				channel = adccsapi_kstrtol(argv[i + 1]);
				csapi_cal_unit_convert_unlock(NULL, channel);
				printk(KERN_INFO "%s: Channel %d, unlocked\n",
				       __func__, channel);
				/* since we are using two arguments here,
				   add one extra to i */
				i++;
			} else {
				printk(KERN_INFO
				       "%s: Not enough parameters for %s\n",
				       __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_data_get") == 0) {
			if (argv[i + 1]) {
				u32 id, p1, p2, p3;
				channel = adccsapi_kstrtol(argv[i + 1]);
				csapi_cal_data_get(NULL, channel, &id, &p1, &p2,
						   &p3);

				printk(KERN_INFO
				       "%s(%s): chl %d, id %d, p1 %d, "
				       "p2 %d, p3 %d\n",
				       __func__, argv[i], channel, id, p1, p2,
				       p3);
				/* since we are using two arguments here,
				   add one extra to i */
				i++;
			} else {
				printk(KERN_INFO
				       "%s: Not enough parameters for %s\n",
				       __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_data_set") == 0) {
			if (argv[i + 5]) {
				u32 id, p1, p2, p3;
				channel = adccsapi_kstrtol(argv[i + 1]);
				id = adccsapi_kstrtol(argv[i + 2]);
				p1 = adccsapi_kstrtol(argv[i + 3]);
				p2 = adccsapi_kstrtol(argv[i + 4]);
				p3 = adccsapi_kstrtol(argv[i + 5]);
				csapi_cal_data_set(NULL, channel, id, p1, p2,
						   p3);
				printk(KERN_INFO
				       "%s(%s): chl %d, id %d, p1 %d, "
				       "p2 %d, p3 %d\n",
				       __func__, argv[i], channel, id, p1, p2,
				       p3);
				/* since we are using five arguments here,
				   add one extra to i */
				i += 5;
			} else {
				printk(KERN_INFO
				       "%s: Not enough parameters for %s\n",
				       __func__, argv[i]);
			}
		} else if (strcmp(argv[i], "csapi_cal_data_calc") == 0) {
			/* only one command in this case */
			if (i == 0) {
				struct csapi_cal_req *cal;
				arg_cnt--;	/* skip command */
				cal =
				    kzalloc(sizeof(struct csapi_cal_req) +
					    (sizeof(struct csapi_cal_data) *
					     (arg_cnt / 4)), GFP_KERNEL);
				cal->size = arg_cnt / 4;
				i = 0;
				while (i < cal->size) {
					cal->data[i].cha =
					    adccsapi_kstrtol(argv[i * 4 + 1]);
					cal->data[i].type =
					    adccsapi_kstrtol(argv[i * 4 + 2]);
					cal->data[i].raw =
					    adccsapi_kstrtol(argv[i * 4 + 3]);
					cal->data[i].ref =
					    adccsapi_kstrtol(argv[i * 4 + 4]);
					cal->data[i].id = 0xfedeabe;
					i++;
				}
				if (cal->size)
					csapi_cal_calc(cal);

				kfree(cal);
			} else {
				printk(KERN_INFO
				       "%s: Calc is only allowed alone",
				       __func__);
			}
			i = arg_cnt;

		}
		i++;
	}
	*offset += len;
	return len;
}

static const struct file_operations bcmpmu_adc_chipset_ops = {
	.open = bcmpmu_adc_chipset_open,
	.write = bcmpmu_adc_chipset_write,
	.release = bcmpmu_adc_chipset_release,
	.owner = THIS_MODULE,
};

/* Framework inteface */

static int __devinit bcmpmu_adc_chipset_api_probe(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	struct bcmpmu_adc_chipset_api *priv_data;

	csapi_pr_info("BCMPMU ADC CHIPSET API Driver\n");
	priv_data = kzalloc(sizeof(struct bcmpmu_adc_chipset_api), GFP_KERNEL);
	if (!priv_data) {
		csapi_pr_info("%s: Memory can not be allocated!!\n", __func__);
		return -ENOMEM;
	}
	priv_data->bcmpmu = bcmpmu;
	bcmpmu_adc_chipset_api = priv_data;
	priv_data->adc_channels = adc_channels;
	priv_data->adc_setting = pdata->adc_setting;

	hal_adc_hw_cal_calc_func = hal_adc_cal_calc_dalton;

	/* When we get calibraion data, call driver to set the value */

	/*bcmpmu->fg_enable (bcmpmu, 1);*/

	proc_create_data("adc_chipset_api", S_IWUSR, NULL,
			 &bcmpmu_adc_chipset_ops, bcmpmu_adc_chipset_api);

	return 0;
}

static int __devexit bcmpmu_adc_chipset_api_remove(struct platform_device *pdev)
{
	struct bcmpmu_adc_chipset_api_driver *priv_data =
	    platform_get_drvdata(pdev);

	kfree(priv_data);
	return 0;
}

struct platform_driver bcmpmu_adc_chipset_api_driver = {
	.probe = bcmpmu_adc_chipset_api_probe,
	.remove = __devexit_p(bcmpmu_adc_chipset_api_remove),
	.driver = {
		   .name = "bcmpmu_adc_chipset_api",
		   }
};

static int __init bcmpmu_adc_chipset_api_init(void)
{
	platform_driver_register(&bcmpmu_adc_chipset_api_driver);
	/* initialize semaphore for ADC access control */
	return 0;
}

/****************************************************************************
*
*  bcmpmu_adc_chipset_api_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit bcmpmu_adc_chipset_api_exit(void)
{
	platform_driver_unregister(&bcmpmu_adc_chipset_api_driver);

}				/* bcm59055_fg_exit */

module_init(bcmpmu_adc_chipset_api_init);
module_exit(bcmpmu_adc_chipset_api_exit);

MODULE_AUTHOR("HJEHG");
MODULE_DESCRIPTION("BCM59055 ADC CHIPSET API Driver");

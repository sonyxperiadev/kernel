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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <linux/mfd/bcmpmu.h>

#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>
#include <linux/gpio.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

#define BCMPMU_TEMP_SNS_EA 0x10 /*Enable*/
#define BCMPMU_TEMP_SNS_DS 0x00 /*Disable*/

/* static int debug_mask = 0xff; */
static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define pr_hwmon(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

struct bcmpmu_adc {
	struct device *hwmon_dev;
	struct bcmpmu *bcmpmu;
	const struct bcmpmu_adc_map *adcmap;
	struct bcmpmu_adc_unit *adcunit;
	const struct bcmpmu_reg_map *ctrlmap;
	struct bcmpmu_adc_setting *adcsetting;
	const struct bcmpmu_temp_map *btmap;
	int btmap_len;
	const struct bcmpmu_temp_map *pmu_temp_map;
	int ptmap_len; /* PMU Temperature MAP len */
	struct bcmpmu_bom_map *bom_map;
	int bom_map_len;
	wait_queue_head_t wait;
	struct mutex lock;
	struct mutex cal_lock;
	struct bcmpmu_adc_req *rtmreq;
};

struct bcmpmu_adc *pgadc;

struct bcmpmu_env {
	struct bcmpmu *bcmpmu;
	const struct bcmpmu_env_info *envregmap;
	unsigned int *env_regs;
	int env_size;
	unsigned long int env_status;
};

struct bcmpmu_adc_irq_data {
	struct bcmpmu_adc *adc;
	struct bcmpmu_adc_req *req;
};

struct bcmpmu_adc_temp_cal_data {
	int gain;
	int offset;
};

static int return_index(const struct bcmpmu_temp_map *slist,
						int len, int element)
{
	int low = 0;
	int high = len - 1;
	while (low <= high) {
		int middle = low + (high - low)/2;

		if (element > slist[middle].adc)
			low = middle + 1;
		else if (element < slist[middle].adc)
			high = middle - 1;
		else
			return middle;
	}
	if (element >= slist[low].adc && element < slist[high].adc)
		return low;
	else
		return high;
}

static int adc_map_batt_temp(const struct bcmpmu_adc *padc, int adc,
			     enum bcmpmu_adc_sig sig)
{
	int i = 0;
	int temp;
	int index;
	int len;
	const struct bcmpmu_temp_map *temp_map;

	if (padc->adcunit && padc->adcunit[sig].lut_ptr) {
		temp_map = (struct bcmpmu_temp_map *)padc->adcunit[sig].lut_ptr;
		len = padc->adcunit[sig].lut_len;
	} else {
		/* This section is for backwards compability */
		temp_map = padc->btmap;
		len = padc->btmap_len;
	}
	temp = temp_map[i].temp;

	for (i = 0; i < len; i++) {
		if ((adc <= temp_map[i].adc) && (adc > temp_map[i + 1].adc)) {
			index = ((temp_map[i].adc - adc) * 1000) /
			    (temp_map[i].adc - temp_map[i + 1].adc);
			temp = temp_map[i].temp +
			    ((temp_map[i + 1].temp -
			      temp_map[i].temp) * index) / 1000;
			break;
		}
	}
	return temp;
}

static int adc_map_pmu_temp(struct bcmpmu_adc *padc, int adc,
						enum bcmpmu_adc_sig sig) {

	const struct bcmpmu_temp_map *pmu_temp_map;
						/*PMU Temperature Sensor map*/
	int delta = 0;
	int pmu_temp = 0; /* PMU Temperature */
	int index = 0;

	pmu_temp_map = padc->pmu_temp_map;
	if (pmu_temp_map == NULL) {
		pr_hwmon(ERROR, "%s pmu_temp_map is NULL\n", __func__);
		return -EINVAL;
	}
	index = return_index(pmu_temp_map, padc->ptmap_len, adc);
	delta = adc - pmu_temp_map[index].adc;
	pmu_temp = pmu_temp_map[index].temp + (delta/2);
	pr_hwmon(DATA, "%s adc-r = %d adc = %d, temp = %d pmu_temp = %d\n",
						__func__, adc,
						pmu_temp_map[index].adc,
						pmu_temp_map[index].temp,
						pmu_temp);

	return pmu_temp;

}


static int adc_map_bom(struct bcmpmu_adc *padc, int value)
{
	int i = 0;
	for (i = 0; i < padc->bom_map_len; i++) {
		if ((value >= padc->bom_map[i].low) &&
		    (value < padc->bom_map[i].high))
			return padc->bom_map[i].bom;
	}
	return -EINVAL;
}

static int read_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	int ret = 0;
	unsigned int val;
	unsigned int val1;
	unsigned int values[2];
	struct bcmpmu_adc_map adcmap;
	req->raw = 0;

	if (req->sig >= PMU_ADC_MAX)
		return -EINVAL;
	switch (req->tm) {
	case PMU_ADC_TM_HK:
		adcmap = padc->adcmap[req->sig];
		break;
	case PMU_ADC_TM_RTM_TX:
	case PMU_ADC_TM_RTM_RX:
	case PMU_ADC_TM_RTM_SW:
	case PMU_ADC_TM_RTM_SW_TEST:
		adcmap = padc->adcmap[PMU_ADC_RTM];
		break;
	case PMU_ADC_TM_MAX:
	default:
		return -EINVAL;
	}

	if ((adcmap.addr0 == 0) && (adcmap.addr1 == 0)) {
		pr_hwmon(ERROR, "%s: sig map failed\n", __func__);
		return -EINVAL;
	} else if (adcmap.addr0 == adcmap.addr1 + 1) {
		ret = padc->bcmpmu->read_dev_bulk(padc->bcmpmu,
						  adcmap.map, adcmap.addr1,
						  values, 2);

		val = values[0];
		val <<= 8;
		val |= values[1];
		pr_hwmon(DATA,
			 "%s: Signal %d, value[0] %x, value[1] %x ==> val %x",
			 __func__, req->sig, values[0], values[1], val);
	} else {
		ret = padc->bcmpmu->read_dev_drct(padc->bcmpmu,
						  adcmap.map, adcmap.addr1,
						  &val,
						  adcmap.dmask | adcmap.vmask);
		if (ret != 0) {
			pr_hwmon(ERROR, "%s: read adc add1 failed\n", __func__);
			return -EINVAL;
		}
		if (adcmap.addr0 != adcmap.addr1) {
			ret = padc->bcmpmu->read_dev_drct(padc->bcmpmu,
							  adcmap.map,
							  adcmap.addr0, &val1,
							  adcmap.dmask | adcmap.
							  vmask);
			if (ret != 0) {
				pr_hwmon(ERROR, "%s: read adc add0 failed\n",
					 __func__);
				return -EINVAL;
			}
			val = (val1 & 0xFF) | ((val << 8) & 0xFF00);
		}
	}
	if ((req->tm == PMU_ADC_TM_HK) && ((val & adcmap.vmask) != 0)) {
		req->raw = -EINVAL;
		pr_hwmon(FLOW, "%s: adc result invalid\n", __func__);
		return ret;
	}
	req->raw = val & adcmap.dmask;
	if (req->sig == PMU_ADC_FG_RAW || req->sig == PMU_ADC_FG_CURRSMPL) {
		int raw_data;
		if (req->raw & 0x8000)
			req->raw |= 0xffff0000;
		raw_data = (int) req->raw;
		if (raw_data > 0)
			raw_data += 2;
		else if (raw_data < 0)
			raw_data -= 2;
		raw_data = raw_data / 4;
		req->raw = (unsigned int) raw_data;
	}
	return ret;
}

/*
static int adc_adjust_curr(struct bcmpmu_adc *padc, int raw_curr)
{
	unsigned short ucurr = (unsigned short)raw_curr;
	short curr = (short)ucurr;
	return (int)(curr / 4);
}
*/

/* cal_adc_result
 * Input: request structure
 * Descripton: if adcunit is defined, req->cal will set to the calibrated
 *voltage in uV.
 * Else req->cal will be set to the raw value.
*/
static void cal_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	struct bcmpmu_adc_req cal_req;
	u16 i;
	static u16 read1, read2;
	static int last_temperature, new_temperature;
	static int gain, offset;
	static u32 now, last_time;

	if (!padc->adcunit) {
		req->cal = req->raw;
		return;
	}

	switch (req->sig) {
	case PMU_ADC_NTC:
		new_temperature = req->raw;
		/* fall through */
	case PMU_ADC_PATEMP:
	case PMU_ADC_32KTEMP:
	case PMU_ADC_BSI:
	case PMU_ADC_BOM:
		if (padc->adcsetting->compensation_interval) {
			/* check if calibration channels should be refreshed */
			mutex_lock(&padc->cal_lock);
			now = get_seconds();
			if ((!read1)
			    || (last_time +
				padc->adcsetting->compensation_interval < now)
			    || (abs(last_temperature - new_temperature) > 16)) {
				pr_hwmon(DATA,
					 "%s: Reading calibration channels:"
					 " last_time %d, now %d, last_temp %d,"
					 " new_temp %d",
					 __func__, last_time, now,
					 last_temperature, new_temperature);
				last_time = get_seconds();
				last_temperature = new_temperature;
				cal_req.tm = PMU_ADC_TM_RTM_SW;
				cal_req.flags = PMU_ADC_RAW_ONLY;
				read1 = 0;
				for (i = 0;
				     i <
				     padc->adcsetting->compensation_samples;) {
					cal_req.sig = PMU_ADC_NTC_CAL_LO;
					if (!padc->bcmpmu->adc_req(
						    padc->bcmpmu, &cal_req) &&
					    cal_req.raw != 0x3ff) {
						pr_hwmon(DATA, "%s: LO[%u]"
							      "cal_req.raw=%u",
							 __func__, i,
							  cal_req.raw);
						read1 += cal_req.raw;
						i++;
					}
				}
				/* add samples/2 for rounding */
				read1 += padc->adcsetting->
					compensation_samples / 2;
				/* Divide by the number of samples */
				read1 /= padc->adcsetting->compensation_samples;
				read2 = 0;
				for (i = 0;
				     i <
				     padc->adcsetting->compensation_samples;) {
					cal_req.sig = PMU_ADC_NTC_CAL_HI;
					if (!padc->bcmpmu->adc_req(
						    padc->bcmpmu, &cal_req) &&
					    cal_req.raw != 0x3ff) {
						pr_hwmon(DATA, "%s: HI[%u]"
							 "cal_req.raw=%u",
							 __func__, i,
							  cal_req.raw);
						read2 += cal_req.raw;
						i++;
					}
				}
				/* For rounding */
				read2 += padc->adcsetting->
					compensation_samples / 2;
				/* Divide to get average */
				read2 /= padc->adcsetting->compensation_samples;
				/* Calculate uvperbit and offset */
				if (read1 != read2) {
					gain =
					    ((padc->adcsetting->
					      compensation_volt_hi -
					      padc->adcsetting->
					      compensation_volt_lo) * 1000) /
					    (read2 - read1);
					offset =
					    padc->adcsetting->
					    compensation_volt_hi * 1000 -
					    (read2 * gain);
				}
				pr_hwmon(DATA,
					 "%s: Value %d, read1 %d, read2 %d, "
					 "gain %d, offset %d, vmax %d",
					 __func__, req->raw, read1, read2, gain,
					 offset, padc->adcunit[req->sig].vmax);
			}
			mutex_unlock(&padc->cal_lock);
		}
		padc->adcunit[req->sig].vstep = gain;
		padc->adcunit[req->sig].voffset = offset;
		break;
	default:
		break;
	}
	req->cal = req->raw * padc->adcunit[req->sig].vstep +
		padc->adcunit[req->sig].voffset; /* vstep, offset is in uV */
	pr_hwmon(DATA, "%s: raw %d, vstep %d, offset %d, cal %d uV", __func__,
		 req->raw, padc->adcunit[req->sig].vstep,
		 padc->adcunit[req->sig].voffset, req->cal);
}

/* cnv_adc_result
 * Input: request structure
 * Descripton: Converts the calibrated value (uV or raw copy) to the
 * appropiate channel unit.
 * Voltage channels will return mV, temperature channels K, current channels
 * mA and resitive
 *channels HOhm.
*/
static void cnv_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	struct bcmpmu_fg *pfg = padc->bcmpmu->fginfo;
	switch (req->sig) {
	case PMU_ADC_VMBATT:
	case PMU_ADC_VBBATT:
	case PMU_ADC_VBUS:
	case PMU_ADC_ID:
	case PMU_ADC_FG_VMBATT:
		if (padc->adcunit) {
			/* uV to mV, include rounding */
			req->cnv = (req->cal + 500) / 1000;
		} else
			req->cnv =
			    (req->cal * padc->adcmap[req->sig].vrng) / 1024;
		break;
	case PMU_ADC_NTC:
	case PMU_ADC_PATEMP:
	case PMU_ADC_32KTEMP:
		if (padc->adcunit) {
			/* req->cal is in uV, table is in mV */
			req->cnv = adc_map_batt_temp(padc, (req->cal / 1000),
						     req->sig);
		} else {
			/* Table is the raw value */
			req->cnv = adc_map_batt_temp(padc, req->cal, req->sig);
		}
		break;
	case PMU_ADC_FG_RAW:
	case PMU_ADC_FG_CURRSMPL:
		{
			int modifier = 0, ibat_to_return;
			int reading;
			int voffset = 0;
			reading = req->raw;
			if (req->cal & 0x8000)	/* negative offset */
				reading |= 0xffff0000;

			if (padc->adcunit && padc->adcunit[req->sig].fg_k) {
				modifier =
				    (padc->adcunit[req->sig].fg_k >
				     128) ? 768 +
				    padc->adcunit[req->sig].fg_k : 1024 +
				    padc->adcunit[req->sig].fg_k;
				ibat_to_return =
				    ((reading * modifier) / 1024) -
				    padc->adcunit[req->sig].voffset;
				pr_hwmon(DATA,
					 "%s: reading %d, ibat_to_return before"
					 " negation and 976 modification %d",
					 __func__, reading, ibat_to_return);
				voffset = padc->adcunit[req->sig].voffset;
			} else
				ibat_to_return = reading;
			ibat_to_return =
			    (ibat_to_return * pfg->fg_factor) / 1000;
			pr_hwmon(DATA,
				 "%s: raw %x, value %d, modifier %d Offset %d,"
				 " ibat %d",
				 __func__, req->raw, reading, modifier,
				 voffset, ibat_to_return);
			req->cnv = ibat_to_return;
		}
		break;
	case PMU_ADC_BSI:
		{
			struct bcmpmu_adc_req ibat_req;
			int iread;
			/* Compensate with ibat */
			if (!padc->adcunit) {
				req->cnv = req->cal;
				return;
			}
			ibat_req.sig = PMU_ADC_FG_RAW;
			ibat_req.tm = PMU_ADC_TM_HK;
			ibat_req.flags = PMU_ADC_RAW_AND_UNIT;
			padc->bcmpmu->adc_req(padc->bcmpmu, &ibat_req);
			pr_hwmon(DATA,
				 "%s: req->cal before %d, ibat_req.cnv %d",
				 __func__, req->cal, ibat_req.cnv);
			/* compensate reading with the 25 mOhm in the
			   FG and battery terminal */
			req->cal -= ((ibat_req.cnv + 20) / 40) * 1000;
			/* Calculate the current in the BSI resistor.
			   Unit is 10 nA */
			iread =
			    ((padc->adcunit[req->sig].vmax -
			      req->cal) * 100) /
			    (padc->adcunit[req->sig].rpullup);
			if (!iread) {
				/* No resistor is present. */
				req->cnv = 0;
				break;
			}
			pr_hwmon(DATA, "%s: req->cal adjusted %d, iread %d",
				 __func__, req->cal, iread);
			/* Calculate the resistor */
			/* Hecto ohm - iread>>1 is for rounding */
			req->cnv = req->cal / iread;
			pr_hwmon(DATA,
				 "%s: req->cnv %d", __func__, req->cnv);
		}
		break;
	case PMU_ADC_BOM:
		if (padc->adcunit[req->sig].lut_ptr) { /* bom_map */
			/* Lookup it up in the look-up table... */
			/* We have voltages in uV */
			req->cnv = adc_map_bom(padc, req->cal / 1000);
		} else
			req->cnv = 0;
		break;

	case PMU_ADC_TEMP_SNS:

		req->cnv = adc_map_pmu_temp(padc, req->raw, req->sig);
		break;

	default:
		req->cnv = req->cal;
		break;
	}
}

static int update_adc_result(struct bcmpmu_adc *padc,
			     struct bcmpmu_adc_req *req)
{
	int ret;
	int insurance = 100;

	req->raw = -EINVAL;
	if (req->sig == PMU_ADC_FG_CURRSMPL || req->sig == PMU_ADC_FG_RAW) {
		ret = padc->bcmpmu->write_dev(padc->bcmpmu,
					      PMU_REG_FG_FRZSMPL,
					      padc->bcmpmu->
					      regmap[PMU_REG_FG_FRZSMPL].mask,
					      padc->bcmpmu->
					      regmap[PMU_REG_FG_FRZSMPL].mask);
		if (ret != 0)
			return ret;
	}
	/* FG ADC channel can return negative value
	and -22 (-EINVAL) is a valid value for FG */
	do {
		ret = read_adc_result(padc, req);
		/* Here we get the raw value */
		if (ret != 0)
			return ret;
		insurance--;
	} while (req->raw ==
		-EINVAL && insurance && req->sig != PMU_ADC_FG_CURRSMPL);

	BUG_ON(insurance == 0 && req->raw == -EINVAL);
	return 0;
}

static void adc_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_adc *padc = data;

	pr_hwmon(FLOW, "%s: called\n", __func__);

	if (irq == PMU_IRQ_RTM_DATA_RDY) {
		if (padc->rtmreq == NULL) {
			pr_hwmon(ERROR, "%s: RTM not requested\n", __func__);
			return;
		}
		padc->rtmreq->ready = 1;
		wake_up(&padc->wait);
		return;
	} else if ((irq == PMU_IRQ_RTM_IN_CON_MEAS) ||
		   (irq == PMU_IRQ_RTM_UPPER) ||
		   (irq == PMU_IRQ_RTM_IGNORE) ||
		   (irq == PMU_IRQ_RTM_OVERRIDDEN)) {
		pr_hwmon(FLOW, "%s: irq %d 'Getting handled'\n", __func__, irq);
	} else {
		pr_hwmon(FLOW, "%s: irq %d unsupported\n", __func__, irq);
		return;
	}
}

static int bcmpmu_adc_request(struct bcmpmu *bcmpmu, struct bcmpmu_adc_req *req)
{
	struct bcmpmu_adc *padc = bcmpmu->adcinfo;
	int ret = -EINVAL, timeout;
	struct pin_config StoredPinmux, TestPinMux;
	unsigned adcsyngpio;
	enum PIN_FUNC adcsyngpiomux;

	pr_hwmon(FLOW, "%s: called: ->sig %d, tm %d, flags %d\n", __func__,
		 req->sig, req->tm, req->flags);
	if (req->flags == PMU_ADC_RAW_ONLY ||
	    req->flags == PMU_ADC_RAW_AND_UNIT) {
		if ((req->tm == PMU_ADC_TM_RTM_SW) ||
		    (req->tm == PMU_ADC_TM_RTM_SW_TEST))
			timeout = padc->adcsetting->sw_timeout;
		else
			timeout = padc->adcsetting->txrx_timeout;

		switch (req->tm) {
		case PMU_ADC_TM_HK:
			ret = update_adc_result(padc, req);
			break;
		case PMU_ADC_TM_RTM_TX:
		case PMU_ADC_TM_RTM_RX:
		case PMU_ADC_TM_RTM_SW:
		case PMU_ADC_TM_RTM_SW_TEST:
			mutex_lock(&padc->lock);
			padc->rtmreq = req;
			if (req->tm == PMU_ADC_TM_RTM_SW_TEST) {
				pinmux_find_gpio(PN_ADCSYN, &adcsyngpio,
						 &adcsyngpiomux);
				pr_hwmon(FLOW, "%s: SW_TEST: Pin:%u, "
					 "Gpio:%u, Mux:%u\n", __func__,
					 PN_ADCSYN, adcsyngpio, adcsyngpiomux);
				/* Setup test pinmuxing */
				StoredPinmux.name = PN_ADCSYN;
				pinmux_get_pin_config(&StoredPinmux);
				TestPinMux.name = PN_ADCSYN;
				pinmux_get_pin_config(&TestPinMux);
				TestPinMux.func = adcsyngpiomux;
				pinmux_set_pin_config(&TestPinMux);
				gpio_request(adcsyngpio, "ADCSYN_GPIO");
				gpio_direction_output(adcsyngpio, 1);
				/* Use TX for test */
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     map,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     addr, 0,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     mask);
				bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_TX);
			}
			/* config hw for rtm adc */
			if (req->tm == PMU_ADC_TM_RTM_TX) {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     map,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     addr,
							     padc->adcsetting->
							     tx_delay << padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     shift,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     mask);
				bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_TX);
			}
			if (req->tm == PMU_ADC_TM_RTM_RX) {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     map,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     addr,
							     padc->adcsetting->
							     rx_delay << padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     shift,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     mask);
				bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_RX);
			}
			req->ready = 0;
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
						     padc->
						     ctrlmap[PMU_ADC_RTM_SEL].
						     map,
						     padc->
						     ctrlmap[PMU_ADC_RTM_SEL].
						     addr,
						     padc->adcmap[req->sig].
						     rtmsel << padc->
						     ctrlmap[PMU_ADC_RTM_SEL].
						     shift,
						     padc->
						     ctrlmap[PMU_ADC_RTM_SEL].
						     mask);
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     map,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     addr, 0,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     mask);
			if (req->tm == PMU_ADC_TM_RTM_SW) {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     map,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     addr, 0,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_DLY].
							     mask);
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
							     padc->
							     ctrlmap
							     [PMU_ADC_RTM_MASK].
							     map,
							     padc->
							     ctrlmap
							    [PMU_ADC_RTM_START].
							     addr,
							     padc->
							     ctrlmap
							    [PMU_ADC_RTM_START].
							     mask,
							     padc->
							     ctrlmap
							    [PMU_ADC_RTM_START].
							     mask);
			}
			pr_hwmon(FLOW, "%s: start rtm adc\n", __func__);
			if (req->tm == PMU_ADC_TM_RTM_SW_TEST) {
				/* Set ADC_SYNC to Low */
				msleep(20);
				gpio_set_value(adcsyngpio, 0);
			}
			if (wait_event_interruptible_timeout(padc->wait,
							     req->ready,
							     timeout) == 0) {
				pr_hwmon(ERROR, "%s: RTM ADC timeout\n",
					 __func__);
				req->raw = 0;
				ret = -ETIMEDOUT;
			} else
				ret = update_adc_result(padc, req);
			padc->rtmreq = NULL;
			pr_hwmon(FLOW, "%s: Wait/update_adc_result returned %d",
				 __func__, ret);
			/* Need to disable RTM to avoid interrrupts from
			   ADC_SYN activated RTM reads */
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     map,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     addr,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     mask,
						     padc->
						     ctrlmap[PMU_ADC_RTM_MASK].
						     mask);
			if (req->tm == PMU_ADC_TM_RTM_SW_TEST) {
				/* Set ADC_SYNC to High */
				gpio_set_value(adcsyngpio, 1);

				/* Restore */
				gpio_free(adcsyngpio);
				pinmux_set_pin_config(&StoredPinmux);
			}
			mutex_unlock(&padc->lock);
			break;
		case PMU_ADC_TM_MAX:
		default:
			ret = -EINVAL;
		}

		if (ret < 0)
			return ret;
	}
	if ((req->flags == PMU_ADC_UNIT_ONLY ||
	     req->flags == PMU_ADC_RAW_AND_UNIT)) {

		/* This gives us a voltage in req->cal */
		cal_adc_result(padc, req);
		/* This updates the req->cnv with the value */
		cnv_adc_result(padc, req);
	} else {
		req->cal = req->raw;
		req->cnv = req->raw;
	}
	pr_hwmon(DATA, "%s: result sig=%d, raw=0x%X, cal=0x%X, cnv=%d\n",
		 __func__, req->sig, req->raw, req->cal, req->cnv);

	return ret;
}

int bcmpmu_read_pmu_temp(void){
	struct bcmpmu *bcmpmu = pgadc->bcmpmu;
	struct bcmpmu_adc_req req;

	bcmpmu->write_dev(bcmpmu, PMU_REG_COMM_CTRL, BCMPMU_TEMP_SNS_EA,
			bcmpmu->regmap[PMU_REG_COMM_CTRL].mask);

	req.sig = PMU_ADC_TEMP_SNS;
	req.tm = PMU_ADC_TM_RTM_SW;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);

	bcmpmu->write_dev(bcmpmu, PMU_REG_COMM_CTRL, BCMPMU_TEMP_SNS_DS,
			bcmpmu->regmap[PMU_REG_COMM_CTRL].mask);
	return req.cnv;

}
EXPORT_SYMBOL(bcmpmu_read_pmu_temp);

/* bcmpmu_get_adcunit_data
 * Input: signal number, pointer to where data should go
 * Descripton: Copies the adc_unit data from main structure to supplied
 *pointer
*/
static int bcmpmu_get_adcunit_data(struct bcmpmu *bcmpmu,
				   enum bcmpmu_adc_sig sig,
				   struct bcmpmu_adc_unit *unit)
{
	struct bcmpmu_adc *padc = bcmpmu->adcinfo;
	struct bcmpmu_adc_unit adcunit;

	/* Do we have unit data at all */
	if (!padc->adcunit)
		return -EINVAL;

	/* Then verify that the channel is defined in the map */
	if ((padc->adcmap[sig].addr0 == 0) && (padc->adcmap[sig].addr1 == 0)) {
		pr_hwmon(ERROR, "%s: sig map failed\n", __func__);
		return -EINVAL;
	}
	adcunit = padc->adcunit[sig];

	/* Then copy the contents of the unit structure */
	memcpy(unit, &adcunit, sizeof(struct bcmpmu_adc_unit));
	return 0;

}

/* bcmpmu_set_adcunit_data
 * Input: signal number, pointer to adc_unit data to use
 * Descripton: Copies the adc_unit data from supplied pointer
 *to main structure
*/
static int bcmpmu_set_adcunit_data(struct bcmpmu *bcmpmu,
				   enum bcmpmu_adc_sig sig,
				   struct bcmpmu_adc_unit *unit)
{
	struct bcmpmu_adc *padc = bcmpmu->adcinfo;
	struct bcmpmu_adc_unit *adcunit;

	/* Do we have unit data at all */
	if (!padc->adcunit)
		return -EINVAL;

	/* Then verify that the channel is defined in the map */
	if ((padc->adcmap[sig].addr0 == 0) && (padc->adcmap[sig].addr1 == 0)) {
		pr_hwmon(ERROR, "%s: sig map failed\n", __func__);
		return -EINVAL;
	}
	adcunit = &padc->adcunit[sig];

	/* Then copy the contents of the unit structure */
	memcpy(adcunit, unit, sizeof(struct bcmpmu_adc_unit));
	return 0;

}

static ssize_t show_vmbatt(struct device *dev, struct device_attribute
			   *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_vbbatt(struct device *dev, struct device_attribute
			   *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_VBBATT;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_vbus(struct device *dev, struct device_attribute
			 *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_VBUS;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_id(struct device *dev, struct device_attribute
		       *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_ID;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_ntc(struct device *dev, struct device_attribute
			*devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_NTC;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_bsi(struct device *dev, struct device_attribute
			*devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_BSI;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_32ktemp(struct device *dev, struct device_attribute
			    *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_32KTEMP;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}

static ssize_t show_fg_vmbatt(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_FG_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}

static ssize_t show_fg_currsmpl(struct device *dev, struct device_attribute
				*devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_FG_CURRSMPL;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return sprintf(buf, "%d\n", req.cnv);
}

static ssize_t show_envupdate(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	int i;
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	unsigned long status;
	bcmpmu->update_env_status(bcmpmu, &status);

	for (i = 0; i < penv->env_size; i++) {
		sprintf(buf, "%8X\n", penv->env_regs[i]);
		buf += 9;
	}
	return penv->env_size * 9;
}

static ssize_t show_fg_acc_mas(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_fg *pfg = bcmpmu->fginfo;
	int result;
	bcmpmu->fg_acc_mas(bcmpmu, &result);

	return sprintf(buf, "fg mas=%d acc=0x%X smplcnt=0x%X slpcnt=0x%X\n",
		       result, pfg->fg_acc, pfg->fg_smpl_cnt, pfg->fg_slp_cnt);
}

static SENSOR_DEVICE_ATTR(vmbatt, S_IRUGO, show_vmbatt, NULL, 0);
static SENSOR_DEVICE_ATTR(vbbatt, S_IRUGO, show_vbbatt, NULL, 1);
static SENSOR_DEVICE_ATTR(vbus, S_IRUGO, show_vbus, NULL, 2);
static SENSOR_DEVICE_ATTR(ntc, S_IRUGO, show_ntc, NULL, 3);
static SENSOR_DEVICE_ATTR(32ktemp, S_IRUGO, show_32ktemp, NULL, 4);
static SENSOR_DEVICE_ATTR(bsi, S_IRUGO, show_bsi, NULL, 5);
static SENSOR_DEVICE_ATTR(id, S_IRUGO, show_id, NULL, 6);
static SENSOR_DEVICE_ATTR(fg_vmbatt, S_IRUGO, show_fg_vmbatt, NULL, 7);
static SENSOR_DEVICE_ATTR(fg_currsmpl, S_IRUGO, show_fg_currsmpl, NULL, 8);
static SENSOR_DEVICE_ATTR(env_all, S_IRUGO, show_envupdate, NULL, 9);
static SENSOR_DEVICE_ATTR(fg_acc_mas, S_IRUGO, show_fg_acc_mas, NULL, 10);

static struct attribute *bcmpmu_hwmon_attrs[] = {
	&sensor_dev_attr_vmbatt.dev_attr.attr,
	&sensor_dev_attr_vbbatt.dev_attr.attr,
	&sensor_dev_attr_vbus.dev_attr.attr,
	&sensor_dev_attr_ntc.dev_attr.attr,
	&sensor_dev_attr_bsi.dev_attr.attr,
	&sensor_dev_attr_id.dev_attr.attr,
	&sensor_dev_attr_32ktemp.dev_attr.attr,
	&sensor_dev_attr_fg_vmbatt.dev_attr.attr,
	&sensor_dev_attr_fg_currsmpl.dev_attr.attr,
	&sensor_dev_attr_env_all.dev_attr.attr,
	&sensor_dev_attr_fg_acc_mas.dev_attr.attr,
	NULL
};

static const struct attribute_group bcmpmu_hwmon_attr_group = {
	.attrs = bcmpmu_hwmon_attrs,
};

static void bcmpmu_update_env_status(struct bcmpmu *bcmpmu,
				     unsigned long *env_status)
{
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	bcmpmu->read_dev_bulk(bcmpmu, bcmpmu->regmap[PMU_REG_ENV1].map,
			      bcmpmu->regmap[PMU_REG_ENV1].addr, penv->env_regs,
			      penv->env_size);
}

static bool bcmpmu_is_env_bit_set(struct bcmpmu *bcmpmu,
				  enum bcmpmu_env_bit_t env_bit)
{
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	int index;
	if (penv->envregmap[env_bit].regmap.mask == 0)
		return false;
	index =
	    penv->envregmap[env_bit].regmap.addr -
	    bcmpmu->regmap[PMU_REG_ENV1].addr;
	if (penv->env_regs[index] & penv->envregmap[index].regmap.mask)
		return true;
	else
		return false;
}

static bool bcmpmu_get_env_bit_status(struct bcmpmu *bcmpmu,
				      enum bcmpmu_env_bit_t env_bit)
{
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	unsigned int val;
	if (penv->envregmap[env_bit].regmap.mask == 0)
		return false;
	bcmpmu->read_dev_drct(bcmpmu, penv->envregmap[env_bit].regmap.map,
			      penv->envregmap[env_bit].regmap.addr, &val,
			      penv->envregmap[env_bit].regmap.mask);
	if (val != 0)
		return true;
	else
		return false;
}

static int bcmpmu_get_fg_currsmpl(struct bcmpmu *bcmpmu, int *data)
{
	int ret;
	struct bcmpmu_adc_req req;
	struct bcmpmu_fg *pfg = bcmpmu->fginfo;
	int curr;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_FRZSMPL,
				bcmpmu->regmap[PMU_REG_FG_FRZSMPL].mask,
				bcmpmu->regmap[PMU_REG_FG_FRZSMPL].mask);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to latch fg smpl.\n", __func__);
		return ret;
	}
	req.sig = PMU_ADC_FG_CURRSMPL;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	ret = bcmpmu->adc_req(bcmpmu, &req);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to get adc result.\n", __func__);
		return ret;
	}
	curr = req.cnv;
	*data = (curr * pfg->fg_factor) / 1000;
	return ret;
}

static int bcmpmu_get_fg_vmbatt(struct bcmpmu *bcmpmu, int *data)
{
	int ret;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_FG_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	ret = bcmpmu->adc_req(bcmpmu, &req);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to get adc result.\n", __func__);
		return ret;
	}
	*data = req.cnv;
	return ret;
}

static int bcmpmu_get_fg_acc_mas(struct bcmpmu *bcmpmu, int *data)
{
	int ret;
	unsigned int acc0, acc1, acc2, acc3;
	long int acc;
	unsigned int cnt, cnt0, cnt1;
	unsigned int slpcnt, slpcnt0, slpcnt1;
	struct bcmpmu_fg *pfg = bcmpmu->fginfo;
	int64_t actacc, slpacc;

	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_FRZREAD,
				bcmpmu->regmap[PMU_REG_FG_FRZREAD].mask,
				bcmpmu->regmap[PMU_REG_FG_FRZREAD].mask);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to latch fg read.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_ACCM0, &acc0, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc0.\n", __func__);
		return ret;
	}

	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_ACCM1, &acc1, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc1.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_ACCM2, &acc2, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc2.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_ACCM3, &acc3, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc3.\n", __func__);
		return ret;
	}

	if ((acc3 & 0x80) == 0) {
		pr_hwmon(DATA, "%s fg data invalid.\n", __func__);
		return -EINVAL;
	}
	acc3 = acc3 & 0x03;
	if (acc3 >= 2)
		acc3 = acc3 | 0xFC;

	acc = (int)(acc0 | (acc1 << 8) | (acc2 << 16) | (acc3 << 24));
	pfg->fg_acc = acc;
	pr_hwmon(DATA, "%s: acc=%ld, acc3=%X acc2=%X acc1=%X acc0=%X\n",
		 __func__, acc, acc3, acc2, acc1, acc0);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_FG_CNT0, &cnt0, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg cnt0.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_FG_CNT1, &cnt1, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg cnt1.\n", __func__);
		return ret;
	}
	cnt = cnt0 | (cnt1 << 8);
	pfg->fg_smpl_cnt = cnt;

	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_SLEEPCNT0, &slpcnt0, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg slpcnt0.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_SLEEPCNT1, &slpcnt1, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg slpcnt1.\n", __func__);
		return ret;
	}

	slpcnt = slpcnt0 | (slpcnt1 << 8);
	pfg->fg_slp_cnt = slpcnt;

	actacc = acc * pfg->fg_smpl_cnt_tm;
	actacc = actacc * pfg->fg_factor;
	slpacc = slpcnt * pfg->fg_slp_cnt_tm;
	slpacc = slpacc * pfg->fg_slp_curr_ua;
	pr_hwmon(DATA, "%s: actacc=%lld, actcnt=%d, slpacc=%lld, slpcnt=%d\n",
		 __func__, actacc, cnt, slpacc, slpcnt);
	actacc = actacc - slpacc;
	actacc = div_s64(actacc, 1000000);
	*data = (int)actacc;
	pfg->fg_columb_cnt += *data;

	if (slpcnt || cnt)
		pfg->fg_ibat_avg =
		    (*data * 1000) / (slpcnt * pfg->fg_slp_cnt_tm +
				      cnt * pfg->fg_smpl_cnt_tm);

	pr_hwmon(FLOW, "%s: fg acc mAsec = %d\n", __func__, *data);

	return ret;
}

static int bcmpmu_fg_enable(struct bcmpmu *bcmpmu, int en)
{
	int ret;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_HOSTEN,
				en << bcmpmu->regmap[PMU_REG_FG_HOSTEN].shift,
				bcmpmu->regmap[PMU_REG_FG_HOSTEN].mask);
	if (ret != 0)
		pr_hwmon(ERROR, "%s failed to write device.\n", __func__);
	return ret;
}

static int bcmpmu_fg_reset(struct bcmpmu *bcmpmu)
{
	int ret;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_RESET,
				1 << bcmpmu->regmap[PMU_REG_FG_RESET].shift,
				bcmpmu->regmap[PMU_REG_FG_RESET].mask);
	if (ret != 0)
		pr_hwmon(ERROR, "%s failed to write device.\n", __func__);
	return ret;
}

static int bcmpmu_fg_offset_cal(struct bcmpmu *bcmpmu)
{
	int ret;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_CAL,
				1 << bcmpmu->regmap[PMU_REG_FG_CAL].shift,
				bcmpmu->regmap[PMU_REG_FG_CAL].mask);
	if (ret != 0)
		pr_hwmon(ERROR, "%s failed to write device.\n", __func__);
	return ret;
}

static int bcmpmu_fg_offset_cal_read(struct bcmpmu *bcmpmu, int *data)
{
	int off, off0, off1, ret;

	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_OFFSET0, &off0, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg offset0.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			       PMU_REG_FG_OFFSET1, &off1, PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg offset1.\n", __func__);
		return ret;
	}
	off = off1 | (off0 << 8);
	*data = off;
	return 0;
}

static int bcmpmu_fg_trim_write(struct bcmpmu *bcmpmu, int data)
{
	int ret;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_GAINTRIM,
				data << bcmpmu->regmap[PMU_REG_FG_CAL].shift,
				bcmpmu->regmap[PMU_REG_FG_CAL].mask);
	if (ret != 0)
		pr_hwmon(ERROR, "%s failed to write device.\n", __func__);
	return ret;
}

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t dbgmsk_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	return sprintf(buf, "debug_mask is %x\n", debug_mask);
}
static ssize_t dbgmsk_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	sscanf(buf, "%x", &debug_mask);
	return count;
}
static ssize_t fg_status_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_fg *pfg = bcmpmu->fginfo;
	return sprintf(buf, "Fuel Gauge Status\n "
	"fg_acc=%d\n fg_smpl_cnt=%d\n fg_slp_cnt=%d\n "
	"fg_smpl_cnt_tm=%d\n fg_slp_cnt_tm=%d\n "
	"fg_slp_curr_ua=%d\n fg_sns_res=%d\n fg_factor=%d\n", pfg->fg_acc,
		       pfg->fg_smpl_cnt, pfg->fg_slp_cnt, pfg->fg_smpl_cnt_tm,
		       pfg->fg_slp_cnt_tm, pfg->fg_slp_curr_ua,
		       pfg->fg_sns_res, pfg->fg_factor);
}

static ssize_t fg_factor_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_fg *pfg = bcmpmu->fginfo;
	return sprintf(buf, "fg_factor=%d\n", pfg->fg_factor);
}
static ssize_t fg_factor_store(struct device *dev,
			struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_fg *pfg = bcmpmu->fginfo;
	sscanf(buf, "%d", &pfg->fg_factor);
	return count;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_store);
static DEVICE_ATTR(fg_status, 0644, fg_status_show, NULL);
static DEVICE_ATTR(fg_factor, 0644, fg_factor_show, fg_factor_store);
#endif

static int __devinit bcmpmu_hwmon_probe(struct platform_device *pdev)
{
	int ret = -ENOMEM;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	struct bcmpmu_adc *padc = NULL;
	struct bcmpmu_env *penv = NULL;
	struct bcmpmu_fg *pfg = NULL;
	int *envregs = NULL;

	pr_hwmon(INIT, "%s: called\n", __func__);

	padc = kzalloc(sizeof(struct bcmpmu_adc), GFP_KERNEL);
	if (padc == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&padc->wait);
	mutex_init(&padc->lock);
	mutex_init(&padc->cal_lock);
	pgadc = padc;
	padc->bcmpmu = bcmpmu;
	padc->adcmap = bcmpmu_get_adcmap(bcmpmu);
	padc->adcunit = bcmpmu_get_adcunit(bcmpmu);
	padc->ctrlmap = bcmpmu_get_adc_ctrl_map(bcmpmu);
	padc->btmap = pdata->batt_temp_map;
	padc->btmap_len = (int)pdata->batt_temp_map_len;
	padc->pmu_temp_map = pdata->pmu_temp_map;
	padc->ptmap_len = (int)pdata->pmu_temp_map_len;
	padc->rtmreq = NULL;
	padc->bom_map = pdata->bom_map;
	padc->bom_map_len = pdata->bom_map_len;
	padc->adcsetting = pdata->adc_setting;
	if (!padc->adcsetting->sw_timeout)
		padc->adcsetting->sw_timeout = 50;
	if (!padc->adcsetting->txrx_timeout)
		padc->adcsetting->txrx_timeout = 2000;
	bcmpmu->adcinfo = (void *)padc;
	bcmpmu->adc_req = bcmpmu_adc_request;
	bcmpmu->unit_get = bcmpmu_get_adcunit_data;
	bcmpmu->unit_set = bcmpmu_set_adcunit_data;

	if (padc->adcunit) {
		padc->adcunit[PMU_ADC_NTC].lut_ptr = pdata->batt_temp_voltmap;
		padc->adcunit[PMU_ADC_PATEMP].lut_ptr = pdata->pa_temp_voltmap;
		padc->adcunit[PMU_ADC_32KTEMP].lut_ptr =
		    pdata->x32_temp_voltmap;
		padc->adcunit[PMU_ADC_BOM].lut_ptr = pdata->bom_map;

		padc->adcunit[PMU_ADC_NTC].lut_len =
		    pdata->batt_temp_voltmap_len;
		padc->adcunit[PMU_ADC_PATEMP].lut_len =
		    pdata->pa_temp_voltmap_len;
		padc->adcunit[PMU_ADC_32KTEMP].lut_len =
		    pdata->x32_temp_voltmap_len;
		padc->adcunit[PMU_ADC_BOM].lut_len = pdata->bom_map_len;
	}

	penv = kzalloc(sizeof(struct bcmpmu_env), GFP_KERNEL);
	if (penv == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		goto err;
	}
	penv->envregmap = bcmpmu_get_envregmap(bcmpmu, &penv->env_size);
	envregs = kzalloc((penv->env_size * sizeof(int)), GFP_KERNEL);
	if (envregs == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		goto err;
	}
	penv->bcmpmu = bcmpmu;
	penv->env_regs = envregs;
	penv->bcmpmu->update_env_status = bcmpmu_update_env_status;
	penv->bcmpmu->get_env_bit_status = bcmpmu_get_env_bit_status;
	penv->bcmpmu->is_env_bit_set = bcmpmu_is_env_bit_set;
	bcmpmu->envinfo = penv;

	pfg = kzalloc(sizeof(struct bcmpmu_fg), GFP_KERNEL);
	if (pfg == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		goto err;
	}
	pfg->bcmpmu = bcmpmu;
	if (pdata->fg_smpl_rate)
		pfg->fg_smpl_cnt_tm = 1000000 / pdata->fg_smpl_rate;
	else
		pfg->fg_smpl_cnt_tm = 1000000 / 2083;
	if (pdata->fg_slp_rate)
		pfg->fg_slp_cnt_tm = 1000000 / pdata->fg_slp_rate;
	else
		pfg->fg_slp_cnt_tm = 1000000 / 32000;
	if (pdata->fg_slp_curr_ua)
		pfg->fg_slp_curr_ua = pdata->fg_slp_curr_ua;
	else
		pfg->fg_slp_curr_ua = 1000;
	if (pdata->fg_sns_res)
		pfg->fg_sns_res = pdata->fg_sns_res;
	else
		pfg->fg_sns_res = 10;	/* default sense resistor */
	if (pdata->fg_factor)
		pfg->fg_factor = pdata->fg_factor;
	else
		pfg->fg_factor = 1000;

	pfg->bcmpmu->fg_currsmpl = bcmpmu_get_fg_currsmpl;
	pfg->bcmpmu->fg_vmbatt = bcmpmu_get_fg_vmbatt;
	pfg->bcmpmu->fg_acc_mas = bcmpmu_get_fg_acc_mas;
	pfg->bcmpmu->fg_enable = bcmpmu_fg_enable;
	pfg->bcmpmu->fg_reset = bcmpmu_fg_reset;
	pfg->bcmpmu->fg_offset_cal = bcmpmu_fg_offset_cal;
	pfg->bcmpmu->fg_offset_cal_read = bcmpmu_fg_offset_cal_read;
	pfg->bcmpmu->fg_trim_write = bcmpmu_fg_trim_write;
	bcmpmu->fginfo = pfg;

	if (pdata->support_fg) {
		bcmpmu_fg_enable(bcmpmu, 1);
		bcmpmu_fg_offset_cal(bcmpmu);
	}

	padc->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(padc->hwmon_dev)) {
		ret = PTR_ERR(padc->hwmon_dev);
		dev_err(&pdev->dev, "Class registration failed (%d)\n", ret);
		goto exit_remove_files;
	}
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_hwmon_attr_group);
	if (ret != 0)
		goto exit_remove_files;

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY, adc_isr, padc);
	/*bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_UPPER, adc_isr, padc);
	 bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_IGNORE, adc_isr, padc);
	 bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN, adc_isr, padc); */

	/*bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_EOC); */
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY);
	/*bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_UPPER);
	   bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_IGNORE);
	   bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN); */

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = device_create_file(&pdev->dev, &dev_attr_dbgmsk);
	ret = device_create_file(&pdev->dev, &dev_attr_fg_status);
	ret = device_create_file(&pdev->dev, &dev_attr_fg_factor);
#endif
	return ret;

exit_remove_files:
	sysfs_remove_group(&padc->hwmon_dev->kobj, &bcmpmu_hwmon_attr_group);
err:
	kfree(padc);
	kfree(penv);
	kfree(pfg);
	kfree(envregs);
	return ret;
}

static int __devexit bcmpmu_hwmon_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY);
	/*bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_UPPER);
	   bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_IGNORE);
	   bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN); */
	return 0;
}

static struct platform_driver bcmpmu_adc_driver = {
	.driver = {
		   .name = "bcmpmu_hwmon",
		   },
	.probe = bcmpmu_hwmon_probe,
	.remove = __devexit_p(bcmpmu_hwmon_remove),
};

static int __init adc_init(void)
{
	return platform_driver_register(&bcmpmu_adc_driver);
}

subsys_initcall(adc_init);

static void __exit adc_exit(void)
{
	platform_driver_unregister(&bcmpmu_adc_driver);
}

module_exit(adc_exit);

MODULE_DESCRIPTION("BCM PMIC adc driver");
MODULE_LICENSE("GPL");

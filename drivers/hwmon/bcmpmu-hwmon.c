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

#define SENSE_RES_COEF		97 /* for sense resistor 10m Ohm */
#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)
static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define pr_hwmon(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

static struct bcmpmu_adc_cal adc_cal[PMU_ADC_MAX] = {
	[PMU_ADC_VMBATT] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_VBBATT] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_VWALL] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_VBUS] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_ID] =		{.gain = 1024, .offset = 0},
	[PMU_ADC_NTC] =		{.gain = 1024, .offset = 0},
	[PMU_ADC_BSI] =		{.gain = 1024, .offset = 0},
	[PMU_ADC_32KTEMP] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_PATEMP] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_ALS] =		{.gain = 1024, .offset = 0},
	[PMU_ADC_RTM] =		{.gain = 1024, .offset = 0},
	[PMU_ADC_FG_CURRSMPL] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_FG_RAW] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_FG_VMBATT] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_BSI_CAL_LO] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_BSI_CAL_HI] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_NTC_CAL_LO] =	{.gain = 1024, .offset = 0},
	[PMU_ADC_NTC_CAL_HI] =	{.gain = 1024, .offset = 0},
};

struct bcmpmu_adc {
	struct device *hwmon_dev;
	struct bcmpmu *bcmpmu;
	const struct bcmpmu_adc_map *adcmap;
	const struct bcmpmu_reg_map *ctrlmap;
	struct bcmpmu_adc_setting *adcsetting;
	const struct bcmpmu_temp_map *btmap;
	int btmap_len;
	wait_queue_head_t wait;
	struct mutex lock;
	struct list_head list;
	bool rtmbusy;
	struct bcmpmu_adc_req *rtmreq;
};

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

static int adc_map_batt_temp(struct bcmpmu_adc *padc, int adc)
{
	int i = 0;
	int temp = padc->btmap[i].temp;
	int index;
	for (i = 0; i < padc->btmap_len; i++) {
		if ((adc <= padc->btmap[i].adc) &&
			(adc > padc->btmap[i+1].adc)) {
			index = ((padc->btmap[i].adc - adc) * 1000)/
				(padc->btmap[i].adc - padc->btmap[i+1].adc);
			temp = padc->btmap[i].temp +
				((padc->btmap[i+1].temp - padc->btmap[i].temp) * index)/1000;
			break;
		}
	}
	return temp;
}

static int read_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	int ret = 0;
	unsigned int val;
	unsigned int val1;
	unsigned int values[2];
	struct bcmpmu_adc_map adcmap;
	req->raw = 0;
	
	if (req->sig >= PMU_ADC_MAX) return -EINVAL;
	switch (req->tm) {
	case PMU_ADC_TM_HK:
		adcmap = padc->adcmap[req->sig];
		break;
	case PMU_ADC_TM_RTM_TX:
	case PMU_ADC_TM_RTM_RX:
	case PMU_ADC_TM_RTM_SW:
		adcmap = padc->adcmap[PMU_ADC_RTM];
		break;
	case PMU_ADC_TM_MAX:
	default:
		return  -EINVAL;
	}
		
	if ((adcmap.addr0 == 0) && (adcmap.addr1 == 0)) {
		pr_hwmon(ERROR, "%s: sig map failed\n",__func__);
		return -EINVAL;
	} else if (adcmap.addr0 == adcmap.addr1 + 1) {
		ret = padc->bcmpmu->read_dev_bulk(padc->bcmpmu,
			adcmap.map, adcmap.addr1, values, 2);

		val = values[0];
		val <<= 8;
		val |= values [1];
		pr_hwmon (DATA, "%s: Signal %d, value[0] %x, value[1] %x ==> val %x", __func__, req->sig, values[0], values[1], val);
	} else {
		ret = padc->bcmpmu->read_dev_drct(padc->bcmpmu,
			adcmap.map, adcmap.addr1, &val, adcmap.dmask|adcmap.vmask);
		if (ret != 0) {
			pr_hwmon(ERROR, "%s: read adc add1 failed\n",__func__);
			return -EINVAL;
		}
		if (adcmap.addr0 != adcmap.addr1) {
			ret = padc->bcmpmu->read_dev_drct(padc->bcmpmu,
				adcmap.map, adcmap.addr0, &val1, adcmap.dmask|adcmap.vmask);
			if (ret != 0) {
				pr_hwmon(ERROR, "%s: read adc add0 failed\n",__func__);
				return -EINVAL;
			}
			val = (val1 & 0xFF) | ((val << 8) & 0xFF00);
		}
	}
	if ((req->tm == PMU_ADC_TM_HK) &&
		((val & adcmap.vmask) != 0)) {
		req->raw = -EINVAL;
		pr_hwmon(FLOW, "%s: adc result invalid\n",__func__);
		return ret;
	}
	req->raw = val & adcmap.dmask;
	return ret;
}

static int adc_adjust_curr(struct bcmpmu_adc *padc, int raw_curr)
{
	unsigned short ucurr = (unsigned short)raw_curr;
	short curr = (short) ucurr; 
	return (int)(curr/4);
}

static void cal_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	req->cal = req->raw * adc_cal[req->sig].gain;
	req->cal /= 1024;
	req->cal += adc_cal[req->sig].offset;
}

static void cnv_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	switch(req->sig) {
		case PMU_ADC_VMBATT:
		case PMU_ADC_VBBATT:
		case PMU_ADC_VBUS:
		case PMU_ADC_ID:
		case PMU_ADC_FG_VMBATT:
			req->cnv = (req->cal * padc->adcmap[req->sig].vrng)/1024;
			break;	
		case PMU_ADC_NTC:
			req->cnv = adc_map_batt_temp(padc, req->cal);
			break;
		case PMU_ADC_FG_CURRSMPL:
			req->cnv = adc_adjust_curr(padc, req->cal);
			break;
		case PMU_ADC_BSI:
		case PMU_ADC_32KTEMP:
		default:
			req->cnv = req->cal;
			break;
	}
}

static int update_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	int ret;
	req->raw = -EINVAL;
	if (req->sig == PMU_ADC_FG_CURRSMPL || req->sig == PMU_ADC_FG_RAW) {
		ret = padc->bcmpmu->write_dev(padc->bcmpmu,
			PMU_REG_FG_FRZSMPL,
			padc->bcmpmu->regmap[PMU_REG_FG_FRZSMPL].mask,
			padc->bcmpmu->regmap[PMU_REG_FG_FRZSMPL].mask);
		if (ret != 0) return ret;
	}

	while (req->raw == -EINVAL) {
		ret = read_adc_result(padc, req);
		if (ret != 0) return ret;
	};
	cal_adc_result(padc, req);
	cnv_adc_result(padc, req);
	pr_hwmon(DATA, "%s: result sig=%d, raw=0x%X, cal=0x%X, cnv=%d\n",
		__func__, req->sig, req->raw, req->cal, req->cnv);

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
		mutex_lock(&padc->lock);
		if (!list_empty(&padc->rtmreq->list))
			list_del_init(&padc->rtmreq->list);
		padc->rtmreq->ready = 1;
		mutex_unlock(&padc->lock);
		wake_up(&padc->wait);

		if (!list_empty(&padc->list)) {
			padc->rtmbusy = true;
			padc->rtmreq = list_entry(padc->list.next,
				struct bcmpmu_adc_req, list);
			/* config hw for rtm adc */
			if (padc->rtmreq->tm == PMU_ADC_TM_RTM_TX) {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
					padc->ctrlmap[PMU_ADC_RTM_DLY].map,
					padc->ctrlmap[PMU_ADC_RTM_DLY].addr,
					padc->adcsetting->tx_delay <<
						padc->ctrlmap[PMU_ADC_RTM_DLY].shift,
					padc->ctrlmap[PMU_ADC_RTM_DLY].mask);
					bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_TX);
			} else {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
					padc->ctrlmap[PMU_ADC_RTM_DLY].map,
					padc->ctrlmap[PMU_ADC_RTM_DLY].addr,
					padc->adcsetting->rx_delay <<
						padc->ctrlmap[PMU_ADC_RTM_DLY].shift,
					padc->ctrlmap[PMU_ADC_RTM_DLY].mask);
					bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_RX);
			}
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
				padc->ctrlmap[PMU_ADC_RTM_SEL].map,
				padc->ctrlmap[PMU_ADC_RTM_SEL].addr,
				padc->adcmap[padc->rtmreq->sig].rtmsel <<
					padc->ctrlmap[PMU_ADC_RTM_SEL].shift,
				padc->ctrlmap[PMU_ADC_RTM_SEL].mask);
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
				padc->ctrlmap[PMU_ADC_RTM_MASK].map,
				padc->ctrlmap[PMU_ADC_RTM_MASK].addr, 0,
				padc->ctrlmap[PMU_ADC_RTM_MASK].mask);
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
				padc->ctrlmap[PMU_ADC_RTM_START].map,
				padc->ctrlmap[PMU_ADC_RTM_START].addr,
				padc->ctrlmap[PMU_ADC_RTM_START].mask,
				padc->ctrlmap[PMU_ADC_RTM_START].mask);
			pr_hwmon(FLOW, "%s: start rtm adc\n", __func__);
		} else
			padc->rtmbusy = false;
		
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

static int bcmpmu_adc_request(struct bcmpmu *bcmpmu,
				struct bcmpmu_adc_req *req)
{
	struct bcmpmu_adc *padc = bcmpmu->adcinfo;
	int ret = -EINVAL;

	switch (req->tm) {
	case PMU_ADC_TM_HK:
		ret =  update_adc_result(padc, req);
		break;
	case PMU_ADC_TM_RTM_TX:
	case PMU_ADC_TM_RTM_RX:
	case PMU_ADC_TM_RTM_SW:
		mutex_lock(&padc->lock);
		list_add_tail(&req->list, &padc->list);
		if ((!list_empty(&padc->list)) &&
			(!padc->rtmbusy)) {
			padc->rtmbusy = true;
			padc->rtmreq = list_entry(padc->list.next,
				struct bcmpmu_adc_req, list);
			/* config hw for rtm adc */
			if (req->tm == PMU_ADC_TM_RTM_TX) {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
					padc->ctrlmap[PMU_ADC_RTM_DLY].map,
					padc->ctrlmap[PMU_ADC_RTM_DLY].addr,
					padc->adcsetting->tx_delay <<
						padc->ctrlmap[PMU_ADC_RTM_DLY].shift,
					padc->ctrlmap[PMU_ADC_RTM_DLY].mask);
					bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_TX);
			} else {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
					padc->ctrlmap[PMU_ADC_RTM_DLY].map,
					padc->ctrlmap[PMU_ADC_RTM_DLY].addr,
					padc->adcsetting->rx_delay <<
						padc->ctrlmap[PMU_ADC_RTM_DLY].shift,
					padc->ctrlmap[PMU_ADC_RTM_DLY].mask);
					bcmpmu_sel_adcsync(PMU_ADC_TM_RTM_RX);
			}
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
				padc->ctrlmap[PMU_ADC_RTM_SEL].map,
				padc->ctrlmap[PMU_ADC_RTM_SEL].addr,
				padc->adcmap[req->sig].rtmsel <<
					padc->ctrlmap[PMU_ADC_RTM_SEL].shift,
				padc->ctrlmap[PMU_ADC_RTM_SEL].mask);
			padc->bcmpmu->write_dev_drct(padc->bcmpmu,
				padc->ctrlmap[PMU_ADC_RTM_MASK].map,
				padc->ctrlmap[PMU_ADC_RTM_MASK].addr, 0,
				padc->ctrlmap[PMU_ADC_RTM_MASK].mask);
			if (req->tm == PMU_ADC_TM_RTM_SW) {
				padc->bcmpmu->write_dev_drct(padc->bcmpmu,
					padc->ctrlmap[PMU_ADC_RTM_MASK].map,
					padc->ctrlmap[PMU_ADC_RTM_START].addr,
					padc->ctrlmap[PMU_ADC_RTM_START].mask,
					padc->ctrlmap[PMU_ADC_RTM_START].mask);
			}
			pr_hwmon(FLOW, "%s: start rtm adc\n", __func__);
		}
		mutex_unlock(&padc->lock);
		
		req->ready = 0;
		if (wait_event_interruptible_timeout(padc->wait, req->ready, 2000) == 0) {
			mutex_lock(&padc->lock);
			if (!list_empty(&padc->rtmreq->list))
				list_del_init(&padc->rtmreq->list);
			padc->rtmbusy = false;
			mutex_unlock(&padc->lock);
			return ret;
		}
		ret =  update_adc_result(padc, req);
		break;
	case PMU_ADC_TM_MAX:
	default:
		ret = -EINVAL;
	}
	return ret;
}

static ssize_t show_vmbatt(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_vbbatt(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_VBBATT;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_vbus(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_VBUS;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_id(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_ID;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_ntc(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_NTC;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_bsi(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_BSI;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}
static ssize_t show_32ktemp(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_32KTEMP;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}

static ssize_t show_fg_vmbatt(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_FG_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}

static ssize_t show_fg_currsmpl(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_FG_CURRSMPL;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	return	sprintf(buf, "%d\n", req.cnv);
}

static ssize_t show_envupdate(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int i;
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	unsigned long status;
	bcmpmu->update_env_status(bcmpmu, &status);

	for (i=0; i<penv->env_size; i++) {
		sprintf(buf, "%8X\n", penv->env_regs[i]);
		buf += 9;
	}
	return penv->env_size * 9;
}

static ssize_t show_fg_acc_mas(struct device *dev, struct device_attribute *attr,
				char *buf)
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

static void bcmpmu_update_env_status(struct bcmpmu *bcmpmu, unsigned long *env_status)
{
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	bcmpmu->read_dev_bulk(bcmpmu, bcmpmu->regmap[PMU_REG_ENV1].map,
		bcmpmu->regmap[PMU_REG_ENV1].addr, penv->env_regs, penv->env_size);
}

static bool bcmpmu_is_env_bit_set(struct bcmpmu *bcmpmu, enum bcmpmu_env_bit_t env_bit)
{
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	int index;
	if (penv->envregmap[env_bit].regmap.mask == 0) return false;
	index = penv->envregmap[env_bit].regmap.addr - bcmpmu->regmap[PMU_REG_ENV1].addr;
	if (penv->env_regs[index] & penv->envregmap[index].regmap.mask)
		return true;
	else
		return false;
}

static bool bcmpmu_get_env_bit_status(struct bcmpmu *bcmpmu, enum bcmpmu_env_bit_t env_bit)
{
	struct bcmpmu_env *penv = bcmpmu->envinfo;
	unsigned int val;
	if (penv->envregmap[env_bit].regmap.mask == 0) return false;
	bcmpmu->read_dev_drct(bcmpmu, penv->envregmap[env_bit].regmap.map,
		penv->envregmap[env_bit].regmap.addr, &val, penv->envregmap[env_bit].regmap.mask);
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
	ret = bcmpmu->adc_req(bcmpmu, &req);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to get adc result.\n", __func__);
		return ret;
	}
	curr = req.cnv;
	*data = (curr * pfg->fg_factor)/1000;
	return	ret;
}

static int bcmpmu_get_fg_vmbatt(struct bcmpmu *bcmpmu, int *data)
{
	int ret;
	struct bcmpmu_adc_req req;
	req.sig = PMU_ADC_FG_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	ret = bcmpmu->adc_req(bcmpmu, &req);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to get adc result.\n", __func__);
		return ret;
	}
	*data = req.cnv;
	return	ret;
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
			PMU_REG_FG_ACCM0,
			&acc0,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc0.\n", __func__);
		return ret;
	}

	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_ACCM1,
			&acc1,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc1.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_ACCM2,
			&acc2,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc2.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_ACCM3,
			&acc3,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg acc3.\n", __func__);
		return ret;
	}
	
	if ((acc3 & 0x80) == 0) {
		pr_hwmon(DATA, "%s fg data invalid.\n", __func__);
		return -EINVAL;
	}
	acc3 = acc3 & 0x03;
	if (acc3 >= 2)	acc3 = acc3 | 0xFC;
	
	acc = (int) (acc0 | (acc1 << 8) | (acc2 << 16) | (acc3 << 24));
	pfg->fg_acc = acc;
	pr_hwmon(DATA, "%s: acc=%ld, acc3=%X acc2=%X acc1=%X acc0=%X\n",
		__func__, acc, acc3, acc2, acc1, acc0);

	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_CNT0,
			&cnt0,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg cnt0.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_CNT1,
			&cnt1,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg cnt1.\n", __func__);
		return ret;
	}
	cnt = cnt0 | (cnt1 << 8);
	pfg->fg_smpl_cnt = cnt;

	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_SLEEPCNT0,
			&slpcnt0,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg slpcnt0.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_SLEEPCNT1,
			&slpcnt1,
			PMU_BITMASK_ALL);
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
	actacc = actacc + slpacc;
	actacc = div_s64(actacc, 1000000);
	*data = (int) actacc;
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
			PMU_REG_FG_OFFSET0,
			&off0,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg offset0.\n", __func__);
		return ret;
	}
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_OFFSET1,
			&off1,
			PMU_BITMASK_ALL);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s failed to read fg offset1.\n", __func__);
		return ret;
	}
	off = off0 | (off1 << 8);
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
	return sprintf(buf, "Fuel Gauge Status\n \
	fg_acc=%d\n fg_smpl_cnt=%d\n fg_slp_cnt=%d\n \
	fg_smpl_cnt_tm=%d\n fg_slp_cnt_tm=%d\n \
	fg_slp_curr_ua=%d\n fg_sns_res=%d\n fg_factor=%d\n",
	pfg->fg_acc,
	pfg->fg_smpl_cnt,
	pfg->fg_slp_cnt,
	pfg->fg_smpl_cnt_tm,
	pfg->fg_slp_cnt_tm,
	pfg->fg_slp_curr_ua,
	pfg->fg_sns_res,
	pfg->fg_factor);
}
static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_store);
static DEVICE_ATTR(fg_status, 0644, fg_status_show, NULL);
#endif

static int __devinit bcmpmu_hwmon_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	struct bcmpmu_adc *padc;
	struct bcmpmu_env *penv;
	struct bcmpmu_fg *pfg;
	int *envregs;
	
	pr_hwmon(INIT, "%s: called\n", __func__);

	padc = kzalloc(sizeof(struct bcmpmu_adc), GFP_KERNEL);
	if (padc == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&padc->wait);
	mutex_init(&padc->lock);
	INIT_LIST_HEAD(&padc->list);
	padc->rtmbusy = false;
	padc->bcmpmu = bcmpmu;
	padc->adcmap = bcmpmu_get_adcmap();
	padc->ctrlmap = bcmpmu_get_adc_ctrl_map();
	padc->btmap = pdata->batt_temp_map;
	padc->btmap_len = pdata->batt_temp_map_len;
	padc->rtmreq = NULL;
	padc->adcsetting = pdata->adc_setting;
	bcmpmu->adcinfo = (void *)padc;
	bcmpmu->adc_req = bcmpmu_adc_request;

	penv = kzalloc(sizeof(struct bcmpmu_env), GFP_KERNEL);
	if (penv == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	penv->envregmap = bcmpmu_get_envregmap(&penv->env_size);
	envregs = kzalloc((penv->env_size * sizeof(int)), GFP_KERNEL);
	if (envregs == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
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
		return -ENOMEM;
	}
	pfg->bcmpmu = bcmpmu;
	if (pdata->fg_smpl_rate)
		pfg->fg_smpl_cnt_tm = 1000000/pdata->fg_smpl_rate;
	else
		pfg->fg_smpl_cnt_tm = 1000000/2083;
	if (pdata->fg_slp_rate)
		pfg->fg_slp_cnt_tm = 1000000/pdata->fg_slp_rate;
	else
		pfg->fg_slp_cnt_tm = 1000000/32000;
	if (pdata->fg_slp_curr_ua)
		pfg->fg_slp_curr_ua = pdata->fg_slp_curr_ua;
	else
		pfg->fg_slp_curr_ua = 1000;
	if (pdata->fg_sns_res)
		pfg->fg_sns_res = pdata->fg_sns_res;
	else
		pfg->fg_sns_res = 10; /* default sense resistor */
	if (pdata->fg_factor)
		pfg->fg_factor = pdata->fg_factor;
	else
		pfg->fg_factor = 1000;

	pfg->fg_factor = pfg->fg_factor * SENSE_RES_COEF;
	pfg->fg_factor = pfg->fg_factor/pfg->fg_sns_res;
	pfg->fg_factor = pfg->fg_factor/10;

	pfg->bcmpmu->fg_currsmpl = bcmpmu_get_fg_currsmpl;
	pfg->bcmpmu->fg_vmbatt = bcmpmu_get_fg_vmbatt;
	pfg->bcmpmu->fg_acc_mas = bcmpmu_get_fg_acc_mas;
	pfg->bcmpmu->fg_enable = bcmpmu_fg_enable;
	pfg->bcmpmu->fg_reset = bcmpmu_fg_reset;
	pfg->bcmpmu->fg_offset_cal = bcmpmu_fg_offset_cal;
	pfg->bcmpmu->fg_offset_cal_read = bcmpmu_fg_offset_cal_read;
	pfg->bcmpmu->fg_trim_write = bcmpmu_fg_trim_write;
	bcmpmu->fginfo = pfg;

	padc->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(padc->hwmon_dev)) {
		ret = PTR_ERR(padc->hwmon_dev);
		dev_err(&pdev->dev, "Class registration failed (%d)\n", ret);
		goto exit_remove_files;
	}
	ret = sysfs_create_group(&pdev->dev.kobj,
				&bcmpmu_hwmon_attr_group);
	if (ret != 0)
		goto exit_remove_files;

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY, adc_isr, padc);
	/*bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_UPPER, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_IGNORE, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN, adc_isr, padc);*/

	/*bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_EOC);*/
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY);
	/*bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_UPPER);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_IGNORE);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN);*/

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = device_create_file(&pdev->dev, &dev_attr_dbgmsk);
	ret = device_create_file(&pdev->dev, &dev_attr_fg_status);
#endif
	return 0;

exit_remove_files:
	sysfs_remove_group(&padc->hwmon_dev->kobj, &bcmpmu_hwmon_attr_group);
	return ret;
}

static int __devexit bcmpmu_hwmon_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY);
	/*bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_UPPER);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_IGNORE);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN);*/
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
module_init(adc_init);

static void __exit adc_exit(void)
{
	platform_driver_unregister(&bcmpmu_adc_driver);
}
module_exit(adc_exit);

MODULE_DESCRIPTION("BCM PMIC adc driver");
MODULE_LICENSE("GPL");

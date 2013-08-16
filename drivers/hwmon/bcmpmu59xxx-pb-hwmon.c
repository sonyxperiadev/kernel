/*****************************************************************************
 *  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#define PMU_TEMP_MULTI_CONST 497
#define KELVIN_CONST 276

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define pr_hwmon(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

#define BCMPMU_ADC_RESOLUTION		(1024)

static inline int INTERPOLATE_LINEAR(int x, int x1, int y1, int x2, int y2)
{
	return (y1) + ((((y2) - (y1)) * ((x) - (x1))) / ((x2) - (x1)));
}

/* Temp */
static struct bcmpmu59xxx *gbcmpmu;
/**
 * private data
 */
struct bcmpmu_adc {
	struct device *hwmon_dev;
	struct mutex chann_mutex[PMU_ADC_CHANN_MAX]; /* per channel mutex */
	struct mutex rtm_mutex;
	struct bcmpmu_adc_pdata *pdata;
};

static int return_index(const struct bcmpmu_adc_lut *slist,
						int len, int element)
{
	int low = 0;
	int high = len - 1;
	while (low <= high) {
		int middle = low + (high - low)/2;

		if (element > slist[middle].raw)
			low = middle + 1;
		else if (element < slist[middle].raw)
			high = middle - 1;
		else
			return middle;
	}
	if (element >= slist[low].raw && element < slist[high].raw)
		return low;
	else
		return high;
}
/**
 * convert ADC @raw value to units
 * if PMU_ADC_FLAG_CONV_LUT (not look-up table) is not set ADC
 * conversion is done using Formula:
 * (raw / BCMPMU_ADC_RESOLUTION) * volt_range + offset
 *
 * If there is a lut, we will do linear interpolation to convert a value
 */
static int bcmpmu_adc_convert(struct bcmpmu59xxx *bcmpmu,
				enum bcmpmu_adc_channel channel,
					struct bcmpmu_adc_result *result)
{
	struct bcmpmu_adc *adc = (struct bcmpmu_adc *)bcmpmu->adc;
	struct bcmpmu_adc_lut *lut;
	int index = 0;
	int len = 0;

	if (adc->pdata[channel].lut) {
		len = adc->pdata[channel].lut_len;
		lut = adc->pdata[channel].lut;
		index = return_index(lut, len, result->raw);
		if (result->raw < lut[0].raw ||
					result->raw > lut[len - 1].raw) {
			pr_hwmon(ERROR, "ADC out of range, raw = %d\n",
								result->raw);
			result->conv = 0;
			return -EINVAL;
		}
		result->conv = INTERPOLATE_LINEAR(result->raw,
					lut[index].raw, lut[index].map,
					lut[index + 1].raw, lut[index + 1].map);

		pr_hwmon(FLOW, "index = %d, raw = %d, map = %d\n",
				index, lut[index].raw, lut[index].map);
		pr_hwmon(FLOW, "%s channel:%d raw = %x conv_lut = %d\n",
				__func__, channel, result->raw, result->conv);
		return 0;
	}
	if (channel != PMU_ADC_CHANN_DIE_TEMP) {
		result->conv = (result->raw * adc->pdata[channel].volt_range) /
					BCMPMU_ADC_RESOLUTION +
					(adc->pdata[channel].adc_offset);
	} else {
		/* temp = raw * 0.497 - 275.7 */
		result->conv =
				((result->raw * PMU_TEMP_MULTI_CONST) / 1000)
								- KELVIN_CONST;
	}

	pr_hwmon(FLOW, "%s channel:%d raw = %x conv_formula = %d\n",
			__func__, channel, result->raw, result->conv);
	return 0;
}
/**
 * Read RTM ADC from PMU.
 * Return: 0 on SUCCESS
 *	   -1 Error
 *	   -EAGAIN on failure.
 * */

int read_rtm_adc(struct bcmpmu59xxx *bcmpmu, enum bcmpmu_adc_channel channel,
					struct bcmpmu_adc_result *result) {
	struct bcmpmu_adc *adc = (struct bcmpmu_adc *)bcmpmu->adc;
	unsigned char rtm_read[ADC_READ_LEN] = {0, 0};
	u8 val = 0;
	int ret = 0;
	int poll;

	pr_hwmon(FLOW, "%s channel = %d\n", __func__, channel);
	if (channel >= PMU_ADC_CHANN_MAX || channel == PMU_ADC_CHANN_RESERVED)
		return -EINVAL;

	mutex_lock(&adc->rtm_mutex);

	val = channel << ADC_RTM_CHANN_SHIFT;
	val |= (ADC_RTM_CONV_ENABLE << ADC_RTM_CONVERSION_SHIFT);
	val |= (ADC_RTM_START << ADC_RTM_START_SHIFT);
	val |= ADC_RTM_MAX_RST_CNT;

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADCCTRL1, val);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s I2C write failed\n", __func__);
		goto err;
	}

	for (poll = 0; poll < ADC_RTM_MAX_POLL; poll++) {
		msleep(ADC_RTM_SLEEP);
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_INT9, &val);
		if (ret != 0) {
			pr_hwmon(ERROR, "%s I2C read failed\n", __func__);
			goto err;
		} else {
			if (val & ADC_RTM_DATA_READY) {
				bcmpmu->write_dev(bcmpmu,
						PMU_REG_INT9,
						(val & ~ADC_RTM_DATA_READY));
				break;
			}
		}
	}

	if (poll == (ADC_RTM_MAX_POLL - 1)) {
		pr_hwmon(ERROR, "%s: exceeded max polls\n", __func__);
		goto err;
	}


	ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL27, rtm_read,
								ADC_READ_LEN);
	if (ret != 0) {
		pr_hwmon(ERROR, "%s I2C read failed :%d\n", __func__, __LINE__);
		goto err;
	}

	if (rtm_read[0] & ADC_READ_INVALID) {
		pr_hwmon(ERROR, "%s RTM read INVALID, try again\n", __func__);
		goto err;

	} else {
		/* clear all except 0, 1 bits*/
		result->raw = (rtm_read[0] & ADC_MSB_DATA);
		result->raw = (result->raw << ADC_MSB_SHIFT);
		result->raw |= rtm_read[1];
	}
	pr_hwmon(FLOW, "%s channel:%d, raw:%x\n", __func__, channel,
								result->raw);
	mutex_unlock(&adc->rtm_mutex);
	return 0;
err:
	mutex_unlock(&adc->rtm_mutex);
	return -EAGAIN;

}
/**
 * Read SAR ADC from PMU.
 * Return: 0 on SUCCESS
 *	   -1 Error
 *	   -EAGAIN on failure.
 * */

int read_sar_adc(struct bcmpmu59xxx *bcmpmu, enum bcmpmu_adc_channel channel,
					struct bcmpmu_adc_result *result)
{
	struct bcmpmu_adc *adc = (struct bcmpmu_adc *)bcmpmu->adc;
	unsigned char val[ADC_READ_LEN];
	int ret = 0;

	pr_hwmon(FLOW, "%s channel = %d\n", __func__, channel);

	switch (channel) {
	case PMU_ADC_CHANN_VMBATT:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_VMBATT]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL3,
				val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_VMBATT]);
		break;

	case PMU_ADC_CHANN_VBBATT:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_VBBATT]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL5,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_VBBATT]);
		break;

	case PMU_ADC_CHANN_VBUS:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_VBUS]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL9,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_VBUS]);
		break;

	case PMU_ADC_CHANN_IDIN:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_IDIN]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL11,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_IDIN]);
		break;

	case PMU_ADC_CHANN_BSI:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_BSI]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL15,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_BSI]);
		break;

	case PMU_ADC_CHANN_BOM:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_BOM]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL17,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_BOM]);
		break;

#if defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV00)
	/* None of these thermisters are on Logan R00 */
	case PMU_ADC_CHANN_NTC:
	case PMU_ADC_CHANN_32KTEMP:
	case PMU_ADC_CHANN_ALS:

	/* The only thermister on Logan R00 */
	case PMU_ADC_CHANN_PATEMP:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_PATEMP]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL21,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_PATEMP]);
		break;

#elif defined(CONFIG_MACH_HAWAII_SS_LOGAN) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_CS02_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVE_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVE_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_COMBINED) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_COMBINED) \
	|| defined(CONFIG_MACH_JAVA_SS_EVAL)

	/* Logan R01 and future */
	case PMU_ADC_CHANN_NTC:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_NTC]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL13,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_NTC]);
		break;

	case PMU_ADC_CHANN_32KTEMP:
	case PMU_ADC_CHANN_ALS:
	case PMU_ADC_CHANN_PATEMP:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_PATEMP]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL21,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_PATEMP]);
		break;

#else
	case PMU_ADC_CHANN_NTC:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_NTC]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL13,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_NTC]);
		break;

	case PMU_ADC_CHANN_32KTEMP:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_32KTEMP]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL19,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_32KTEMP]);
		break;

	case PMU_ADC_CHANN_ALS:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_ALS]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL23,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_ALS]);
		break;

	case PMU_ADC_CHANN_PATEMP:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_PATEMP]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL21,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_PATEMP]);
		break;
#endif

	case PMU_ADC_CHANN_DIE_TEMP:
		mutex_lock(&adc->chann_mutex[PMU_ADC_CHANN_DIE_TEMP]);
		ret = bcmpmu->read_dev_bulk(bcmpmu, PMU_REG_ADCCTRL25,
						val, ADC_READ_LEN);
		mutex_unlock(&adc->chann_mutex[PMU_ADC_CHANN_DIE_TEMP]);
		break;

	default:
		pr_hwmon(ERROR, "INVALID SAR ADC read request\n");
		return -EINVAL;


	}

	if (ret != 0) {
		pr_hwmon(ERROR, "%s I2C bulk read failed\n", __func__);
		return -EAGAIN;
	}

	if (val[0] & ADC_READ_INVALID) {
		pr_hwmon(ERROR, "%s SAR ADC read fail, try again\n", __func__);
		return -EAGAIN;
	} else {
		/* clear all except 0, 1 bits*/
		result->raw = (val[0] & ADC_MSB_DATA);
		result->raw = (result->raw << ADC_MSB_SHIFT);
		result->raw |= val[1];
	}

	pr_hwmon(FLOW, "%s channel:%d, raw:%x\n", __func__, channel,
								result->raw);
	return 0;

}

/**
 * bcmpmu_adc_read - Read ADC sample
 *
 * @bcmpmu59xxx:		Pointer to bcmpmu59xxx struct
 * @channel:			ADC channel
 * @req:			Request type
 * @result:			ADC result
 *
 *  Read the ADC channel and return the ADC value in @result. @return->raw
 *  contains ADC raw value (10-bit SAR before any conversion formula is
 *  applied), while @result->conv contains converted value for ADC
 *
 *  Each channel is protected by mutex i.e at max only one read can be pending
 *  for specific channel. While this will protect individual channel reading,
 *  it will block the reading the other channel
 *
 * RETURNS
 * function returns 0 on success and a negative value on error. if function
 * returns -EAGAIN, then called can retry again or give up
 */

int bcmpmu_adc_read(struct bcmpmu59xxx *bcmpmu, enum bcmpmu_adc_channel channel,
		enum bcmpmu_adc_req req, struct bcmpmu_adc_result *result)
{
	int ret = 0;
	if (channel < PMU_ADC_CHANN_VMBATT || channel >= PMU_ADC_CHANN_MAX) {
		pr_hwmon(ERROR, "%s channel:%d out of range\n",
							__func__, channel);
		return -EINVAL;
	}

	if (result == NULL) {
		pr_hwmon(ERROR, "%s result struct is NULL\n", __func__);
		return -EINVAL;
	}

	if (req == PMU_ADC_REQ_SAR_MODE) {
		ret = read_sar_adc(bcmpmu, channel, result);
		if (ret != 0)
			return -EAGAIN;
	}
	if (req == PMU_ADC_REQ_RTM_MODE) {
		ret = read_rtm_adc(bcmpmu, channel, result);
		if (ret != 0)
			return -EAGAIN;
	}

	bcmpmu_adc_convert(bcmpmu, channel, result);

	return 0;
}
EXPORT_SYMBOL(bcmpmu_adc_read);

/*
 *sysfs hook function
 */
static ssize_t adc_read(struct device *dev,
			struct device_attribute *devattr, char *buf) {

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct bcmpmu59xxx *bcmpmu = gbcmpmu;
	struct bcmpmu_adc_result result;
	int ret = 0;
	int len = 0;

	len += sprintf(buf + len, "SAR READ OF channel = %d", attr->index);
	ret = bcmpmu_adc_read(bcmpmu, attr->index,
						PMU_ADC_REQ_SAR_MODE, &result);
	if (ret < 0)
		return ret;
	/* Powersupply frame work expects temperature to be
	 * reported in Tenth multiple of centigrade. so we are maintaning
	 * the temperature look up table in same manner in board file.
	 * But reporting the same to user confuses.
	 * so again dividing by 10 and reporting*/
	if ((attr->index == PMU_ADC_CHANN_NTC) ||
			(attr->index == PMU_ADC_CHANN_32KTEMP) ||
			(attr->index == PMU_ADC_CHANN_PATEMP))
		result.conv = result.conv / 10;

	len += sprintf(buf + len, " raw = 0x%x conv = %d\n",
						result.raw, result.conv);

	len += sprintf(buf + len, "RTM READ OF channel = %d", attr->index);
	ret = bcmpmu_adc_read(bcmpmu, attr->index,
						PMU_ADC_REQ_RTM_MODE, &result);
	if (ret < 0)
		return ret;
	/* Powersupply frame work expects temperature to be
	 * reported in Tenth multiple of centigrade. so we are maintaning
	 * the temperature look up table in same manner in board file.
	 * But reporting the same to user confuses.
	 * so again dividing by 10 and reporting*/
	if ((attr->index == PMU_ADC_CHANN_NTC) ||
			(attr->index == PMU_ADC_CHANN_32KTEMP) ||
			(attr->index == PMU_ADC_CHANN_PATEMP))
		result.conv = result.conv / 10;
	len += sprintf(buf + len, " raw = 0x%x conv = %d\n",
						result.raw, result.conv);

	return len;

}

static SENSOR_DEVICE_ATTR(vmbatt, S_IRUGO, adc_read,
						NULL, PMU_ADC_CHANN_VMBATT);
static SENSOR_DEVICE_ATTR(vbbatt, S_IRUGO, adc_read,
						NULL, PMU_ADC_CHANN_VBBATT);
static SENSOR_DEVICE_ATTR(vbus, S_IRUGO, adc_read, NULL, PMU_ADC_CHANN_VBUS);
static SENSOR_DEVICE_ATTR(idin, S_IRUGO, adc_read, NULL, PMU_ADC_CHANN_IDIN);
static SENSOR_DEVICE_ATTR(ntc, S_IRUGO, adc_read, NULL, PMU_ADC_CHANN_NTC);
static SENSOR_DEVICE_ATTR(bsi, S_IRUGO, adc_read, NULL, PMU_ADC_CHANN_BSI);
static SENSOR_DEVICE_ATTR(bom, S_IRUGO, adc_read, NULL, PMU_ADC_CHANN_BOM);
static SENSOR_DEVICE_ATTR(32ktemp, S_IRUGO, adc_read,
						NULL, PMU_ADC_CHANN_32KTEMP);
static SENSOR_DEVICE_ATTR(patemp, S_IRUGO, adc_read,
						NULL, PMU_ADC_CHANN_PATEMP);
static SENSOR_DEVICE_ATTR(als, S_IRUGO, adc_read, NULL, PMU_ADC_CHANN_ALS);
static SENSOR_DEVICE_ATTR(die_temp, S_IRUGO, adc_read,
						NULL, PMU_ADC_CHANN_DIE_TEMP);

static struct attribute *bcmpmu_hwmon_attrs[] = {
	&sensor_dev_attr_vmbatt.dev_attr.attr,
	&sensor_dev_attr_vbbatt.dev_attr.attr,
	&sensor_dev_attr_vbus.dev_attr.attr,
	&sensor_dev_attr_idin.dev_attr.attr,
	&sensor_dev_attr_ntc.dev_attr.attr,
	&sensor_dev_attr_bsi.dev_attr.attr,
	&sensor_dev_attr_bom.dev_attr.attr,
	&sensor_dev_attr_32ktemp.dev_attr.attr,
	&sensor_dev_attr_patemp.dev_attr.attr,
	&sensor_dev_attr_als.dev_attr.attr,
	&sensor_dev_attr_die_temp.dev_attr.attr,
	NULL
};

static const struct attribute_group bcmpmu_hwmon_attr_group = {
	.attrs = bcmpmu_hwmon_attrs,
};
#ifdef CONFIG_DEBUG_FS
void *debugfs_adc_dir;
static void bcmpmu_adc_debug_init(struct bcmpmu59xxx *bcmpmu)
{
	if (!bcmpmu->dent_bcmpmu) {
		pr_err("%s: Failed to initialize debugfs\n", __func__);
		return;
	}

	debugfs_adc_dir = debugfs_create_dir("adc", bcmpmu->dent_bcmpmu);
	if (!debugfs_adc_dir)
		return;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				debugfs_adc_dir, &debug_mask))
		goto err ;

	return;
err:
	debugfs_remove(debugfs_adc_dir);
}

#endif
static int __devexit bcmpmu_adc_remove(struct platform_device *pdev)
{
	hwmon_device_register(&pdev->dev);
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_hwmon_attr_group);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(debugfs_adc_dir);
#endif
	kfree(gbcmpmu->adc);
	return 0;
}

static int __devinit bcmpmu_adc_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_adc	*adc;
	int i, ret = 0;

	printk(KERN_DEBUG "%s: called\n", __func__);

	gbcmpmu = bcmpmu;
	adc = kzalloc(sizeof(struct bcmpmu_adc), GFP_KERNEL);
	if (adc == NULL) {
		pr_hwmon(ERROR, "%s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	bcmpmu->adc = (void *)adc;

	adc->pdata = (struct bcmpmu_adc_pdata *)pdev->dev.platform_data;

	adc->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(adc->hwmon_dev)) {
		ret = PTR_ERR(adc->hwmon_dev);
		dev_err(&pdev->dev, "Class registration failed (%d)\n", ret);
		goto error;
	}

	/* SAR per channel mutes */
	for (i = 0; i < PMU_ADC_CHANN_MAX; i++)
		mutex_init(&adc->chann_mutex[i]);

	/* RTM mutex */
	mutex_init(&adc->rtm_mutex);

	/* Mask interrupts */
	bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY);
	bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS);
	bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTM_UPPER);
	bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTM_IGNORE);
	bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN);

	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_hwmon_attr_group);
	if (ret != 0)
		goto exit_remove_files;
#ifdef CONFIG_DEBUG_FS
	bcmpmu_adc_debug_init(bcmpmu);
#endif
	return 0;
exit_remove_files:
	 sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_hwmon_attr_group);
error:
	kfree(adc);
	return 0;
}

static struct platform_driver bcmpmu_adc_driver = {
	.driver = {
		.name = "bcmpmu_adc",
	},
	.probe = bcmpmu_adc_probe,
	.remove = __devexit_p(bcmpmu_adc_remove),
};

static int __init bcmpmu_adc_init(void)
{
	return platform_driver_register(&bcmpmu_adc_driver);
}
subsys_initcall(bcmpmu_adc_init);

static void __exit bcmpmu_adc_exit(void)
{
	platform_driver_unregister(&bcmpmu_adc_driver);
}
module_exit(bcmpmu_adc_exit);

MODULE_DESCRIPTION("Broadcom PMU ADC Driver");
MODULE_LICENSE("GPL");

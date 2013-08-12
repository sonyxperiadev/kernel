
/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/param.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/broadcom/kona_tmon.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/sort.h>
#include <mach/rdb/brcm_rdb_tmon.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <linux/reboot.h>

#define CLR_INT 1
#define INVALID_INX	0xFFFFFFFF
#define THOLD_VAL_MAX 0xD
#define INIT_WORK_DELAY 20
#define TEMP_SAMPLES 8
#define MIN_SAMPLE_DLY_US 5000
#define MAX_SAMPLE_DLY_US 6000
struct kona_tmon {
	int irq;
	struct work_struct tmon_work;
	struct delayed_work poll_work;
	struct delayed_work init_work;
	struct workqueue_struct *wqueue;
	struct clk *tmon_1m_clk;
	struct clk *tmon_apb_clk;
	struct kona_tmon_pdata *pdata;
	struct atomic_notifier_head notifiers;
	int thresh_inx;
	int poll_inx;
	int vtmon_sel;
	int hysteresis;
	int init;
	atomic_t suspend;
};

static struct kona_tmon *kona_tmon;

static int tmon_dbg_mask;
enum {
	TMON_LOG_ERR = 1 << 0,		/*b 00000001*/
	TMON_LOG_DBG = 1 << 1,		/*b 00000010*/
	TMON_LOG_DBGFS = 1 << 2,	/*b 00000100*/
	TMON_LOG_INIT = 1 << 3,		/*b 00001000*/
};

enum {
	TEMP_RAW,
	TEMP_CELCIUS,
	INT_THRESH_RAW,
	INT_THRESH_CELCIUS,
	CRIT_THRESH_RAW,
	CRIT_THRESH_CELCIUS,
	SW_SHDWN_TEMP,
};

static long raw_to_celcius(long raw)
{
	return (426000 - (562 * raw)) / 1000;
}

static unsigned long celcius_to_raw(long celcius)
{
	return (426000 - celcius * 1000) / 562;
}

static int cmp(const void *a, const void *b)
{
	if (*((unsigned long *)a) < *((unsigned long *)b))
		return -1;
	if (*((unsigned long *)a) > *((unsigned long *)b))
		return 1;
	return 0;
}

static int interquartile_mean(unsigned long *data, int num)
{
	int i, j;
	unsigned avg = 0;

	BUG_ON((!data) || ((num % 4) != 0));

	sort(data, num, sizeof(int), cmp, NULL);

	i = num / 4;
	j = num - i;

	for ( ; i < j; i++)
		avg += data[i];

	avg = avg / (j - (num / 4));

	return avg;
}

long tmon_get_current_temp(bool celcius, bool avg)
{
	unsigned long raw_temp, temp_data[TEMP_SAMPLES];
	int i = 0;
	struct kona_tmon_pdata *pdata;
	BUG_ON(kona_tmon == NULL);

	pdata = kona_tmon->pdata;
	raw_temp = readl(pdata->base_addr + TMON_TEMP_VAL_OFFSET) &
		TMON_TEMP_VAL_TEMP_VAL_MASK;

	if (avg) {
		temp_data[i] = raw_temp;
		for (i = 1; i < TEMP_SAMPLES; i++) {
			usleep_range(MIN_SAMPLE_DLY_US, MAX_SAMPLE_DLY_US);
			raw_temp = readl(pdata->base_addr +
			TMON_TEMP_VAL_OFFSET) & TMON_TEMP_VAL_TEMP_VAL_MASK;
			temp_data[i] = raw_temp;
		}
		raw_temp = interquartile_mean(temp_data, TEMP_SAMPLES);
	}

	if (celcius)
		return raw_to_celcius(raw_temp);
	else
		return raw_temp;
}
EXPORT_SYMBOL_GPL(tmon_get_current_temp);

static int tmon_reset_enable(struct kona_tmon *tmon, bool enable)
{
	u32 reg;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	reg = readl(pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
	if (enable)
		reg |= TMON_CFG_RST_EN_RST_ENABLE_MASK;
	else
		reg &= ~TMON_CFG_RST_EN_RST_ENABLE_MASK;
	writel(reg, pdata->base_addr + TMON_CFG_RST_EN_OFFSET);

	return 0;
}

static long tmon_get_critical_thold(struct kona_tmon *tmon, bool celcius)
{
	unsigned long raw_temp;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	raw_temp = readl(pdata->base_addr + TMON_CFG_RST_THRESH_OFFSET)
		& TMON_CFG_RST_THRESH_RST_THRESH_MASK;
	if (celcius)
		return raw_to_celcius(raw_temp);
	else
		return raw_temp;
}

static int tmon_set_critical_thold(struct kona_tmon *tmon, long thresh_val,
		bool celcius)
{
	unsigned long thresh;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	tmon_reset_enable(tmon, 1);
	if (celcius)
		thresh = celcius_to_raw(thresh_val);
	else
		thresh = thresh_val;
	writel(thresh, pdata->base_addr + TMON_CFG_RST_THRESH_OFFSET);
	return 0;
}

static long tmon_get_int_thold(struct kona_tmon *tmon, bool celcius)
{
	unsigned long raw_temp;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	raw_temp = readl(pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET)
		& TMON_CFG_INT_THRESH_INT_THRESH_MASK;
	if (celcius)
		return raw_to_celcius(raw_temp);
	else
		return raw_temp;
}

static int tmon_set_int_thold(struct kona_tmon *tmon, long thresh_val,
		bool celcius)
{
	unsigned long thresh;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	if (celcius)
		thresh = celcius_to_raw(thresh_val);
	else
		thresh = thresh_val;
	writel(thresh, pdata->base_addr + TMON_CFG_INT_THRESH_OFFSET);
	return 0;
}

static int tmon_enable(struct kona_tmon *tmon, bool enable)
{
	u32 reg;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	reg = readl(pdata->base_addr + TMON_CFG_ENBALE_OFFSET);
	if (enable)
		reg &= ~TMON_CFG_ENBALE_ENABLE_MASK;
	else
		reg |= TMON_CFG_ENBALE_ENABLE_MASK;
	writel(reg, pdata->base_addr + TMON_CFG_ENBALE_OFFSET);
	return 0;
}

static int tmon_set_interval(struct kona_tmon *tmon, int interval_ms)
{
	struct kona_tmon_pdata *pdata = tmon->pdata;

	writel(interval_ms * 32, pdata->base_addr +
			TMON_CFG_INTERVAL_VAL_OFFSET);
	return 0;
}

static unsigned long tmon_get_interval(struct kona_tmon *tmon)
{
	unsigned long ticks;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	ticks = readl(pdata->base_addr + TMON_CFG_INTERVAL_VAL_OFFSET);

	return ticks/32;
}

static int tmon_init_set_thold(struct kona_tmon *tmon)
{
	long curr;
	int i;
	struct kona_tmon_pdata *pdata = tmon->pdata;

	/*setting the threshold value */
	curr = tmon_get_current_temp(CELCIUS, true);
	pr_info("%s: current temperature is %ld\n", __func__, curr);
	for (i = pdata->thold_size - 1; i > 0; i--) {
		if (curr >= (pdata->thold[i].rising - tmon->hysteresis)) {
			tmon->poll_inx = i - 1;
			break;
		}
	}
	tmon->thresh_inx = i;

	writel(CLR_INT, pdata->base_addr + TMON_CFG_CLR_INT_OFFSET);
	if (pdata->thold[tmon->thresh_inx].flags & TMON_HW_SHDWN) {
		tmon_set_int_thold(tmon, THOLD_VAL_MAX, RAW_VAL);
		disable_irq_nosync(tmon->irq);
		tmon->thresh_inx = INVALID_INX;
	} else
		tmon_set_int_thold(tmon,
		pdata->thold[tmon->thresh_inx].rising, CELCIUS);
	return 0;
}

static void tmon_poll_work(struct work_struct *ws)
{
	long curr_temp;
	int poll_inx, falling;
	int intr_en = 0;
	struct kona_tmon *tmon = container_of((struct delayed_work *)ws,
			struct kona_tmon, poll_work);
	struct kona_tmon_pdata *pdata = tmon->pdata;

	BUG_ON(tmon->poll_inx == INVALID_INX);
	curr_temp = tmon_get_current_temp(CELCIUS, true);
	poll_inx = tmon->poll_inx;
	falling = pdata->thold[poll_inx].rising - pdata->falling;
	if (curr_temp <= (falling + kona_tmon->hysteresis)) {
		pr_info("%s: reached the polling temp %d\n", __func__, falling);
		/* updating threshold value and indexes*/
		tmon_set_int_thold(tmon, pdata->thold[poll_inx].rising,
				CELCIUS);
		if (tmon->thresh_inx == INVALID_INX)
			intr_en = 1;
		atomic_notifier_call_chain(&tmon->notifiers, falling, NULL);
		tmon->thresh_inx = poll_inx;
		tmon->poll_inx = poll_inx ? (poll_inx - 1)
			: INVALID_INX;
		writel(CLR_INT, pdata->base_addr + TMON_CFG_CLR_INT_OFFSET);
		if (intr_en)
			enable_irq(tmon->irq);
	}

	if (tmon->poll_inx != INVALID_INX && !atomic_read(&tmon->suspend))
		queue_delayed_work(tmon->wqueue, &tmon->poll_work,
				msecs_to_jiffies(pdata->poll_rate_ms));
}

static void tmon_irq_work(struct work_struct *ws)
{
	struct kona_tmon *tmon = container_of(ws, struct kona_tmon, tmon_work);
	struct kona_tmon_pdata *pdata = tmon->pdata;

	if (tmon->poll_inx != INVALID_INX) {
		queue_delayed_work(tmon->wqueue, &tmon->poll_work,
				msecs_to_jiffies(pdata->poll_rate_ms));
		atomic_notifier_call_chain(&tmon->notifiers,
				pdata->thold[tmon->poll_inx].rising, NULL);

		if (pdata->thold[tmon->poll_inx].flags & TMON_SW_SHDWN) {
			pr_info("%s:reached Thermal SW shdwn temp\n", __func__);
			kernel_halt();
		}
	}
}

static void tmon_init_work(struct work_struct *ws)
{
	struct kona_tmon *tmon = container_of((struct delayed_work *)ws,
			struct kona_tmon, init_work);
	tmon_init_set_thold(tmon);
}

static irqreturn_t tmon_isr(int irq, void *drvdata)
{
	struct kona_tmon *tmon =  drvdata;
	struct kona_tmon_pdata *pdata = tmon->pdata;
	long curr_temp;

	BUG_ON(tmon->thresh_inx == INVALID_INX);
	curr_temp = tmon_get_current_temp(CELCIUS, false);
	pr_info(KERN_ALERT "SoC temperature threshold of %d exceeded."\
			"Current temperature is %ld\n",
	pdata->thold[tmon->thresh_inx].rising, curr_temp);

	/*Clear interrupt*/
	writel(CLR_INT, pdata->base_addr + TMON_CFG_CLR_INT_OFFSET);
	tmon->poll_inx = tmon->thresh_inx;

	/*Find next rising thold*/
	if ((tmon->thresh_inx < pdata->thold_size - 1) && (
		 (pdata->thold[tmon->thresh_inx + 1].flags & TMON_NOTIFY) ||
		 (pdata->thold[tmon->thresh_inx + 1].flags & TMON_SW_SHDWN))) {
		tmon->thresh_inx++;
		tmon_set_int_thold(tmon, pdata->thold[tmon->thresh_inx].rising,
				CELCIUS);
	} else {
		/*Set THOLD to max value for HW shdwn*/
		tmon_set_int_thold(tmon, THOLD_VAL_MAX, RAW_VAL);
		disable_irq_nosync(tmon->irq);
		tmon->thresh_inx = INVALID_INX;
	}
	queue_work(tmon->wqueue, &tmon->tmon_work);
	return IRQ_HANDLED;
}

static int tmon_init(struct kona_tmon *tmon, int vtmon)
{
	u32 reg;
	int i;
	int rst_inx = INVALID_INX;
	struct kona_tmon_pdata *pdata;

	BUG_ON(!tmon);
	pdata = tmon->pdata;

	reg = readl(pdata->chipreg_addr +
				CHIPREG_SPARE_CONTROL0_OFFSET);
	switch (vtmon) {
	case VTMON:
		reg &= ~CHIPREG_SPARE_CONTROL0_VTMON_SELECT_MASK;
		break;
	case PVTMON:
		reg |= CHIPREG_SPARE_CONTROL0_VTMON_SELECT_MASK;
		break;
	default:
		BUG();
	}
	writel(reg, pdata->chipreg_addr +
				CHIPREG_SPARE_CONTROL0_OFFSET);

	tmon_enable(tmon, 1);
	tmon_set_interval(tmon, pdata->interval_ms);

	/*setting the HW shdwn theshold value */
	for (i = 0; i < pdata->thold_size; i++) {
		if (pdata->thold[i].flags & TMON_HW_SHDWN) {
			rst_inx = i;
			pr_info("%s: HW shdwn temperature is %d\n",
					__func__, pdata->thold[rst_inx].rising);
		}
	}
	if (rst_inx != INVALID_INX)
		tmon_set_critical_thold(tmon,
			pdata->thold[rst_inx].rising, CELCIUS);
	else
		tmon_reset_enable(tmon, 0);

	if (tmon->init)
		queue_delayed_work(tmon->wqueue, &tmon->init_work,
				msecs_to_jiffies(INIT_WORK_DELAY));
	else {
		mdelay(INIT_WORK_DELAY);
		tmon_init_set_thold(tmon);
	}

	return 0;
}

int tmon_register_notifier(struct notifier_block *notifier)
{
	return atomic_notifier_chain_register(&kona_tmon->notifiers, notifier);
}
EXPORT_SYMBOL_GPL(tmon_register_notifier);

int tmon_unregister_notifier(struct notifier_block *notifier)
{
	return atomic_notifier_chain_unregister(&kona_tmon->notifiers,
			notifier);
}
EXPORT_SYMBOL_GPL(tmon_unregister_notifier);

static ssize_t tmon_get_name(struct device *dev,
struct device_attribute *devattr, char *buf)
{
	return snprintf(buf, 10, "soctemp\n");
}

static ssize_t tmon_get_val(struct device *dev,
struct device_attribute *devattr, char *buf)
{
	int i, raw, celcius = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	 struct kona_tmon_pdata *pdata = kona_tmon->pdata;

	switch (attr->index) {
	case TEMP_RAW:
		raw = tmon_get_current_temp(RAW_VAL, true);
		return snprintf(buf, 10, "%x\n", raw);
	case TEMP_CELCIUS:
		celcius = tmon_get_current_temp(CELCIUS, true);
		return snprintf(buf, 10, "%d\n", celcius);
	case INT_THRESH_RAW:
		raw = tmon_get_int_thold(kona_tmon, RAW_VAL);
		return snprintf(buf, 10, "%x\n", raw);
	case INT_THRESH_CELCIUS:
		celcius = tmon_get_int_thold(kona_tmon, CELCIUS);
		return snprintf(buf, 10, "%d\n", celcius);
	case CRIT_THRESH_RAW:
		raw = tmon_get_critical_thold(kona_tmon, RAW_VAL);
		return snprintf(buf, 10, "%x\n", raw);
	case CRIT_THRESH_CELCIUS:
		celcius = tmon_get_critical_thold(kona_tmon, CELCIUS);
		return snprintf(buf, 10, "%d\n", celcius);
	case SW_SHDWN_TEMP:
		for (i = pdata->thold_size - 1; i >= 0; i--) {
			if (pdata->thold[i].flags & TMON_SW_SHDWN)
				return snprintf(buf, 10, "%d\n",
					pdata->thold[i].rising);
		}
	default:
		return -EINVAL;
	}
}

static ssize_t tmon_get_threshold_levels(struct device *dev,
struct device_attribute *devattr, char *buf)
{
	u32 len = 0;
	int i;
	char out_str[200];
	struct kona_tmon_pdata *pdata = kona_tmon->pdata;

	memset(out_str, 0, sizeof(out_str));
	len += snprintf(out_str + len, sizeof(out_str) - len,
			"Level\t\tRising Temp\tFalling Temp\n");
	for (i = 0; i < pdata->thold_size - 2; i++)
		len += snprintf(out_str + len, sizeof(out_str) - len,
		"Level%d:\t\t%d\t\t%d\n", i, pdata->thold[i].rising,
		pdata->thold[i].rising - pdata->falling);
	len += snprintf(out_str + len, sizeof(out_str) - len,
			"SW SHDWN temp : %d\n", pdata->thold[i].rising);
	len += snprintf(out_str + len, sizeof(out_str) - len,
			"HW SHDWN temp : %d\n", pdata->thold[++i].rising);

	return snprintf(buf, len, "%s", out_str);
}

static ssize_t tmon_set_threshold_levels(struct device *dev,
	struct device_attribute *devattr, const char *buf, size_t count)
{
	int i, inx, thold, prev, next, max = 0;
	struct kona_tmon_pdata *pdata = kona_tmon->pdata;
	int num_thold = pdata->thold_size;

	sscanf(buf, "%d%d", &inx, &thold);

	/*check if inx is valid*/
	if (inx < 0 || inx >= num_thold)
		goto exit;

	for (i = 0; i < num_thold; i++) {
		if ((pdata->thold[i].flags & TMON_HW_SHDWN) ||
				(pdata->thold[i].flags & TMON_SW_SHDWN))
			break;
	}
	max = i - 1;

	/*checking if thold is in ascending order*/
	prev = inx - 1;
	next = inx + 1;
	if (((inx > 0) && (thold < pdata->thold[prev].rising)) ||
		((inx <= max) && (thold > pdata->thold[next].rising)))
		goto exit;

	pdata->thold[inx].rising = thold;
	tmon_init_set_thold(kona_tmon);
	return count;
exit:
	tmon_dbg(TMON_LOG_ERR, "USAGE:\necho inx thold > threshold_levels\n");
	tmon_dbg(TMON_LOG_ERR,
	"inx range: [0-%d]\ntholds shd be in ascending order\n", max);
	return count;
}

static ssize_t tmon_set_raw_val(struct device *dev,
	struct device_attribute *devattr, const char *buf, size_t count)
{
	int raw;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);

	/* coverity[secure_coding] */
	sscanf(buf, "%x", &raw);

	switch (attr->index) {
	case INT_THRESH_RAW:
		tmon_set_int_thold(kona_tmon, raw, RAW_VAL);
		break;
	case CRIT_THRESH_RAW:
		tmon_set_critical_thold(kona_tmon, raw, RAW_VAL);
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static ssize_t tmon_set_celcius_val(struct device *dev,
	struct device_attribute *devattr, const char *buf, size_t count)
{
	int i, num_thold;
	long celcius;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct kona_tmon_pdata *pdata = kona_tmon->pdata;
	/* coverity[secure_coding] */
	sscanf(buf, "%ld", &celcius);

	num_thold = pdata->thold_size;
	switch (attr->index) {
	case CRIT_THRESH_CELCIUS:
		for (i = num_thold - 1; i >= 0; i--) {
			if (pdata->thold[i].flags & TMON_HW_SHDWN)
				break;
		}
		if (i && (celcius < pdata->thold[i-1].rising))
			tmon_dbg(TMON_LOG_ERR,
					"HW shdwn tempearture not valid\n");
		else {
			pdata->thold[i].rising = celcius;
			tmon_set_critical_thold(kona_tmon, celcius, CELCIUS);
		}
		break;
	case SW_SHDWN_TEMP:
		for (i = num_thold - 1; i >= 0; i--) {
			if (pdata->thold[i].flags & TMON_SW_SHDWN)
				break;
		}
		if (((i > 0) && (celcius < pdata->thold[i - 1].rising)) ||
		(i < num_thold - 1 && celcius > pdata->thold[i + 1].rising)) {
			tmon_dbg(TMON_LOG_ERR,
				"SW shdwn tempearture not valid\n");
			break;
		}
		pdata->thold[i].rising = celcius;
		break;
	default:
		return -EINVAL;
	}
	return count;
}
static ssize_t kona_tmon_set_interval(struct device *dev,
	struct device_attribute *devattr, const char *buf, size_t count)
{
	long msecs;

	/* coverity[secure_coding] */
	sscanf(buf, "%ld", &msecs);
	tmon_set_interval(kona_tmon, msecs);
	return count;
}

static ssize_t kona_tmon_get_interval(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	unsigned long ticks;

	ticks = tmon_get_interval(kona_tmon);
	return snprintf(buf, 10, "%lu\n", ticks);
}

static ssize_t tmon_vtmon_sel(struct device *dev,
	struct device_attribute *devattr, const char *buf, size_t count)
{
	int vtmon;
	/* coverity[secure_coding] */
	sscanf(buf, "%d", &vtmon);

	if (vtmon == kona_tmon->vtmon_sel)
		return count;

	kona_tmon->vtmon_sel = vtmon;
	tmon_init(kona_tmon, vtmon);
	return count;
}

static ssize_t tmon_get_vtmon(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	return snprintf(buf, 10, "%d\n", kona_tmon->vtmon_sel);
}

static ssize_t tmon_set_dbg_mask(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count)
{
	int val;
	/* coverity[secure_coding] */
	sscanf(buf, "%d", &val);
	tmon_dbg_mask = val;
	return count;
}

static ssize_t tmon_get_dbg_mask(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	return snprintf(buf, 10, "%d\n", tmon_dbg_mask);
}

static ssize_t tmon_set_hysteresis(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count)
{
	int val;
	sscanf(buf, "%d", &val);
	kona_tmon->hysteresis = val;
	return count;
}

static ssize_t tmon_get_hysteresis(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	return snprintf(buf, 10, "%d\n", kona_tmon->hysteresis);
}

static ssize_t tmon_set_falling_offset(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count)
{
	int val;
	struct kona_tmon_pdata *pdata = kona_tmon->pdata;

	sscanf(buf, "%d", &val);
	pdata->falling = val;
	return count;
}

static ssize_t tmon_get_falling_offset(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct kona_tmon_pdata *pdata = kona_tmon->pdata;
	return snprintf(buf, 10, "%d\n", pdata->falling);
}
static struct sensor_device_attribute kona_attrs[] = {
SENSOR_ATTR(name, S_IRUGO, tmon_get_name, NULL, 0),
SENSOR_ATTR(interval, S_IWUSR | S_IRUGO, kona_tmon_get_interval,
kona_tmon_set_interval, 0),
SENSOR_ATTR(temp_raw, S_IRUGO, tmon_get_val,
		NULL, TEMP_RAW),
SENSOR_ATTR(temp_celcius, S_IRUGO, tmon_get_val, NULL, TEMP_CELCIUS),
SENSOR_ATTR(threshold_raw, S_IWUSR | S_IRUGO, tmon_get_val,
		tmon_set_raw_val, INT_THRESH_RAW),
SENSOR_ATTR(threshold_celcius, S_IWUSR | S_IRUGO, tmon_get_val,
		NULL, INT_THRESH_CELCIUS),
SENSOR_ATTR(critical_raw, S_IWUSR | S_IRUGO, tmon_get_val,
		tmon_set_raw_val, CRIT_THRESH_RAW),
SENSOR_ATTR(critical_celcius, S_IWUSR | S_IRUGO, tmon_get_val,
		tmon_set_celcius_val, CRIT_THRESH_CELCIUS),
SENSOR_ATTR(sw_shdwn_temp, S_IRUGO | S_IWUSR, tmon_get_val,
		tmon_set_celcius_val, SW_SHDWN_TEMP),
SENSOR_ATTR(threshold_table, S_IWUSR | S_IRUGO, tmon_get_threshold_levels,
		tmon_set_threshold_levels, 0),
SENSOR_ATTR(vtmon, S_IWUSR | S_IRUGO, tmon_get_vtmon,
		tmon_vtmon_sel, 0),
SENSOR_ATTR(tmon_dbg_mask, S_IWUSR | S_IRUGO, tmon_get_dbg_mask,
		tmon_set_dbg_mask, 0),
SENSOR_ATTR(hysteresis, S_IWUSR | S_IRUGO, tmon_get_hysteresis,
		tmon_set_hysteresis, 0),
SENSOR_ATTR(falling, S_IWUSR | S_IRUGO, tmon_get_falling_offset,
		tmon_set_falling_offset, 0),
};

static int kona_tmon_suspend(struct platform_device *pdev, pm_message_t state)
{
	atomic_set(&kona_tmon->suspend, 1);
	if (kona_tmon->poll_inx != INVALID_INX) {
		flush_delayed_work_sync(&kona_tmon->poll_work);
		tmon_dbg(TMON_LOG_DBG, "%s: cancelling work queue\n", __func__);
	}
	return 0;
}

static int kona_tmon_resume(struct platform_device *pdev)
{
	atomic_set(&kona_tmon->suspend, 0);
	if (kona_tmon->poll_inx != INVALID_INX) {
		queue_delayed_work(kona_tmon->wqueue, &kona_tmon->poll_work,
				msecs_to_jiffies(0));
		tmon_dbg(TMON_LOG_DBG, "%s: starting work queue\n", __func__);
	}
	return 0;
}

static int kona_tmon_probe(struct platform_device *pdev)
{
	u32 val, *addr;
	const char *clk_name;
	int size, i, irq;
	int rc = 0;
	struct resource *iomem;
	struct kona_tmon_pdata *pdata;
	int ret;
	int *thold;
	kona_tmon = kzalloc(sizeof(struct kona_tmon), GFP_KERNEL);
	if (pdev->dev.platform_data)
		pdata =	(struct kona_tmon_pdata *)pdev->dev.platform_data;
	else if (pdev->dev.of_node) {
		pdata = kzalloc(sizeof(struct kona_tmon_pdata),	GFP_KERNEL);
		if (!pdata) {
			rc = -ENOMEM;
			goto err_free_dev_mem;
		}

		/* Get register memory resource */
		iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!iomem) {
			tmon_dbg(TMON_LOG_ERR, "no mem resource\n");
			rc = -ENODEV;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->base_addr = (u32)ioremap(iomem->start,
					resource_size(iomem));
		if (!pdata->base_addr) {
			tmon_dbg(TMON_LOG_ERR, "unable to map in registers\n");
			kfree(pdata);
			rc = -ENOMEM;
			goto err_free_dev_mem;
		}

		/* Get interrupt number */
		irq = platform_get_irq(pdev, 0);
		if (irq == -ENXIO) {
			tmon_dbg(TMON_LOG_ERR, "no irq resource\n");
			kfree(pdata);
			rc = -ENODEV;
			goto err_free_dev_mem;
		}
		pdata->irq = irq;

		addr = (u32 *)of_get_property(pdev->dev.of_node, "chipreg_addr",
				&size);
		if (!addr) {
			kfree(pdata);
			rc = -EINVAL;
			goto err_free_dev_mem;
		}
		val = *(addr + 1);
		pdata->chipreg_addr = (u32)ioremap(be32_to_cpu(*addr),
				be32_to_cpu(val));

		ret = of_property_read_u32(pdev->dev.of_node,
				"thold_size", &val);
		if (ret != 0) {
			kfree(pdata);
			rc = -EINVAL;
			goto err_free_dev_mem;
		}
		pdata->thold_size = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"poll_rate_ms", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->poll_rate_ms = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"hysteresis", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->hysteresis = val;

		if (of_property_read_u32(pdev->dev.of_node, "flags", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->flags = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"interval_ms", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->interval_ms = val;

		if (of_property_read_string(pdev->dev.of_node,
					"tmon_apb_clk", &clk_name)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->tmon_apb_clk = clk_name;

		if (of_property_read_string(pdev->dev.of_node,
					"tmon_1m_clk", &clk_name)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->tmon_1m_clk = clk_name;

		if (of_property_read_u32(pdev->dev.of_node,
					"falling", &val)) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->falling = val;

		thold = (int *)of_get_property(pdev->dev.of_node,
				"thold", &size);
		if (!thold) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}
		pdata->thold =
		kzalloc(sizeof(struct tmon_state)*pdata->thold_size,
				GFP_KERNEL);
		if (!pdata->thold) {
			rc = -EINVAL;
			kfree(pdata);
			goto err_free_dev_mem;
		}

		for (i = 0; i < pdata->thold_size; i++) {
			pdata->thold[i].rising =
				be32_to_cpu(*thold++);
			pdata->thold[i].flags = be32_to_cpu(*thold++);
		}
	} else {
		tmon_dbg(TMON_LOG_ERR, "no platform data found\n");
		rc = -EINVAL;
		goto err_free_dev_mem;
	}

	kona_tmon->pdata = pdata;

	/* Enable clocks */
	kona_tmon->tmon_apb_clk = clk_get(NULL, pdata->tmon_apb_clk);
	if (IS_ERR(kona_tmon->tmon_apb_clk)) {
		tmon_dbg(TMON_LOG_ERR, "couldn't get tmon_apb_clk\n");
		rc = PTR_ERR(kona_tmon->tmon_apb_clk);
		goto err_free_dev_mem;
	}
	clk_enable(kona_tmon->tmon_apb_clk);

	kona_tmon->tmon_1m_clk = clk_get(NULL, pdata->tmon_1m_clk);
	if (IS_ERR(kona_tmon->tmon_1m_clk)) {
		tmon_dbg(TMON_LOG_ERR, "couldn't get tmon_1m_clk\n");
		rc = PTR_ERR(kona_tmon->tmon_1m_clk);
		goto err_disable_apb_clk;
	}
	if (clk_set_rate(kona_tmon->tmon_1m_clk, 1000000)) {
		tmon_dbg(TMON_LOG_ERR, "couldn't set rate of tmon_1m_clk\n");
		rc = -ENODEV;
		goto err_put_1m_clk;
	}
	clk_enable(kona_tmon->tmon_1m_clk);

	/* Create sysfs files */
	for (i = 0; i < ARRAY_SIZE(kona_attrs); i++)
		if (device_create_file(&pdev->dev, &kona_attrs[i].dev_attr))
			goto err_remove_files;

	kona_tmon->vtmon_sel = pdata->flags & VTMON;
	kona_tmon->hysteresis = pdata->hysteresis;

	INIT_WORK(&kona_tmon->tmon_work, tmon_irq_work);
	kona_tmon->wqueue = create_workqueue("tmon_polling_wq");
	INIT_DELAYED_WORK(&kona_tmon->poll_work, tmon_poll_work);
	INIT_DELAYED_WORK(&kona_tmon->init_work, tmon_init_work);

	/* Register interrupt handler */
	kona_tmon->irq = pdata->irq;
	if (0 != request_irq(kona_tmon->irq, tmon_isr,
				IRQ_LEVEL | IRQF_NO_SUSPEND | IRQF_DISABLED,
				pdev->name, kona_tmon)) {
		tmon_dbg(TMON_LOG_ERR, "unable to register isr\n");
		rc = -1;
		goto err_unregister_device;
	}

	kona_tmon->init = 1;
	kona_tmon->poll_inx = INVALID_INX;
	tmon_init(kona_tmon, kona_tmon->vtmon_sel);
	kona_tmon->init = 0;

	/* Register hwmon device */
	if (!hwmon_device_register(&pdev->dev)) {
		tmon_dbg(TMON_LOG_ERR, "hwmon device register failed\n");
		rc = -ENODEV;
		goto err_free_dev_mem;
	}

	ATOMIC_INIT_NOTIFIER_HEAD(&kona_tmon->notifiers);

	return 0;
err_unregister_device:
	hwmon_device_unregister(&pdev->dev);

err_remove_files:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(&pdev->dev, &kona_attrs[i].dev_attr);
	clk_disable(kona_tmon->tmon_1m_clk);

err_put_1m_clk:
	clk_put(kona_tmon->tmon_1m_clk);

err_disable_apb_clk:
	clk_disable(kona_tmon->tmon_apb_clk);
	clk_put(kona_tmon->tmon_apb_clk);

err_free_dev_mem:
	kfree(kona_tmon);
	return rc;
}

static int kona_tmon_remove(struct platform_device *pdev)
{
	int i;
	disable_irq(kona_tmon->irq);
	free_irq(kona_tmon->irq, kona_tmon);

	for (i = ARRAY_SIZE(kona_attrs) - 1; i >= 0; i--)
		device_remove_file(&pdev->dev, &kona_attrs[i].dev_attr);

	hwmon_device_unregister(&pdev->dev);

	clk_disable(kona_tmon->tmon_1m_clk);
	clk_put(kona_tmon->tmon_1m_clk);

	clk_disable(kona_tmon->tmon_apb_clk);
	clk_put(kona_tmon->tmon_apb_clk);

	destroy_workqueue(kona_tmon->wqueue);
	kfree(kona_tmon);

	return 0;
}


static const struct of_device_id kona_tmon_dt_ids[] = {
	{ .compatible = "bcm,tmon", },
	{},
};

MODULE_DEVICE_TABLE(of, kona_tmon_dt_ids);

static struct platform_driver kona_tmon_driver = {
	.driver = {
		.name = "kona_tmon",
		.owner = THIS_MODULE,
		.of_match_table = kona_tmon_dt_ids,
	},
	.probe = kona_tmon_probe,
	.remove = kona_tmon_remove,
	.suspend = kona_tmon_suspend,
	.resume = kona_tmon_resume,
};

static int __init kona_tmon_init(void)
{
	return platform_driver_register(&kona_tmon_driver);
}

static void __exit kona_tmon_exit(void)
{
	platform_driver_unregister(&kona_tmon_driver);
}

rootfs_initcall(kona_tmon_init);
module_exit(kona_tmon_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Temperature Monitor");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

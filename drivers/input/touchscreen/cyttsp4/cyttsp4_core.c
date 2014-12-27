/*
 * cyttsp4_core.c
 * Cypress TrueTouch(TM) Standard Product V4 Core driver module.
 * For use with Cypress Txx4xx parts.
 * Supported parts include:
 * TMA4XX
 * TMA1036
 *
 * Copyright (C) 2012 Cypress Semiconductor
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 * Modified by: Cypress Semiconductor to add device functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <ttdrivers@cypress.com>
 *
 */

#include <linux/cyttsp4_bus.h>

#include <asm/unaligned.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include <linux/cyttsp4_core.h>
#include "cyttsp4_regs.h"

/* [Optical][Touch] Implement T2 test function, 20130724, Add Start */
#include <linux/fs.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
/* [Optical][Touch] Implement T2 test function, 20130724, Add End */
/**/
#include "cyttsp4_mt_common.h"
/**/
/* [OPT] change circuit design after DVT2 phase, 20131008, Add Start */
#include <linux/regulator/consumer.h>
//Alin
//#include <mach/cci_hw_id.h>
/* [OPT] change circuit design after DVT2 phase, 20131008, Add End */

/* [OPT] */
int silicon_id = IC_445;
/* [OPT] */

/* [OPT] Cypress AE solved the upgrade failed, 20130918, Add Start */
#define CY_CORE_BL_HOST_SYNC_BYTE 0xFF
/* [OPT] Cypress AE solved the upgrade failed, 20130918, Add End */

/* Timeout in ms. */
#define CY_CORE_REQUEST_EXCLUSIVE_TIMEOUT	500
#define CY_CORE_SLEEP_REQUEST_EXCLUSIVE_TIMEOUT	5000
#define CY_CORE_WAIT_SYSINFO_MODE_TIMEOUT	2000
#define CY_CORE_MODE_CHANGE_TIMEOUT		1000
#define CY_CORE_RESET_AND_WAIT_TIMEOUT		500
#define CY_CORE_WAKEUP_TIMEOUT			50

#define CY_CORE_STARTUP_RETRY_COUNT		3

#define IS_DEEP_SLEEP_CONFIGURED(x) \
		((x) == 0 || (x) == 0xFF)

#define IS_TMO(t)	((t) == 0)

#define PUT_FIELD16(si, val, addr) \
do { \
	if (IS_LITTLEENDIAN((si)->si_ptrs.cydata->device_info)) \
		put_unaligned_le16(val, addr); \
	else \
		put_unaligned_be16(val, addr); \
} while (0)

#define GET_FIELD16(si, addr) \
({ \
	u16 __val; \
	if (IS_LITTLEENDIAN((si)->si_ptrs.cydata->device_info)) \
		__val = get_unaligned_le16(addr); \
	else \
		__val = get_unaligned_be16(addr); \
	__val; \
})

#define RETRY_OR_EXIT(retry_cnt, retry_label, exit_label) \
do { \
	if (retry_cnt) \
		goto retry_label; \
	goto exit_label; \
} while (0)

static const u8 security_key[] = {
	0xA5, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0x5A
};

static const u8 ldr_exit[] = {
	0xFF, 0x01, 0x3B, 0x00, 0x00, 0x4F, 0x6D, 0x17
};

static const u8 ldr_err_app[] = {
	0x01, 0x02, 0x00, 0x00, 0x55, 0xDD, 0x17
};

MODULE_FIRMWARE(CY_FW_FILE_NAME);

const char *cy_driver_core_name = CYTTSP4_CORE_NAME;
const char *cy_driver_core_version = CY_DRIVER_VERSION;
const char *cy_driver_core_date = CY_DRIVER_DATE;

/* [OPT] Implement factory T2 self test, 20131002, Add Start */
#define JTOUCH	0
#define TRULY	1
#define BIEL	2
#define TPK		3

 struct raw_data_range {
	int lpwc_max;
	int lpwc_min;
	int lpwc_self_max;
	int lpwc_self_min;
	int raw_max;
	int raw_min;
	int base_max;
	int base_min;
};

enum NUM_TP_SOURCE{
	MAIN_SOURCE,
	SECOND_SOURCE,
	THIRD_SOURCE,
	FOURTH_SOURCE,
	TOTAL_SUN,
};

enum DATATYPE{
	RAWDATA,
	BASELINE,
	DIFFCNT,
	LIDAC,
	SELF_LIDAC,
	TYPE_MAX,
};
// TODO: Need to chage shipping version when F/W changed.
static u16 shipping_version =0x0600;

static struct raw_data_range ftm_raw_data[TOTAL_SUN] = {
	{18, 5, 43, 14, 131, -324, 15, -15},      // main source(truly)
	{80, 5, 80, 5, 1000, -1500, 30, -30},      // second source(Jtouch)
	{0, 0, 0, 0, 0, 0, 0, 0},      // third source
	{0, 0, 0, 0, 0, 0, 0, 0},      // fourth source
};
/* [OPT] Implement factory T2 self test, 20131002, Add End */


enum cyttsp4_sleep_state {
	SS_SLEEP_OFF,
	SS_SLEEP_ON,
	SS_SLEEPING,
	SS_WAKING,
};

enum cyttsp4_startup_state {
	STARTUP_NONE,
	STARTUP_QUEUED,
	STARTUP_RUNNING,
};

struct cyttsp4_core_data {
	struct device *dev;
	struct cyttsp4_core *core;
	struct list_head atten_list[CY_ATTEN_NUM_ATTEN];
	struct mutex system_lock;
	struct mutex adap_lock;
	enum cyttsp4_mode mode;
	enum cyttsp4_sleep_state sleep_state;
	enum cyttsp4_startup_state startup_state;
	int int_status;
	int cmd_toggle;
	spinlock_t spinlock;
	struct cyttsp4_core_platform_data *pdata;
	wait_queue_head_t wait_q;
	int irq;
	struct work_struct startup_work;
	struct cyttsp4_sysinfo sysinfo;
	void *exclusive_dev;
	int exclusive_waits;
	atomic_t ignore_irq;
	bool irq_enabled;
	bool irq_wake;
	bool wake_initiated_by_device;
	bool invalid_touch_app;
	int max_xfer;
	int apa_mc_en;
	int glove_en;
	int stylus_en;
	int proximity_en;
	u8 default_scantype;
	u8 easy_wakeup_gesture;
	unsigned int active_refresh_cycle_ms;
	u8 heartbeat_count;
#ifdef VERBOSE_DEBUG
	u8 pr_buf[CY_MAX_PRBUF_SIZE];
#endif
	struct work_struct watchdog_work;
	struct timer_list watchdog_timer;
/* [OPT] Implement factory T2 self test, 20131017, Add Start */
	struct raw_data_range factory_criteria;
/* [OPT] Implement factory T2 self test, 20131017, Add End */
};

struct atten_node {
	struct list_head node;
	int (*func)(struct cyttsp4_device *);
	struct cyttsp4_device *ttsp;
	int mode;
};

static int _cyttsp4_put_device_into_deep_sleep(struct cyttsp4_core_data *cd,
		u8 hst_mode_reg);

static inline size_t merge_bytes(u8 high, u8 low)
{
	return (high << 8) + low;
}

#ifdef VERBOSE_DEBUG
void cyttsp4_pr_buf(struct device *dev, u8 *pr_buf, u8 *dptr, int size,
		const char *data_name)
{
	int i, k;
	const char fmt[] = "%02X ";
	int max;

	if (!size)
		return;

	max = (CY_MAX_PRBUF_SIZE - 1) - sizeof(CY_PR_TRUNCATED);

	pr_buf[0] = 0;
	for (i = k = 0; i < size && k < max; i++, k += 3)
		scnprintf(pr_buf + k, CY_MAX_PRBUF_SIZE, fmt, dptr[i]);

	dev_vdbg(dev, "%s:  %s[0..%d]=%s%s\n", __func__, data_name, size - 1,
			pr_buf, size <= max ? "" : CY_PR_TRUNCATED);
}
EXPORT_SYMBOL_GPL(cyttsp4_pr_buf);
#endif

static inline int cyttsp4_adap_read(struct cyttsp4_core_data *cd, u16 addr,
		void *buf, int size)
{
	return cd->core->adap->read(cd->core->adap, addr, buf, size,
			cd->max_xfer);
}

static inline int cyttsp4_adap_write(struct cyttsp4_core_data *cd, u16 addr,
		const void *buf, int size)
{
	return cd->core->adap->write(cd->core->adap, addr, buf, size,
			cd->max_xfer);
}

/* cyttsp4_platform_detect_read()
 *
 * This function is passed to platform detect
 * function to perform a read operation
 */
static int cyttsp4_platform_detect_read(struct device *dev, u16 addr,
		void *buf, int size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	return cd->core->adap->read(cd->core->adap, addr, buf, size,
			cd->max_xfer);
}

static u16 cyttsp4_calc_partial_app_crc(const u8 *data, int size, u16 crc)
{
	int i, j;

	for (i = 0; i < size; i++) {
		crc ^= ((u16)data[i] << 8);
		for (j = 8; j > 0; j--)
			if (crc & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
	}

	return crc;
}

static inline u16 cyttsp4_calc_app_crc(const u8 *data, int size)
{
	return cyttsp4_calc_partial_app_crc(data, size, 0xFFFF);
}

static const u8 *cyttsp4_get_security_key_(struct cyttsp4_device *ttsp,
		int *size)
{
	if (size)
		*size = sizeof(security_key);

	return security_key;
}

static inline void cyttsp4_get_touch_axis(struct cyttsp4_core_data *cd,
		int *axis, int size, int max, u8 *xy_data, int bofs)
{
	int nbyte;
	int next;

	for (nbyte = 0, *axis = 0, next = 0; nbyte < size; nbyte++) {
		dev_vdbg(cd->dev,
			"%s: *axis=%02X(%d) size=%d max=%08X xy_data=%p"
			" xy_data[%d]=%02X(%d) bofs=%d\n",
			__func__, *axis, *axis, size, max, xy_data, next,
			xy_data[next], xy_data[next], bofs);
		*axis = (*axis * 256) + (xy_data[next] >> bofs);
		next++;
	}

	*axis &= max - 1;

	dev_vdbg(cd->dev,
		"%s: *axis=%02X(%d) size=%d max=%08X xy_data=%p"
		" xy_data[%d]=%02X(%d)\n",
		__func__, *axis, *axis, size, max, xy_data, next,
		xy_data[next], xy_data[next]);
}

/*
 * cyttsp4_get_touch_record_()
 *
 * Fills touch info for a touch record specified by rec_no
 * Should only be called in Operational mode IRQ attention and
 * rec_no should be less than the number of current touch records
 */
static void cyttsp4_get_touch_record_(struct cyttsp4_device *ttsp,
		int rec_no, int *rec_abs)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	struct device *dev = cd->dev;
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	u8 *xy_data = si->xy_data + (rec_no * si->si_ofs.tch_rec_size);
	enum cyttsp4_tch_abs abs;

	memset(rec_abs, 0, CY_TCH_NUM_ABS * sizeof(int));
	for (abs = CY_TCH_X; abs < CY_TCH_NUM_ABS; abs++) {
		cyttsp4_get_touch_axis(cd, &rec_abs[abs],
			si->si_ofs.tch_abs[abs].size,
			si->si_ofs.tch_abs[abs].max,
			xy_data + si->si_ofs.tch_abs[abs].ofs,
			si->si_ofs.tch_abs[abs].bofs);
		dev_vdbg(dev, "%s: get %s=%04X(%d)\n", __func__,
			cyttsp4_tch_abs_string[abs],
			rec_abs[abs], rec_abs[abs]);
	}
}

static int cyttsp4_load_status_and_touch_regs(struct cyttsp4_core_data *cd,
		bool optimize)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	struct device *dev = cd->dev;
	int first_read_len;
	int second_read_off;
	int num_read_rec;
	u8 num_cur_rec;
	u8 hst_mode;
	u8 rep_len;
	u8 rep_stat;
	u8 tt_stat;
	int rc;

	if (!si->xy_mode) {
		dev_err(cd->dev, "%s: NULL xy_mode pointer\n",
			__func__);
		return -EINVAL;
	}

	first_read_len = si->si_ofs.rep_hdr_size;
	/* Read one touch record additionally */
	if (optimize)
		first_read_len += si->si_ofs.tch_rec_size;

	rc = cyttsp4_adap_read(cd, si->si_ofs.rep_ofs,
			&si->xy_mode[si->si_ofs.rep_ofs], first_read_len);
	if (rc < 0) {
		dev_err(dev, "%s: fail read mode regs r=%d\n",
			__func__, rc);
		return rc;
	}

	/* print xy data */
	cyttsp4_pr_buf(dev, cd->pr_buf, si->xy_mode,
		si->si_ofs.mode_size, "xy_mode");

	hst_mode = si->xy_mode[CY_REG_BASE];
	rep_len = si->xy_mode[si->si_ofs.rep_ofs];
	rep_stat = si->xy_mode[si->si_ofs.rep_ofs + 1];
	tt_stat = si->xy_mode[si->si_ofs.tt_stat_ofs];
	dev_vdbg(dev, "%s: %s%02X %s%d %s%02X %s%02X\n", __func__,
		"hst_mode=", hst_mode, "rep_len=", rep_len,
		"rep_stat=", rep_stat, "tt_stat=", tt_stat);

	num_cur_rec = GET_NUM_TOUCH_RECORDS(tt_stat);
	dev_vdbg(dev, "%s: num_cur_rec=%d\n", __func__, num_cur_rec);

	if (rep_len == 0 && num_cur_rec > 0) {
		dev_err(dev, "%s: report length error rep_len=%d num_rec=%d\n",
			__func__, rep_len, num_cur_rec);
		return -EIO;
	}

	num_read_rec = num_cur_rec;
	second_read_off = si->si_ofs.tt_stat_ofs + 1;
	if (optimize) {
		num_read_rec--;
		second_read_off += si->si_ofs.tch_rec_size;
	}

	if (num_read_rec <= 0)
		goto exit_print;

	rc = cyttsp4_adap_read(cd, second_read_off,
			&si->xy_mode[second_read_off],
			num_read_rec * si->si_ofs.tch_rec_size);
	if (rc < 0) {
		dev_err(dev, "%s: read fail on touch regs r=%d\n",
			__func__, rc);
		return rc;
	}

exit_print:
	/* print xy data */
	cyttsp4_pr_buf(dev, cd->pr_buf, si->xy_data,
		num_cur_rec * si->si_ofs.tch_rec_size, "xy_data");

	return 0;
}

static int cyttsp4_handshake(struct cyttsp4_core_data *cd, u8 mode)
{
	u8 cmd = mode ^ CY_HST_TOGGLE;
	int rc;

	if (mode & CY_HST_MODE_CHANGE) {
		dev_err(cd->dev, "%s: Host mode change bit set, NO handshake\n",
				__func__);
		return 0;
	}

	rc = cyttsp4_adap_write(cd, CY_REG_BASE, &cmd, sizeof(cmd));
	if (rc < 0)
		dev_err(cd->dev, "%s: bus write fail on handshake (ret=%d)\n",
				__func__, rc);

	return rc;
}

static int cyttsp4_toggle_low_power_(struct cyttsp4_core_data *cd, u8 mode)
{
	u8 cmd = mode ^ CY_HST_LOWPOW;

	int rc = cyttsp4_adap_write(cd, CY_REG_BASE, &cmd, sizeof(cmd));
	if (rc < 0)
		dev_err(cd->dev,
			"%s: bus write fail on toggle low power (ret=%d)\n",
			__func__, rc);
	return rc;
}

static int cyttsp4_toggle_low_power(struct cyttsp4_core_data *cd, u8 mode)
{
	int rc;

	mutex_lock(&cd->system_lock);
	rc = cyttsp4_toggle_low_power_(cd, mode);
	mutex_unlock(&cd->system_lock);

	return rc;
}

static int cyttsp4_hw_soft_reset_(struct cyttsp4_core_data *cd)
{
	u8 cmd = CY_HST_RESET;

	int rc = cyttsp4_adap_write(cd, CY_REG_BASE, &cmd, sizeof(cmd));
	if (rc < 0) {
		dev_err(cd->dev, "%s: FAILED to execute SOFT reset\n",
				__func__);
		return rc;
	}
	dev_dbg(cd->dev, "%s: execute SOFT reset\n", __func__);
	return 0;
}

static int cyttsp4_hw_soft_reset(struct cyttsp4_core_data *cd)
{
	int rc;

	mutex_lock(&cd->system_lock);
	rc = cyttsp4_hw_soft_reset_(cd);
	mutex_unlock(&cd->system_lock);

	return rc;
}

static int cyttsp4_hw_hard_reset_(struct cyttsp4_core_data *cd)
{
	if (cd->pdata->xres) {
		cd->pdata->xres(cd->pdata, cd->dev);
		if(cd->core->dbg_msg_level & 0x2)
			dev_dbg(cd->dev, "%s: execute HARD reset\n", __func__);
		return 0;
	}
	dev_err(cd->dev, "%s: FAILED to execute HARD reset\n", __func__);
	return -ENOSYS;
}

static int cyttsp4_hw_hard_reset(struct cyttsp4_core_data *cd)
{
	int rc;

	mutex_lock(&cd->system_lock);
	rc = cyttsp4_hw_hard_reset_(cd);
	mutex_unlock(&cd->system_lock);

	return rc;
}

static int cyttsp4_hw_reset_(struct cyttsp4_core_data *cd)
{
	int rc = cyttsp4_hw_hard_reset_(cd);
	if (rc == -ENOSYS)
		rc = cyttsp4_hw_soft_reset_(cd);
	return rc;
}

static inline int cyttsp4_bits_2_bytes(int nbits, int *max)
{
	*max = 1 << nbits;
	return (nbits + 7) / 8;
}

static int cyttsp4_si_data_offsets(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	int rc = cyttsp4_adap_read(cd, CY_REG_BASE, &si->si_data,
				   sizeof(si->si_data));
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read sysinfo data offsets r=%d\n",
			__func__, rc);
		return rc;
	}

	/* Print sysinfo data offsets */
	cyttsp4_pr_buf(cd->dev, cd->pr_buf, (u8 *)&si->si_data,
		       sizeof(si->si_data), "sysinfo_data_offsets");

	/* convert sysinfo data offset bytes into integers */

	si->si_ofs.map_sz = merge_bytes(si->si_data.map_szh,
			si->si_data.map_szl);
	si->si_ofs.map_sz = merge_bytes(si->si_data.map_szh,
			si->si_data.map_szl);
	si->si_ofs.cydata_ofs = merge_bytes(si->si_data.cydata_ofsh,
			si->si_data.cydata_ofsl);
	si->si_ofs.test_ofs = merge_bytes(si->si_data.test_ofsh,
			si->si_data.test_ofsl);
	si->si_ofs.pcfg_ofs = merge_bytes(si->si_data.pcfg_ofsh,
			si->si_data.pcfg_ofsl);
	si->si_ofs.opcfg_ofs = merge_bytes(si->si_data.opcfg_ofsh,
			si->si_data.opcfg_ofsl);
	si->si_ofs.ddata_ofs = merge_bytes(si->si_data.ddata_ofsh,
			si->si_data.ddata_ofsl);
	si->si_ofs.mdata_ofs = merge_bytes(si->si_data.mdata_ofsh,
			si->si_data.mdata_ofsl);
	return rc;
}

static int cyttsp4_si_get_cydata(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	int read_offset;
	int mfgid_sz, calc_mfgid_sz;
	void *p;
	int rc;

	si->si_ofs.cydata_size = si->si_ofs.test_ofs - si->si_ofs.cydata_ofs;
#if 0
	dev_dbg(cd->dev, "%s: cydata size: %d\n", __func__,
			si->si_ofs.cydata_size);
#endif
	if (si->si_ofs.cydata_size <= 0)
		return -EINVAL;

	p = krealloc(si->si_ptrs.cydata, si->si_ofs.cydata_size, GFP_KERNEL);
	if (p == NULL) {
		dev_err(cd->dev, "%s: fail alloc cydata memory\n", __func__);
		return -ENOMEM;
	}
	si->si_ptrs.cydata = p;

	read_offset = si->si_ofs.cydata_ofs;

	/* Read the CYDA registers up to MFGID field */
	rc = cyttsp4_adap_read(cd, read_offset, si->si_ptrs.cydata,
			offsetof(struct cyttsp4_cydata, mfgid_sz)
			+ sizeof(si->si_ptrs.cydata->mfgid_sz));
	/* [OPT] Implement T2 self test, 20131017, Add Start */
	cd->core->sillicon_id = si->si_ptrs.cydata->jtag_si_id2;
	if(si->si_ptrs.cydata->jtag_si_id2==0x34)	// means 442 IC
		silicon_id = IC_442;
	/* [OPT] Implement T2 self test, 20131017, Add End */
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read cydata r=%d\n",
			__func__, rc);
		return rc;
	}

	/* Check MFGID size */
	mfgid_sz = si->si_ptrs.cydata->mfgid_sz;
	calc_mfgid_sz = si->si_ofs.cydata_size - sizeof(struct cyttsp4_cydata);
	if (mfgid_sz != calc_mfgid_sz) {
		dev_err(cd->dev, "%s: mismatch in MFGID size, reported:%d calculated:%d\n",
			__func__, mfgid_sz, calc_mfgid_sz);
		return -EINVAL;
	}

	read_offset += offsetof(struct cyttsp4_cydata, mfgid_sz)
			+ sizeof(si->si_ptrs.cydata->mfgid_sz);

	/* Read the CYDA registers for MFGID field */
	rc = cyttsp4_adap_read(cd, read_offset, si->si_ptrs.cydata->mfg_id,
			si->si_ptrs.cydata->mfgid_sz);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read cydata r=%d\n",
			__func__, rc);
		return rc;
	}

	read_offset += si->si_ptrs.cydata->mfgid_sz;

	/* Read the rest of the CYDA registers */
	rc = cyttsp4_adap_read(cd, read_offset, &si->si_ptrs.cydata->cyito_idh,
			sizeof(struct cyttsp4_cydata)
			- offsetof(struct cyttsp4_cydata, cyito_idh));
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read cydata r=%d\n",
			__func__, rc);
		return rc;
	}

	cyttsp4_pr_buf(cd->dev, cd->pr_buf, (u8 *)si->si_ptrs.cydata,
		si->si_ofs.cydata_size - mfgid_sz, "sysinfo_cydata");
	cyttsp4_pr_buf(cd->dev, cd->pr_buf, si->si_ptrs.cydata->mfg_id,
		mfgid_sz, "sysinfo_cydata_mfgid");
	return rc;
}

static int cyttsp4_si_get_test_data(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	void *p;
	int rc;

	si->si_ofs.test_size = si->si_ofs.pcfg_ofs - si->si_ofs.test_ofs;

	if (si->si_ofs.test_size <= 0)
		return -EINVAL;

	p = krealloc(si->si_ptrs.test, si->si_ofs.test_size, GFP_KERNEL);
	if (p == NULL) {
		dev_err(cd->dev, "%s: fail alloc test memory\n", __func__);
		return -ENOMEM;
	}
	si->si_ptrs.test = p;

	rc = cyttsp4_adap_read(cd, si->si_ofs.test_ofs, si->si_ptrs.test,
			si->si_ofs.test_size);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read test data r=%d\n",
			__func__, rc);
		return rc;
	}

	cyttsp4_pr_buf(cd->dev, cd->pr_buf,
		       (u8 *)si->si_ptrs.test, si->si_ofs.test_size,
		       "sysinfo_test_data");
	if (si->si_ptrs.test->post_codel &
	    CY_POST_CODEL_WDG_RST)
		dev_info(cd->dev, "%s: %s codel=%02X\n",
			 __func__, "Reset was a WATCHDOG RESET",
			 si->si_ptrs.test->post_codel);

	if (!(si->si_ptrs.test->post_codel &
	      CY_POST_CODEL_CFG_DATA_CRC_FAIL))
		dev_info(cd->dev, "%s: %s codel=%02X\n", __func__,
			 "Config Data CRC FAIL",
			 si->si_ptrs.test->post_codel);

	if (!(si->si_ptrs.test->post_codel &
	      CY_POST_CODEL_PANEL_TEST_FAIL))
		dev_info(cd->dev, "%s: %s codel=%02X\n",
			 __func__, "PANEL TEST FAIL",
			 si->si_ptrs.test->post_codel);

	dev_info(cd->dev, "%s: SCANNING is %s codel=%02X\n",
		 __func__, si->si_ptrs.test->post_codel & 0x08 ?
		 "ENABLED" : "DISABLED",
		 si->si_ptrs.test->post_codel);
	return rc;
}

static int cyttsp4_si_get_pcfg_data(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	void *p;
	int rc;

	dev_vdbg(cd->dev, "%s: get pcfg data\n", __func__);
	si->si_ofs.pcfg_size = si->si_ofs.opcfg_ofs - si->si_ofs.pcfg_ofs;

	if (si->si_ofs.pcfg_size <= 0)
		return -EINVAL;

	p = krealloc(si->si_ptrs.pcfg, si->si_ofs.pcfg_size, GFP_KERNEL);
	if (p == NULL) {
		rc = -ENOMEM;
		dev_err(cd->dev, "%s: fail alloc pcfg memory r=%d\n",
			__func__, rc);
		return rc;
	}
	si->si_ptrs.pcfg = p;

	rc = cyttsp4_adap_read(cd, si->si_ofs.pcfg_ofs, si->si_ptrs.pcfg,
			si->si_ofs.pcfg_size);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read pcfg data r=%d\n",
			__func__, rc);
		return rc;
	}

	si->si_ofs.max_x = merge_bytes((si->si_ptrs.pcfg->res_xh
			& CY_PCFG_RESOLUTION_X_MASK), si->si_ptrs.pcfg->res_xl);
	si->si_ofs.x_origin = !!(si->si_ptrs.pcfg->res_xh
			& CY_PCFG_ORIGIN_X_MASK);
	si->si_ofs.max_y = merge_bytes((si->si_ptrs.pcfg->res_yh
			& CY_PCFG_RESOLUTION_Y_MASK), si->si_ptrs.pcfg->res_yl);
	si->si_ofs.y_origin = !!(si->si_ptrs.pcfg->res_yh
			& CY_PCFG_ORIGIN_Y_MASK);
	si->si_ofs.max_p = merge_bytes(si->si_ptrs.pcfg->max_zh,
			si->si_ptrs.pcfg->max_zl);
	/* [OPT] Implement T2 self test, 20131017, Add Start */
	cd->core->panel_info = si->si_ptrs.pcfg->panel_info0;
	/* [OPT] Implement T2 self test, 20131017, Add End */

	cyttsp4_pr_buf(cd->dev, cd->pr_buf,
		       (u8 *)si->si_ptrs.pcfg,
		       si->si_ofs.pcfg_size, "sysinfo_pcfg_data");
	return rc;
}

static int cyttsp4_si_get_opcfg_data(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	int i;
	enum cyttsp4_tch_abs abs;
	void *p;
	int rc;

//	dev_vdbg(cd->dev, "%s: get opcfg data\n", __func__);
	si->si_ofs.opcfg_size = si->si_ofs.ddata_ofs - si->si_ofs.opcfg_ofs;

	if (si->si_ofs.opcfg_size <= 0)
		return -EINVAL;

	p = krealloc(si->si_ptrs.opcfg, si->si_ofs.opcfg_size, GFP_KERNEL);
	if (p == NULL) {
		dev_err(cd->dev, "%s: fail alloc opcfg memory\n", __func__);
		rc = -ENOMEM;
		goto cyttsp4_si_get_opcfg_data_exit;
	}
	si->si_ptrs.opcfg = p;

	rc = cyttsp4_adap_read(cd, si->si_ofs.opcfg_ofs, si->si_ptrs.opcfg,
			si->si_ofs.opcfg_size);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail read opcfg data r=%d\n",
			__func__, rc);
		goto cyttsp4_si_get_opcfg_data_exit;
	}
	si->si_ofs.cmd_ofs = si->si_ptrs.opcfg->cmd_ofs;
	si->si_ofs.rep_ofs = si->si_ptrs.opcfg->rep_ofs;
	si->si_ofs.rep_sz = (si->si_ptrs.opcfg->rep_szh * 256) +
		si->si_ptrs.opcfg->rep_szl;
	si->si_ofs.num_btns = si->si_ptrs.opcfg->num_btns;
	si->si_ofs.num_btn_regs = (si->si_ofs.num_btns +
		CY_NUM_BTN_PER_REG - 1) / CY_NUM_BTN_PER_REG;
	si->si_ofs.tt_stat_ofs = si->si_ptrs.opcfg->tt_stat_ofs;
	si->si_ofs.obj_cfg0 = si->si_ptrs.opcfg->obj_cfg0;
	si->si_ofs.max_tchs = si->si_ptrs.opcfg->max_tchs &
		CY_BYTE_OFS_MASK;
	si->si_ofs.tch_rec_size = si->si_ptrs.opcfg->tch_rec_size &
		CY_BYTE_OFS_MASK;

	/* Get the old touch fields */
	for (abs = CY_TCH_X; abs < CY_NUM_TCH_FIELDS; abs++) {
		si->si_ofs.tch_abs[abs].ofs =
			si->si_ptrs.opcfg->tch_rec_old[abs].loc &
			CY_BYTE_OFS_MASK;
		si->si_ofs.tch_abs[abs].size =
			cyttsp4_bits_2_bytes
			(si->si_ptrs.opcfg->tch_rec_old[abs].size,
			&si->si_ofs.tch_abs[abs].max);
		si->si_ofs.tch_abs[abs].bofs =
			(si->si_ptrs.opcfg->tch_rec_old[abs].loc &
			CY_BOFS_MASK) >> CY_BOFS_SHIFT;
	}

	/* button fields */
	si->si_ofs.btn_rec_size = si->si_ptrs.opcfg->btn_rec_size;
	si->si_ofs.btn_diff_ofs = si->si_ptrs.opcfg->btn_diff_ofs;
	si->si_ofs.btn_diff_size = si->si_ptrs.opcfg->btn_diff_size;

	if (IS_TTSP_VER_GE(si, 2, 3)) {
		/* Get the extended touch fields */
		for (i = 0; i < CY_NUM_EXT_TCH_FIELDS; abs++, i++) {
			si->si_ofs.tch_abs[abs].ofs =
				si->si_ptrs.opcfg->tch_rec_new[i].loc &
				CY_BYTE_OFS_MASK;
			si->si_ofs.tch_abs[abs].size =
				cyttsp4_bits_2_bytes
				(si->si_ptrs.opcfg->tch_rec_new[i].size,
				&si->si_ofs.tch_abs[abs].max);
			si->si_ofs.tch_abs[abs].bofs =
				(si->si_ptrs.opcfg->tch_rec_new[i].loc
				& CY_BOFS_MASK) >> CY_BOFS_SHIFT;
		}
	}

	if (IS_TTSP_VER_GE(si, 2, 4)) {
		si->si_ofs.noise_data_ofs = si->si_ptrs.opcfg->noise_data_ofs;
		si->si_ofs.noise_data_sz = si->si_ptrs.opcfg->noise_data_sz;
	}
#if 0
	for (abs = 0; abs < CY_TCH_NUM_ABS; abs++) {
		dev_dbg(cd->dev, "%s: tch_rec_%s\n", __func__,
			cyttsp4_tch_abs_string[abs]);
		dev_dbg(cd->dev, "%s:     ofs =%2d\n", __func__,
			si->si_ofs.tch_abs[abs].ofs);
		dev_dbg(cd->dev, "%s:     siz =%2d\n", __func__,
			si->si_ofs.tch_abs[abs].size);
		dev_dbg(cd->dev, "%s:     max =%2d\n", __func__,
			si->si_ofs.tch_abs[abs].max);
		dev_dbg(cd->dev, "%s:     bofs=%2d\n", __func__,
			si->si_ofs.tch_abs[abs].bofs);
	}
#endif
	si->si_ofs.mode_size = si->si_ofs.tt_stat_ofs + 1;
	si->si_ofs.data_size = si->si_ofs.max_tchs *
		si->si_ptrs.opcfg->tch_rec_size;
	si->si_ofs.rep_hdr_size = si->si_ofs.mode_size - si->si_ofs.rep_ofs;

	cyttsp4_pr_buf(cd->dev, cd->pr_buf, (u8 *)si->si_ptrs.opcfg,
		si->si_ofs.opcfg_size, "sysinfo_opcfg_data");

cyttsp4_si_get_opcfg_data_exit:
	return rc;
}

static int cyttsp4_si_get_ddata(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	void *p;
	int rc;

	dev_vdbg(cd->dev, "%s: get ddata data\n", __func__);
	si->si_ofs.ddata_size = si->si_ofs.mdata_ofs - si->si_ofs.ddata_ofs;

	if (si->si_ofs.ddata_size <= 0)
		return -EINVAL;

	p = krealloc(si->si_ptrs.ddata, si->si_ofs.ddata_size, GFP_KERNEL);
	if (p == NULL) {
		dev_err(cd->dev, "%s: fail alloc ddata memory\n", __func__);
		return -ENOMEM;
	}
	si->si_ptrs.ddata = p;

	rc = cyttsp4_adap_read(cd, si->si_ofs.ddata_ofs, si->si_ptrs.ddata,
			si->si_ofs.ddata_size);
	if (rc < 0)
		dev_err(cd->dev, "%s: fail read ddata data r=%d\n",
			__func__, rc);
	else
		cyttsp4_pr_buf(cd->dev, cd->pr_buf,
			       (u8 *)si->si_ptrs.ddata,
			       si->si_ofs.ddata_size, "sysinfo_ddata");
	return rc;
}

static int cyttsp4_si_get_mdata(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	void *p;
	int rc;

	dev_vdbg(cd->dev, "%s: get mdata data\n", __func__);
	si->si_ofs.mdata_size = si->si_ofs.map_sz - si->si_ofs.mdata_ofs;

	if (si->si_ofs.mdata_size <= 0)
		return -EINVAL;

	p = krealloc(si->si_ptrs.mdata, si->si_ofs.mdata_size, GFP_KERNEL);
	if (p == NULL) {
		dev_err(cd->dev, "%s: fail alloc mdata memory\n", __func__);
		return -ENOMEM;
	}
	si->si_ptrs.mdata = p;

	rc = cyttsp4_adap_read(cd, si->si_ofs.mdata_ofs, si->si_ptrs.mdata,
			si->si_ofs.mdata_size);
	if (rc < 0)
		dev_err(cd->dev, "%s: fail read mdata data r=%d\n",
			__func__, rc);
	else
		cyttsp4_pr_buf(cd->dev, cd->pr_buf,
			       (u8 *)si->si_ptrs.mdata,
			       si->si_ofs.mdata_size, "sysinfo_mdata");
	return rc;
}

static int cyttsp4_si_get_btn_data(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	int btn;
	int num_defined_keys;
	u16 *key_table;
	void *p;
	int rc = 0;

	dev_vdbg(cd->dev, "%s: get btn data\n", __func__);

	if (!si->si_ofs.num_btns) {
		si->si_ofs.btn_keys_size = 0;
		kfree(si->btn);
		si->btn = NULL;
		return rc;
	}

	si->si_ofs.btn_keys_size = si->si_ofs.num_btns *
		sizeof(struct cyttsp4_btn);

	if (si->si_ofs.btn_keys_size <= 0)
		return -EINVAL;

	p = krealloc(si->btn, si->si_ofs.btn_keys_size, GFP_KERNEL|__GFP_ZERO);
	if (p == NULL) {
		dev_err(cd->dev, "%s: %s\n", __func__,
			"fail alloc btn_keys memory");
		return -ENOMEM;
	}
	si->btn = p;

	if (cd->pdata->sett[CY_IC_GRPNUM_BTN_KEYS] == NULL)
		num_defined_keys = 0;
	else if (cd->pdata->sett[CY_IC_GRPNUM_BTN_KEYS]->data == NULL)
		num_defined_keys = 0;
	else
		num_defined_keys = cd->pdata->sett[CY_IC_GRPNUM_BTN_KEYS]->size;

	for (btn = 0; btn < si->si_ofs.num_btns
			&& btn < num_defined_keys; btn++) {
		key_table = (u16 *)cd->pdata->sett[CY_IC_GRPNUM_BTN_KEYS]->data;
		si->btn[btn].key_code = key_table[btn];
		si->btn[btn].state = CY_BTN_RELEASED;
		si->btn[btn].enabled = true;
	}
	for (; btn < si->si_ofs.num_btns; btn++) {
		si->btn[btn].key_code = KEY_RESERVED;
		si->btn[btn].state = CY_BTN_RELEASED;
		si->btn[btn].enabled = true;
	}

	return rc;
}

static int cyttsp4_si_get_op_data_ptrs(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	void *p;
	int size;

	p = krealloc(si->xy_mode, si->si_ofs.mode_size +
			si->si_ofs.data_size, GFP_KERNEL|__GFP_ZERO);
	if (p == NULL)
		return -ENOMEM;
	si->xy_mode = p;
	si->xy_data = &si->xy_mode[si->si_ofs.tt_stat_ofs + 1];

	size = si->si_ofs.btn_rec_size * si->si_ofs.num_btns;
	if (!size)
		return 0;

	p = krealloc(si->btn_rec_data, size, GFP_KERNEL|__GFP_ZERO);
	if (p == NULL)
		return -ENOMEM;
	si->btn_rec_data = p;

	return 0;
}
#if 0
static void cyttsp4_si_put_log_data(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	dev_dbg(cd->dev, "%s: cydata_ofs =%4d siz=%4d\n", __func__,
		si->si_ofs.cydata_ofs, si->si_ofs.cydata_size);
	dev_dbg(cd->dev, "%s: test_ofs   =%4d siz=%4d\n", __func__,
		si->si_ofs.test_ofs, si->si_ofs.test_size);
	dev_dbg(cd->dev, "%s: pcfg_ofs   =%4d siz=%4d\n", __func__,
		si->si_ofs.pcfg_ofs, si->si_ofs.pcfg_size);
	dev_dbg(cd->dev, "%s: opcfg_ofs  =%4d siz=%4d\n", __func__,
		si->si_ofs.opcfg_ofs, si->si_ofs.opcfg_size);
	dev_dbg(cd->dev, "%s: ddata_ofs  =%4d siz=%4d\n", __func__,
		si->si_ofs.ddata_ofs, si->si_ofs.ddata_size);
	dev_dbg(cd->dev, "%s: mdata_ofs  =%4d siz=%4d\n", __func__,
		si->si_ofs.mdata_ofs, si->si_ofs.mdata_size);

	dev_dbg(cd->dev, "%s: cmd_ofs       =%4d\n", __func__,
		si->si_ofs.cmd_ofs);
	dev_dbg(cd->dev, "%s: rep_ofs       =%4d\n", __func__,
		si->si_ofs.rep_ofs);
	dev_dbg(cd->dev, "%s: rep_sz        =%4d\n", __func__,
		si->si_ofs.rep_sz);
	dev_dbg(cd->dev, "%s: num_btns      =%4d\n", __func__,
		si->si_ofs.num_btns);
	dev_dbg(cd->dev, "%s: num_btn_regs  =%4d\n", __func__,
		si->si_ofs.num_btn_regs);
	dev_dbg(cd->dev, "%s: tt_stat_ofs   =%4d\n", __func__,
		si->si_ofs.tt_stat_ofs);
	dev_dbg(cd->dev, "%s: tch_rec_size   =%4d\n", __func__,
		si->si_ofs.tch_rec_size);
	dev_dbg(cd->dev, "%s: max_tchs      =%4d\n", __func__,
		si->si_ofs.max_tchs);
	dev_dbg(cd->dev, "%s: mode_size     =%4d\n", __func__,
		si->si_ofs.mode_size);
	dev_dbg(cd->dev, "%s: data_size     =%4d\n", __func__,
		si->si_ofs.data_size);
	dev_dbg(cd->dev, "%s: rep_hdr_size  =%4d\n", __func__,
		si->si_ofs.rep_hdr_size);
	dev_dbg(cd->dev, "%s: map_sz        =%4d\n", __func__,
		si->si_ofs.map_sz);

	dev_dbg(cd->dev, "%s: btn_rec_size   =%2d\n", __func__,
		si->si_ofs.btn_rec_size);
	dev_dbg(cd->dev, "%s: btn_diff_ofs  =%2d\n", __func__,
		si->si_ofs.btn_diff_ofs);
	dev_dbg(cd->dev, "%s: btn_diff_size  =%2d\n", __func__,
		si->si_ofs.btn_diff_size);

	dev_dbg(cd->dev, "%s: max_x    = 0x%04X (%d)\n", __func__,
		si->si_ofs.max_x, si->si_ofs.max_x);
	dev_dbg(cd->dev, "%s: x_origin = %d (%s)\n", __func__,
		si->si_ofs.x_origin,
		si->si_ofs.x_origin == CY_NORMAL_ORIGIN ?
		"left corner" : "right corner");
	dev_dbg(cd->dev, "%s: max_y    = 0x%04X (%d)\n", __func__,
		si->si_ofs.max_y, si->si_ofs.max_y);
	dev_dbg(cd->dev, "%s: y_origin = %d (%s)\n", __func__,
		si->si_ofs.y_origin,
		si->si_ofs.y_origin == CY_NORMAL_ORIGIN ?
		"upper corner" : "lower corner");
	dev_dbg(cd->dev, "%s: max_p    = 0x%04X (%d)\n", __func__,
		si->si_ofs.max_p, si->si_ofs.max_p);

	dev_dbg(cd->dev, "%s: xy_mode=%p xy_data=%p\n", __func__,
		si->xy_mode, si->xy_data);
}
#endif
static int cyttsp4_get_sysinfo_regs(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	int rc;

	rc = cyttsp4_si_data_offsets(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_cydata(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_test_data(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_pcfg_data(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_opcfg_data(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_ddata(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_mdata(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_btn_data(cd);
	if (rc < 0)
		return rc;

	rc = cyttsp4_si_get_op_data_ptrs(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get_op_data\n",
			__func__);
		return rc;
	}

//	cyttsp4_si_put_log_data(cd);

	/* provide flow control handshake */
	rc = cyttsp4_handshake(cd, si->si_data.hst_mode);
	if (rc < 0)
		dev_err(cd->dev, "%s: handshake fail on sysinfo reg\n",
			__func__);

	mutex_lock(&cd->system_lock);
	si->ready = true;
	mutex_unlock(&cd->system_lock);
	return rc;
}

static void cyttsp4_queue_startup_(struct cyttsp4_core_data *cd)
{
	if (cd->startup_state == STARTUP_NONE) {
		cd->startup_state = STARTUP_QUEUED;
		schedule_work(&cd->startup_work);
		dev_info(cd->dev, "%s: cyttsp4_startup queued\n", __func__);
	} else {
		dev_dbg(cd->dev, "%s: startup_state = %d\n", __func__,
			cd->startup_state);
	}
}

static void cyttsp4_queue_startup(struct cyttsp4_core_data *cd)
{
	dev_vdbg(cd->dev, "%s: enter\n", __func__);
	mutex_lock(&cd->system_lock);
	cyttsp4_queue_startup_(cd);
	mutex_unlock(&cd->system_lock);
}

static void call_atten_cb(struct cyttsp4_core_data *cd,
		enum cyttsp4_atten_type type, int mode)
{
	struct atten_node *atten, *atten_n;

	dev_vdbg(cd->dev, "%s: check list type=%d mode=%d\n",
		__func__, type, mode);
	spin_lock(&cd->spinlock);
	list_for_each_entry_safe(atten, atten_n,
			&cd->atten_list[type], node) {
		if (!mode || atten->mode & mode) {
			spin_unlock(&cd->spinlock);
			dev_vdbg(cd->dev, "%s: attention for '%s'", __func__,
				dev_name(&atten->ttsp->dev));
			atten->func(atten->ttsp);
			spin_lock(&cd->spinlock);
		}
	}
	spin_unlock(&cd->spinlock);
}

static irqreturn_t cyttsp4_hard_irq(int irq, void *handle)
{
	struct cyttsp4_core_data *cd = handle;

	/*
	 * Check whether this IRQ should be ignored (external)
	 * This should be the very first thing to check since
	 * ignore_irq may be set for a very short period of time
	 */
	if (atomic_read(&cd->ignore_irq)) {
		dev_vdbg(cd->dev, "%s: Ignoring IRQ\n", __func__);
		return IRQ_HANDLED;
	}

	return IRQ_WAKE_THREAD;
}

static irqreturn_t cyttsp4_irq(int irq, void *handle)
{
	struct cyttsp4_core_data *cd = handle;
	struct device *dev = cd->dev;
	enum cyttsp4_mode cur_mode;
	u8 cmd_ofs = cd->sysinfo.si_ofs.cmd_ofs;
	bool command_complete = false;
	u8 mode[3];
	int rc;
	u8 cat_masked_cmd;

/* [Optical][Touch] Driver porting, 20130717, Del Start */
//TODO:enable/disable log
//	dev_dbg(dev, "%s int:0x%x\n", __func__, cd->int_status);
/* [Optical][Touch] Driver porting, 20130717, Del End */

	mutex_lock(&cd->system_lock);

	rc = cyttsp4_adap_read(cd, CY_REG_BASE, mode, sizeof(mode));
	if (rc) {
		dev_err(cd->dev, "%s: Fail read adapter r=%d\n", __func__, rc);
		goto cyttsp4_irq_exit;
	}
	dev_vdbg(dev, "%s mode[0-2]:0x%X 0x%X 0x%X\n", __func__,
			mode[0], mode[1], mode[2]);
	/* [OPT] Cypress AE solved the upgrade failed, 20130918, Add Start */
	if (unlikely(mode[0] == CY_CORE_BL_HOST_SYNC_BYTE)
	        && unlikely(cd->mode == CY_MODE_BOOTLOADER)) {
		dev_err(dev, "%s: False interrupt in bootloader mode\n", __func__);
		goto cyttsp4_irq_exit;
	}
	/* [OPT] Cypress AE solved the upgrade failed, 20130918, Add End */
	if (IS_BOOTLOADER(mode[0], mode[1])) {
		cur_mode = CY_MODE_BOOTLOADER;
		dev_vdbg(dev, "%s: bl running\n", __func__);
		call_atten_cb(cd, CY_ATTEN_IRQ, cur_mode);

		/* switch to bootloader */
		if (cd->mode != CY_MODE_BOOTLOADER)
			dev_dbg(dev, "%s: restart switch to bl m=%d -> m=%d\n",
			__func__, cd->mode, cur_mode);

		/* catch operation->bl glitch */
		if (cd->mode != CY_MODE_BOOTLOADER
				&& cd->mode != CY_MODE_UNKNOWN) {
			/* Incase startup_state do not let startup_() */
			cd->mode = CY_MODE_UNKNOWN;
			cyttsp4_queue_startup_(cd);
			goto cyttsp4_irq_exit;
		}

		/* Recover if stuck in bootloader idle mode */
		if (cd->mode == CY_MODE_BOOTLOADER) {
			if (IS_BOOTLOADER_IDLE(mode[0], mode[1])) {
				if (cd->heartbeat_count > 3) {
					cd->heartbeat_count = 0;
					cyttsp4_queue_startup_(cd);
					goto cyttsp4_irq_exit;
				}
				cd->heartbeat_count++;
			}
		}

		cd->mode = cur_mode;
		/* Signal bootloader heartbeat heard */
		wake_up(&cd->wait_q);
		goto cyttsp4_irq_exit;
	}

	switch (mode[0] & CY_HST_DEVICE_MODE) {
	case CY_HST_OPERATE:
		cur_mode = CY_MODE_OPERATIONAL;
		dev_vdbg(dev, "%s: operational\n", __func__);
		break;
	case CY_HST_CAT:
		cur_mode = CY_MODE_CAT;
		/* set the start sensor mode state. */
		cat_masked_cmd = mode[2] & CY_CMD_MASK;

		/* Get the Debug info for the interrupt. */
		if (cat_masked_cmd != CY_CMD_CAT_NULL &&
				cat_masked_cmd !=
					CY_CMD_CAT_RETRIEVE_PANEL_SCAN &&
				cat_masked_cmd != CY_CMD_CAT_EXEC_PANEL_SCAN)
			if(cd->core->dbg_msg_level & 0x2)
				dev_info(cd->dev,
					"%s: cyttsp4_CaT_IRQ=%02X %02X %02X\n",
				__func__, mode[0], mode[1], mode[2]);
		dev_vdbg(dev, "%s: CaT\n", __func__);
		break;
	case CY_HST_SYSINFO:
		cur_mode = CY_MODE_SYSINFO;
		dev_vdbg(dev, "%s: sysinfo\n", __func__);
		break;
	default:
		cur_mode = CY_MODE_UNKNOWN;
		dev_err(dev, "%s: unknown HST mode 0x%02X\n", __func__,
			mode[0]);
		break;
	}

	/* Check whether this IRQ should be ignored (internal) */
	if (cd->int_status & CY_INT_IGNORE) {
		if (IS_DEEP_SLEEP_CONFIGURED(cd->easy_wakeup_gesture)) {
			/* Put device back to sleep on premature wakeup */
			dev_dbg(dev, "%s: Put device back to sleep\n",
				__func__);
			_cyttsp4_put_device_into_deep_sleep(cd, mode[0]);
			goto cyttsp4_irq_exit;
		}
		/* Check for Wait for Event command */
		if ((mode[cmd_ofs] & CY_CMD_MASK) == CY_CMD_OP_WAIT_FOR_EVENT
				&& mode[cmd_ofs] & CY_CMD_COMPLETE) {
			cd->wake_initiated_by_device = 1;
			call_atten_cb(cd, CY_ATTEN_WAKE, 0);
			goto cyttsp4_irq_handshake;
		}
	}

	/* Check for wake up interrupt */
	if (cd->int_status & CY_INT_AWAKE) {
		cd->int_status &= ~CY_INT_AWAKE;
		wake_up(&cd->wait_q);
		dev_vdbg(dev, "%s: Received wake up interrupt\n", __func__);
		goto cyttsp4_irq_handshake;
	}

	/* Expecting mode change interrupt */
	if ((cd->int_status & CY_INT_MODE_CHANGE)
			&& (mode[0] & CY_HST_MODE_CHANGE) == 0) {
		cd->int_status &= ~CY_INT_MODE_CHANGE;
//		dev_dbg(dev, "%s: finish mode switch m=%d -> m=%d\n",
//				__func__, cd->mode, cur_mode);
		cd->mode = cur_mode;
		wake_up(&cd->wait_q);
		goto cyttsp4_irq_handshake;
	}

	/* compare current core mode to current device mode */
	dev_vdbg(dev, "%s: cd->mode=%d cur_mode=%d\n",
			__func__, cd->mode, cur_mode);
	if ((mode[0] & CY_HST_MODE_CHANGE) == 0 && cd->mode != cur_mode) {
		/* Unexpected mode change occurred */
		dev_err(dev, "%s %d->%d 0x%x\n", __func__, cd->mode,
				cur_mode, cd->int_status);
		dev_vdbg(dev, "%s: Unexpected mode change, startup\n",
				__func__);
		cyttsp4_queue_startup_(cd);
		goto cyttsp4_irq_exit;
	}

	/* Expecting command complete interrupt */
	dev_vdbg(dev, "%s: command byte:0x%x, toggle:0x%x\n",
			__func__, mode[cmd_ofs], cd->cmd_toggle);
	if ((cd->int_status & CY_INT_EXEC_CMD)
			&& mode[cmd_ofs] & CY_CMD_COMPLETE) {
		command_complete = true;
		cd->int_status &= ~CY_INT_EXEC_CMD;
		dev_vdbg(dev, "%s: Received command complete interrupt\n",
				__func__);
		wake_up(&cd->wait_q);
		/*
		 * It is possible to receive a single interrupt for
		 * command complete and touch/button status report.
		 * Continue processing for a possible status report.
		 */
	}

	/* Copy the mode registers */
	if (cd->sysinfo.xy_mode)
		memcpy(cd->sysinfo.xy_mode, mode, sizeof(mode));

	/* This should be status report, read status and touch regs */
	if (cd->mode == CY_MODE_OPERATIONAL) {
		dev_vdbg(dev, "%s: Read status and touch registers\n",
			__func__);
		rc = cyttsp4_load_status_and_touch_regs(cd, !command_complete);
		if (rc < 0)
			dev_err(dev, "%s: fail read mode/touch regs r=%d\n",
				__func__, rc);
	}

	/* attention IRQ */
	call_atten_cb(cd, CY_ATTEN_IRQ, cd->mode);

cyttsp4_irq_handshake:
	/* handshake the event */
	dev_vdbg(dev, "%s: Handshake mode=0x%02X r=%d\n",
			__func__, mode[0], rc);
	rc = cyttsp4_handshake(cd, mode[0]);
	if (rc < 0)
		dev_err(dev, "%s: Fail handshake mode=0x%02X r=%d\n",
				__func__, mode[0], rc);

	/*
	 * a non-zero udelay period is required for using
	 * IRQF_TRIGGER_LOW in order to delay until the
	 * device completes isr deassert
	 */
	udelay(cd->pdata->level_irq_udelay);

cyttsp4_irq_exit:
	mutex_unlock(&cd->system_lock);
	dev_vdbg(dev, "%s: irq done\n", __func__);
	return IRQ_HANDLED;
}

static void cyttsp4_start_wd_timer(struct cyttsp4_core_data *cd)
{
	if (!CY_WATCHDOG_TIMEOUT)
		return;

	mod_timer(&cd->watchdog_timer, jiffies +
			msecs_to_jiffies(CY_WATCHDOG_TIMEOUT));
}

static void cyttsp4_stop_wd_timer(struct cyttsp4_core_data *cd)
{
	if (!CY_WATCHDOG_TIMEOUT)
		return;

	/*
	 * Ensure we wait until the watchdog timer
	 * running on a different CPU finishes
	 */
	del_timer_sync(&cd->watchdog_timer);
	cancel_work_sync(&cd->watchdog_work);
	del_timer_sync(&cd->watchdog_timer);
}

static void cyttsp4_watchdog_timer(unsigned long handle)
{
	struct cyttsp4_core_data *cd = (struct cyttsp4_core_data *)handle;

	dev_vdbg(cd->dev, "%s: Timer triggered\n", __func__);

	if (!cd)
		return;

	if (!work_pending(&cd->watchdog_work))
		schedule_work(&cd->watchdog_work);

	return;
}

static int cyttsp4_write_(struct cyttsp4_device *ttsp, int mode, u16 addr,
	const void *buf, int size)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	int rc = 0;

	mutex_lock(&cd->adap_lock);
	if (mode != cd->mode) {
		dev_dbg(dev, "%s: %s (having %x while %x requested)\n",
			__func__, "attempt to write in missing mode",
			cd->mode, mode);
		rc = -EACCES;
		goto exit;
	}
	rc = cyttsp4_adap_write(cd, addr, buf, size);
exit:
	mutex_unlock(&cd->adap_lock);
	return rc;
}

static int cyttsp4_read_(struct cyttsp4_device *ttsp, int mode, u16 addr,
	void *buf, int size)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	int rc = 0;

	mutex_lock(&cd->adap_lock);
	if (mode != cd->mode) {
		dev_dbg(dev, "%s: %s (having %x while %x requested)\n",
			__func__, "attempt to read in missing mode",
			cd->mode, mode);
		rc = -EACCES;
		goto exit;
	}
	rc = cyttsp4_adap_read(cd, addr, buf, size);
exit:
	mutex_unlock(&cd->adap_lock);
	return rc;
}

static int cyttsp4_subscribe_attention_(struct cyttsp4_device *ttsp,
	enum cyttsp4_atten_type type,
	int (*func)(struct cyttsp4_device *), int mode)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	struct atten_node *atten, *atten_new;

	atten_new = kzalloc(sizeof(*atten_new), GFP_KERNEL);
	if (!atten_new) {
		dev_err(cd->dev, "%s: Fail alloc atten node\n", __func__);
		return -ENOMEM;
	}

	if(core->dbg_msg_level & 0x2)
		dev_dbg(cd->dev, "%s from '%s'\n", __func__, dev_name(cd->dev));

	spin_lock(&cd->spinlock);
	list_for_each_entry(atten, &cd->atten_list[type], node) {
		if (atten->ttsp == ttsp && atten->mode == mode) {
			spin_unlock(&cd->spinlock);
			kfree(atten_new);
			dev_vdbg(cd->dev, "%s: %s=%p %s=%d\n",
				 __func__,
				 "already subscribed attention",
				 ttsp, "mode", mode);

			return 0;
		}
	}

	atten_new->ttsp = ttsp;
	atten_new->mode = mode;
	atten_new->func = func;

	list_add(&atten_new->node, &cd->atten_list[type]);
	spin_unlock(&cd->spinlock);

	return 0;
}

static int cyttsp4_unsubscribe_attention_(struct cyttsp4_device *ttsp,
	enum cyttsp4_atten_type type, int (*func)(struct cyttsp4_device *),
	int mode)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	struct atten_node *atten, *atten_n;

	spin_lock(&cd->spinlock);
	list_for_each_entry_safe(atten, atten_n, &cd->atten_list[type], node) {
		if (atten->ttsp == ttsp && atten->mode == mode) {
			list_del(&atten->node);
			spin_unlock(&cd->spinlock);
			kfree(atten);
			dev_vdbg(cd->dev, "%s: %s=%p %s=%d\n",
				__func__,
				"unsub for atten->ttsp", atten->ttsp,
				"atten->mode", atten->mode);
			return 0;
		}
	}
	spin_unlock(&cd->spinlock);

	return -ENODEV;
}

static int request_exclusive(struct cyttsp4_core_data *cd, void *ownptr,
		int timeout_ms)
{
	int t = msecs_to_jiffies(timeout_ms);
	bool with_timeout = (timeout_ms != 0);

	mutex_lock(&cd->system_lock);
	if (!cd->exclusive_dev && cd->exclusive_waits == 0) {
		cd->exclusive_dev = ownptr;
		goto exit;
	}

	cd->exclusive_waits++;
wait:
	mutex_unlock(&cd->system_lock);
	if (with_timeout) {
		t = wait_event_timeout(cd->wait_q, !cd->exclusive_dev, t);
		if (IS_TMO(t)) {
			dev_err(cd->dev, "%s: tmo waiting exclusive access\n",
				__func__);
			mutex_lock(&cd->system_lock);
			cd->exclusive_waits--;
			mutex_unlock(&cd->system_lock);
			return -ETIME;
		}
	} else {
		wait_event(cd->wait_q, !cd->exclusive_dev);
	}
	mutex_lock(&cd->system_lock);
	if (cd->exclusive_dev)
		goto wait;
	cd->exclusive_dev = ownptr;
	cd->exclusive_waits--;
exit:
	mutex_unlock(&cd->system_lock);
	dev_vdbg(cd->dev, "%s: request_exclusive ok=%p\n",
		__func__, ownptr);

	return 0;
}

static int cyttsp4_request_exclusive_(struct cyttsp4_device *ttsp,
		int timeout_ms)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	return request_exclusive(cd, (void *)ttsp, timeout_ms);
}

/*
 * returns error if was not owned
 */
static int release_exclusive(struct cyttsp4_core_data *cd, void *ownptr)
{
	mutex_lock(&cd->system_lock);
	if (cd->exclusive_dev != ownptr) {
		mutex_unlock(&cd->system_lock);
		return -EINVAL;
	}

	dev_vdbg(cd->dev, "%s: exclusive_dev %p freed\n",
		__func__, cd->exclusive_dev);
	cd->exclusive_dev = NULL;
	wake_up(&cd->wait_q);
	mutex_unlock(&cd->system_lock);
	return 0;
}

static int cyttsp4_release_exclusive_(struct cyttsp4_device *ttsp)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	return release_exclusive(cd, (void *)ttsp);
}

static int cyttsp4_wait_bl_heartbeat(struct cyttsp4_core_data *cd)
{
	long t;
	int rc = 0;

	/* wait heartbeat */
	dev_vdbg(cd->dev, "%s: wait heartbeat...\n", __func__);
	t = wait_event_timeout(cd->wait_q, cd->mode == CY_MODE_BOOTLOADER,
			msecs_to_jiffies(CY_CORE_RESET_AND_WAIT_TIMEOUT));
	if (IS_TMO(t)) {
		dev_err(cd->dev, "%s: tmo waiting bl heartbeat cd->mode=%d\n",
			__func__, cd->mode);
		rc = -ETIME;
	}

	return rc;
}

static int cyttsp4_wait_sysinfo_mode(struct cyttsp4_core_data *cd)
{
	long t;

	dev_vdbg(cd->dev, "%s: wait sysinfo...\n", __func__);

	t = wait_event_timeout(cd->wait_q, cd->mode == CY_MODE_SYSINFO,
			msecs_to_jiffies(CY_CORE_WAIT_SYSINFO_MODE_TIMEOUT));
	if (IS_TMO(t)) {
		dev_err(cd->dev, "%s: tmo waiting exit bl cd->mode=%d\n",
			__func__, cd->mode);
		mutex_lock(&cd->system_lock);
		cd->int_status &= ~CY_INT_MODE_CHANGE;
		mutex_unlock(&cd->system_lock);
		return -ETIME;
	}

	return 0;
}

static int cyttsp4_reset_and_wait(struct cyttsp4_core_data *cd)
{
	int rc;

	/* reset hardware */
	mutex_lock(&cd->system_lock);
	if(cd->core->dbg_msg_level & 0x2)
		dev_dbg(cd->dev, "%s: reset hw...\n", __func__);
	rc = cyttsp4_hw_reset_(cd);
	cd->mode = CY_MODE_UNKNOWN;
	mutex_unlock(&cd->system_lock);
	if (rc < 0) {
		dev_err(cd->dev, "%s: %s adap='%s' r=%d\n", __func__,
			"Fail hw reset", cd->core->adap->id, rc);
		return rc;
	}

	return cyttsp4_wait_bl_heartbeat(cd);
}

/*
 * returns err if refused or timeout; block until mode change complete
 * bit is set (mode change interrupt)
 */
static int set_mode(struct cyttsp4_core_data *cd, int new_mode)
{
	u8 new_dev_mode;
	u8 mode;
	long t;
	int rc;

	switch (new_mode) {
	case CY_MODE_OPERATIONAL:
		new_dev_mode = CY_HST_OPERATE;
		break;
	case CY_MODE_SYSINFO:
		new_dev_mode = CY_HST_SYSINFO;
		break;
	case CY_MODE_CAT:
		new_dev_mode = CY_HST_CAT;
		break;
	default:
		dev_err(cd->dev, "%s: invalid mode: %02X(%d)\n",
			__func__, new_mode, new_mode);
		return -EINVAL;
	}

	/* change mode */
//	dev_dbg(cd->dev, "%s: %s=%p new_dev_mode=%02X new_mode=%d\n",
//			__func__, "have exclusive", cd->exclusive_dev,
//			new_dev_mode, new_mode);

	mutex_lock(&cd->system_lock);
	rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
	if (rc < 0) {
		mutex_unlock(&cd->system_lock);
		dev_err(cd->dev, "%s: Fail read mode r=%d\n",
			__func__, rc);
		goto exit;
	}

	/* Clear device mode bits and set to new mode */
	mode &= ~CY_HST_DEVICE_MODE;
	mode |= new_dev_mode | CY_HST_MODE_CHANGE;

	cd->int_status |= CY_INT_MODE_CHANGE;
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, &mode, sizeof(mode));
	mutex_unlock(&cd->system_lock);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Fail write mode change r=%d\n",
				__func__, rc);
		goto exit;
	}

	/* wait for mode change done interrupt */
	t = wait_event_timeout(cd->wait_q,
			(cd->int_status & CY_INT_MODE_CHANGE) == 0,
			msecs_to_jiffies(CY_CORE_MODE_CHANGE_TIMEOUT));
//	dev_dbg(cd->dev, "%s: back from wait t=%ld cd->mode=%d\n",
//			__func__, t, cd->mode);

	if (IS_TMO(t)) {
		dev_err(cd->dev, "%s: %s\n", __func__,
				"tmo waiting mode change");
		mutex_lock(&cd->system_lock);
		cd->int_status &= ~CY_INT_MODE_CHANGE;
		mutex_unlock(&cd->system_lock);
		rc = -EINVAL;
	}

exit:
	return rc;
}

/*
 * returns err if refused or timeout(core uses fixed timeout period) occurs;
 * blocks until ISR occurs
 */
static int cyttsp4_request_reset_(struct cyttsp4_device *ttsp)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	int rc;

	mutex_lock(&cd->system_lock);
	cd->sysinfo.ready = false;
	mutex_unlock(&cd->system_lock);

	rc = cyttsp4_reset_and_wait(cd);
	if (rc < 0)
		dev_err(cd->dev, "%s: Error on h/w reset r=%d\n",
			__func__, rc);

	return rc;
}

/*
 * returns err if refused ; if no error then restart has completed
 * and system is in normal operating mode
 */
static int cyttsp4_request_restart_(struct cyttsp4_device *ttsp, bool wait)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);

	mutex_lock(&cd->system_lock);
	cd->sysinfo.ready = false;
	mutex_unlock(&cd->system_lock);

	cyttsp4_queue_startup(cd);

	if (wait)
		wait_event(cd->wait_q, cd->startup_state == STARTUP_NONE);

	return 0;
}

static int cyttsp4_request_set_mode_(struct cyttsp4_device *ttsp, int mode)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	int rc;

	rc = set_mode(cd, mode);
	if (rc < 0)
		dev_err(cd->dev, "%s: fail set_mode=%02X(%d)\n",
			__func__, cd->mode, cd->mode);

	return rc;
}

/*
 * returns NULL if sysinfo has not been acquired from the device yet
 */
static struct cyttsp4_sysinfo *cyttsp4_request_sysinfo_(
		struct cyttsp4_device *ttsp)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	bool ready;

	mutex_lock(&cd->system_lock);
	ready = cd->sysinfo.ready;
	mutex_unlock(&cd->system_lock);
	if (ready)
		return &cd->sysinfo;

	return NULL;
}

static struct cyttsp4_loader_platform_data *cyttsp4_request_loader_pdata_(
		struct cyttsp4_device *ttsp)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	return cd->pdata->loader_pdata;
}

static int cyttsp4_request_handshake_(struct cyttsp4_device *ttsp, u8 mode)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	int rc;

	rc = cyttsp4_handshake(cd, mode);
	if (rc < 0)
		dev_err(&core->dev, "%s: Fail handshake r=%d\n", __func__, rc);

	return rc;
}

static int cyttsp4_request_toggle_lowpower_(struct cyttsp4_device *ttsp,
		u8 mode)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	int rc = cyttsp4_toggle_low_power(cd, mode);
	if (rc < 0)
		dev_err(&core->dev, "%s: Fail toggle low power r=%d\n",
				__func__, rc);
	return rc;
}

static int _cyttsp4_wait_cmd_exec(struct cyttsp4_core_data *cd, int timeout_ms)
{
	struct device *dev = cd->dev;
	int rc;

	rc = wait_event_timeout(cd->wait_q,
			(cd->int_status & CY_INT_EXEC_CMD) == 0,
			msecs_to_jiffies(timeout_ms));
	if (IS_TMO(rc)) {
		dev_err(dev, "%s: Command execution timed out\n",
				__func__);
		cd->int_status &= ~CY_INT_EXEC_CMD;
		return -ETIME;
	}
	return 0;
}

static int _get_cmd_offs(struct cyttsp4_core_data *cd, u8 mode)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	struct device *dev = cd->dev;
	int cmd_ofs;

	switch (mode) {
	case CY_MODE_CAT:
		cmd_ofs = CY_REG_CAT_CMD;
		break;
	case CY_MODE_OPERATIONAL:
		cmd_ofs = si->si_ofs.cmd_ofs;
		break;
	default:
		dev_err(dev, "%s: Unsupported mode %x for exec cmd\n",
				__func__, mode);
		return -EACCES;
	}

	return cmd_ofs;
}

/*
 * Send command to device for CAT and OP modes
 * return negative value on error, 0 on success
 */
static int _cyttsp4_exec_cmd(struct cyttsp4_core_data *cd, u8 mode,
		u8 *cmd_buf, size_t cmd_size)
{
	struct device *dev = cd->dev;
	int cmd_ofs;
	int cmd_param_ofs;
	u8 command;
	u8 *cmd_param_buf;
	size_t cmd_param_size;
	int rc;

	if (mode != cd->mode) {
		dev_err(dev, "%s: %s (having %x while %x requested)\n",
				__func__, "attempt to exec cmd in missing mode",
				cd->mode, mode);
		return -EACCES;
	}

	cmd_ofs = _get_cmd_offs(cd, mode);
	if (cmd_ofs < 0)
		return -EACCES;

	cmd_param_ofs = cmd_ofs + 1;
	cmd_param_buf = cmd_buf + 1;
	cmd_param_size = cmd_size - 1;

	/* Check if complete is set, so write new command */
	rc = cyttsp4_adap_read(cd, cmd_ofs, &command, 1);
	if (rc < 0) {
		dev_err(dev, "%s: Error on read r=%d\n", __func__, rc);
		return rc;
	}

	cd->cmd_toggle = GET_TOGGLE(command);
	cd->int_status |= CY_INT_EXEC_CMD;

	if ((command & CY_CMD_COMPLETE_MASK) == 0)
		return -EBUSY;

	/*
	 * Write new command
	 * Only update command bits 0:5
	 * Clear command complete bit & toggle bit
	 */
	cmd_buf[0] = cmd_buf[0] & CY_CMD_MASK;
	/* Write command parameters first */
	if (cmd_size > 1) {
		rc = cyttsp4_adap_write(cd, cmd_param_ofs, cmd_param_buf,
				cmd_param_size);
		if (rc < 0) {
			dev_err(dev, "%s: Error on write command parameters r=%d\n",
				__func__, rc);
			return rc;
		}
	}
	/* Write the command */
	rc = cyttsp4_adap_write(cd, cmd_ofs, cmd_buf, 1);
	if (rc < 0) {
		dev_err(dev, "%s: Error on write command r=%d\n",
				__func__, rc);
		return rc;
	}

	return 0;
}

static int cyttsp4_exec_cmd(struct cyttsp4_core_data *cd, u8 mode,
		u8 *cmd_buf, size_t cmd_size, u8 *return_buf,
		size_t return_buf_size, int timeout_ms)
{
	struct device *dev = cd->dev;
	int cmd_ofs;
	int cmd_return_ofs;
	int rc;

	mutex_lock(&cd->system_lock);
	rc = _cyttsp4_exec_cmd(cd, mode, cmd_buf, cmd_size);
	mutex_unlock(&cd->system_lock);

	if (rc == -EBUSY) {
		rc = _cyttsp4_wait_cmd_exec(cd, CY_COMMAND_COMPLETE_TIMEOUT);
		if (rc)
			return rc;
		mutex_lock(&cd->system_lock);
		rc = _cyttsp4_exec_cmd(cd, mode, cmd_buf, cmd_size);
		mutex_unlock(&cd->system_lock);
	}

	if (rc < 0)
		return rc;

	if (timeout_ms == 0)
		return 0;

	/*
	 * Wait command to be completed
	 */
	rc = _cyttsp4_wait_cmd_exec(cd, timeout_ms);
	if (rc < 0)
		return rc;

	if (return_buf_size == 0 || return_buf == NULL)
		return 0;

	mutex_lock(&cd->system_lock);
	cmd_ofs = _get_cmd_offs(cd, mode);
	mutex_unlock(&cd->system_lock);
	if (cmd_ofs < 0)
		return -EACCES;

	cmd_return_ofs = cmd_ofs + 1;

	rc = cyttsp4_adap_read(cd, cmd_return_ofs, return_buf, return_buf_size);
	if (rc < 0) {
		dev_err(dev, "%s: Error on read 3 r=%d\n", __func__, rc);
		return rc;
	}

	return 0;
}

static int cyttsp4_request_exec_cmd_(struct cyttsp4_device *ttsp, u8 mode,
		u8 *cmd_buf, size_t cmd_size, u8 *return_buf,
		size_t return_buf_size, int timeout_ms)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	return cyttsp4_exec_cmd(cd, mode, cmd_buf, cmd_size,
			return_buf, return_buf_size, timeout_ms);
}

static int cyttsp4_get_parameter(struct cyttsp4_core_data *cd, u8 param_id,
		u32 *param_value)
{
	u8 command_buf[CY_CMD_OP_GET_PARAM_CMD_SZ];
	u8 return_buf[CY_CMD_OP_GET_PARAM_RET_SZ];
	u8 param_size;
	u8 *value_buf;
	int rc;

	command_buf[0] = CY_CMD_OP_GET_PARAM;
	command_buf[1] = param_id;
	rc = cyttsp4_exec_cmd(cd, CY_MODE_OPERATIONAL,
			command_buf, CY_CMD_OP_GET_PARAM_CMD_SZ,
			return_buf, CY_CMD_OP_GET_PARAM_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Unable to execute get parameter command.\n",
			__func__);
		return rc;
	}

	if (return_buf[0] != param_id) {
		dev_err(cd->dev, "%s: Fail to execute get parameter command.\n",
			__func__);
		return -EIO;
	}

	param_size = return_buf[1];
	value_buf = &return_buf[2];

	*param_value = 0;
	while (param_size--)
		*param_value += *(value_buf++) << (8 * param_size);

	return 0;
}

static int cyttsp4_set_parameter(struct cyttsp4_core_data *cd, u8 param_id,
		u8 param_size, u32 param_value)
{
	u8 command_buf[CY_CMD_OP_SET_PARAM_CMD_SZ];
	u8 return_buf[CY_CMD_OP_SET_PARAM_RET_SZ];
	int rc;

	command_buf[0] = CY_CMD_OP_SET_PARAM;
	command_buf[1] = param_id;
	command_buf[2] = param_size;

	if (param_size == 1) {
		command_buf[3] = (u8)param_value;
	} else if (param_size == 2) {
		command_buf[3] = (u8)(param_value >> 8);
		command_buf[4] = (u8)param_value;
	} else if (param_size == 4) {
		command_buf[3] = (u8)(param_value >> 24);
		command_buf[4] = (u8)(param_value >> 16);
		command_buf[5] = (u8)(param_value >> 8);
		command_buf[6] = (u8)param_value;
	} else {
		dev_err(cd->dev, "%s: Invalid parameter size %d\n",
			__func__, param_size);
		return -EINVAL;
	}

	rc = cyttsp4_exec_cmd(cd, CY_MODE_OPERATIONAL,
			command_buf, 3 + param_size,
			return_buf, CY_CMD_OP_SET_PARAM_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Unable to execute set parameter command.\n",
			__func__);
		return rc;
	}

	if (return_buf[0] != param_id || return_buf[1] != param_size) {
		dev_err(cd->dev, "%s: Fail to execute set parameter command.\n",
			__func__);
		return -EIO;
	}

	return 0;
}

static int cyttsp4_get_scantype(struct cyttsp4_core_data *cd, u8 *scantype)
{
	int rc;
	u32 value;

	rc = cyttsp4_get_parameter(cd, CY_RAM_ID_SCAN_TYPE, &value);
	if (!rc)
		*scantype = (u8)value;

	return rc;
}

static int cyttsp4_set_scantype(struct cyttsp4_core_data *cd, u8 scantype)
{
	int rc;

	rc = cyttsp4_set_parameter(cd, CY_RAM_ID_SCAN_TYPE, 1, scantype);

	return rc;
}

static u8 _cyttsp4_generate_new_scantype(struct cyttsp4_core_data *cd)
{
	u8 new_scantype = cd->default_scantype;

	if (cd->apa_mc_en)
		new_scantype |= CY_SCAN_TYPE_APA_MC;
	if (cd->glove_en)
		new_scantype |= CY_SCAN_TYPE_GLOVE;
	if (cd->stylus_en)
		new_scantype |= CY_SCAN_TYPE_STYLUS;
	if (cd->proximity_en)
		new_scantype |= CY_SCAN_TYPE_PROXIMITY;

	return new_scantype;
}

static int cyttsp4_set_new_scan_type(struct cyttsp4_core_data *cd,
		u8 scan_type, bool enable)
{
	int inc = enable ? 1 : -1;
	int *en;
	int rc;
	u8 new_scantype;

	switch (scan_type) {
	case CY_ST_GLOVE:
		en = &cd->glove_en;
		break;
	case CY_ST_STYLUS:
		en = &cd->stylus_en;
		break;
	case CY_ST_PROXIMITY:
		en = &cd->proximity_en;
		break;
	case CY_ST_APA_MC:
		en = &cd->apa_mc_en;
		break;
	default:
		return -EINVAL;
	}

	*en += inc;

	new_scantype = _cyttsp4_generate_new_scantype(cd);

	rc = cyttsp4_set_scantype(cd, new_scantype);
	if (rc)
		*en -= inc;

	return rc;
}

static int cyttsp4_request_enable_scan_type_(struct cyttsp4_device *ttsp,
		u8 scan_type)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);

	return cyttsp4_set_new_scan_type(cd, scan_type, true);
}

static int cyttsp4_request_disable_scan_type_(struct cyttsp4_device *ttsp,
		u8 scan_type)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);

	return cyttsp4_set_new_scan_type(cd, scan_type, false);
}

static int cyttsp4_read_config_block(struct cyttsp4_core_data *cd, u8 ebid,
		u16 row, u8 *data, u16 length)
{
	u8 command_buf[CY_CMD_CAT_READ_CFG_BLK_CMD_SZ];
	u8 *return_buf;
	int return_buf_sz;
	u16 crc;
	int rc;

	/* Allocate buffer for read config block command response
	 * Header(5) + Data(length) + CRC(2)
	 */
	return_buf_sz = CY_CMD_CAT_READ_CFG_BLK_RET_SZ + length;
	return_buf = kmalloc(return_buf_sz, GFP_KERNEL);
	if (!return_buf) {
		dev_err(cd->dev, "%s: Cannot allocate buffer\n",
			__func__);
		rc = -ENOMEM;
		goto exit;
	}

	command_buf[0] = CY_CMD_CAT_READ_CFG_BLK;
	command_buf[1] = HI_BYTE(row);
	command_buf[2] = LO_BYTE(row);
	command_buf[3] = HI_BYTE(length);
	command_buf[4] = LO_BYTE(length);
	command_buf[5] = ebid;

	rc = cyttsp4_exec_cmd(cd, CY_MODE_CAT,
			command_buf, CY_CMD_CAT_READ_CFG_BLK_CMD_SZ,
			return_buf, return_buf_sz,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc) {
		dev_err(cd->dev, "%s: Error executing command r=%d\n",
			__func__, rc);
		goto free_buffer;
	}

	crc = cyttsp4_calc_app_crc(
		&return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ], length);

	/* Validate response */
	if (return_buf[0] != CY_CMD_STATUS_SUCCESS
			|| return_buf[1] != ebid
			|| return_buf[2] != HI_BYTE(length)
			|| return_buf[3] != LO_BYTE(length)
			|| return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ
				+ length] != HI_BYTE(crc)
			|| return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ
				+ length + 1] != LO_BYTE(crc)) {
		dev_err(cd->dev, "%s: Fail executing command\n",
				__func__);
		rc = -EINVAL;
		goto free_buffer;
	}

	memcpy(data, &return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ], length);

	cyttsp4_pr_buf(cd->dev, cd->pr_buf, data, length, "read_config_block");

free_buffer:
	kfree(return_buf);
exit:
	return rc;
}

static int cyttsp4_write_config_block(struct cyttsp4_core_data *cd, u8 ebid,
		u16 row, const u8 *data, u16 length)
{
	u8 return_buf[CY_CMD_CAT_WRITE_CFG_BLK_RET_SZ];
	u8 *command_buf;
	int command_buf_sz;
	u16 crc;
	int rc;

	/* Allocate buffer for write config block command
	 * Header(6) + Data(length) + Security Key(8) + CRC(2)
	 */
	command_buf_sz = CY_CMD_CAT_WRITE_CFG_BLK_CMD_SZ + length
		+ sizeof(security_key);
	command_buf = kmalloc(command_buf_sz, GFP_KERNEL);
	if (!command_buf) {
		dev_err(cd->dev, "%s: Cannot allocate buffer\n",
			__func__);
		rc = -ENOMEM;
		goto exit;
	}

	crc = cyttsp4_calc_app_crc(data, length);

	command_buf[0] = CY_CMD_CAT_WRITE_CFG_BLK;
	command_buf[1] = HI_BYTE(row);
	command_buf[2] = LO_BYTE(row);
	command_buf[3] = HI_BYTE(length);
	command_buf[4] = LO_BYTE(length);
	command_buf[5] = ebid;

	command_buf[CY_CMD_CAT_WRITE_CFG_BLK_CMD_HDR_SZ + length
		+ sizeof(security_key)] = HI_BYTE(crc);
	command_buf[CY_CMD_CAT_WRITE_CFG_BLK_CMD_HDR_SZ + 1 + length
		+ sizeof(security_key)] = LO_BYTE(crc);

	memcpy(&command_buf[CY_CMD_CAT_WRITE_CFG_BLK_CMD_HDR_SZ], data,
		length);
	memcpy(&command_buf[CY_CMD_CAT_WRITE_CFG_BLK_CMD_HDR_SZ + length],
		security_key, sizeof(security_key));

	cyttsp4_pr_buf(cd->dev, cd->pr_buf, command_buf, command_buf_sz,
		"write_config_block");

	rc = cyttsp4_exec_cmd(cd, CY_MODE_CAT,
			command_buf, command_buf_sz,
			return_buf, CY_CMD_CAT_WRITE_CFG_BLK_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc) {
		dev_err(cd->dev, "%s: Error executing command r=%d\n",
			__func__, rc);
		goto free_buffer;
	}

	/* Validate response */
	if (return_buf[0] != CY_CMD_STATUS_SUCCESS
			|| return_buf[1] != ebid
			|| return_buf[2] != HI_BYTE(length)
			|| return_buf[3] != LO_BYTE(length)) {
		dev_err(cd->dev, "%s: Fail executing command\n",
				__func__);
		rc = -EINVAL;
		goto free_buffer;
	}

free_buffer:
	kfree(command_buf);
exit:
	return rc;
}

static int cyttsp4_get_config_row_size(struct cyttsp4_core_data *cd,
		u16 *config_row_size)
{
	u8 command_buf[CY_CMD_CAT_GET_CFG_ROW_SIZE_CMD_SZ];
	u8 return_buf[CY_CMD_CAT_GET_CFG_ROW_SIZE_RET_SZ];
	int rc;

	command_buf[0] = CY_CMD_CAT_GET_CFG_ROW_SZ;

	rc = cyttsp4_exec_cmd(cd, CY_MODE_CAT,
			command_buf, CY_CMD_CAT_GET_CFG_ROW_SIZE_CMD_SZ,
			return_buf, CY_CMD_CAT_GET_CFG_ROW_SIZE_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc) {
		dev_err(cd->dev, "%s: Error executing command r=%d\n",
			__func__, rc);
		goto exit;
	}

	*config_row_size = get_unaligned_be16(&return_buf[0]);

exit:
	return rc;
}

static int cyttsp4_request_config_row_size_(struct cyttsp4_device *ttsp,
		u16 *config_row_size)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);

	return cyttsp4_get_config_row_size(cd, config_row_size);
}

static int cyttsp4_verify_config_block_crc(struct cyttsp4_core_data *cd,
		u8 ebid, u16 *calc_crc, u16 *stored_crc, bool *match)
{
	u8 command_buf[CY_CMD_CAT_VERIFY_CFG_BLK_CRC_CMD_SZ];
	u8 return_buf[CY_CMD_CAT_VERIFY_CFG_BLK_CRC_RET_SZ];
	int rc;

	command_buf[0] = CY_CMD_CAT_VERIFY_CFG_BLK_CRC;
	command_buf[1] = ebid;

	rc = cyttsp4_exec_cmd(cd, CY_MODE_CAT,
			command_buf, CY_CMD_CAT_VERIFY_CFG_BLK_CRC_CMD_SZ,
			return_buf, CY_CMD_CAT_VERIFY_CFG_BLK_CRC_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc) {
		dev_err(cd->dev, "%s: Error executing command r=%d\n",
			__func__, rc);
		goto exit;
	}

	*calc_crc = get_unaligned_be16(&return_buf[1]);
	*stored_crc = get_unaligned_be16(&return_buf[3]);
	if (match)
		*match = !return_buf[0];
exit:
	return rc;
}

static int cyttsp4_get_config_block_crc(struct cyttsp4_core_data *cd,
		u8 ebid, u16 *crc)
{
	u8 command_buf[CY_CMD_OP_GET_CFG_BLK_CRC_CMD_SZ];
	u8 return_buf[CY_CMD_OP_GET_CFG_BLK_CRC_RET_SZ];
	int rc;

	command_buf[0] = CY_CMD_OP_GET_CRC;
	command_buf[1] = ebid;

	rc = cyttsp4_exec_cmd(cd, CY_MODE_OPERATIONAL,
			command_buf, CY_CMD_OP_GET_CFG_BLK_CRC_CMD_SZ,
			return_buf, CY_CMD_OP_GET_CFG_BLK_CRC_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc) {
		dev_err(cd->dev, "%s: Error executing command r=%d\n",
			__func__, rc);
		goto exit;
	}

	/* Validate response */
	if (return_buf[0] != CY_CMD_STATUS_SUCCESS) {
		dev_err(cd->dev, "%s: Fail executing command\n",
				__func__);
		rc = -EINVAL;
		goto exit;
	}

	*crc = get_unaligned_be16(&return_buf[1]);

exit:
	return rc;
}

static int cyttsp4_get_ttconfig_version(struct cyttsp4_core_data *cd,
		u16 *version)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	u8 data[CY_TTCONFIG_VERSION_OFFSET + CY_TTCONFIG_VERSION_SIZE];
	int rc;
	bool ready;

	mutex_lock(&cd->system_lock);
	ready = si->ready;
	mutex_unlock(&cd->system_lock);

	if (!ready) {
		rc  = -ENODEV;
		goto exit;
	}

	rc = cyttsp4_read_config_block(cd, CY_TCH_PARM_EBID,
			CY_TTCONFIG_VERSION_ROW, data, sizeof(data));
	if (rc) {
		dev_err(cd->dev, "%s: Error on read config block\n",
			__func__);
		goto exit;
	}

	*version = GET_FIELD16(si, &data[CY_TTCONFIG_VERSION_OFFSET]);

exit:
	return rc;
}

static int cyttsp4_get_config_length(struct cyttsp4_core_data *cd, u8 ebid,
		u16 *length, u16 *max_length)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	u8 data[CY_CONFIG_LENGTH_INFO_SIZE];
	int rc;
	bool ready;

	mutex_lock(&cd->system_lock);
	ready = si->ready;
	mutex_unlock(&cd->system_lock);

	if (!ready) {
		rc  = -ENODEV;
		goto exit;
	}

	rc = cyttsp4_read_config_block(cd, ebid, CY_CONFIG_LENGTH_INFO_OFFSET,
			data, sizeof(data));
	if (rc) {
		dev_err(cd->dev, "%s: Error on read config block\n",
			__func__);
		goto exit;
	}

	*length = GET_FIELD16(si, &data[CY_CONFIG_LENGTH_OFFSET]);
	*max_length = GET_FIELD16(si, &data[CY_CONFIG_MAXLENGTH_OFFSET]);

exit:
	return rc;
}

static int cyttsp4_write_config_common(struct cyttsp4_core_data *cd, u8 ebid,
		u16 offset, u8 *data, u16 length)
{
	u16 cur_block, cur_off, end_block, end_off;
	int copy_len;
	u16 config_row_size = 0;
	u8 *row_data = NULL;
	int rc;

	rc = cyttsp4_get_config_row_size(cd, &config_row_size);
	if (rc) {
		dev_err(cd->dev, "%s: Cannot get config row size\n",
			__func__);
		goto exit;
	}

	cur_block = offset / config_row_size;
	cur_off = offset % config_row_size;

	end_block = (offset + length) / config_row_size;
	end_off = (offset + length) % config_row_size;

	/* Check whether we need to fetch the whole block first */
	if (cur_off == 0)
		goto no_offset;

	row_data = kmalloc(config_row_size, GFP_KERNEL);
	if (!row_data) {
		dev_err(cd->dev, "%s: Cannot allocate buffer\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}

	copy_len = (cur_block == end_block) ?
		length : config_row_size - cur_off;

	/* Read up to current offset, append the new data and write it back */
	rc = cyttsp4_read_config_block(cd, ebid, cur_block, row_data, cur_off);
	if (rc) {
		dev_err(cd->dev, "%s: Error on read config block\n", __func__);
		goto free_row_data;
	}

	memcpy(&row_data[cur_off], data, copy_len);

	rc = cyttsp4_write_config_block(cd, ebid, cur_block, row_data,
			cur_off + copy_len);
	if (rc) {
		dev_err(cd->dev, "%s: Error on initial write config block\n",
			__func__);
		goto free_row_data;
	}

	data += copy_len;
	cur_off = 0;
	cur_block++;

no_offset:
	while (cur_block < end_block) {
		rc = cyttsp4_write_config_block(cd, ebid, cur_block, data,
				config_row_size);
		if (rc) {
			dev_err(cd->dev, "%s: Error on write config block\n",
				__func__);
			goto free_row_data;
		}

		data += config_row_size;
		cur_block++;
	}

	/* Last block */
	if (cur_block == end_block) {
		rc = cyttsp4_write_config_block(cd, ebid, end_block, data,
				end_off);
		if (rc) {
			dev_err(cd->dev, "%s: Error on last write config block\n",
				__func__);
			goto free_row_data;
		}
	}

free_row_data:
	kfree(row_data);
exit:
	return rc;
}

static int cyttsp4_write_config(struct cyttsp4_core_data *cd, u8 ebid,
		u16 offset, u8 *data, u16 length) {
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	u16 crc_new, crc_old;
	u16 crc_offset;
	u16 conf_len;
	u8 crc_data[2];
	int rc;
	bool ready;

	mutex_lock(&cd->system_lock);
	ready = si->ready;
	mutex_unlock(&cd->system_lock);

	if (!ready) {
		rc  = -ENODEV;
		goto exit;
	}

	/* CRC is stored at config max length offset */
	rc = cyttsp4_get_config_length(cd, ebid, &conf_len, &crc_offset);
	if (rc) {
		dev_err(cd->dev, "%s: Error on get config length\n",
			__func__);
		goto exit;
	}

	/* Allow CRC update also */
	if (offset + length > crc_offset + 2) {
		dev_err(cd->dev, "%s: offset + length exceeds max length(%d)\n",
			__func__, crc_offset + 2);
		rc = -EINVAL;
		goto exit;
	}

	rc = cyttsp4_write_config_common(cd, ebid, offset, data, length);
	if (rc) {
		dev_err(cd->dev, "%s: Error on write config\n",
			__func__);
		goto exit;
	}

	/* Verify config block CRC */
	rc = cyttsp4_verify_config_block_crc(cd, ebid,
			&crc_new, &crc_old, NULL);
	if (rc) {
		dev_err(cd->dev, "%s: Error on verify config block crc\n",
			__func__);
		goto exit;
	}

	dev_vdbg(cd->dev, "%s: crc_new:%04X crc_old:%04X\n",
		__func__, crc_new, crc_old);

	if (crc_new == crc_old) {
		dev_vdbg(cd->dev, "%s: Calculated crc matches stored crc\n",
			__func__);
		goto exit;
	}

	PUT_FIELD16(si, crc_new, crc_data);

	rc = cyttsp4_write_config_common(cd, ebid, crc_offset, crc_data, 2);
	if (rc) {
		dev_err(cd->dev, "%s: Error on write config crc\n",
			__func__);
		goto exit;
	}

exit:
	return rc;
}

static int cyttsp4_request_write_config_(struct cyttsp4_device *ttsp, u8 ebid,
		u16 offset, u8 *data, u16 length) {
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);

	return cyttsp4_write_config(cd, ebid, offset, data, length);
}

/* [OPT] change circuit design after DVT2 phase, 20131008, Add Start */
extern struct regulator *vreg_l27;
int reset_count=0;
/* [OPT] change circuit design after DVT2 phase, 20131008, Add End */

#ifdef CYTTSP4_WATCHDOG_NULL_CMD
static void cyttsp4_watchdog_work(struct work_struct *work)
{
	struct cyttsp4_core_data *cd =
		container_of(work, struct cyttsp4_core_data, watchdog_work);
	u8 cmd_buf[CY_CMD_OP_NULL_CMD_SZ];
	bool restart = false;
	int rc;

	rc = request_exclusive(cd, cd->core, 1);
	if (rc < 0) {
		dev_vdbg(cd->dev, "%s: fail get exclusive ex=%p own=%p\n",
				__func__, cd->exclusive_dev, cd->core);
		goto exit;
	}

	cmd_buf[0] = CY_CMD_OP_NULL;
	rc = cyttsp4_exec_cmd(cd, cd->mode,
			cmd_buf, CY_CMD_OP_NULL_CMD_SZ,
			NULL, CY_CMD_OP_NULL_RET_SZ,
			CY_COMMAND_COMPLETE_TIMEOUT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Watchdog NULL cmd failed.\n", __func__);
		restart = true;
	}

	if (release_exclusive(cd, cd->core) < 0)
		dev_err(cd->dev, "%s: fail to release exclusive\n", __func__);
	else
		dev_vdbg(cd->dev, "%s: pass release exclusive\n", __func__);
exit:
	if (restart)
		cyttsp4_queue_startup(cd);
	else
		cyttsp4_start_wd_timer(cd);
}
#else
static void cyttsp4_watchdog_work(struct work_struct *work)
{
	struct cyttsp4_core_data *cd =
		container_of(work, struct cyttsp4_core_data, watchdog_work);
	u8 mode[2];
	bool restart = false;
	int rc;

	if (cd == NULL) {
		dev_err(cd->dev, "%s: NULL context pointer\n", __func__);
		return;
	}

	mutex_lock(&cd->system_lock);
	rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
	if (rc) {
		dev_err(cd->dev, "%s: failed to access device r=%d\n",
			__func__, rc);
		restart = true;
		goto exit;
	}

	if (IS_BOOTLOADER(mode[0], mode[1])) {
		dev_err(cd->dev, "%s: device found in bootloader mode\n",
			__func__);
		restart = true;
		goto exit;
	}
exit:
	/* [OPT] change circuit design after DVT2 phase, 20131008, Mod Start */
#if 0
	if (restart)
		cyttsp4_queue_startup_(cd);
	else
		cyttsp4_start_wd_timer(cd);
#endif
	if (restart)
	{
		reset_count = 1;
		printk("[TP] enter %s, reset_count = %d\n", __func__, reset_count);
		cyttsp4_queue_startup_(cd);
	}else
		cyttsp4_start_wd_timer(cd);
	/* [OPT] change circuit design after DVT2 phase, 20131008, Mod End */

	mutex_unlock(&cd->system_lock);
}
#endif

static int cyttsp4_request_stop_wd_(struct cyttsp4_device *ttsp)
{
	struct cyttsp4_core *core = ttsp->core;
	struct cyttsp4_core_data *cd = dev_get_drvdata(&core->dev);
	cyttsp4_stop_wd_timer(cd);
	return 0;
}

static int _cyttsp4_put_device_into_deep_sleep(struct cyttsp4_core_data *cd,
		u8 hst_mode_reg)
{
	int rc;

	hst_mode_reg |= CY_HST_SLEEP;

	dev_vdbg(cd->dev, "%s: write DEEP SLEEP...\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, &hst_mode_reg,
			sizeof(hst_mode_reg));
	if (rc) {
		dev_err(cd->dev, "%s: Fail write adapter r=%d\n", __func__, rc);
		return -EINVAL;
	}
	dev_vdbg(cd->dev, "%s: write DEEP SLEEP succeeded\n", __func__);

	if (cd->pdata->power) {
		if(cd->core->dbg_msg_level & 0x2)
			dev_dbg(cd->dev, "%s: Power down HW\n", __func__);
		rc = cd->pdata->power(cd->pdata, 0, cd->dev, &cd->ignore_irq);
	} else {
		dev_dbg(cd->dev, "%s: No power function\n", __func__);
		rc = 0;
	}
	if (rc < 0) {
		dev_err(cd->dev, "%s: HW Power down fails r=%d\n",
				__func__, rc);
		return -EINVAL;
	}

	return 0;
}

static int _cyttsp4_put_device_into_easy_wakeup(struct cyttsp4_core_data *cd)
{
	u8 command_buf[CY_CMD_OP_WAIT_FOR_EVENT_CMD_SZ];
	int rc;

	if (!IS_TTSP_VER_GE(&cd->sysinfo, 2, 5))
		return -EINVAL;

	command_buf[0] = CY_CMD_OP_WAIT_FOR_EVENT;
	command_buf[1] = cd->easy_wakeup_gesture;

	rc = _cyttsp4_exec_cmd(cd, CY_MODE_OPERATIONAL, command_buf,
			CY_CMD_OP_WAIT_FOR_EVENT_CMD_SZ);
	cd->int_status &= ~CY_INT_EXEC_CMD;
	if (rc)
		dev_err(cd->dev, "%s: Error executing command r=%d\n",
			__func__, rc);

	return rc;
}

static int _cyttsp4_wait_for_refresh_cycle(struct cyttsp4_core_data *cd,
		int cycle)
{
	int active_refresh_cycle_ms;

	if (cd->active_refresh_cycle_ms)
		active_refresh_cycle_ms = cd->active_refresh_cycle_ms;
	else
		active_refresh_cycle_ms = 20;

	msleep(cycle * active_refresh_cycle_ms);

	return 0;
}

static int _cyttsp4_put_device_into_sleep(struct cyttsp4_core_data *cd,
		u8 hst_mode_reg)
{
	int rc;

	if (IS_DEEP_SLEEP_CONFIGURED(cd->easy_wakeup_gesture))
		rc = _cyttsp4_put_device_into_deep_sleep(cd, hst_mode_reg);
	else
		rc = _cyttsp4_put_device_into_easy_wakeup(cd);

	return rc;
}

static int cyttsp4_core_sleep_(struct cyttsp4_core_data *cd)
{
	u8 mode[2];
	int rc = 0;

	cyttsp4_stop_wd_timer(cd);

	/* Wait until currently running IRQ handler exits and disable IRQ */
	disable_irq(cd->irq);

	mutex_lock(&cd->system_lock);
	/* Already in sleep mode? */
	if (cd->sleep_state == SS_SLEEP_ON)
		goto exit;

	cd->sleep_state = SS_SLEEPING;

	rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
	if (rc) {
		dev_err(cd->dev, "%s: Fail read adapter r=%d\n", __func__, rc);
		goto exit;
	}

	if (IS_BOOTLOADER(mode[0], mode[1])) {
		dev_err(cd->dev, "%s: Device in BOOTLADER mode.\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	/* Deep sleep is only allowed in Operating mode */
	if (GET_HSTMODE(mode[0]) != CY_HST_OPERATE) {
		dev_err(cd->dev, "%s: Device is not in Operating mode (%02X)\n",
			__func__, GET_HSTMODE(mode[0]));
		mutex_unlock(&cd->system_lock);
		enable_irq(cd->irq);
		/* Try switching to Operating mode */
		rc = set_mode(cd, CY_MODE_OPERATIONAL);
		disable_irq(cd->irq);
		mutex_lock(&cd->system_lock);
		if (rc < 0) {
			dev_err(cd->dev, "%s: failed to set mode to Operational rc=%d\n",
				__func__, rc);
			cyttsp4_queue_startup_(cd);
			rc = 0;
			goto exit;
		}

		/* Get the new host mode register value */
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
		if (rc) {
			dev_err(cd->dev, "%s: Fail read adapter r=%d\n",
				__func__, rc);
			goto exit;
		}
	}

	rc = _cyttsp4_put_device_into_sleep(cd, mode[0]);

exit:
	if (rc) {
		cd->sleep_state = SS_SLEEP_OFF;
		cyttsp4_start_wd_timer(cd);
	} else {
		cd->sleep_state = SS_SLEEP_ON;
		cd->int_status |= CY_INT_IGNORE;
	}

	mutex_unlock(&cd->system_lock);
	enable_irq(cd->irq);

	return rc;
}

static int cyttsp4_core_sleep(struct cyttsp4_core_data *cd)
{
	int rc;

	rc = request_exclusive(cd, cd->core,
			CY_CORE_SLEEP_REQUEST_EXCLUSIVE_TIMEOUT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail get exclusive ex=%p own=%p\n",
				__func__, cd->exclusive_dev, cd->core);
		return 0;
	}

	rc = cyttsp4_core_sleep_(cd);

	if (release_exclusive(cd, cd->core) < 0)
		dev_err(cd->dev, "%s: fail to release exclusive\n", __func__);
	else
		dev_vdbg(cd->dev, "%s: pass release exclusive\n", __func__);

	/* Give time to FW to sleep */
	_cyttsp4_wait_for_refresh_cycle(cd, 2);

	return rc;
}

static int _cyttsp4_awake_device_from_deep_sleep(struct cyttsp4_core_data *cd,
		int timeout_ms)
{
	struct device *dev = cd->dev;
	u8 mode;
	int t;
	int rc;

	cd->int_status |= CY_INT_AWAKE;

	if (cd->pdata->power) {
		/* Wake up using platform power function */
		if(cd->core->dbg_msg_level & 0x2)
			dev_dbg(dev, "%s: Power up HW\n", __func__);
		rc = cd->pdata->power(cd->pdata, 1, dev, &cd->ignore_irq);
	} else {
		/* Initiate a read transaction to wake up */
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
	}
	if (rc < 0) {
		dev_err(dev, "%s: HW Power up fails r=%d\n", __func__, rc);
		/* Initiate another read transaction to wake up */
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
	} else
		dev_vdbg(cd->dev, "%s: HW power up succeeds\n", __func__);
	mutex_unlock(&cd->system_lock);

	t = wait_event_timeout(cd->wait_q,
			(cd->int_status & CY_INT_AWAKE) == 0,
			msecs_to_jiffies(timeout_ms));
	mutex_lock(&cd->system_lock);
	if (IS_TMO(t)) {
		dev_dbg(dev, "%s: TMO waiting for wakeup\n", __func__);
		cd->int_status &= ~CY_INT_AWAKE;
		/* Perform a read transaction to check if device is awake */
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &mode, sizeof(mode));
		if (rc < 0 || GET_HSTMODE(mode) != CY_HST_OPERATE) {
			dev_err(dev, "%s: Queueing startup\n", __func__);
			/* Try starting up */
			cyttsp4_queue_startup_(cd);
		}
	}

	return rc;
}

static int _cyttsp4_awake_device(struct cyttsp4_core_data *cd)
{
	int timeout_ms;

	if (cd->wake_initiated_by_device) {
		cd->wake_initiated_by_device = 0;
		/* To prevent sequential wake/sleep caused by ttsp modules */
		msleep(20);
		return 0;
	}

	if (IS_DEEP_SLEEP_CONFIGURED(cd->easy_wakeup_gesture))
		timeout_ms = CY_CORE_WAKEUP_TIMEOUT;
	else
		timeout_ms = CY_CORE_WAKEUP_TIMEOUT * 4;

	return _cyttsp4_awake_device_from_deep_sleep(cd, timeout_ms);
}

static int cyttsp4_core_wake_(struct cyttsp4_core_data *cd)
{
	int rc;

	/* Already woken? */
	mutex_lock(&cd->system_lock);
	if (cd->sleep_state == SS_SLEEP_OFF) {
		mutex_unlock(&cd->system_lock);
		return 0;
	}

	cd->int_status &= ~CY_INT_IGNORE;
	cd->sleep_state = SS_WAKING;

	rc = _cyttsp4_awake_device(cd);

	cd->sleep_state = SS_SLEEP_OFF;
	mutex_unlock(&cd->system_lock);

	cyttsp4_start_wd_timer(cd);

	return 0;
}

static int cyttsp4_core_wake(struct cyttsp4_core_data *cd)
{
	int rc;

	rc = request_exclusive(cd, cd->core,
			CY_CORE_REQUEST_EXCLUSIVE_TIMEOUT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail get exclusive ex=%p own=%p\n",
				__func__, cd->exclusive_dev, cd->core);
		return 0;
	}

	rc = cyttsp4_core_wake_(cd);

	if (release_exclusive(cd, cd->core) < 0)
		dev_err(cd->dev, "%s: fail to release exclusive\n", __func__);
	else
		dev_vdbg(cd->dev, "%s: pass release exclusive\n", __func__);

	/* If a startup queued in wake, wait it to finish */
	wait_event_timeout(cd->wait_q, cd->startup_state == STARTUP_NONE,
			msecs_to_jiffies(CY_CORE_RESET_AND_WAIT_TIMEOUT));

	return rc;
}

static int cyttsp4_get_ttconfig_info(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	u16 length, max_length;
	u16 version = 0;
	u16 crc = 0;
	int rc;

	rc = set_mode(cd, CY_MODE_CAT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to set mode to CAT rc=%d\n",
			__func__, rc);
		return rc;
	}

	rc = cyttsp4_get_ttconfig_version(cd, &version);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get ttconfig version rc=%d\n",
			__func__, rc);
		return rc;
	}

	rc = cyttsp4_get_config_length(cd, CY_TCH_PARM_EBID,
			&length, &max_length);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get ttconfig length rc=%d\n",
			__func__, rc);
		return rc;
	}

	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to set mode to Operational rc=%d\n",
			__func__, rc);
		return rc;
	}

	rc = cyttsp4_get_config_block_crc(cd, CY_TCH_PARM_EBID, &crc);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get ttconfig crc rc=%d\n",
			__func__, rc);
		return rc;
	}

	si->ttconfig.version = version;
	si->ttconfig.length = length;
	si->ttconfig.max_length = max_length;
	si->ttconfig.crc = crc;

	dev_vdbg(cd->dev, "%s: TT Config Version:%04X Length:%d Max Length:%d CRC:%04X\n",
		__func__, si->ttconfig.version, si->ttconfig.length,
		si->ttconfig.length, si->ttconfig.crc);

	return 0;
}
static int cyttsp4_get_active_refresh_cycle(struct cyttsp4_core_data *cd)
{
	int rc;
	u32 value;

	rc = cyttsp4_get_parameter(cd, CY_RAM_ID_REFRESH_INTERVAL, &value);
	if (!rc)
		cd->active_refresh_cycle_ms = (u8)value;

	return rc;
}

static int cyttsp4_set_initial_scantype(struct cyttsp4_core_data *cd)
{
	u8 new_scantype;
	int rc;

	rc = cyttsp4_get_scantype(cd, &cd->default_scantype);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get scantype rc=%d\n",
			__func__, rc);
		goto exit;
	}

	/* Disable proximity sensing by default */
	cd->default_scantype &= ~CY_SCAN_TYPE_PROXIMITY;

	new_scantype = _cyttsp4_generate_new_scantype(cd);

	rc = cyttsp4_set_scantype(cd, new_scantype);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to set scantype rc=%d\n",
			__func__, rc);
		goto exit;
	}
exit:
	return rc;
}

static int cyttsp4_startup_(struct cyttsp4_core_data *cd)
{
	int retry = CY_CORE_STARTUP_RETRY_COUNT;
	int rc;
	bool detected = false;

//	dev_dbg(cd->dev, "%s: enter...\n", __func__);

	cyttsp4_stop_wd_timer(cd);

reset:

	printk("[TP] retry = %d, reset_count = %d\n", retry, reset_count);
	/* [OPT] change circuit design after DVT2 phase, 20131008, Mod Start */
		if((retry==1) && (reset_count==1))
		{
			printk("[TP] disable vreg_l27\n");
			regulator_disable(vreg_l27);
			mdelay(5);
			printk("[TP] enable vreg_l27\n");
			regulator_enable(vreg_l27);
			mdelay(5);
			reset_count = 0;
			cd->core->enable_vreg_l27 = 1;
		}
	/* [OPT] change circuit design after DVT2 phase, 20131008, Mod End */

	if (retry != CY_CORE_STARTUP_RETRY_COUNT)
		dev_dbg(cd->dev, "%s: Retry %d\n", __func__,
			CY_CORE_STARTUP_RETRY_COUNT - retry);

	/* reset hardware and wait for heartbeat */
	rc = cyttsp4_reset_and_wait(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Error on h/w reset r=%d\n", __func__, rc);
		RETRY_OR_EXIT(retry--, reset, exit);
	}

	detected = true;

	/* exit bl into sysinfo mode */
	dev_vdbg(cd->dev, "%s: write exit ldr...\n", __func__);
	mutex_lock(&cd->system_lock);
	cd->int_status &= ~CY_INT_IGNORE;
	cd->int_status |= CY_INT_MODE_CHANGE;

	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ldr_exit,
			sizeof(ldr_exit));
	mutex_unlock(&cd->system_lock);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Fail write adap='%s' r=%d\n",
			__func__, cd->core->adap->id, rc);
		RETRY_OR_EXIT(retry--, reset, exit);
	}

	rc = cyttsp4_wait_sysinfo_mode(cd);
	if (rc < 0) {
		u8 buf[sizeof(ldr_err_app)];
		int rc1;

		/* Check for invalid/corrupted touch application */
		rc1 = cyttsp4_adap_read(cd, CY_REG_BASE, buf,
				sizeof(ldr_err_app));
		if (rc1) {
			dev_err(cd->dev, "%s: Fail read adap='%s' r=%d\n",
				__func__, cd->core->adap->id, rc1);
		} else if (!memcmp(buf, ldr_err_app, sizeof(ldr_err_app))) {
			dev_err(cd->dev, "%s: Error launching touch application\n",
				__func__);
			mutex_lock(&cd->system_lock);
			cd->invalid_touch_app = true;
			mutex_unlock(&cd->system_lock);
			goto exit_no_wd;
		}

		RETRY_OR_EXIT(retry--, reset, exit);
	}

	mutex_lock(&cd->system_lock);
	cd->invalid_touch_app = false;
	mutex_unlock(&cd->system_lock);

	/* read sysinfo data */
	dev_vdbg(cd->dev, "%s: get sysinfo regs..\n", __func__);
	rc = cyttsp4_get_sysinfo_regs(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get sysinfo regs rc=%d\n",
			__func__, rc);
		RETRY_OR_EXIT(retry--, reset, exit);
	}

	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to set mode to operational rc=%d\n",
			__func__, rc);
		RETRY_OR_EXIT(retry--, reset, exit);
	}

	rc = cyttsp4_set_initial_scantype(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get scantype rc=%d\n",
			__func__, rc);
		RETRY_OR_EXIT(retry--, reset, exit);
	}

	rc = cyttsp4_get_ttconfig_info(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: failed to get ttconfig info rc=%d\n",
			__func__, rc);
		RETRY_OR_EXIT(retry--, reset, exit);
	}

	rc = cyttsp4_get_active_refresh_cycle(cd);
	if (rc < 0)
		dev_err(cd->dev, "%s: failed to get refresh cycle time rc=%d\n",
			__func__, rc);

	/* attention startup */
	call_atten_cb(cd, CY_ATTEN_STARTUP, 0);

	/* restore to sleep if was suspended */
	mutex_lock(&cd->system_lock);
	if (cd->sleep_state == SS_SLEEP_ON) {
		cd->sleep_state = SS_SLEEP_OFF;
		mutex_unlock(&cd->system_lock);
		/* watchdog is restarted by cyttsp4_core_sleep_() on error */
		cyttsp4_core_sleep_(cd);
		goto exit_no_wd;
	}
	mutex_unlock(&cd->system_lock);

exit:
	cyttsp4_start_wd_timer(cd);

exit_no_wd:
	if (!detected)
		rc = -ENODEV;

	/* Required for signal to the TTHE */
	if(cd->core->dbg_msg_level & 0x2)
		dev_info(cd->dev, "%s: cyttsp4_exit startup r=%d...\n", __func__, rc);

	return rc;
}

static int cyttsp4_startup(struct cyttsp4_core_data *cd)
{
	int rc;

	mutex_lock(&cd->system_lock);
	cd->startup_state = STARTUP_RUNNING;
	mutex_unlock(&cd->system_lock);

	rc = request_exclusive(cd, cd->core,
			CY_CORE_REQUEST_EXCLUSIVE_TIMEOUT);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail get exclusive ex=%p own=%p\n",
				__func__, cd->exclusive_dev, cd->core);
		goto exit;
	}

	rc = cyttsp4_startup_(cd);

	if (release_exclusive(cd, cd->core) < 0)
		/* Don't return fail code, mode is already changed. */
		dev_err(cd->dev, "%s: fail to release exclusive\n", __func__);
	else
		dev_vdbg(cd->dev, "%s: pass release exclusive\n", __func__);

exit:
	mutex_lock(&cd->system_lock);
	cd->startup_state = STARTUP_NONE;
	mutex_unlock(&cd->system_lock);

	/* Wake the waiters for end of startup */
	wake_up(&cd->wait_q);

	return rc;
}

static void cyttsp4_startup_work_function(struct work_struct *work)
{
	struct cyttsp4_core_data *cd =  container_of(work,
		struct cyttsp4_core_data, startup_work);
	int rc;

	/*
	 * Force clear exclusive access
	 * startup queue is called for abnormal case,
	 * and when a this called access can be acquired in other context
	 */
	mutex_lock(&cd->system_lock);
	if (cd->exclusive_dev != cd->core)
		cd->exclusive_dev = NULL;
	mutex_unlock(&cd->system_lock);
	rc = cyttsp4_startup(cd);
	if (rc < 0)
		dev_err(cd->dev, "%s: Fail queued startup r=%d\n",
			__func__, rc);
}

static void cyttsp4_free_si_ptrs(struct cyttsp4_core_data *cd)
{
	struct cyttsp4_sysinfo *si = &cd->sysinfo;

	if (!si)
		return;

	kfree(si->si_ptrs.cydata);
	kfree(si->si_ptrs.test);
	kfree(si->si_ptrs.pcfg);
	kfree(si->si_ptrs.opcfg);
	kfree(si->si_ptrs.ddata);
	kfree(si->si_ptrs.mdata);
	kfree(si->btn);
	kfree(si->xy_mode);
	kfree(si->btn_rec_data);
}

struct cyttsp4_core_data *global_cd;
bool probe_flag=false;
int cyttsp4_core_early_suspend(void)
{
	int rc=-1;
	printk("[TP][%s] start\n", __func__);
	if(probe_flag==true)
		rc = cyttsp4_core_sleep(global_cd);
	if(rc < 0) {
		printk("%s: Error on sleep\n", __func__);
		return -EAGAIN;
	}
	return 0;
}

int cyttsp4_core_lately_resume(void)
{
	int rc=-1;
	printk("[TP][%s] start\n", __func__);

	if(probe_flag==true)
		rc = cyttsp4_core_wake(global_cd);
	if (rc < 0) {
		printk("%s: Error on wake\n", __func__);
		return -EAGAIN;
	}

	return 0;
}

#if defined(CONFIG_PM_RUNTIME)
static int cyttsp4_core_rt_suspend(struct device *dev)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	int rc;

	rc = cyttsp4_core_sleep(cd);
	if (rc < 0) {
		dev_err(dev, "%s: Error on sleep\n", __func__);
		return -EAGAIN;
	}
	return 0;
}

static int cyttsp4_core_rt_resume(struct device *dev)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	int rc;

	rc = cyttsp4_core_wake(cd);
	if (rc < 0) {
		dev_err(dev, "%s: Error on wake\n", __func__);
		return -EAGAIN;
	}

	return 0;
}
#endif

#if defined(CONFIG_PM_SLEEP)
static int cyttsp4_core_suspend(struct device *dev)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
printk("[TP][%s] start\n", __func__);
	if (!(cd->pdata->flags & CY_CORE_FLAG_WAKE_ON_GESTURE))
		return 0;

	/*
	 * This will not prevent resume
	 * Required to prevent interrupts before i2c awake
	 */
	disable_irq(cd->irq);

	if (device_may_wakeup(dev)) {
		dev_dbg(dev, "%s Device MAY wakeup\n", __func__);
		if (!enable_irq_wake(cd->irq))
			cd->irq_wake = 1;
	} else {
		dev_dbg(dev, "%s Device may NOT wakeup\n", __func__);
	}

	return 0;
}

static int cyttsp4_core_resume(struct device *dev)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
printk("[TP][%s] start\n", __func__);
	if (!(cd->pdata->flags & CY_CORE_FLAG_WAKE_ON_GESTURE))
		return 0;

	enable_irq(cd->irq);

	if (device_may_wakeup(dev)) {
		dev_dbg(dev, "%s Device MAY wakeup\n", __func__);
		if (cd->irq_wake) {
			disable_irq_wake(cd->irq);
			cd->irq_wake = 0;
		}
	} else {
		dev_dbg(dev, "%s Device may NOT wakeup\n", __func__);
	}

	return 0;
}
#endif

static const struct dev_pm_ops cyttsp4_core_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(cyttsp4_core_suspend, cyttsp4_core_resume)
	SET_RUNTIME_PM_OPS(cyttsp4_core_rt_suspend, cyttsp4_core_rt_resume,
			NULL)
};

/*
 * Show Firmware version via sysfs
 */
static ssize_t cyttsp4_ic_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	struct cyttsp4_cydata *cydata;

	mutex_lock(&cd->system_lock);
	if (!cd->sysinfo.ready) {
		if (cd->invalid_touch_app) {
			mutex_unlock(&cd->system_lock);
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
					"Corrupted Touch application!\n");
		} else {
			mutex_unlock(&cd->system_lock);
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
					"System Information not ready!\n");
		}
	}
	mutex_unlock(&cd->system_lock);

	cydata = cd->sysinfo.si_ptrs.cydata;

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"%s: 0x%02X 0x%02X\n"
		"%s: 0x%02X\n"
		"%s: 0x%02X\n"
		"%s: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n"
		"%s: 0x%04X\n"
		"%s: 0x%02X\n"
		"%s: 0x%02X\n",
		"TrueTouch Product ID", cydata->ttpidh, cydata->ttpidl,
		"Firmware Major Version", cydata->fw_ver_major,
		"Firmware Minor Version", cydata->fw_ver_minor,
		"Revision Control Number", cydata->revctrl[0],
		cydata->revctrl[1], cydata->revctrl[2], cydata->revctrl[3],
		cydata->revctrl[4], cydata->revctrl[5], cydata->revctrl[6],
		cydata->revctrl[7],
		"TrueTouch Config Version", cd->sysinfo.ttconfig.version,
		"Bootloader Major Version", cydata->blver_major,
		"Bootloader Minor Version", cydata->blver_minor);
}

/*
 * Show TT Config version via sysfs
 */
static ssize_t cyttsp4_ttconfig_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);

	return snprintf(buf, CY_MAX_PRBUF_SIZE, "0x%04X\n",
			cd->sysinfo.ttconfig.version);
}

/*
 * Show Driver version via sysfs
 */
static ssize_t cyttsp4_drv_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Driver: %s\nVersion: %s\nDate: %s\n",
		cy_driver_core_name, cy_driver_core_version,
		cy_driver_core_date);
}

/*
 * HW reset via sysfs
 */
static ssize_t cyttsp4_hw_reset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	int rc = 0;

	rc = cyttsp4_startup(cd);
	if (rc < 0)
		dev_err(dev, "%s: HW reset failed r=%d\n",
			__func__, rc);

	return size;
}

/*
 * Show IRQ status via sysfs
 */
static ssize_t cyttsp4_hw_irq_stat_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	int retval;

	if (cd->pdata->irq_stat) {
		retval = cd->pdata->irq_stat(cd->pdata, dev);
		switch (retval) {
		case 0:
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Interrupt line is LOW.\n");
		case 1:
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Interrupt line is HIGH.\n");
		default:
			return snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Function irq_stat() returned %d.\n", retval);
		}
	}

	return snprintf(buf, CY_MAX_PRBUF_SIZE,
		"Function irq_stat() undefined.\n");
}

/*
 * Show IRQ enable/disable status via sysfs
 */
static ssize_t cyttsp4_drv_irq_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t ret;

	mutex_lock(&cd->system_lock);
	if (cd->irq_enabled)
		ret = snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Driver interrupt is ENABLED\n");
	else
		ret = snprintf(buf, CY_MAX_PRBUF_SIZE,
			"Driver interrupt is DISABLED\n");
	mutex_unlock(&cd->system_lock);

	return ret;
}

/*
 * Enable/disable IRQ via sysfs
 */
static ssize_t cyttsp4_drv_irq_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	unsigned long value;
	int retval;

	retval = kstrtoul(buf, 10, &value);
	if (retval < 0) {
		dev_err(dev, "%s: Invalid value\n", __func__);
		goto cyttsp4_drv_irq_store_error_exit;
	}

	mutex_lock(&cd->system_lock);
	switch (value) {
	case 0:
		if (cd->irq_enabled) {
			cd->irq_enabled = false;
			/* Disable IRQ */
			disable_irq_nosync(cd->irq);
			dev_info(dev, "%s: Driver IRQ now disabled\n",
				__func__);
		} else
			dev_info(dev, "%s: Driver IRQ already disabled\n",
				__func__);
		break;

	case 1:
		if (cd->irq_enabled == false) {
			cd->irq_enabled = true;
			/* Enable IRQ */
			enable_irq(cd->irq);
			dev_info(dev, "%s: Driver IRQ now enabled\n",
				__func__);
		} else
			dev_info(dev, "%s: Driver IRQ already enabled\n",
				__func__);
		break;

	default:
		dev_err(dev, "%s: Invalid value\n", __func__);
	}
	mutex_unlock(&(cd->system_lock));

cyttsp4_drv_irq_store_error_exit:

	return size;
}

/*
 * Debugging options via sysfs
 */
static ssize_t cyttsp4_drv_debug_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	unsigned long value = 0;
	int rc = 0;

	rc = kstrtoul(buf, 10, &value);
	if (rc < 0) {
		dev_err(dev, "%s: Invalid value\n", __func__);
		goto cyttsp4_drv_debug_store_exit;
	}

	switch (value) {
	case CY_DBG_SUSPEND:
		dev_info(dev, "%s: SUSPEND (cd=%p)\n", __func__, cd);
		rc = cyttsp4_core_sleep(cd);
		if (rc)
			dev_err(dev, "%s: Suspend failed rc=%d\n",
				__func__, rc);
		else
			dev_info(dev, "%s: Suspend succeeded\n", __func__);
		break;

	case CY_DBG_RESUME:
		dev_info(dev, "%s: RESUME (cd=%p)\n", __func__, cd);
		rc = cyttsp4_core_wake(cd);
		if (rc)
			dev_err(dev, "%s: Resume failed rc=%d\n",
				__func__, rc);
		else
			dev_info(dev, "%s: Resume succeeded\n", __func__);
		break;
	case CY_DBG_SOFT_RESET:
		dev_info(dev, "%s: SOFT RESET (cd=%p)\n", __func__, cd);
		rc = cyttsp4_hw_soft_reset(cd);
		break;
	case CY_DBG_RESET:
		dev_info(dev, "%s: HARD RESET (cd=%p)\n", __func__, cd);
		rc = cyttsp4_hw_hard_reset(cd);
		break;
	default:
		dev_err(dev, "%s: Invalid value\n", __func__);
	}

cyttsp4_drv_debug_store_exit:
	return size;
}

/*
 * Show system status on deep sleep status via sysfs
 */
static ssize_t cyttsp4_sleep_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t ret;

	mutex_lock(&cd->system_lock);
	if (cd->sleep_state == SS_SLEEP_ON)
		ret = snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Deep Sleep is ENABLED\n");
	else
		ret = snprintf(buf, CY_MAX_PRBUF_SIZE,
				"Deep Sleep is DISABLED\n");
	mutex_unlock(&cd->system_lock);

	return ret;
}

static ssize_t cyttsp4_easy_wakeup_gesture_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t ret;

	mutex_lock(&cd->system_lock);
	ret = snprintf(buf, CY_MAX_PRBUF_SIZE, "0x%02X\n",
			cd->easy_wakeup_gesture);
	mutex_unlock(&cd->system_lock);
	return ret;
}

static ssize_t cyttsp4_easy_wakeup_gesture_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = kstrtoul(buf, 10, &value);
	if (ret < 0)
		return ret;

	if (value > 0xFF && value < 0)
		return -EINVAL;

	pm_runtime_get_sync(dev);

	mutex_lock(&cd->system_lock);
	if (cd->sysinfo.ready && IS_TTSP_VER_GE(&cd->sysinfo, 2, 5))
		cd->easy_wakeup_gesture = (u8)value;
	else
		ret = -ENODEV;
	mutex_unlock(&cd->system_lock);

	pm_runtime_put(dev);

	if (ret)
		return ret;

	return size;
}

/* [Optical][Touch] Implement T2 test function, 20130724, Add Start */
#define raw_data_file "/data/raw_data_file.txt"
#define baseline_file "/data/baseline_data.txt"
#define diffcount_file "/data/diffcnt_data.txt"
#define calbration_file "/data/cal_data.txt"
/* [OPT] Self Capacitance calibration, 20131015, Add Start */
#define self_calbration_file	"/data/self_cal_data.txt"
/* [OPT] Self Capacitance calibration, 20131015, Add End */
#define NUM_TX	12
#define NUM_RX	21
short RawData[NUM_RX][NUM_TX];
short Baseline[NUM_RX][NUM_TX];
short DiffCnt[NUM_RX][NUM_TX];
short LPWC[NUM_RX][NUM_TX];
/* [OPT] Self Capacitance calibration, 20131015, Add Start */
short LPWC_Self[NUM_RX+NUM_TX];
/* [OPT] Self Capacitance calibration, 20131015, Add End */
u8 short_test_result[16];

static const u8 init_baseline[] = { 0x20, 0x00, 0x0A, 0x00 };
static const u8 scan_panel[] = {0x20, 0x00, 0x0B };

#define EMODE	1	/* change mode failed*/
#define EI2C		2	/* I2C failed */
#define ECOMPLETE	3	/* command is not completed*/
#define EFAIL	4	/* command failed */

u8 global_idac = 0;
/* [OPT] Self Capacitance calibration, 20131015, Add Start */
u8 global_self_tx_idac = 0;
u8 global_self_rx_idac = 0;
/* [OPT] Self Capacitance calibration, 20131015, Add End */
int cal_flag = 0;	// record calibrate or not
int raw_flag = 0;	// record read raw data or not
int base_flag = 0;	// record read base line or not
int diff_flag = 0;	// record read base line or not

int ping_result = 0;

int cyttsp4_ping_ic(struct cyttsp4_core_data *cd)
{
	int rc;
	int retry = 50;
	int test_result = 0;
	u8 result;
	u8 ping_command[] = {
		0x20, 0x00, 0x00, 0x00
	};

	printk("[TP][%s] start ...\n", __func__);

	// Step 1 - Set HST_MODE to CAT mode
	if(cd->core->dbg_msg_level & 0x4)
		printk("[Tp] Set HST_MODE to CAT mode\n");
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0){
		printk("[TP] failed to set mode to CAT rc=%d\n", rc);
		test_result = 0;
		return 0;
	}

	// Step 2 - Run ping command (test code = 0x00)
	if(cd->core->dbg_msg_level & 0x4)
		printk("[TP] Run Ping command\n");
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ping_command, sizeof(ping_command));
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE+2, &result, sizeof(result));

		if(cd->core->dbg_msg_level & 0x4)
			printk("0x%x\n", result);

		retry--;
		mdelay(5);
	}while( (!(result & 0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP] Failed, wait for complete command timeout!\n");
		goto op_mode;
	}

	if(result & 0x80)
	{
		test_result = 1;
	}else{
		printk("[TP] Failed, toggle not ready!\n");
		test_result = 0;
	}
op_mode:
	// Step 3 - Set HST_MODE to OP mode
	if(cd->core->dbg_msg_level & 0x4)
		printk("[TP] Set HST_MODE to OP mode\n");

	if(test_result)
		printk("[TP] Ping pass!\n");
	else
		printk("[TP] Ping fail!\n");

	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0){
		printk("[TP] failed to set mode to OP rc=%d\n", rc);
	}

	return test_result;
}

static ssize_t cyttsp4_ping_show(struct device *dev,
                        struct device_attribute *attr, char *buf)
{
	ssize_t ret;

	if(ping_result == 0)
		ret = sprintf(buf, "Failed\n");
	else if(ping_result == 1)
		ret = sprintf(buf, "Pass\n");

	return ret;
}

static ssize_t cyttsp4_test_ping_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;
	int rc;
	int retry = 500;
	u8 result;
	u8 ping_command[] = {
		0x20, 0x00, 0x00, 0x00
	};

	printk("[TP][%s] start ...\n", __func__);

	// Step 1 - Set HST_MODE to CAT mode
	printk("[Tp] Set HST_MODE to CAT mode\n");
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0){
		printk("[TP] failed to set mode to CAT rc=%d\n", rc);
		num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, mode to CAT is failed, rc=%d\n", rc);
		return num_read_chars;
	}

	// Step 2 - Run ping command (test code = 0x00)
	printk("[TP] Run Ping command\n");
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ping_command, sizeof(ping_command));
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE+2, &result, sizeof(result));
		printk("0x%x\n", result);

		retry--;
		mdelay(5);
	}while( (!(result & 0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP] Failed, wait for complete command timeout!\n");
		num_read_chars += sprintf(&buf[num_read_chars], "Failed\n");
		goto op_mode;
	}

	if(result & 0x80)
		num_read_chars += sprintf(&buf[num_read_chars], "Pass\n");
	else
	{
		printk("[TP] Failed, toggle not ready!\n");
		num_read_chars += sprintf(&buf[num_read_chars], "Failed\n");
	}

op_mode:
	// Step 3 - Set HST_MODE to OP mode
	printk("[TP] Set HST_MODE to OP mode\n");
	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0){
		printk("[TP] failed to set mode to OP rc=%d\n", rc);
		num_read_chars += sprintf(&buf[num_read_chars], "Failed, mode to OP is failed, rc=%d\n", rc);
		return num_read_chars;
	}

	return num_read_chars;
}

int save_raw_data(const char *filename, char *type, int row, int col, int data_type)
{
	int i, j;
	char buf[127]="";
	struct file *filp;
	mm_segment_t fs;

	if(filename!=NULL)
	{
		filp = filp_open(filename, O_CREAT | O_RDWR, 0664);
		if(!IS_ERR(filp))
		{
			printk("[TP][%s] Open return=%ld\n", __func__, IS_ERR(filp));
			fs=get_fs();
			set_fs(KERNEL_DS);
			filp->f_op->llseek(filp, 0, SEEK_SET);

			if(type!=NULL)
			{
				sprintf(buf, "%s:\n", type);
				filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
			}

			if(data_type == SELF_LIDAC)
			{
				sprintf(buf, "Global IDAC Self Rx:%d\n", global_self_rx_idac);
				filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
				for(i=0 ; i<row ; i++)
				{
					sprintf(buf, "%d ", LPWC_Self[i]);
					filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
				}

				sprintf(buf,"\n");
				filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
			}else{
				if(data_type == LIDAC)
				{
					sprintf(buf, "Global IDAC Mutual:%d\n", global_idac);
					filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
				}

				for(i=0 ; i<row ; i++)
				{
					for(j=0 ; j<col ; j++)
					{
						switch(data_type)
						{
							case RAWDATA:
								sprintf(buf, "%d ", RawData[i][j]);
								break;
							case BASELINE:
								sprintf(buf, "%d ", Baseline[i][j]);
								break;
							case DIFFCNT:
								sprintf(buf, "%d ", DiffCnt[i][j]);
								break;
							case LIDAC:
								sprintf(buf, "%d ", LPWC[i][j]);
							default:
								break;
						}

						filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
					}
					sprintf(buf,"\n");
					filp->f_op->write(filp, (char *)buf, strlen(buf),&filp->f_pos);
				}
			}
			set_fs(fs);
			filp_close(filp,NULL);
			return 0;
		}else{
			printk("[TP][%s] Open file fail, return=%ld\n", __func__, IS_ERR(filp));
			return IS_ERR(filp);
		}
	}else{
		printk("[TP][%s] no save data path!\n", __func__);
		return -1;
	}
}
static void cyttsp4_assign_factory_criteria(struct cyttsp4_core_data *cd)
{
	u8 panel_info = cd->core->panel_info;
	switch(panel_info)
	{
		case TRULY:
			cd->factory_criteria.lpwc_max = ftm_raw_data[MAIN_SOURCE].lpwc_max;
			cd->factory_criteria.lpwc_min = ftm_raw_data[MAIN_SOURCE].lpwc_min;
			cd->factory_criteria.lpwc_self_max= ftm_raw_data[MAIN_SOURCE].lpwc_self_max;
			cd->factory_criteria.lpwc_self_min= ftm_raw_data[MAIN_SOURCE].lpwc_self_min;
			cd->factory_criteria.raw_max = ftm_raw_data[MAIN_SOURCE].raw_max;
			cd->factory_criteria.raw_min = ftm_raw_data[MAIN_SOURCE].raw_min;
			cd->factory_criteria.base_max= ftm_raw_data[MAIN_SOURCE].base_max;
			cd->factory_criteria.base_min= ftm_raw_data[MAIN_SOURCE].base_min;
			break;
		case JTOUCH:
			cd->factory_criteria.lpwc_max = ftm_raw_data[SECOND_SOURCE].lpwc_max;
			cd->factory_criteria.lpwc_min = ftm_raw_data[SECOND_SOURCE].lpwc_min;
			cd->factory_criteria.lpwc_self_max= ftm_raw_data[SECOND_SOURCE].lpwc_self_max;
			cd->factory_criteria.lpwc_self_min= ftm_raw_data[SECOND_SOURCE].lpwc_self_min;
			cd->factory_criteria.raw_max = ftm_raw_data[SECOND_SOURCE].raw_max;
			cd->factory_criteria.raw_min = ftm_raw_data[SECOND_SOURCE].raw_min;
			cd->factory_criteria.base_max= ftm_raw_data[SECOND_SOURCE].base_max;
			cd->factory_criteria.base_min= ftm_raw_data[SECOND_SOURCE].base_min;
			break;
		case BIEL:
			cd->factory_criteria.lpwc_max = ftm_raw_data[THIRD_SOURCE].lpwc_max;
			cd->factory_criteria.lpwc_min = ftm_raw_data[THIRD_SOURCE].lpwc_min;
			cd->factory_criteria.lpwc_self_max= ftm_raw_data[THIRD_SOURCE].lpwc_self_max;
			cd->factory_criteria.lpwc_self_min= ftm_raw_data[THIRD_SOURCE].lpwc_self_min;
			cd->factory_criteria.raw_max = ftm_raw_data[THIRD_SOURCE].raw_max;
			cd->factory_criteria.raw_min = ftm_raw_data[THIRD_SOURCE].raw_min;
			cd->factory_criteria.base_max= ftm_raw_data[THIRD_SOURCE].base_max;
			cd->factory_criteria.base_min= ftm_raw_data[THIRD_SOURCE].base_min;
			break;
		case TPK:
			cd->factory_criteria.lpwc_max = ftm_raw_data[FOURTH_SOURCE].lpwc_max;
			cd->factory_criteria.lpwc_min = ftm_raw_data[FOURTH_SOURCE].lpwc_min;
			cd->factory_criteria.lpwc_self_max= ftm_raw_data[FOURTH_SOURCE].lpwc_self_max;
			cd->factory_criteria.lpwc_self_min= ftm_raw_data[FOURTH_SOURCE].lpwc_self_min;
			cd->factory_criteria.raw_max = ftm_raw_data[FOURTH_SOURCE].raw_max;
			cd->factory_criteria.raw_min = ftm_raw_data[FOURTH_SOURCE].raw_min;
			cd->factory_criteria.base_max= ftm_raw_data[FOURTH_SOURCE].base_max;
			cd->factory_criteria.base_min= ftm_raw_data[FOURTH_SOURCE].base_min;
			break;
		default:
			printk("[TP] No such source.\n");
			cd->factory_criteria.lpwc_max = 0;
			cd->factory_criteria.lpwc_min = 0;
			cd->factory_criteria.lpwc_self_max= 0;
			cd->factory_criteria.lpwc_self_min= 0;
			cd->factory_criteria.raw_max = 0;
			cd->factory_criteria.raw_min = 0;
			cd->factory_criteria.base_max= 0;
			cd->factory_criteria.base_min= 0;
			break;
	}
	printk("lpwc=[%d~%d], lpwc_self=[%d~%d], raw=[%d~%d],baseline=[%d~%d]\n",
		cd->factory_criteria.lpwc_max, cd->factory_criteria.lpwc_min,
		cd->factory_criteria.lpwc_self_max, cd->factory_criteria.lpwc_self_min,
		cd->factory_criteria.raw_max, cd->factory_criteria.raw_min,
		cd->factory_criteria.base_max, cd->factory_criteria.base_min);
}
/* [OPT] Implement factory T2 selftest, 20131002, Add Start */
static int cyttsp4_read_diff_cnt(struct cyttsp4_core_data *cd)
{
	u8 raw_data[NUM_TX*NUM_RX*2];
	int rc;
	int tx=0, rx=0, k=0;
	int retry = 500;
	static const u8 ret_diff_cnt_data[] = {
		0x20, 0x00, 0x0C, 0x00, 0x00, 0x00, 0xFC, 0x02
	};
	u8 result[8];

	printk("[TP] %s ... start\n", __func__);

	rc = cyttsp4_core_wake(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Error on wake\n", __func__);
	}

	memset(DiffCnt, 0, sizeof(DiffCnt));
	memset(raw_data, 0, sizeof(raw_data));

	// Step 1 - Set HST_MODE to CAT mode
	printk("[TP][%s] Step 1. Set HST_MODE to CAT mode\n", __func__);
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0){
		printk("[TP] failed to set mode to CAT rc=%d\n", rc);
		rc = -EMODE;
		goto err;
	}

	// Step 2 - Run Initialize Baseline command (test code = 0x0A)
	printk("[TP][%s] Step 2. Run initialize baseline command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)init_baseline, sizeof(init_baseline));

	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 3 - Execute Panel Scan command (test code = 0x0B)
	printk("[TP][%s] Step 3. Execute panel scan command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)scan_panel, sizeof(scan_panel));

	retry = 500;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
	        mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0) );

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 4 - Retrieve baseline and panel scan data
	printk("[TP] Retrieve baseline and panel scan data\n");
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ret_diff_cnt_data, sizeof(ret_diff_cnt_data));

	retry = 500;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
	if(rc < 0)
	{
		printk("[TP][%s] i2c read failed!\n", __func__);
		rc = -EI2C;
		goto err;
	}else{
		for(k=0 ; k<sizeof(result) ; k++)
			printk("0x%x ", result[k]);
	}
	printk("\n");

	for(k=0 ; k<NUM_RX*NUM_TX*2 ; k++)
		rc = cyttsp4_adap_read(cd, CY_REG_BASE+8+k, &raw_data[k], sizeof(raw_data[k]));

	if(rc < 0)
	{
		printk("[TP][%s] i2c read failed!\n", __func__);
		rc = -EI2C;
		goto err;
	}else{
		// arrange the raw_data[NUM_TX*NUM_RX_2] to DiffCnt[NUM_RX][NUM_TX]
		k = 0;
		for(tx=0 ; tx<NUM_TX ; tx++)
		{
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				DiffCnt[rx][tx] = (raw_data[k] | raw_data[k+1]<<8);
				k=k+2;
			}
		}
		// write the DiffCount data to /data/diffcnt_data.txt
		save_raw_data(diffcount_file, "DiffCount", NUM_RX, NUM_TX, DIFFCNT);
		// return the DiffCount data to user space
		printk("[TP] DiffCount:\n");
		for(rx =0 ; rx<NUM_RX ; rx++)
		{
			for(tx=0 ; tx<NUM_TX ; tx++)
				printk("%d ", DiffCnt[rx][tx]);

			printk("\n");
		}

		diff_flag = 1;
	}

err:
	printk("[TP][%s] Step 4. Set HST_MODE to OP mode\n", __func__);
	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0){
		printk("[TP] failed to set mode to OP rc=%d\n", rc);
		return -EMODE;
	}

	printk("[TP] %s ... end\n", __func__);

	return 0;
}

static int cyttsp4_read_raw_data(struct cyttsp4_core_data *cd)
{
	int rc;
	int tx=0, rx=0, k=0;
	int retry = 500;
	u8 result[8];
	u8 raw_data[NUM_TX*NUM_RX*2];
	static const u8 ret_mutual_raw_data[] = {
		0x20, 0x00, 0x0C, 0x00, 0x00, 0x00, 0xFC, 0x00
	};

	printk("[TP] %s ... start\n", __func__);

	rc = cyttsp4_core_wake(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Error on wake\n", __func__);
	}

	memset(RawData, 0, sizeof(RawData));
	memset(raw_data, 0, sizeof(raw_data));

	// Step 1 - Set HST_MODE to CAT mode
	printk("[TP][%s] Step 1. Set HST_MODE to CAT mode\n", __func__);
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0){
		printk("[TP] failed to set mode to CAT rc=%d\n", rc);
		rc = -EMODE;
		goto err;
	}

	// Step 2 - Run Initialize Baseline command (test code = 0x0A)
	printk("[TP][%s] Step 2. Run initialize baseline command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)init_baseline, sizeof(init_baseline));

	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 3 - Execute Panel Scan command (test code = 0x0B)
	printk("[TP][%s] Step 3. Execute panel scan command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)scan_panel, sizeof(scan_panel));

	retry = 500;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
	        mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0) );

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 4 - Retrieve baseline and panel scan data
	printk("[TP][%s] Step 4. Retrieve baseline and panel scan data\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ret_mutual_raw_data, sizeof(ret_mutual_raw_data));

	retry = 500;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
	if(rc < 0)
	{
		printk("[TP][%s] i2c read failed!\n", __func__);
		rc = -EI2C;
		goto err;
	}else{
		for(k=0 ; k<sizeof(result) ; k++)
			printk("0x%x ", result[k]);
	}
	printk("\n");

	for(k=0 ; k<NUM_RX*NUM_TX*2 ; k++)
		rc = cyttsp4_adap_read(cd, CY_REG_BASE+8+k, &raw_data[k], sizeof(raw_data[k]));

	if(rc < 0)
	{
		printk("[TP][%s] i2c read failed!\n", __func__);
		rc = -EI2C;
		goto err;
	}else{
		k = 0;
		for(tx=0 ; tx<NUM_TX ; tx++)
		{
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				RawData[rx][tx] = (raw_data[k] | raw_data[k+1]<<8);
				k=k+2;
			}
		}

		// write the raw data to /data/raw_data_file.txt
		save_raw_data(raw_data_file, "Raw data", NUM_RX, NUM_TX, RAWDATA);
		// return the raw data to user space
		printk("[TP] Raw data:\n");
		for(rx =0 ; rx<NUM_RX ; rx++)
		{
			for(tx=0 ; tx<NUM_TX ; tx++)
				printk("%d ", RawData[rx][tx]);
			printk("\n");
		}

		raw_flag = 1;
	}
err:
	printk("[TP][%s] Step 4. Set HST_MODE to OP mode\n", __func__);
	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0){
		printk("[TP] failed to set mode to OP rc=%d\n", rc);
		return -EMODE;
	}

	printk("[TP] %s ... end\n", __func__);

	return 0;
}

static int cyttsp4_read_base_line(struct cyttsp4_core_data *cd)
{
	int rc;
	int tx=0, rx=0, k=0;
	int retry = 500;
	u8 raw_data[NUM_TX*NUM_RX*2];
	static const u8 ret_mutual_baseline[] = {
		0x20, 0x00, 0x0C, 0x00, 0x00, 0x00, 0xFC, 0x01
	};
	u8 result[8];

	printk("[TP] %s ... start\n", __func__);

	rc = cyttsp4_core_wake(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Error on wake\n", __func__);
	}

	memset(Baseline, 0, sizeof(Baseline));
	memset(raw_data, 0, sizeof(raw_data));

	// Step 1 - Set HST_MODE to CAT mode
	printk("[TP][%s] Step 1. Set HST_MODE to CAT mode\n", __func__);
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0){
		printk("[TP] failed to set mode to CAT rc=%d\n", rc);
		rc = -EMODE;
		goto err;
	}

	// Step 2 - Run Initialize Baseline command (test code = 0x0A)
	printk("[TP][%s] Step 2. Run initialize baseline command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)init_baseline, sizeof(init_baseline));

	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 3 - Execute Panel Scan command (test code = 0x0B)
	printk("[TP][%s] Step 3. Execute panel scan command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)scan_panel, sizeof(scan_panel));

	retry = 500;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
	        mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0) );

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 4 - Retrieve baseline and panel scan data
	printk("[TP][%s] Step 4. Retrieve baseline and panel scan data\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ret_mutual_baseline, sizeof(ret_mutual_baseline));

	retry = 500;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(result[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(result[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	rc = cyttsp4_adap_read(cd, CY_REG_BASE, &result, sizeof(result));
	if(rc < 0)
	{
		printk("[TP][%s] i2c read failed!\n", __func__);
		rc = -EI2C;
		goto err;
	}else{
		for(k=0 ; k<sizeof(result) ; k++)
			printk("0x%x ", result[k]);
	}
	printk("\n");

	for(k=0 ; k<NUM_RX*NUM_TX*2 ; k++)
		rc = cyttsp4_adap_read(cd, CY_REG_BASE+8+k, &raw_data[k], sizeof(raw_data[k]));

	if(rc < 0)
	{
		printk("[TP][%s] i2c read failed!\n", __func__);
		rc = -EI2C;
		goto err;
	}else{
		k = 0;
		for(tx=0 ; tx<NUM_TX ; tx++)
		{
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				Baseline[rx][tx] = (raw_data[k] | raw_data[k+1]<<8);
				k=k+2;
			}
		}

		// write the Baseline data to /data/baselin_data.txt
		save_raw_data(baseline_file, "Baseline", NUM_RX, NUM_TX, BASELINE);
		// return the Baseline data to user space
		printk("Baseline:\n");
		for(rx =0 ; rx<NUM_RX ; rx++)
		{
			for(tx=0 ; tx<NUM_TX ; tx++)
				printk("%d ", Baseline[rx][tx]);

			printk("\n");
		}

		base_flag = 1;
	}

err:
	// Step 5 - Set HST_MODE to OP mode
	printk("[TP][%s] Step 5. Set HST_MODE to OP mode\n", __func__);
	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0){
		printk("[TP] failed to set mode to CAT rc=%d\n", rc);
		return -EMODE;
	}

	printk("[TP] %s ... end\n", __func__);

	return 0;
}

static ssize_t cyttsp4_diff_count_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;
	int rc = 0;
	int tx=0, rx=0;

	printk("[TP][%s] start\n", __func__);

	rc = cyttsp4_read_diff_cnt(cd);
	switch(rc)
	{
		case 0:
			num_read_chars += sprintf(&(buf[num_read_chars]), "DiffCount:\n");
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				for(tx=0 ; tx<NUM_TX ; tx++)
				{
					num_read_chars += sprintf(&(buf[num_read_chars]), "%d ", DiffCnt[rx][tx]);
				}
				num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
			}
			break;
		case -EMODE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, IC change mode failed\n");
			break;
		case -EI2C:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, I2C I/O failed\n");
			break;
		case -ECOMPLETE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command not complete\n");
			break;
		case -EFAIL:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command failed\n");
			break;
		default:
			num_read_chars += sprintf(&(buf[num_read_chars]), "No such case\n");
			break;
	}

	printk("[TP] %s ... end\n", __func__);

	return num_read_chars;
}

static ssize_t cyttsp4_raw_data_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;
	int rc = 0;
	int tx=0, rx=0;

	printk("[TP][%s] start\n", __func__);

	rc = cyttsp4_read_raw_data(cd);
	switch(rc)
	{
		case 0:
			num_read_chars += sprintf(&(buf[num_read_chars]), "RawData:\n");
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				for(tx=0 ; tx<NUM_TX ; tx++)
				{
					num_read_chars += sprintf(&(buf[num_read_chars]), "%d ", RawData[rx][tx]);
				}
				num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
			}
			break;
		case -EMODE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, IC change mode failed\n");
			break;
		case -EI2C:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, I2C I/O failed\n");
			break;
		case -ECOMPLETE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command not complete\n");
			break;
		case -EFAIL:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command failed\n");
			break;
		default:
			num_read_chars += sprintf(&(buf[num_read_chars]), "No such case\n");
			break;
	}

	printk("[TP] %s ... end\n", __func__);

	return num_read_chars;
}

static ssize_t cyttsp4_baseline_data_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;
	int rc = 0;
	int tx=0, rx=0;

	printk("[TP][%s] start\n", __func__);

	rc = cyttsp4_read_base_line(cd);
	switch(rc)
	{
		case 0:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Baseline:\n");
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				for(tx=0 ; tx<NUM_TX ; tx++)
				{
					num_read_chars += sprintf(&(buf[num_read_chars]), "%d ", Baseline[rx][tx]);
				}
				num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
			}
			break;
		case -EMODE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, IC change mode failed\n");
			break;
		case -EI2C:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, I2C I/O failed\n");
			break;
		case -ECOMPLETE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command not complete\n");
			break;
		case -EFAIL:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command failed\n");
			break;
		default:
			num_read_chars += sprintf(&(buf[num_read_chars]), "No such case\n");
			break;
	}

	printk("[TP] %s ... end\n", __func__);

	return num_read_chars;
}

static int cyttsp4_execute_calibrate(struct cyttsp4_core_data *cd)
{
	static const u8 exe_mutual_cal_cmd[] = {0x20, 0x00, 0x09, 0x00};
	static const u8 ret_mutual_cal_cmd[] = {0x20, 0x00, 0x10, 0x00, 0x00, 0x00, 0xFD, 0x00};
	/* [OPT] Self Capacitance calibration, 20131015, Add Start */
	static const u8 exe_self_cal_cmd[] = {0x20, 0x00, 0x09, 0x02};
	static const u8 ret_self_cal_cmd[] = {0x20, 0x00, 0x10, 0x00, 0x00, 0x00, 0x23, 0x01};
	/* [OPT] Self Capacitance calibration, 20131015, Add End */
	u8 data[NUM_TX*NUM_RX];
	u8 self_data[NUM_TX+NUM_RX];
	u8 state[10];
	int retry = 500;
	int tx=0, rx=0, i=0;
	int rc = 0;

	memset(LPWC, 0, sizeof(LPWC));
	memset(LPWC_Self, 0, sizeof(LPWC_Self));

	printk("[TP][%s] start\n", __func__);
	printk("[TP][%s] Step 1. Set HST_MODE to CAT mode\n", __func__);
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0)
	{
		printk("[TP][%s] failed to set mode to CAT, rc = %d\n", __func__, rc);
		return -EMODE;
	}

	printk("[TP][%s] Step 2. Execute Mutual Calibration Command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)exe_mutual_cal_cmd, sizeof(exe_mutual_cal_cmd));
	do // wait for command completed.
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &state, sizeof(state));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(state[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(state[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	printk("[TP][%s] Step 3. Retrieve Mutual Calibration data\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ret_mutual_cal_cmd, sizeof(ret_mutual_cal_cmd));

	retry = 1000;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &state, sizeof(state));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(state[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(state[3]){
		printk("[TP][%s] Retrieve command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	global_idac = state[8];	// Golbal IDAC Mutual Value

	// read local pwc value
	rc = cyttsp4_adap_read(cd, CY_REG_BASE+9, &data, sizeof(data));
	if(rc < 0)
	{
		printk("[TP] i2c read failed");
		rc = -EI2C;
		goto err;
	}else{

		// arrange the local PWC data[rx*tx] to LPWC[rx][tx]
		for(tx=0 ; tx<NUM_TX ; tx++)
		{
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				LPWC[rx][tx] = data[i++];
				printk("%d ", LPWC[rx][tx]);
			}
			printk("\n");
		}

		save_raw_data(calbration_file, "Calibration Data:", NUM_RX, NUM_TX, LIDAC);

	}
	/* [OPT] Self Capacitance calibration, 20131015, Add Start */
	printk("[TP][%s] Step 4. Execute Self Calibration Command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)exe_self_cal_cmd, sizeof(exe_self_cal_cmd));
	do // wait for command completed.
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &state, sizeof(state));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(state[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(state[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	printk("[TP][%s] Step 5. Retrieve Self Calibration data\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ret_self_cal_cmd, sizeof(ret_self_cal_cmd));

	retry = 1000;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &state, sizeof(state));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(state[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(state[3]){
		printk("[TP][%s] Retrieve command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	global_self_rx_idac= state[8];	// Golbal IDAC Self Rx Value
	global_self_tx_idac= state[9];	// Golbal IDAC Self Tx Value

	// read local pwc value
	rc = cyttsp4_adap_read(cd, CY_REG_BASE+10, &self_data, sizeof(self_data));
	if(rc < 0)
	{
		printk("[TP] i2c read failed");
		rc = -EI2C;
		goto err;
	}else{
		cal_flag = 1;

		// arrange the local PWC data[rx*tx] to LPWC[rx][tx]
		printk("[TP] Global Self Rx IDAC:%d, Global Self Tx IDAC:%d\n", global_self_rx_idac, global_self_tx_idac);
		printk("[TP] Self Local PWC:\n");
		for(i=0 ; i<NUM_RX+NUM_TX ; i++)
		{
				LPWC_Self[i] = self_data[i];
				printk("%d ", LPWC_Self[i]);
		}
		printk("\n");

		save_raw_data(self_calbration_file, "Self Calibration Data:", NUM_RX, NUM_TX, SELF_LIDAC);
	}

	/* [OPT] Self Capacitance calibration, 20131015, Add End */
err:
	printk("[TP][%s] Step 6. Set HST_MODE to OP mode\n", __func__);
	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0)
	{
		printk("[TP][%s] failed to set mode to OP, rc = %d\n", __func__, rc);
		return -EMODE;
	}

	printk("[TP][%s] end\n", __func__);

	return 0;
}

static int cyttsp4_short_test(struct cyttsp4_core_data *cd)
{
	static const u8 short_test_cmd[] = { 0x20, 0x00, 0x07, 0x04 };
	static const u8 ret_result_cmd[] = {0x20, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0F, 0x04};

	int rc=0, i=0;
	u8 state[8];
	int retry = 500;

	memset(short_test_result, 0, sizeof(short_test_result));

	printk("[TP][%s] start\n", __func__);
	printk("[TP][%s] Step 1. Set HST_MODE to CAT mode\n", __func__);
	rc = set_mode(cd, CY_MODE_CAT);
	if(rc < 0)
	{
		printk("[TP][%s] failed to set mode to CAT, rc = %d\n", __func__, rc);
		return -EMODE;
	}

	// Step 2 - Send Automatic Shorts Command
	printk("[TP][%s] Step 2. Send Automatic Shorts Command\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)short_test_cmd, sizeof(short_test_cmd));
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &state, sizeof(state));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(state[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(state[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	// Step 3 - Get short test results
	printk("[TP][%s] Step 3. Get short test results\n", __func__);
	rc = cyttsp4_adap_write(cd, CY_REG_BASE, (u8 *)ret_result_cmd, sizeof(ret_result_cmd));

	retry = 50;
	do
	{
		rc = cyttsp4_adap_read(cd, CY_REG_BASE, &state, sizeof(state));
		if(rc < 0)
			printk("[TP] i2c read failed!\n");

		retry--;
		mdelay(5);
	}while( (!(state[2]&0x40)) || (retry==0));

	if(retry==0)
	{
		printk("[TP][%s] wait for complete command timeout!\n", __func__);
		rc = -ECOMPLETE;
		goto err;
	}else if(state[3]){
		printk("[TP][%s] Calibration command is failed!\n", __func__);
		rc = -EFAIL;
		goto err;
	}

	printk("[TP][%s] Read short test results\n", __func__);
	rc = cyttsp4_adap_read(cd, CY_REG_BASE+8, &short_test_result, sizeof(short_test_result));
	if(rc < 0)
	{
		printk("[TP] i2c read failed");
		rc = -EI2C;
		goto err;
	}else{
		for(i=0 ; i<16 ; i++)
		{
			printk("0x%x ", short_test_result[i]);
		}
		printk("\n");
	}

err:
	// Step 4 - Set HST_MODE to OPERATIONAL mode
	printk("[TP][%s] Step 4. Set HST_MODE to OP mode\n", __func__);
	rc = set_mode(cd, CY_MODE_OPERATIONAL);
	if(rc < 0)
	{
		printk("[TP][%s] failed to set mode to OP, rc = %d\n", __func__, rc);
		return -EMODE;
	}

	printk("[TP] %s ... end\n", __func__);

	return 0;
}

/* [OPT] Implement factory T2 selftest, 20131002, Add End */

static ssize_t cyttsp4_execute_calibration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;
	int rc = 0;
	int tx=0, rx=0, i=0;

	printk("[TP][%s] start\n", __func__);

	rc = cyttsp4_core_wake(cd);
	if (rc < 0) {
		dev_err(cd->dev, "%s: Error on wake\n", __func__);
	}

	rc = cyttsp4_execute_calibrate(cd);
	switch(rc)
	{
		case 0:
			printk("[TP] Mutual data: max = %d, min = %d\n",cd->factory_criteria.lpwc_max, cd->factory_criteria.lpwc_min);
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				for(tx=0 ; tx<NUM_TX ; tx++)
				{
					if(LPWC[rx][tx]>cd->factory_criteria.lpwc_max
						|| LPWC[rx][tx]<cd->factory_criteria.lpwc_min)
					{
						num_read_chars += sprintf(&(buf[num_read_chars]),
							"Failed, LIDAC value(%d) out of range in [Rx, Tx]=[%d, %d]\n",LPWC[rx][tx], rx, tx);
						break;
					}
				}
			}
			printk("[TP] Self data: max = %d, min = %d\n",cd->factory_criteria.lpwc_self_max, cd->factory_criteria.lpwc_self_min);
			for(i=0 ; i<NUM_RX ; i++)
			{
				if(LPWC_Self[i] > cd->factory_criteria.lpwc_self_max
					|| LPWC_Self[i] < cd->factory_criteria.lpwc_self_min)
				{
					num_read_chars += sprintf(&(buf[num_read_chars]),
						"Failed, Self LIDAC value(%d) out of range in [Rx]=[%d]\n",LPWC_Self[i], i);
					break;
				}
			}

			num_read_chars += sprintf(&(buf[num_read_chars]), "Golbal IDAC Mutual:%d\n", global_idac);
			num_read_chars += sprintf(&(buf[num_read_chars]), "Mutual Local PWC:\n");
			for(rx=0 ; rx<NUM_RX ; rx++)
			{
				for(tx=0 ; tx<NUM_TX ; tx++)
				{
					num_read_chars += sprintf(&(buf[num_read_chars]), "%d ", LPWC[rx][tx]);
				}
				num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
			}

			num_read_chars += sprintf(&(buf[num_read_chars]), "Golbal IDAC Self Rx:%d\n", global_self_rx_idac);
			num_read_chars += sprintf(&(buf[num_read_chars]), "Self Local PWC:\n");
			for(i=0 ; i<NUM_RX ; i++)
			{
				num_read_chars += sprintf(&(buf[num_read_chars]), "%d ", LPWC_Self[i]);
			}
			num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
			break;
		case -EMODE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, IC change mode failed\n");
			break;
		case -EI2C:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, I2C I/O failed\n");
			break;
		case -ECOMPLETE:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command not complete\n");
			break;
		case -EFAIL:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, command failed\n");
			break;
		default:
			num_read_chars += sprintf(&(buf[num_read_chars]), "No such case\n");
			break;
	}

	printk("[TP] %s ... end\n", __func__);

	return num_read_chars;
}

static ssize_t cyttsp4_short_test_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;
	int i=0, rc=0;

	rc = cyttsp4_short_test(cd);

	if(short_test_result[0] != 0x0)
	{
		num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, ");
		for(i=1 ; i<16 ; i++)
			num_read_chars += sprintf(&(buf[num_read_chars]), "0x%x ", short_test_result[i]);
		num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
		goto err;
	}

	num_read_chars += sprintf(&(buf[num_read_chars]), "Pass\n");
err:
	return num_read_chars;
}
/* [Optical][Touch] Implement T2 test function, 20130724, Add End */
/**/
static ssize_t cyttsp4_enable_touch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);

	return sprintf(buf, "enable_touch=%d\n ", cd->core->enable_touch);
}

static ssize_t cyttsp4_enable_touch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = kstrtoul(buf, 10, &value);
	if (ret < 0)
		return ret;

	if (value > 1 && value < 0)
		return -EINVAL;

	cd->core->enable_touch = value;

	return size;
}
/**/
/**/
static ssize_t cyttsp4_enable_dbg_msg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);

	return sprintf(buf, "debug message level = %d, re-enable vreg_l27 = %d, HW ID = %d\n ",
					cd->core->dbg_msg_level, cd->core->enable_vreg_l27, cd->core->cci_hwid);
}

static ssize_t cyttsp4_enable_dbg_msg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = kstrtoul(buf, 10, &value);
	if (ret < 0)
		return ret;

	if (value > 8 && value < 0)
		return -EINVAL;

	cd->core->dbg_msg_level= value;

	return size;
}
/**/
/* [OPT] Implement T2 self test, 20131002, Add Start */
static ssize_t cyttsp4_self_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	int tx=0, rx=0, rc=0, i=0;
	ssize_t num_read_chars = 0;

	// check TP F/W
	if(cd->sysinfo.ttconfig.version != shipping_version)
	{
		printk("[TP][%s] IC F/W check failed, IC(0x%04X), shipping(0x%04X)\n ", __func__, cd->sysinfo.ttconfig.version, shipping_version);
		num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, IC F/W check failed, IC(0x%04X), shipping(0x%04X)\n ", cd->sysinfo.ttconfig.version, shipping_version);
		goto err;
	}

	if(!raw_flag)
	{
		printk("[TP][%s] Failed, read raw data failed\n", __func__);
		num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, read raw data failed\n");
		goto err;
	}else{
		printk("[TP][%s] Read raw data success\n", __func__);
	}

	if(!base_flag)
	{
		printk("[TP][%s] Failed, read base line data failed\n", __func__);
		num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, read base line data failed\n");
		goto err;
	}else{
		printk("[TP][%s] Read base line data success\n", __func__);
	}

printk("[TP] Raw data: max = %d, min = %d\n",cd->factory_criteria.raw_max, cd->factory_criteria.raw_min);
printk("[TP] Baseline data: max = %d, min = %d\n",cd->factory_criteria.base_max, cd->factory_criteria.base_min);
	for(rx=0 ; rx<NUM_RX ; rx++)
	{
		for(tx=0 ; tx<NUM_TX ; tx++)
		{
			if(RawData[rx][tx] > cd->factory_criteria.raw_max
				|| RawData[rx][tx] < cd->factory_criteria.raw_min)
			{
				num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, Raw data out of range in [Rx, Tx]=[%d, %d]\n", rx, tx);
				goto err;
			}
			if(Baseline[rx][tx]>cd->factory_criteria.base_max
				|| Baseline[rx][tx]<cd->factory_criteria.base_min)
			{
				num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, Baseline data out of range in [Rx, Tx]=[%d, %d]\n", rx, tx);
				goto err;
			}
		}
	}

	rc = cyttsp4_short_test(cd);
	if(short_test_result[0] != 0x0)
	{
		num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, ");
		for(i=1 ; i<16 ; i++)
			num_read_chars += sprintf(&(buf[num_read_chars]), "0x%x ", short_test_result[i]);
		num_read_chars += sprintf(&(buf[num_read_chars]), "\n");
		goto err;
	}

	num_read_chars += sprintf(&(buf[num_read_chars]), "Pass\n");
err:
	return num_read_chars;
}

static int cyttsp4_sillicon_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	struct cyttsp4_sysinfo *si = &cd->sysinfo;
	ssize_t num_read_chars = 0;

	num_read_chars += sprintf(&(buf[num_read_chars]), "Silicon ID = 0x%x\n", si->si_ptrs.cydata->jtag_si_id2);

	return num_read_chars;

}

static int cyttsp4_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	u8 panel_info = cd->core->panel_info;
	ssize_t num_read_chars = 0;

	num_read_chars += sprintf(&(buf[num_read_chars]), "Panel Info = 0x%x, ", panel_info);
	switch(panel_info)
	{
		case TRULY:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Vendor:Truly\n");
			break;
		case JTOUCH:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Vendor:JTouch\n");
			break;
		case BIEL:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Vendor:Biel Crystal\n");
			break;
		case TPK:
			num_read_chars += sprintf(&(buf[num_read_chars]), "Vendor:TPK\n");
			break;
		default:
			num_read_chars += sprintf(&(buf[num_read_chars]), "No such panel source\n");
			break;
	}
	return num_read_chars;

}

static int cyttsp4_firmware_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);
	ssize_t num_read_chars = 0;

	// check TP F/W
	if(cd->sysinfo.ttconfig.version != shipping_version)
	{
		printk("[TP][%s] IC F/W check failed, IC(0x%04X), shipping(0x%04X)\n ", __func__, cd->sysinfo.ttconfig.version, shipping_version);
		return num_read_chars += sprintf(&(buf[num_read_chars]), "Failed, IC F/W check failed, IC(0x%04X), shipping(0x%04X)\n ",
			cd->sysinfo.ttconfig.version, shipping_version);
	}

	return num_read_chars += sprintf(&(buf[num_read_chars]), "Pass, IC F/W (0x%04X)\n ", cd->sysinfo.ttconfig.version);

}
/* [OPT] Implement T2 self test, 20131002, Add End */

static struct device_attribute attributes[] = {
	__ATTR(ic_ver, S_IRUGO, cyttsp4_ic_ver_show, NULL),
	__ATTR(ttconfig_ver, S_IRUGO, cyttsp4_ttconfig_ver_show, NULL),
	__ATTR(drv_ver, S_IRUGO, cyttsp4_drv_ver_show, NULL),
	__ATTR(hw_reset, S_IWUSR, NULL, cyttsp4_hw_reset_store),
	__ATTR(hw_irq_stat, S_IRUSR, cyttsp4_hw_irq_stat_show, NULL),
	__ATTR(drv_irq, S_IRUSR | S_IWUSR, cyttsp4_drv_irq_show,
		cyttsp4_drv_irq_store),
	__ATTR(drv_debug, S_IWUSR, NULL, cyttsp4_drv_debug_store),
	__ATTR(sleep_status, S_IRUSR, cyttsp4_sleep_status_show, NULL),
	__ATTR(easy_wakeup_gesture, S_IRUSR | S_IWUSR,
		cyttsp4_easy_wakeup_gesture_show,
		cyttsp4_easy_wakeup_gesture_store),
/* [Optical][Touch] Implement T2 test function, 20130724, Add Start */
	__ATTR(raw_data, S_IRUGO, cyttsp4_raw_data_show, NULL),
	__ATTR(baseline, S_IRUGO, cyttsp4_baseline_data_show, NULL),
	__ATTR(diff_cnt, S_IRUGO, cyttsp4_diff_count_show, NULL),
	__ATTR(cal_command, S_IRUGO, cyttsp4_execute_calibration_show, NULL),
	__ATTR(test_ping, 0664, cyttsp4_test_ping_show, NULL),
	__ATTR(ping, 0664, cyttsp4_ping_show, NULL),
/* [Optical][Touch] Implement T2 test function, 20130724, Add End */
/**/
	__ATTR(enable_touch, 0664, cyttsp4_enable_touch_show, cyttsp4_enable_touch_store),
/**/
/**/
	__ATTR(enable_dbg_msg, 0664, cyttsp4_enable_dbg_msg_show, cyttsp4_enable_dbg_msg_store),
/**/
/* [OPT] Implement T2 self test, 20131002, Add Start */
	__ATTR(selftest, 0664, cyttsp4_self_test_show, NULL),
	__ATTR(short_test, 0664, cyttsp4_short_test_show, NULL),
	__ATTR(sillicon_id, 0664, cyttsp4_sillicon_id_show, NULL),
	__ATTR(check_fw, 0664, cyttsp4_firmware_check_show, NULL),
	__ATTR(panel_info, 0664, cyttsp4_panel_info_show, NULL),
/* [OPT] Implement T2 self test, 20131002, Add End */
};

static int add_sysfs_interfaces(struct cyttsp4_core_data *cd,
		struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto undo;

	return 0;
undo:
	for (i--; i >= 0 ; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: failed to create sysfs interface\n", __func__);
	return -ENODEV;
}

static void remove_sysfs_interfaces(struct cyttsp4_core_data *cd,
		struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}

static int cyttsp4_core_probe(struct cyttsp4_core *core)
{
	struct cyttsp4_core_data *cd;
	struct device *dev = &core->dev;
	struct cyttsp4_core_platform_data *pdata = dev_get_platdata(dev);
	enum cyttsp4_atten_type type;
	unsigned long irq_flags;
	int rc = 0;

	dev_info(dev, "%s: startup\n", __func__);
//	dev_dbg(dev, "%s: debug on\n", __func__);
	dev_vdbg(dev, "%s: verbose debug on\n", __func__);

//    shipping_version = (get_cci_project_id() == CCI_PROJECTID_VY58_59)?0x0800:0x0600;

	if (pdata == NULL) {
		dev_err(dev, "%s: Missing platform data\n", __func__);
		rc = -ENODEV;
		goto error_no_pdata;
	}

	/* get context and debug print buffers */
	cd = kzalloc(sizeof(*cd), GFP_KERNEL);
	if (cd == NULL) {
		dev_err(dev, "%s: Error, kzalloc\n", __func__);
		rc = -ENOMEM;
		goto error_alloc_data;
	}

	/* Initialize device info */
	cd->core = core;
	cd->dev = dev;
	cd->pdata = pdata;
	cd->max_xfer = CY_DEFAULT_ADAP_MAX_XFER;
	if (pdata->max_xfer_len) {
		if (pdata->max_xfer_len < CY_ADAP_MIN_XFER) {
			dev_err(dev, "%s: max_xfer_len invalid (min=%d)\n",
				__func__, CY_ADAP_MIN_XFER);
			rc = -EINVAL;
			goto error_max_xfer;
		}
		cd->max_xfer = pdata->max_xfer_len;
		dev_dbg(dev, "%s: max_xfer set to %d\n",
			__func__, cd->max_xfer);
	}

	/* Initialize mutexes and spinlocks */
	mutex_init(&cd->system_lock);
	mutex_init(&cd->adap_lock);
	spin_lock_init(&cd->spinlock);

	/* Initialize attention lists */
	for (type = 0; type < CY_ATTEN_NUM_ATTEN; type++)
		INIT_LIST_HEAD(&cd->atten_list[type]);

	/* Initialize wait queue */
	init_waitqueue_head(&cd->wait_q);

	/* Initialize works */
	INIT_WORK(&cd->startup_work, cyttsp4_startup_work_function);
	INIT_WORK(&cd->watchdog_work, cyttsp4_watchdog_work);

	/* Initialize IRQ */
	cd->irq = gpio_to_irq(pdata->irq_gpio);
	if (cd->irq < 0) {
		rc = -EINVAL;
		goto error_gpio_irq;
	}
	cd->irq_enabled = true;

	dev_set_drvdata(dev, cd);

	/* Call platform init function */
	if (cd->pdata->init) {
		dev_info(cd->dev, "%s: Init HW\n", __func__);
		rc = cd->pdata->init(cd->pdata, 1, cd->dev);
	} else {
		dev_info(cd->dev, "%s: No HW INIT function\n", __func__);
		rc = 0;
	}
	if (rc < 0)
		dev_err(cd->dev, "%s: HW Init fail r=%d\n", __func__, rc);

	/* Call platform detect function */
	if (cd->pdata->detect) {
		dev_info(cd->dev, "%s: Detect HW\n", __func__);
		rc = cd->pdata->detect(cd->pdata, cd->dev,
				cyttsp4_platform_detect_read);
		if (rc) {
			dev_info(cd->dev, "%s: No HW detected\n", __func__);
			rc = -ENODEV;
			goto error_detect;
		}
	}

	dev_dbg(dev, "%s: initialize threaded irq=%d\n", __func__, cd->irq);
	if (cd->pdata->level_irq_udelay > 0)
		/* use level triggered interrupts */
		irq_flags = IRQF_TRIGGER_LOW | IRQF_ONESHOT;
	else
		/* use edge triggered interrupts */
		irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;

	rc = request_threaded_irq(cd->irq, cyttsp4_hard_irq, cyttsp4_irq,
			irq_flags, dev_name(dev), cd);
	if (rc < 0) {
		dev_err(dev, "%s: Error, could not request irq\n", __func__);
		goto error_request_irq;
	}

	/* Setup watchdog timer */
	setup_timer(&cd->watchdog_timer, cyttsp4_watchdog_timer,
		(unsigned long)cd);

	pm_runtime_enable(dev);

	/*
	 * call startup directly to ensure that the device
	 * is tested before leaving the probe
	 */
	dev_dbg(dev, "%s: call startup\n", __func__);

	pm_runtime_get_sync(dev);
	rc = cyttsp4_startup(cd);
	pm_runtime_put(dev);

	/* Do not fail probe if startup fails but the device is detected */
	if (rc == -ENODEV) {
		dev_err(cd->dev, "%s: Fail initial startup r=%d\n",
			__func__, rc);
		goto error_startup;
	}

	if (IS_TTSP_VER_GE(&cd->sysinfo, 2, 5))
		cd->easy_wakeup_gesture = pdata->easy_wakeup_gesture;
	else
		cd->easy_wakeup_gesture = 0xFF;

	dev_dbg(dev, "%s: add sysfs interfaces\n", __func__);
	rc = add_sysfs_interfaces(cd, dev);
	if (rc < 0) {
		dev_err(dev, "%s: Error, fail sysfs init\n", __func__);
		goto error_startup;
	}

	device_init_wakeup(dev, 0);

	printk("[TP] Sillicon ID = 0x%x, Panel ID = 0x%x\n", cd->core->sillicon_id, cd->core->panel_info);
	cyttsp4_assign_factory_criteria(cd);

/* [Optical] T1 ping function, Add Start */
	global_cd = cd;
	probe_flag = true;
	rc = request_exclusive(cd, cd->core,
			15000);
	if (rc < 0) {
		dev_err(cd->dev, "%s: fail get exclusive ex=%p own=%p\n",
				__func__, cd->exclusive_dev, cd->core);
		goto exit;
	}

	ping_result = cyttsp4_ping_ic(cd);

	if (release_exclusive(cd, cd->core) < 0)
		/* Don't return fail code, mode is already changed. */
		dev_err(cd->dev, "%s: fail to release exclusive\n", __func__);
	else
		dev_vdbg(cd->dev, "%s: pass release exclusive\n", __func__);

exit:
/* [Optical] T1 ping function, Add End */

	dev_dbg(dev, "%s: ok\n", __func__);
	return 0;

error_startup:
	cancel_work_sync(&cd->startup_work);
	cyttsp4_stop_wd_timer(cd);
	pm_runtime_disable(dev);
	cyttsp4_free_si_ptrs(cd);
	free_irq(cd->irq, cd);
error_request_irq:
error_detect:
	if (pdata->init)
		pdata->init(pdata, 0, dev);
	dev_set_drvdata(dev, NULL);
error_gpio_irq:
error_max_xfer:
	kfree(cd);
error_alloc_data:
error_no_pdata:
	dev_err(dev, "%s failed.\n", __func__);
	return rc;
}

static int cyttsp4_core_release(struct cyttsp4_core *core)
{
	struct device *dev = &core->dev;
	struct cyttsp4_core_data *cd = dev_get_drvdata(dev);

	dev_dbg(dev, "%s\n", __func__);

	/*
	 * Suspend the device before freeing the startup_work and stopping
	 * the watchdog since sleep function restarts watchdog on failure
	 */
	pm_runtime_suspend(dev);
	pm_runtime_disable(dev);

	cancel_work_sync(&cd->startup_work);

	cyttsp4_stop_wd_timer(cd);

	remove_sysfs_interfaces(cd, dev);
	free_irq(cd->irq, cd);
	if (cd->pdata->init)
		cd->pdata->init(cd->pdata, 0, dev);
	dev_set_drvdata(dev, NULL);
	cyttsp4_free_si_ptrs(cd);
	kfree(cd);
	return 0;
}

static struct cyttsp4_core_driver cyttsp4_core_driver = {
	.probe = cyttsp4_core_probe,
	.remove = cyttsp4_core_release,
	.subscribe_attention = cyttsp4_subscribe_attention_,
	.unsubscribe_attention = cyttsp4_unsubscribe_attention_,
	.request_exclusive = cyttsp4_request_exclusive_,
	.release_exclusive = cyttsp4_release_exclusive_,
	.request_reset = cyttsp4_request_reset_,
	.request_restart = cyttsp4_request_restart_,
	.request_set_mode = cyttsp4_request_set_mode_,
	.request_sysinfo = cyttsp4_request_sysinfo_,
	.request_loader_pdata = cyttsp4_request_loader_pdata_,
	.request_handshake = cyttsp4_request_handshake_,
	.request_exec_cmd = cyttsp4_request_exec_cmd_,
	.request_stop_wd = cyttsp4_request_stop_wd_,
	.request_toggle_lowpower = cyttsp4_request_toggle_lowpower_,
	.request_config_row_size = cyttsp4_request_config_row_size_,
	.request_write_config = cyttsp4_request_write_config_,
	.request_enable_scan_type = cyttsp4_request_enable_scan_type_,
	.request_disable_scan_type = cyttsp4_request_disable_scan_type_,
	.get_security_key = cyttsp4_get_security_key_,
	.get_touch_record = cyttsp4_get_touch_record_,
	.write = cyttsp4_write_,
	.read = cyttsp4_read_,
	.driver = {
		.name = CYTTSP4_CORE_NAME,
		.bus = &cyttsp4_bus_type,
		.owner = THIS_MODULE,
		.pm = &cyttsp4_core_pm_ops,
	},
};

static int __init cyttsp4_core_init(void)
{
	int rc = 0;

	rc = cyttsp4_register_core_driver(&cyttsp4_core_driver);
	pr_info("%s: Cypress TTSP v4 core driver (Built %s) rc=%d\n",
		 __func__, CY_DRIVER_DATE, rc);
	return rc;
}
module_init(cyttsp4_core_init);

static void __exit cyttsp4_core_exit(void)
{
	cyttsp4_unregister_core_driver(&cyttsp4_core_driver);
	pr_info("%s: module exit\n", __func__);
}
module_exit(cyttsp4_core_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cypress TrueTouch(R) Standard touchscreen core driver");
MODULE_AUTHOR("Aleksej Makarov <aleksej.makarov@sonyericsson.com>");

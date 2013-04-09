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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <plat/kona_reset_reason.h>

#define REGL_PMMODE_REG_MAX 4
#define REGL_PMMODE_VAL_MAX 7
#define PMMODE_ON   0
#define PMMODE_LPM  1
#define PMMODE_OFF  2
#define PMMODE_3BIT_PM0_SHIFT 0
#define PMMODE_3BIT_PMx_MASK   7
#define PMMODE_3BIT_PM1_SHIFT 3
#define PMMODE_3BIT_PM_MASK   0x3F

#define PMMODE_2BIT_PM0_SHIFT 0
#define PMMODE_2BIT_PMx_MASK   3
#define PMMODE_2BIT_PM1_SHIFT 2
#define PMMODE_2BIT_PM2_SHIFT 4
#define PMMODE_2BIT_PM3_SHIFT 6
#define PMMODE_2BIT_PM_MASK   0xFF

#define TRIM_UP_MIN_INX	0
#define TRIM_UP_MAX_INX	31
#define TRIM_DOWN_MIN_INX	32
#define TRIM_DOWN_MAX_INX	63
#define SR_VOLT_START_INX 2

static int bcmpmuldo_get_voltage(struct regulator_dev *rdev);
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev,
				 int min_uv, int max_uv, unsigned *selector);
static int bcmpmuldo_list_voltage(struct regulator_dev *rdev,
				  unsigned selector);
static int bcmpmureg_get_status(struct regulator_dev *rdev);
static int bcmpmureg_disable(struct regulator_dev *rdev);
static int bcmpmureg_enable(struct regulator_dev *rdev);
static int bcmpmureg_is_enabled(struct regulator_dev *rdev);
static int bcmpmuldo_set_mode(struct regulator_dev *rdev, u32 mode);
static u32 bcmpmuldo_get_mode(struct regulator_dev *rdev);
static struct regulator_dev *regl[BCMPMU_REGULATOR_MAX];

struct bcmpmu59xxx_rgltr_param {
	struct bcmpmu59xxx_regulator_pdata *pdata;
	struct bcmpmu59xxx *bcmpmu;
#ifdef CONFIG_DEBUG_FS
	struct dentry *rgltr_dbgfs;
#endif
	int def_trim;
	int curr_inx;
};

/** voltage regulator details.  */
struct regulator_ops bcmpmu59xxx_ldo_ops = {
	.list_voltage = bcmpmuldo_list_voltage,
	.enable = bcmpmureg_enable,
	.disable = bcmpmureg_disable,
	.is_enabled = bcmpmureg_is_enabled,
	.get_status = bcmpmureg_get_status,
	.set_voltage = bcmpmuldo_set_voltage,
	.get_voltage = bcmpmuldo_get_voltage,
	.set_mode = bcmpmuldo_set_mode,
	.get_mode = bcmpmuldo_get_mode,
};

static int debug_mask =  BCMPMU_PRINT_INIT | BCMPMU_PRINT_ERROR;

#define pr_rgltr(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

#ifdef CONFIG_DEBUG_FS
struct pwr_mode_reg {
	u8 pwr_mode[REGL_PMMODE_REG_MAX];
};

struct pwr_mode_reg rgltr_pmode_buf[BCMPMU_REGULATOR_MAX];

static int force_enable;
#endif

static u32 __rgltr_to_pmmode(u32 rmode)
{
	u32 pmmode = PMMODE_ON;
	switch (rmode) {
	case REGULATOR_MODE_FAST:
	case REGULATOR_MODE_NORMAL:
		pmmode = PMMODE_ON;
		break;
	case REGULATOR_MODE_IDLE:
		pmmode = PMMODE_OFF;
		break;
	case REGULATOR_MODE_STANDBY:
		pmmode = PMMODE_LPM;
		break;
	}
	return pmmode;
}

static u32 __dsm_mode_to_rgltr(u32 dsmmode)
{
	u32 mode = REGULATOR_MODE_NORMAL;
	switch (dsmmode) {
	case BCMPMU_REGL_ON_IN_DSM:
		mode = REGULATOR_MODE_NORMAL;
		break;
	case BCMPMU_REGL_LPM_IN_DSM:
		mode = REGULATOR_MODE_STANDBY;
		break;
	case BCMPMU_REGL_OFF_IN_DSM:
		mode = REGULATOR_MODE_IDLE;
		break;
	}
	return mode;
}

static u32 __pmmode_to_dsm_mode(u32 pmumode)
{
	u32 dsmmode = BCMPMU_REGL_ON_IN_DSM;
	switch (pmumode) {
	case PMMODE_ON:
		dsmmode = BCMPMU_REGL_ON_IN_DSM;
		break;
	case PMMODE_LPM:
		dsmmode = BCMPMU_REGL_LPM_IN_DSM;
		break;
	case PMMODE_OFF:
		dsmmode = BCMPMU_REGL_OFF_IN_DSM;
		break;
	};
	return dsmmode;
}

static u32 __dsm_mode_to_pmmode(u32 dsm_mode)
{
	u32 opmode = PMMODE_ON;
	switch (dsm_mode) {

	case BCMPMU_REGL_ON_IN_DSM:
		opmode = PMMODE_ON;
		break;

	case BCMPMU_REGL_LPM_IN_DSM:
		opmode = PMMODE_LPM;
		break;

	case BCMPMU_REGL_OFF_IN_DSM:
		opmode = PMMODE_OFF;
		break;
	};
	return opmode;
}

static int __3bit_pmmode_frm_map(u32 pcpin_map, u32 dsm_pmmode,
					u8 *pmmode)
{
	int count = 0;
	u32 i;
	u32 val;
	u32 set1, set0;
	set0 = PCPIN_MAP_GET_SET(pcpin_map, 0);
	set1 = PCPIN_MAP_GET_SET(pcpin_map, 1);


	for (i = 0; i <= REGL_PMMODE_VAL_MAX; i++) {
		if (PCPIN_MAP_IS_SET0_MATCH(set0, i) ||
				PCPIN_MAP_IS_SET1_MATCH(set1, i))
			val = PMMODE_ON;
		else
			val = dsm_pmmode & PMMODE_3BIT_PMx_MASK;

		if (i & 1) {
			pmmode[count] |= val << PMMODE_3BIT_PM1_SHIFT;
				count++;
		} else
			pmmode[count] = val << PMMODE_3BIT_PM0_SHIFT;
	}
	BUG_ON(count != REGL_PMMODE_REG_MAX);
	return count;
}

static int __2bit_pmmode_frm_map(u32 pcpin_map, u32 dsm_pmmode,
				u8 *pmmode)
{
	u32 shift;
	int count = 0;
	u32 i;
	u32 val;
	u32 set1, set0;
	set0 = PCPIN_MAP_GET_SET(pcpin_map, 0);
	set1 = PCPIN_MAP_GET_SET(pcpin_map, 1);

	for (i = 0; i <= REGL_PMMODE_VAL_MAX; i++) {
		if (PCPIN_MAP_IS_SET0_MATCH(set0, i) ||
				PCPIN_MAP_IS_SET1_MATCH(set1, i))
			val = PMMODE_ON;
		else
			val = dsm_pmmode & PMMODE_2BIT_PMx_MASK;
		switch (PMMODE_2BIT_PMx_MASK & i) {
		case 0:
			shift = PMMODE_2BIT_PM0_SHIFT;
			pmmode[count] = 0;
			break;
		case 1:
			shift = PMMODE_2BIT_PM1_SHIFT;
			break;
		case 2:
			shift = PMMODE_2BIT_PM2_SHIFT;
			break;
		case 3:
			shift = PMMODE_2BIT_PM3_SHIFT;
			break;
		}

		pmmode[count] |=  val << shift;
		if ((PMMODE_2BIT_PMx_MASK & i) == 3)
				count++;
	}
	BUG_ON(count != 2);
	return count;

}

static int __is_3bit_pmmode_regl_enabled(u32 pcpin_map, u8 *pmmode)
{
	int enabled = 0;
	u32 i;
	u32 shift;
	u32 inx = 0;
	u32 set1, set0;
	set0 = PCPIN_MAP_GET_SET(pcpin_map, 0);
	set1 = PCPIN_MAP_GET_SET(pcpin_map, 1);

	shift = PMMODE_3BIT_PM0_SHIFT;
	for (i = 0; i <= REGL_PMMODE_VAL_MAX; i++) {
		if (PCPIN_MAP_IS_SET0_MATCH(set0, i) ||
				PCPIN_MAP_IS_SET1_MATCH(set1, i)) {
			if ((PMMODE_3BIT_PMx_MASK &
					(pmmode[inx] >> shift)) == PMMODE_ON) {
				enabled = 1;
				break;
			}
		}

		if (i & 1) {
			shift = PMMODE_3BIT_PM0_SHIFT;
			inx++;
		} else
			shift = PMMODE_3BIT_PM1_SHIFT;
	}
	return enabled;
}

static int __is_2bit_pmmode_regl_enabled(u32 pcpin_map, u8 *pmmode)
{
	int enabled = 0;
	u32 i;
	u32 shift;
	u32 inx = 0;
	u32 set1, set0;
	set0 = PCPIN_MAP_GET_SET(pcpin_map, 0);
	set1 = PCPIN_MAP_GET_SET(pcpin_map, 1);

	for (i = 0; i <= REGL_PMMODE_VAL_MAX; i++) {
		switch (PMMODE_2BIT_PMx_MASK & i) {
		case 0:
			shift = PMMODE_2BIT_PM0_SHIFT;
			break;
		case 1:
			shift = PMMODE_2BIT_PM1_SHIFT;
			break;
		case 2:
			shift = PMMODE_2BIT_PM2_SHIFT;
			break;
		case 3:
			shift = PMMODE_2BIT_PM3_SHIFT;
			break;
	}

		if (PCPIN_MAP_IS_SET0_MATCH(set0, i) ||
				PCPIN_MAP_IS_SET1_MATCH(set1, i)) {
			if ((PMMODE_2BIT_PMx_MASK &
					(pmmode[inx] >> shift)) == PMMODE_ON) {
				enabled = 1;
				break;
			}
		}

		if ((PMMODE_2BIT_PMx_MASK & i) == 3)
			inx++;

	}
	return enabled;
}

/*
* @is_enabled: Return 1 if the regulator is enabled, 0 if not.
* May also return negative errno.
*/
static int bcmpmureg_is_enabled(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;
	int id ;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	id = rdev_get_id(rdev);
	BUG_ON(id >= BCMPMU_REGULATOR_MAX ||
			rinfo == NULL);
	pr_rgltr(FLOW, "<%s> id %d state %d\n",
			__func__, id, !!(rinfo[id].flags & RGLR_ON));
	return  !!(rinfo[id].flags & RGLR_ON);
}

/*
* @enable: Configure the regulator as enabled.
*/
static int __bcmpmureg_enable(struct bcmpmu59xxx *bcmpmu, int id)
{
	u8 pmmode[REGL_PMMODE_REG_MAX] = {0};
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_regulator_init_data *initdata;
	struct bcmpmu59xxx_rgltr_param *param;
	int count, i;
	int ret = 0;

	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	param = bcmpmu->rgltr_data;
	pr_rgltr(FLOW, "<%s> id =  %d\n",
		__func__, id);
	initdata = param->pdata->bcmpmu_rgltr + id;

	BUG_ON(!rinfo || !param || !initdata);

	if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
		count = __3bit_pmmode_frm_map(initdata->pc_pins_map,
				__dsm_mode_to_pmmode(initdata->dsm_mode),
						pmmode);
	else
		count = __2bit_pmmode_frm_map(initdata->pc_pins_map,
				__dsm_mode_to_pmmode(initdata->dsm_mode),
					pmmode);
	for (i = 0; i < count; i++) {
#ifdef CONFIG_DEBUG_FS
		if (force_enable == 0) {
#endif
			/*TODO: should we change to read-modify-write ??*/
			ret = bcmpmu->write_dev(bcmpmu,
					rinfo[id].reg_pmctrl1 + i,
					pmmode[i]);
			if (ret)
				break;
			pr_rgltr(FLOW, "<%s> wrtite : reg[%x] =  %x\n",
					__func__, rinfo[id].reg_pmctrl1 + i,
					pmmode[i]);
#ifdef CONFIG_DEBUG_FS
		} else {
			rgltr_pmode_buf[id].pwr_mode[i] = pmmode[i];
		}
#endif
	}
	if (ret) {
		pr_rgltr(ERROR, "regltr enable error <%s>\n", __func__);
		return ret;
	}
	rinfo[id].flags |= RGLR_ON;
	return 0;
}

static int bcmpmureg_enable(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	int id;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	BUG_ON(bcmpmu == NULL || id >= BCMPMU_REGULATOR_MAX);

	return __bcmpmureg_enable(bcmpmu, id);
}

/*
* @disable: Configure the regulator as disabled.
*/
static int __bcmpmureg_disable(struct bcmpmu59xxx *bcmpmu, int id)
{
	struct bcmpmu59xxx_regulator_info *rinfo;
	int reg_cnt, i;
	int ret = 0;
	u8 val;

	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	pr_rgltr(FLOW, "<%s> id =  %d\n",
		__func__, id);
	BUG_ON(rinfo == NULL);

	if (rinfo[id].flags & RGLR_3BIT_PMCTRL) {
		val = PMMODE_OFF << PMMODE_3BIT_PM0_SHIFT |
				PMMODE_OFF << PMMODE_3BIT_PM1_SHIFT;
		reg_cnt = REGL_PMMODE_REG_MAX;
	} else {
		val = PMMODE_OFF << PMMODE_2BIT_PM0_SHIFT |
				PMMODE_OFF << PMMODE_2BIT_PM1_SHIFT |
				PMMODE_OFF << PMMODE_2BIT_PM2_SHIFT |
				PMMODE_OFF << PMMODE_2BIT_PM3_SHIFT;
		reg_cnt = 2;
	}

#ifdef CONFIG_DEBUG_FS
	if (force_enable == 0) {
#endif
		for (i = 0; i < reg_cnt; i++) {
			/*TODO: should we change to read-modify-write ??*/
			ret = bcmpmu->write_dev(bcmpmu,
					rinfo[id].reg_pmctrl1 + i,
					val);
			if (ret)
				break;
			pr_rgltr(FLOW, "<%s> wrtite : reg[%x] =  %x\n",
					__func__, rinfo[id].reg_pmctrl1 + i,
					val);
		}
		if (ret) {
			pr_rgltr(ERROR, "ERROR <%s>\n", __func__);
			return ret;
		}
#ifdef CONFIG_DEBUG_FS
	} else {
		for (i = 0; i < reg_cnt; i++)
			rgltr_pmode_buf[id].pwr_mode[i] = val;
	}
#endif
	rinfo[id].flags &= ~RGLR_ON;
	return 0;
}

static int bcmpmureg_disable(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	int id;
	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	BUG_ON(bcmpmu == NULL || id >= BCMPMU_REGULATOR_MAX);
	return __bcmpmureg_disable(bcmpmu, id);
}

/*
* @get_status: Return actual (not as-configured) status of regulator, as a
* REGULATOR_STATUS value (or negative errno)
*/
static int bcmpmureg_get_status(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	int id;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_regulator_init_data *initdata;
	struct bcmpmu59xxx_rgltr_param *param;

	int reg_cnt, i;
	int ret = 0;
	int enabled;
	u8 pmmode[REGL_PMMODE_REG_MAX] = {0};

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	param = bcmpmu->rgltr_data;

	BUG_ON(id >= BCMPMU_REGULATOR_MAX ||
			rinfo == NULL || param == NULL);
	initdata = param->pdata->bcmpmu_rgltr + id;

	if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
			reg_cnt = REGL_PMMODE_REG_MAX;
	else
		reg_cnt = 2;

	for (i = 0; i < reg_cnt; i++) {
		/*TODO: should we change to read-modify-write ??*/
		ret = bcmpmu->read_dev(bcmpmu, rinfo[id].reg_pmctrl1 + i,
								&pmmode[i]);
		if (ret)
			break;

		pr_rgltr(FLOW, "<%s> read : reg[%x] =  %x\n",
		__func__, rinfo[id].reg_pmctrl1 + i, pmmode[i]);
	}
	if (ret) {
		pr_rgltr(ERROR, "regltr read error <%s>\n", __func__);
		return ret;
	}
	if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
		enabled  = __is_3bit_pmmode_regl_enabled(initdata->pc_pins_map,
								pmmode);
	else
		enabled  = __is_2bit_pmmode_regl_enabled(initdata->pc_pins_map,
								pmmode);

	if (enabled)
		return REGULATOR_STATUS_NORMAL;
	return REGULATOR_STATUS_OFF;
}


/*
* @list_voltage: Return one of the supported voltages, in microvolts; zero if
* the selector indicates a voltage that is unusable on this system; or
* negative errno.  Selectors range from zero to one less than
* regulator_desc.n_voltages.  Voltages may be reported in any order.
*/

static int bcmpmuldo_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	struct bcmpmu59xxx *bcmpmu = rdev_get_drvdata(rdev);
	struct bcmpmu59xxx_regulator_info *rinfo;
	u8 id = rdev_get_id(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	if (selector >= rinfo[id].num_voltages)
		return -EINVAL;
	return rinfo[id].v_table[selector];
}

/*
* @set_voltage: Set the voltage for the regulator within the range specified.
* The driver should select the voltage closest to min_uV.
*/
static int bcmpmuldo_set_voltage(struct regulator_dev *rdev, int min_uv,
				 int max_uv, unsigned *sel)
{
	struct bcmpmu59xxx *bcmpmu;
	int id, i;
	struct bcmpmu59xxx_regulator_info *rinfo;
	u8 val;
	u32 uv;
	int match;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	BUG_ON(id >= BCMPMU_REGULATOR_MAX ||
			rinfo == NULL);

	*sel = -1;
	match = rinfo[id].num_voltages; /*init to max*/
	/*Find nearest match*/
	for (i = 0; i < rinfo[id].num_voltages; i++) {
		uv = rinfo[id].v_table[i];
		pr_rgltr(FLOW, "uv = %u min_uv = %d max_uv = %d match = %d\n",
			uv, min_uv, max_uv, match);
		if (min_uv <= uv && uv <= max_uv &&
			(match == rinfo[id].num_voltages ||
			rinfo[id].v_table[match] > uv)) {
				match = i;
				pr_rgltr(FLOW, "i = %d, match_v = %u\n", i,
					rinfo[id].v_table[match]);
				if (uv == min_uv)
					break;
			}
	}
	pr_rgltr(FLOW, "<%s> match = %x\n", __func__, match);
	if (match < rinfo[id].num_voltages) {
		*sel = match;
		bcmpmu->read_dev(bcmpmu, rinfo[id].reg_vout, &val);
		val &= ~rinfo[id].vout_mask;
		val |= (match  << rinfo[id].vout_shift) &
						rinfo[id].vout_mask;
		pr_rgltr(FLOW, "<%s> wrtite : reg[%x] =  %x\n",
			__func__, rinfo[id].reg_vout, val);
		return bcmpmu->write_dev(bcmpmu,
				rinfo[id].reg_vout, val);
	}
	return -EINVAL;
}

static int bcmpmuldo_get_voltage(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	int id;
	struct bcmpmu59xxx_regulator_info *rinfo;
	int ret = 0;
	u8 val;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	BUG_ON(id >= BCMPMU_REGULATOR_MAX ||
			rinfo == NULL);

	ret = bcmpmu->read_dev(bcmpmu, rinfo[id].reg_vout, &val);
	if (ret < 0) {
		pr_rgltr(ERROR,
			"ERR %s reading regulator voltage register.\n",
			__func__);
		return ret;
	}
	pr_info("%s:id = %d, reg[%x] = %x\n", __func__, id,
			rinfo[id].reg_vout, val);
	val = (rinfo[id].vout_mask & val) >> rinfo[id].vout_shift;
	BUG_ON(val >= rinfo[id].num_voltages);
	return rinfo[id].v_table[val];
}

static u32 bcmpmuldo_get_mode(struct regulator_dev *rdev)
{
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_init_data *rgltr_pdata;
	struct bcmpmu59xxx_rgltr_param *param;
	int id;

	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);

	BUG_ON(id >= BCMPMU_REGULATOR_MAX);

	param = bcmpmu->rgltr_data;
	rgltr_pdata = param->pdata->bcmpmu_rgltr + id;
	BUG_ON(!param || !rgltr_pdata);

	return __dsm_mode_to_rgltr(rgltr_pdata->dsm_mode);
}

static int bcmpmuldo_set_mode(struct regulator_dev *rdev, u32 mode)
{
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_regulator_init_data *rgltr_pdata;
	struct bcmpmu59xxx_rgltr_param *param;
	u32 pmu_mode;
	u8 pmmode[REGL_PMMODE_REG_MAX] = {0};
	int id, count, i;
	int ret = 0;

	if ((!rdev || !rdev->desc))
		return -1;
	bcmpmu = rdev_get_drvdata(rdev);
	id = rdev_get_id(rdev);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	BUG_ON(id >= BCMPMU_REGULATOR_MAX ||
			rinfo == NULL);
	pr_rgltr(FLOW, "<%s> id =  %d\n",
		__func__, id);

	param = bcmpmu->rgltr_data;
	rgltr_pdata = param->pdata->bcmpmu_rgltr + id;

	BUG_ON(!param || !rgltr_pdata);

	if (!rgltr_pdata->initdata->constraints.always_on) {
		pr_rgltr(ERROR,
			"set_mode only supported for alway on rglt\n");
		return -EINVAL;
	}

	pmu_mode = __rgltr_to_pmmode(mode);
	if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
		count = __3bit_pmmode_frm_map(rgltr_pdata->pc_pins_map,
						pmu_mode, pmmode);
	else
		count = __2bit_pmmode_frm_map(rgltr_pdata->pc_pins_map,
						pmu_mode, pmmode);
	for (i = 0; i < count; i++) {
#ifdef CONFIG_DEBUG_FS
		if (force_enable == 0) {
#endif
			ret = bcmpmu->write_dev(bcmpmu,
					rinfo[id].reg_pmctrl1 + i,
					pmmode[i]);
			if (ret)
				break;
			pr_rgltr(FLOW, "<%s> wrtite : reg[%x] =  %x\n",
					__func__, rinfo[id].reg_pmctrl1 + i,
					pmmode[i]);
#ifdef CONFIG_DEBUG_FS
		} else {
			rgltr_pmode_buf[id].pwr_mode[i] = pmmode[i];
		}
#endif
	}
	if (ret) {
		pr_rgltr(ERROR, "regltr set mode error <%s>\n", __func__);
		return ret;
	}

	rgltr_pdata->dsm_mode = __pmmode_to_dsm_mode(pmu_mode);
	return ret;
}

static int rgltr_set_trim_volt(struct regulator *rglr, int rgltr_id, int vindex)
{
	int ret;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;

	bcmpmu = regulator_get_drvdata(rglr);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	ret = bcmpmu->write_dev(bcmpmu, rinfo[rgltr_id].vout_trim, vindex);
	return ret;
}

static int rgltr_get_volt_inx(struct regulator *rglr, int rgltr_id,
		int vreq, int level)
{
	int i, match = SR_VOLT_START_INX;
	u32 voltage;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_rgltr_param *param;

	bcmpmu = regulator_get_drvdata(rglr);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	param = bcmpmu->rgltr_data;

	for (i = SR_VOLT_START_INX; i <= rinfo[rgltr_id].num_voltages; i++) {
		voltage = rinfo[rgltr_id].v_table[i];
		if (level) {
			/*upper bound*/
			if (voltage >= vreq &&
				(voltage <= rinfo[rgltr_id].v_table[match]
				 || match == SR_VOLT_START_INX))
				match = i;
		}

		/*lower bound*/
		else {
			voltage = rinfo[rgltr_id].v_table[i];
			if (voltage <= vreq &&
				(voltage >= rinfo[rgltr_id].v_table[match]
				 || match == SR_VOLT_START_INX))
				match = i;
		}
	}
	return match;
}

static int rgltr_cal_trimup_inx(struct regulator *rglr, int vdiff)
{
	int next, prev, i, match, *trim_tbl;
	struct bcmpmu59xxx *bcmpmu;

	bcmpmu = regulator_get_drvdata(rglr);
	trim_tbl = bcmpmu59xxx_get_trim_table(bcmpmu);

	match = TRIM_UP_MIN_INX;
	if (vdiff > trim_tbl[TRIM_UP_MAX_INX] ||
		vdiff < trim_tbl[TRIM_UP_MIN_INX])
		return -1;

	for (i = TRIM_UP_MIN_INX; i <= TRIM_UP_MAX_INX; i++) {
		prev = (vdiff > trim_tbl[match]) ? (vdiff - trim_tbl[match]) :
			(trim_tbl[match] - vdiff);
		next = (vdiff > trim_tbl[i]) ? (vdiff - trim_tbl[i]) :
			(trim_tbl[i] - vdiff);
		if (next < prev) {
			match = i;
			if (next == vdiff)
				return i;
		}
	}
	return match;
}

static int rgltr_cal_trimdwn_inx(struct regulator *rglr, int vdiff)
{
	int next, prev, i, match, *trim_tbl;
	struct bcmpmu59xxx *bcmpmu;

	bcmpmu = regulator_get_drvdata(rglr);
	trim_tbl = bcmpmu59xxx_get_trim_table(bcmpmu);
	match = TRIM_DOWN_MAX_INX;
	if (vdiff > trim_tbl[TRIM_DOWN_MAX_INX] ||
		vdiff < trim_tbl[TRIM_DOWN_MIN_INX])
		return -1;

	for (i = TRIM_DOWN_MAX_INX; i >= TRIM_DOWN_MIN_INX; i--) {
		prev = (vdiff > trim_tbl[match]) ? (vdiff - trim_tbl[match]) :
			(trim_tbl[match] - vdiff);
		next = (vdiff > trim_tbl[i]) ? (vdiff - trim_tbl[i]) :
			(trim_tbl[i] - vdiff);
		if (next < prev)
			match = i;
	}
	return match;

}

static int rgltr_cal_trim(struct regulator *rglr, int rgltr_id, long vreq)
{
	long vcurr, trim;
	int *trim_tbl;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_rgltr_param *param;

	bcmpmu = regulator_get_drvdata(rglr);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	param = bcmpmu->rgltr_data;
	trim_tbl = bcmpmu59xxx_get_trim_table(bcmpmu);

	vcurr = rinfo[rgltr_id].v_table[param->curr_inx];
	trim = (((vreq - vcurr) * (100 * 100)) / vcurr) +
		(trim_tbl[param->def_trim]);
	pr_rgltr(FLOW, "%s: vcurr = %ld, trim = %ld\n", __func__, vcurr, trim);

	if (!(trim >= trim_tbl[TRIM_DOWN_MIN_INX] &&
				trim <= trim_tbl[TRIM_UP_MAX_INX]))
		return -1;

	if (trim >= 0)
		return rgltr_cal_trimup_inx(rglr, trim);
	else
		return rgltr_cal_trimdwn_inx(rglr, trim);

	return -1;
}

static int rgltr_cal_trim_inx(struct regulator *rglr, int rgltr_id, u32 vreq)
{
	int trim_inx = -1;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx_rgltr_param *param;

	bcmpmu = regulator_get_drvdata(rglr);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	param = bcmpmu->rgltr_data;

	/*lower volatge*/
		param->curr_inx = rgltr_get_volt_inx(rglr, rgltr_id, vreq, 0);
		pr_rgltr(FLOW, "lower volatge value is %d\n",
				rinfo[rgltr_id].v_table[param->curr_inx]);
		trim_inx = rgltr_cal_trim(rglr, rgltr_id, vreq);
		if (trim_inx != -1)
			return trim_inx;

	/*higher volatge*/
		param->curr_inx = rgltr_get_volt_inx(rglr, rgltr_id, vreq, 1);
		pr_rgltr(FLOW, "higher volatge value is %d\n",
				rinfo[rgltr_id].v_table[param->curr_inx]);
		trim_inx = rgltr_cal_trim(rglr, rgltr_id, vreq);
		if (trim_inx != -1)
			return trim_inx;

	return -1;
}

static int rgltr_set_trim_vlt(const char *consumer, u32 vreq, int rgltr_id)
{
	int trim_inx;
	u8 val;
	struct regulator *rglr;
	struct bcmpmu59xxx_regulator_info *rinfo;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_rgltr_param *param;

	rglr = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(rglr))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return -EINVAL;
	}
	bcmpmu = regulator_get_drvdata(rglr);
	BUG_ON(!bcmpmu);
	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	param = bcmpmu->rgltr_data;

	/*default trim value*/
	bcmpmu->read_dev(bcmpmu, rinfo[rgltr_id].vout_trim, &val);
	param->def_trim = val;
	pr_rgltr(FLOW, "%s: default trim value is %d\n", __func__,
			param->def_trim);

	trim_inx = rgltr_cal_trim_inx(rglr, rgltr_id, vreq);
	pr_info("%s:trim_inx is %d for vreq = %d\n",
			__func__, trim_inx, vreq);
	if (trim_inx != -1) {
		rgltr_set_trim_volt(rglr, rgltr_id, trim_inx);
		regulator_set_voltage(rglr,
		rinfo[rgltr_id].v_table[param->curr_inx],
		rinfo[rgltr_id].v_table[param->curr_inx]);
	}

	regulator_put(rglr);
	return 0;
}

#ifdef CONFIG_DEBUG_FS

static ssize_t bcmpmu_dbg_rgltr_enable(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	int ret;
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s", consumer);
	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	ret = regulator_enable(regl);
	pr_info("%s regulator_enable : ret- %d\n", consumer, ret);
	regulator_put(regl);
	return count;
}

static ssize_t bcmpmu_dbg_rgltr_disable(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	int ret;
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s", consumer);
	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	ret = regulator_disable(regl);
	pr_info("%s regulator_disable : ret- %d\n", consumer, ret);
	regulator_put(regl);
	return count;
}

static ssize_t bcmpmu_dbg_rgltr_set_vlt(struct file *file,
		char const __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	u32 min_v = 0xFFFFFFFF;
	u32 max_v = 0xFFFFFFFF;
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s%u%u", consumer, &min_v, &max_v);
	if (consumer[0] == 0 || min_v == 0xFFFFFFFF ||
			max_v == 0xFFFFFFFF) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	pr_info("%s, %u %u\n", consumer, min_v, max_v);
	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	ret = regulator_set_voltage(regl, min_v, max_v);
	pr_info("%s regulator_set_voltage : ret- %d\n", consumer, ret);
	regulator_put(regl);
	return count;
}

static ssize_t bcmpmu_dbg_rgltr_get_vlt(struct file *file,
		char const __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s", consumer);
	if (consumer[0] == 0) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	ret = regulator_get_voltage(regl);
	pr_info("%s regulator_get_voltage : ret- %d\n", consumer, ret);
	regulator_put(regl);
	return count;
}

static ssize_t bcmpmu_dbg_is_rgltr_enable(struct file *file,
		char const __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s", consumer);
	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	ret = regulator_is_enabled(regl);
	pr_info("%s regulator_is_enabled : ret- %d\n", consumer, ret);
	regulator_put(regl);
	return count;
}

static ssize_t bcmpmu_dbg_rgltr_set_mode(struct file *file,
		char const __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	u32 mode = 0xFFFFFFFF;
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;

	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s%u", consumer, &mode);
	if (consumer[0] == 0 || mode == 0xFFFFFFFF) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}
	pr_info("%s, %u\n", consumer, mode);
	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	ret = regulator_set_mode(regl, mode);
	pr_info("%s regulator_set_mode : ret- %d\n", consumer, ret);
	regulator_put(regl);
	return count;
}

static ssize_t bcmpmu_dbg_rgltr_get_mode(struct file *file,
		char const __user *buf,
		size_t count, loff_t *offset)
{
	u32 len;
	char input_str[100] = {0};
	char consumer[20] = {0};
	u32 mode;
	struct regulator *regl;
	struct bcmpmu59xxx *bcmpmu = file->private_data;
	BUG_ON(!bcmpmu);
	if (count > 100)
		len = 100;
	else
		len = count;
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%s", consumer);
	if (consumer[0] == 0) {
		pr_info("invalid param !!\n");
		return -EFAULT;
	}

	pr_info("%s\n", consumer);
	regl = regulator_get(NULL, consumer);
	if (unlikely(IS_ERR_OR_NULL(regl))) {
		pr_info("regulator_get for %s failed\n", consumer);
		return count;
	}
	mode = regulator_get_mode(regl);
	pr_info("%s regulator_get_mode : mode = %d\n", consumer, mode);
	regulator_put(regl);
	if (mode == REGULATOR_MODE_NORMAL)
		pr_info("NORMAL/ON MODE\n");
	else if (mode == REGULATOR_MODE_STANDBY)
		pr_info("STANDYBY/LPM MODE\n");
	else if (mode == REGULATOR_MODE_IDLE)
		pr_info("IDLE/OFF MODE\n");

	return count;
}


static const struct file_operations debug_rgltr_en_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_enable,
};

static const struct file_operations debug_rgltr_dis_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_disable,
};

static const struct file_operations debug_rgltr_state_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_is_rgltr_enable,
};

static const struct file_operations debug_rgltr_set_vlt_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_set_vlt,
};

static const struct file_operations debug_rgltr_get_vlt_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_get_vlt,
};

static const struct file_operations debug_rgltr_set_mode_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_set_mode,
};

static const struct file_operations debug_rgltr_get_mode_fops = {
	.open = bcmpmu_debugfs_open,
	.write = bcmpmu_dbg_rgltr_get_mode,
};

static int dbg_enable_all_regulators(struct bcmpmu59xxx *bcmpmu)
{
	int id;
	struct bcmpmu59xxx_regulator_info *rinfo;
	int count, i;
	int ret = 0;

	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	BUG_ON(rinfo == NULL);
	for (id = 0; id < BCMPMU_REGULATOR_MAX; id++) {
		if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
			count = REGL_PMMODE_REG_MAX;
		else
			count = 2;
		for (i = 0; i < count; i++) {
			ret = bcmpmu->write_dev(bcmpmu,
					rinfo[id].reg_pmctrl1 + i, 0);
			if (ret)
				break;
		}
		if (ret) {
			pr_info("rgltr enable error <%s>\n", __func__);
			return ret;
		}
	}
	return 0;
}

static int dbg_restore_rgltr_reg(struct bcmpmu59xxx *bcmpmu)
{
	int id;
	struct bcmpmu59xxx_regulator_info *rinfo;
	int count, i;
	int ret = 0;

	rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
	BUG_ON(rinfo == NULL);
	for (id = 0; id < BCMPMU_REGULATOR_MAX; id++) {
		if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
			count = REGL_PMMODE_REG_MAX;
		else
			count = 2;
		for (i = 0; i < count; i++) {
			ret = bcmpmu->write_dev(bcmpmu,
					rinfo[id].reg_pmctrl1 + i,
					rgltr_pmode_buf[id].pwr_mode[i]);
			if (ret)
				break;
		}
		if (ret) {
			pr_info("rgltr restore error <%s>\n", __func__);
			return ret;
		}
	}
	return 0;
}

static int bcmpmu_debug_get_enable_all(void *data, u64 *val)
{
	*val = (u64)force_enable;
	return 0;
}

static int bcmpmu_debug_set_enable_all(void *data, u64 val)
{
	struct bcmpmu59xxx *bcmpmu = (struct bcmpmu59xxx *)data;
	BUG_ON(!bcmpmu);

	if (val == 1 && force_enable == 0) {
		int id;
		struct bcmpmu59xxx_regulator_info *rinfo;
		struct bcmpmu59xxx_regulator_init_data *initdata;
		struct bcmpmu59xxx_rgltr_param *param;
		int count, i;
		u8 reg_val;
		rinfo = bcmpmu59xxx_get_rgltr_info(bcmpmu);
		param = bcmpmu->rgltr_data;
		BUG_ON(rinfo == NULL || param == NULL);
		for (id = 0; id < BCMPMU_REGULATOR_MAX; id++) {
			initdata = param->pdata->bcmpmu_rgltr + id;
			if (rinfo[id].flags & RGLR_3BIT_PMCTRL)
				count = REGL_PMMODE_REG_MAX;
			else
				count = 2;
			for (i = 0; i < count; i++) {
				bcmpmu->read_dev(bcmpmu,
						rinfo[id].reg_pmctrl1 + i,
						&reg_val);
				rgltr_pmode_buf[id].pwr_mode[i] = reg_val;
			}
		}
		dbg_enable_all_regulators(bcmpmu);
		force_enable = 1;
	} else if (val == 0 && force_enable == 1) {
		dbg_restore_rgltr_reg(bcmpmu);
		force_enable = 0;
	} else
		return -EINVAL;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debug_rgltr_enable_all_fops,
		bcmpmu_debug_get_enable_all,
		bcmpmu_debug_set_enable_all, "%llu\n");

static void bcmpmu59xxx_debug_init(struct bcmpmu59xxx_rgltr_param *param)
{
	struct bcmpmu59xxx *bcmpmu = param->bcmpmu;
	if (!bcmpmu || !bcmpmu->dent_bcmpmu) {
		pr_err("Failed to initialize debugfs\n");
		return;
	}

	param->rgltr_dbgfs = debugfs_create_dir("rgltr", bcmpmu->dent_bcmpmu);
	if (!param->rgltr_dbgfs)
		return;

	if (!debugfs_create_file("is_enabled", S_IWUSR | S_IRUSR,
					param->rgltr_dbgfs, bcmpmu,
					&debug_rgltr_state_fops))
		goto err;

	if (!debugfs_create_file("enable", S_IWUSR | S_IRUSR,
					param->rgltr_dbgfs, bcmpmu,
					&debug_rgltr_en_fops))
		goto err;

	if (!debugfs_create_file("disable", S_IWUSR | S_IRUSR,
					param->rgltr_dbgfs, bcmpmu,
					&debug_rgltr_dis_fops))
		goto err;

	if (!debugfs_create_file("get_vlt", S_IWUSR | S_IRUSR,
					param->rgltr_dbgfs, bcmpmu,
					&debug_rgltr_get_vlt_fops))
		goto err;

	if (!debugfs_create_file("set_vlt", S_IWUSR | S_IRUSR,
					param->rgltr_dbgfs, bcmpmu,
					&debug_rgltr_set_vlt_fops))
		goto err;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				param->rgltr_dbgfs, &debug_mask))
		goto err ;

	if (!debugfs_create_file("force_enable_all", S_IWUSR | S_IRUSR,
				param->rgltr_dbgfs, bcmpmu,
				&debug_rgltr_enable_all_fops))
			goto err ;

	if (!debugfs_create_file("set_mode", S_IWUSR | S_IRUSR,
				param->rgltr_dbgfs, bcmpmu,
				&debug_rgltr_set_mode_fops))
			goto err ;

	if (!debugfs_create_file("get_mode", S_IWUSR | S_IRUSR,
				param->rgltr_dbgfs, bcmpmu,
				&debug_rgltr_get_mode_fops))
			goto err;
	return;
err:
	debugfs_remove(param->rgltr_dbgfs);
}

#endif
static int bcmpmu_regulator_probe(struct platform_device *pdev)
{
	int i;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_regulator_pdata *pdata;
	struct bcmpmu59xxx_regulator_init_data *bcmpmu_rgltrs;
	struct bcmpmu59xxx_regulator_info *rgltr_info;
	struct bcmpmu59xxx_rgltr_param *param;
	int num_rgltr;
	int rgltr_id, ret = 0;

	pdata = (struct  bcmpmu59xxx_regulator_pdata *)
		pdev->dev.platform_data;
	BUG_ON(pdata == NULL);
	bcmpmu_rgltrs = pdata->bcmpmu_rgltr;
	num_rgltr = pdata->num_rgltr;
	pr_rgltr(INIT, "###########%s############: called\n", __func__);
	bcmpmu59xxx_rgltr_info_init(bcmpmu);
	/*
	 * register regulator
	 */
	rgltr_info = bcmpmu59xxx_get_rgltr_info(bcmpmu);

	if ((rgltr_info == NULL) || (rgltr_info->rdesc == NULL)) {
		pr_rgltr(ERROR, "ERR %s regulator info and desc not avail.\n",
				__func__);
		return -EINVAL;
	}

	param =  kzalloc(sizeof(struct bcmpmu59xxx_rgltr_param), GFP_KERNEL);
	if (!param) {
		pr_rgltr(ERROR, "ERR: %s failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	param->pdata = pdata;
	param->bcmpmu = bcmpmu;
	bcmpmu->rgltr_data = param;
	for (i = 0; i < BCMPMU_REGULATOR_MAX; i++) {
		rgltr_id = bcmpmu_rgltrs[i].id;
		if (bcmpmu_rgltrs[i].initdata) {
			pr_rgltr(INIT,
				"%s:id:%d, name:%s dsm_mode=%x pc_map=%x\n",
					__func__, rgltr_id,
					rgltr_info->rdesc[rgltr_id].name,
					bcmpmu_rgltrs[i].dsm_mode,
					bcmpmu_rgltrs[i].pc_pins_map);
			if (bcmpmu_rgltrs[i].initdata->constraints.always_on ||
				bcmpmu_rgltrs[i].initdata->constraints.boot_on)
				__bcmpmureg_enable(bcmpmu, rgltr_id);
			else {
				/*As per design, dsm_mode shouldn't be set to
				BCMPMU_REGL_OFF_IN_DSM for LDOs managed by SW*/
				WARN_ON(bcmpmu_rgltrs[i].dsm_mode ==
						BCMPMU_REGL_OFF_IN_DSM);
				__bcmpmureg_disable(bcmpmu, rgltr_id);
			}
			regl[i] =
				regulator_register(rgltr_info[rgltr_id].rdesc,
						&pdev->dev,
						bcmpmu_rgltrs[i].initdata,
						bcmpmu, pdev->dev.of_node);
			if (IS_ERR_OR_NULL(regl[i])) {
				dev_err(&pdev->dev, "failed to register %s\n",
					rgltr_info->rdesc[rgltr_id].name);
				ret = PTR_ERR(regl[i]);

				goto register_fail;
			}

			if (bcmpmu_rgltrs[i].req_volt && !is_soft_reset())
				rgltr_set_trim_vlt(bcmpmu_rgltrs[i].initdata->
				consumer_supplies[0].supply,
				bcmpmu_rgltrs[i].req_volt, i);
		} else
			regl[i] = NULL;
	}
	regulator_has_full_constraints();

#ifdef CONFIG_DEBUG_FS
	bcmpmu59xxx_debug_init(param);
#endif
	return ret;
register_fail:
	while (i-- > 0) {
		if (regl[i])
			regulator_unregister(regl[i]);
	}
	rgltr_info->rdesc = NULL;
	rgltr_info = NULL;
	kfree(param);
	return ret;
}

static int bcmpmu_regulator_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_rgltr_param *param = bcmpmu->rgltr_data;
	int i;

	for (i = 0; i < BCMPMU_REGULATOR_MAX; i++) {
		if (regl[i])
			regulator_unregister(regl[i]);
	}

#ifdef CONFIG_DEBUG_FS
	if (param)
		debugfs_remove(param->rgltr_dbgfs);
#endif
	kfree(param);
	return 0;
}

static struct platform_driver bcmpmu_regulator_driver = {
	.probe = bcmpmu_regulator_probe,
	.remove = bcmpmu_regulator_remove,
	.driver = {
		   .name = "bcmpmu59xxx-regulator",
		   },
};

static int __init bcmpmu_regulator_init(void)
{
	return platform_driver_register(&bcmpmu_regulator_driver);
}
subsys_initcall(bcmpmu_regulator_init);

static void __exit bcmpmu_regulator_exit(void)
{
	platform_driver_unregister(&bcmpmu_regulator_driver);
}
module_exit(bcmpmu_regulator_exit);

/*
* Module information
*/
MODULE_DESCRIPTION("BCMPMU regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcmpmu-regulator");

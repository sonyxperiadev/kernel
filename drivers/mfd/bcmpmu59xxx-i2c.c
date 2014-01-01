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
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <mach/pwr_mgr.h>
#include <linux/spinlock.h>
#include <plat/pwr_mgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#define PWRMGR_I2C_RETRY_DELAY_US  10
#define PWRMGR_I2C_RDWR_MAX_TRIES 10

#define I2C_LOG_BUF_SZ		(SZ_1K)
#define I2C_LOG_BUF_OFF		0

static int debug_mask = BCMPMU_PRINT_INIT |  BCMPMU_PRINT_ERROR;
#define pr_pmui2c(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

enum { I2C_TRANS_NONE, I2C_TRANS_READ, I2C_TRANS_WRITE,
};
static struct platform_device bcmpmu59xxx_pdev = {
	.name = "bcmpmu59xxx_core",
	.id = -1,
	.dev.platform_data = NULL,
};

#if 0
#ifdef CONFIG_HAS_WAKELOCK
static DEFINE_SPINLOCK(wl_lock);
#endif
#endif

static u32 *i2c_log_buf_v;
static u32 *i2c_log_buf_p;

/**
 * bcmpmu_i2c_log - log i2c data to un-cached buffer
 *
 * @bcmpmu_i2c:		Pointer to bcmpmu_i2c struct
 * @rdwr:		READ/WRITE flag. 1 for read, 0 for write
 *			and negative value for ERROR
 * @slave:		Slave id
 * @reg:		register address
 * @val:		value
 *
 * Log the i2c data in uncached buffer. Offset 0 in the
 * @bcmpmu_i2c->i2c_log_buf_v is reserved for index purpose.
 * Value at offset 0 shows the current index where new data will be
 * written. @rdwr, @slave, @reg and @val are packed in a interger
 * and then written to the buffer
 */

static void bcmpmu_i2c_log(u32 *buf_v, char rdwr, u8 slave, u8 reg, u8 val)
{
	int new_off;

	BUG_ON(buf_v == NULL);

	new_off = *(buf_v + I2C_LOG_BUF_OFF);
	if ((new_off > (I2C_LOG_BUF_SZ / SZ_4)) || (new_off == I2C_LOG_BUF_OFF))
		new_off = I2C_LOG_BUF_OFF + 1;

	*(buf_v + new_off) = ((slave << 24) | (reg << 16) |
			(val << 8) | ((rdwr < 0) ? 'X' : (rdwr ? 'R' : 'W')));
	*(buf_v + I2C_LOG_BUF_OFF) = ++new_off;
}

static inline void bcmpmu_i2c_lock(struct bcmpmu59xxx *bcmpmu)
{
	/**
	 * This mechanism creates a problem in 3.10 kernel
	 * during suspend (dpm sees this a active wakeup source)
	 * Disable this for time being
	 */
#if 0
#ifdef CONFIG_HAS_WAKELOCK
	/*Any thread can lock/unlock a wake lock.
	   Since PMU I2C APIs can be invoked from mutiple threads,
	   ref counting is added to manage PMU I2C wake lock.
	   I2C wake lock will the aquired by the first threading entering
	   I2C read/write function and the wake lock will be released
	   only by the last thread returning from the read/write function. */
	spin_lock(&wl_lock);
	if (bcmpmu->pmu_bus->ref_count++ == 0)
		wake_lock(&bcmpmu->pmu_bus->i2c_lock);
	spin_unlock(&wl_lock);
#endif
#endif
	mutex_lock(&bcmpmu->pmu_bus->i2c_mutex);
}

static inline void bcmpmu_i2c_unlock(struct bcmpmu59xxx *bcmpmu)
{
	mutex_unlock(&bcmpmu->pmu_bus->i2c_mutex);
#if 0
#ifdef CONFIG_HAS_WAKELOCK
	spin_lock(&wl_lock);
	BUG_ON(bcmpmu->pmu_bus->ref_count == 0);
	if (--bcmpmu->pmu_bus->ref_count == 0)
		wake_unlock(&bcmpmu->pmu_bus->i2c_lock);
	spin_unlock(&wl_lock);
#endif
#endif
}

#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)

int last_trans = I2C_TRANS_NONE;

static int bcmpmu_i2c_try_write(struct bcmpmu59xxx *bcmpmu, u32 reg, u8 value)
{
	int err = 0;
	int tries = PWRMGR_I2C_RDWR_MAX_TRIES;
	u8 temp = 0;
	u8 check_fifo = DEC_REG_FLAG(reg) & 1;
	if (DEC_REG_ADD(reg) >= PMU_REG_MAX)
		return -ENODEV;
	while (tries--) {
		if (check_fifo && (last_trans == I2C_TRANS_WRITE)) {
			err =
			pwr_mgr_pmu_reg_read((u8)
						DEC_REG_ADD(PMU_REG_I2CCTRL1),
						bcmpmu_get_slaveid(bcmpmu,
						PMU_REG_I2CCTRL1),
						&temp);
			if (err == 0) {
				if (!(temp & I2CCTRL1_FIFOFULL_MASK)) {
					err = pwr_mgr_pmu_reg_write
					    ((u8) DEC_REG_ADD(reg),
					     bcmpmu_get_slaveid(bcmpmu, reg),
					     value);
					if (err == 0) {
						last_trans = I2C_TRANS_WRITE;
						break;
					}
				}
			}
			udelay(PWRMGR_I2C_RETRY_DELAY_US);
		} else {
			err =
			    pwr_mgr_pmu_reg_write((u8) DEC_REG_ADD(reg),
						  bcmpmu_get_slaveid(bcmpmu,
								     reg),
						  value);
			if (err == 0) {
				last_trans = I2C_TRANS_WRITE;
				break;
			}
		}
	}
	if (tries <= 0) {
		err = -EAGAIN;
		pr_pmui2c(ERROR, "ERR: I2C SW SEQ Write MAX Tries\n");
	}
	if (!err)
		bcmpmu_i2c_log(i2c_log_buf_v, 0,
				bcmpmu_get_slaveid(bcmpmu, reg),
				DEC_REG_ADD(reg),
				value);
	else
		bcmpmu_i2c_log(i2c_log_buf_v, (char)err,
				bcmpmu_get_slaveid(bcmpmu, reg),
				DEC_REG_ADD(reg),
				value);

	return err;
}

static int bcmpmu_i2c_try_read(struct bcmpmu59xxx *bcmpmu, u32 reg, u8 * value)
{
	int err = 0;
	int tries = PWRMGR_I2C_RDWR_MAX_TRIES;
	u8 temp = 0;
	u8 check_fifo = DEC_REG_FLAG(reg) & 1;

	if (DEC_REG_ADD(reg) >= PMU_REG_MAX)
		return -ENODEV;
	while (tries--) {
		/**
		 * Read the FIFORDBLOCK Bit of PMU before
		 *initiating an read transaction if the last
		 *trasaction was i2c write. This bit is set by
		 *the
		 * PMU when its busy finishing previous write
		 *operation (if this bit is ignored there is a
		 *chance of reading a stale data of the
		 *register !!)
		 */
		if (check_fifo && (last_trans == I2C_TRANS_WRITE)) {
			err =
			pwr_mgr_pmu_reg_read((u8)
					DEC_REG_ADD(PMU_REG_I2CCTRL1),
					bcmpmu_get_slaveid(bcmpmu,
					PMU_REG_I2CCTRL2),
					&temp);
			if (!err && (temp & I2CCTRL1_FIFOEMPTY_MASK)) {

				err = pwr_mgr_pmu_reg_read((u8)
							   DEC_REG_ADD
							   (reg),
							   bcmpmu_get_slaveid
							   (bcmpmu, reg),
							   value);
				if (err == 0) {
					last_trans = I2C_TRANS_READ;
					break;
				}
			}
		} else {
			err =
				pwr_mgr_pmu_reg_read((u8) DEC_REG_ADD(reg),
						bcmpmu_get_slaveid(bcmpmu,
							reg),
						value);
			if (err == 0) {
				last_trans = I2C_TRANS_READ;
				break;
			}
		}
		udelay(PWRMGR_I2C_RETRY_DELAY_US);
	}
	if (tries <= 0) {
		pr_pmui2c(ERROR, "ERR: I2C SW SEQ Max Tries\n");
		err = -EAGAIN;
	}

	if (!err)
		bcmpmu_i2c_log(i2c_log_buf_v, 1,
				bcmpmu_get_slaveid(bcmpmu, reg),
				DEC_REG_ADD(reg),
				*value);
	else
		bcmpmu_i2c_log(i2c_log_buf_v, (char)err,
				bcmpmu_get_slaveid(bcmpmu, reg),
				DEC_REG_ADD(reg),
				*value);

	return err;
}

static int bcmpmu_i2c_pwrmgr_read(struct bcmpmu59xxx *bcmpmu, u32 reg, u8 * val)
{
	int ret;
	if (!bcmpmu || !val)
		return -EINVAL;

	bcmpmu_i2c_lock(bcmpmu);
	ret = bcmpmu_i2c_try_read(bcmpmu, reg, val);
	bcmpmu_i2c_unlock(bcmpmu);
	pr_pmui2c(DATA, "RD done reg %x val %x\n", reg, *val);
	return ret;
}

static int bcmpmu_i2c_pwrmgr_write(struct bcmpmu59xxx *bcmpmu, u32 reg, u8 val)
{
	int ret = 0;
	bcmpmu_i2c_lock(bcmpmu);
	ret = bcmpmu_i2c_try_write(bcmpmu, reg, val);
	bcmpmu_i2c_unlock(bcmpmu);
	pr_pmui2c(DATA, "WR done reg %x val %x\n", reg, val);
	return ret;
}

static int bcmpmu_i2c_pwrmgr_read_bulk(struct bcmpmu59xxx *bcmpmu,
				       u32 reg, u8 *val, int len)
{
	int err = 0;
	int i;
	u8 temp = 0;

	bcmpmu_i2c_lock(bcmpmu);
	for (i = 0; i < len; i++) {
		err = bcmpmu_i2c_try_read(bcmpmu, reg + i, &temp);
		if (err < 0)
			break;
		val[i] = temp;
	}
	bcmpmu_i2c_unlock(bcmpmu);
	return err;
}

static int bcmpmu_i2c_pwrmgr_write_bulk(struct bcmpmu59xxx *bcmpmu,
					u32 reg, u8 *val, int len)
{
	int err = 0;
	int i;
	u8 temp;

	bcmpmu_i2c_lock(bcmpmu);
	for (i = 0; i < len; i++) {
		temp = val[i];
		err = bcmpmu_i2c_try_write(bcmpmu, reg + i, temp);
		if (err < 0)
			break;
	}
	bcmpmu_i2c_unlock(bcmpmu);
	return err;
}

#else

static inline struct i2c_client *bcmpmu_get_client(struct bcmpmu59xxx *bcmpmu,
						   u32 reg)
{
	u8 map = DEC_MAP_ADD(reg);
	if (map)
		return bcmpmu->pmu_bus->companinon[map - 1];
	else
		return bcmpmu->pmu_bus->i2c;

}

/* Return Error codes in i2c read/write interfaces
 *-ENODEV = if wrong register has been passed
 *-ENXIO = if register exists but no map found
 *-EIO = i2c read/write error
*/
static int bcmpmu_i2c_read_device(struct bcmpmu59xxx *bcmpmu, u32 reg, u8 *val)
{
	int err;
	struct i2c_client *clt;

	if (DEC_REG_ADD(reg) >= PMU_REG_MAX)
		return -ENODEV;
	clt = bcmpmu_get_client(bcmpmu, reg);

	bcmpmu_i2c_lock(bcmpmu);
	err = i2c_smbus_read_byte_data(clt, (u8) DEC_REG_ADD(reg));
	bcmpmu_i2c_unlock(bcmpmu);
	if (err < 0)
		return err;
	*val = err;
	return 0;
}

static int bcmpmu_i2c_write_device(struct bcmpmu59xxx *bcmpmu, u32 reg,
				   u8 value)
{
	int err;
	struct i2c_client *clt;

	if (DEC_REG_ADD(reg) >= PMU_REG_MAX)
		return -ENODEV;

	clt = bcmpmu_get_client(bcmpmu, reg);

	bcmpmu_i2c_lock(bcmpmu);
	err = i2c_smbus_write_byte_data(clt, (u8) DEC_REG_ADD(reg), value);
	bcmpmu_i2c_unlock(bcmpmu);
	return (err < 0 ? err : 0);
}

static int bcmpmu_i2c_read_device_direct_bulk(struct bcmpmu59xxx *bcmpmu,
					      u32 reg, u8 *val, int len)
{
	int err = 0;
	struct i2c_client *clt;

	if ((DEC_REG_ADD(reg) + len) >= PMU_REG_MAX)
		return -ENODEV;
	clt = bcmpmu_get_client(bcmpmu, reg);

	bcmpmu_i2c_lock(bcmpmu);
	err =
	    i2c_smbus_read_i2c_block_data(clt, (u8) DEC_REG_ADD(reg), len, val);
	bcmpmu_i2c_unlock(bcmpmu);
	return (err < 0 ? err : 0);
}

static int bcmpmu_i2c_write_device_direct_bulk(struct bcmpmu59xxx *bcmpmu,
					       u32 reg, u8 *val, int len)
{
	int err = 0;
	struct i2c_client *clt;

	if ((DEC_REG_ADD(reg) + len) >= PMU_REG_MAX)
		return -ENODEV;
	clt = bcmpmu_get_client(bcmpmu, reg);

	bcmpmu_i2c_lock(bcmpmu);
	err =
		i2c_smbus_write_i2c_block_data(clt, (u8) DEC_REG_ADD(reg),
				len, val);
	bcmpmu_i2c_unlock(bcmpmu);
	return (err < 0 ? err : 0);
}
#endif

static int bcmpmu_i2c_set_dev_mode(struct bcmpmu59xxx *bcmpmu, int poll)
{
#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
	return pwr_mgr_set_i2c_mode(1);
#else
	return -EPERM;
#endif
}

static void bcmpmu59xxx_debug_init(struct bcmpmu59xxx *bcmpmu)
{
	if (!bcmpmu->dent_bcmpmu) {
		bcmpmu->dent_bcmpmu = debugfs_create_dir("bcmpmu59xxx", NULL);
		if (!bcmpmu->dent_bcmpmu) {
			pr_err("Failed to initialize debugfs\n");
			return;
		}
	}
	bcmpmu->pmu_bus->dentry  =
		debugfs_create_dir("i2c", bcmpmu->dent_bcmpmu);

	if (!bcmpmu->pmu_bus->dentry)
		pr_err("Failed to setup i2c debugfs\n");

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				bcmpmu->pmu_bus->dentry , &debug_mask))
		debugfs_remove(bcmpmu->pmu_bus->dentry);

}
static int bcmpmu59xxx_i2c_probe(struct i2c_client *i2c,
				 const struct i2c_device_id *id)
{
	struct bcmpmu59xxx_platform_data *pdata;
	struct bcmpmu59xxx *bcmpmu = NULL;
	struct bcmpmu59xxx_bus *ic_bus = NULL;
	int ret = 0;
#if !defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
	u8 i;
	struct i2c_adapter *adp;
#endif
	void *virt;
	dma_addr_t phy;

	pdata = (struct bcmpmu59xxx_platform_data *)i2c->dev.platform_data;
	pr_pmui2c(INIT, "%s\n", __func__);
	pr_pmui2c(INIT, "%s called\n", __func__);
	ic_bus = kzalloc(sizeof(struct bcmpmu59xxx_bus), GFP_KERNEL);
	bcmpmu = kzalloc(sizeof(struct bcmpmu59xxx), GFP_KERNEL);
	virt = dma_zalloc_coherent(NULL, I2C_LOG_BUF_SZ, &phy, GFP_KERNEL);

	if ((bcmpmu == NULL) || (ic_bus == NULL) || (virt == NULL)) {
		pr_pmui2c(ERROR, "ERR: %s failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	bcmpmu->pmu_bus = ic_bus;
	bcmpmu->pdata = pdata;

	i2c_log_buf_p = (u32 *)phy;
	i2c_log_buf_v = (u32 *)virt;

	pr_pmui2c(INIT, "%s: i2c log buff phy_addr %p virt_addr %p\n",
			__func__, i2c_log_buf_p, i2c_log_buf_v);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&ic_bus->i2c_lock, WAKE_LOCK_SUSPEND,
		"bcmpmu_i2c");
	ic_bus->ref_count = 0;
#endif
	bcmpmu->pmu_bus->i2c = i2c;
	i2c_set_clientdata(i2c, bcmpmu);
	bcmpmu->dev = &bcmpmu->pmu_bus->i2c->dev;
	mutex_init(&bcmpmu->pmu_bus->i2c_mutex);
	dev_set_drvdata(bcmpmu->dev, bcmpmu);
#if !defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
	adp = i2c_get_adapter(pdata->i2c_adapter_id);
	for (i = 0; i < pdata->companion; i++) {
		bcmpmu->pmu_bus->companinon[i] =
			i2c_new_dummy(adp, pdata->i2c_companion_info[i].addr);
		if (!bcmpmu->pmu_bus->companinon[i])
			pr_pmui2c(ERROR,
				"%s dummy i2c dev failed\n", __func__);
		else
			i2c_set_clientdata(bcmpmu->pmu_bus->companinon[i],
					bcmpmu);
	}
	bcmpmu->read_dev = bcmpmu_i2c_read_device;
	bcmpmu->write_dev = bcmpmu_i2c_write_device;
	bcmpmu->read_dev_bulk = bcmpmu_i2c_read_device_direct_bulk;
	bcmpmu->write_dev_bulk = bcmpmu_i2c_write_device_direct_bulk;
	bcmpmu->set_dev_mode = bcmpmu_i2c_set_dev_mode;
#else
	mach_init_sequencer();
	bcmpmu->read_dev = bcmpmu_i2c_pwrmgr_read;
	bcmpmu->write_dev = bcmpmu_i2c_pwrmgr_write;
	bcmpmu->read_dev_bulk = bcmpmu_i2c_pwrmgr_read_bulk;
	bcmpmu->write_dev_bulk = bcmpmu_i2c_pwrmgr_write_bulk;
	bcmpmu->set_dev_mode = bcmpmu_i2c_set_dev_mode;
#endif
	bcmpmu59xxx_pdev.dev.platform_data = bcmpmu;
	platform_device_register(&bcmpmu59xxx_pdev);
#ifdef CONFIG_DEBUG_FS
	bcmpmu59xxx_debug_init(bcmpmu);
#endif
	return ret;
err:
	kfree(bcmpmu);
	kfree(ic_bus);
	return ret;
}

static int bcmpmu59xxx_i2c_remove(struct i2c_client *i2c)
{
	struct bcmpmu59xxx *bcmpmu = i2c_get_clientdata(i2c);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(bcmpmu->pmu_bus->dentry);
#endif
	platform_device_unregister(&bcmpmu59xxx_pdev);
	dma_free_coherent(NULL, I2C_LOG_BUF_SZ, i2c_log_buf_v,
			(dma_addr_t)i2c_log_buf_p);
	kfree(bcmpmu->pmu_bus);
	kfree(bcmpmu);
	return 0;
}

static const struct i2c_device_id bcmpmu59xxx_i2c_id[] = {
	{"bcmpmu59xxx_i2c", 0}, {}
};

MODULE_DEVICE_TABLE(i2c, bcmpmu59xxx_i2c_id);
static struct i2c_driver __refdata bcmpmu59xxx_i2c_driver = {
	.driver = {
		   .name = "bcmpmu59xxx_i2c",
		   .owner = THIS_MODULE,
		   },
	.probe = bcmpmu59xxx_i2c_probe,
	.remove = bcmpmu59xxx_i2c_remove,
	.id_table = bcmpmu59xxx_i2c_id,
};

static int __init bcmpmu59xxx_i2c_init(void)
{
	return i2c_add_driver(&bcmpmu59xxx_i2c_driver);
}

/* init early so consumer devices can complete system boot */
subsys_initcall(bcmpmu59xxx_i2c_init);

static void __exit bcmpmu59xxx_i2c_exit(void)
{
	i2c_del_driver(&bcmpmu59xxx_i2c_driver);
}
module_exit(bcmpmu59xxx_i2c_exit);

MODULE_DESCRIPTION("I2C support for BCMPMU590XX PMIC");
MODULE_LICENSE("GPL");

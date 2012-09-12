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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/mfd/bcmpmu.h>
#include <mach/pwr_mgr.h>
#include <linux/spinlock.h>
#include <plat/pwr_mgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif


#define PWRMGR_I2C_RDWR_MAX_TRIES	(10)
#define PWRMGR_I2C_RETRY_DELAY_US	(10)

#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
/**
 * Power manager i2c sequencer transaction types
 */
enum {
	I2C_TRANS_NONE,
	I2C_TRANS_READ,
	I2C_TRANS_WRITE,
};
#endif

struct bcmpmu_i2c {
	struct bcmpmu *bcmpmu;
	struct mutex i2c_mutex;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock i2c_lock;
	u32 ref_count;
#endif
	struct i2c_client *i2c_client;
	struct i2c_client *i2c_client1;
	int pagesize;
};

#ifdef CONFIG_HAS_WAKELOCK
static DEFINE_SPINLOCK(wl_lock);
#endif

static inline void bcmpmu_i2c_lock(struct bcmpmu_i2c *i2c)
{
#ifdef CONFIG_HAS_WAKELOCK
	/*Any thread can lock/unlock a wake lock.
	Since PMU I2C APIs can be invoked from mutiple threads,
	ref counting is added to manage PMU I2C wake lock.
	I2C wake lock will the aquired by the first threading entering
	I2C read/write function and the wake lock will be released
	only by the last thread returning from the read/write function.*/
	spin_lock(&wl_lock);
	if (i2c->ref_count++ == 0)
		wake_lock(&i2c->i2c_lock);
	spin_unlock(&wl_lock);
#endif
	mutex_lock(&i2c->i2c_mutex);
}

static inline void bcmpmu_i2c_unlock(struct bcmpmu_i2c *i2c)
{
	mutex_unlock(&i2c->i2c_mutex);
#ifdef CONFIG_HAS_WAKELOCK
	spin_lock(&wl_lock);
	BUG_ON(i2c->ref_count == 0);
	if (--i2c->ref_count == 0)
		wake_unlock(&i2c->i2c_lock);
	spin_unlock(&wl_lock);
#endif
}


#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
int last_i2c_trans = I2C_TRANS_NONE;
#endif

#if !defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
/* Return Error codes in i2c read/write interfaces
 *-ENODEV = if wrong register has been passed
 *-ENXIO = if register exists but no map found
 *-EIO = i2c read/write error
*/
static int bcmpmu_i2c_read_device(struct bcmpmu *bcmpmu, int reg,
				  unsigned int *val, unsigned int msk)
{
	struct bcmpmu_reg_map map;
	int err;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;

	if (reg >= PMU_REG_MAX)
		return -ENODEV;
	map = bcmpmu->regmap[reg];
	if ((map.addr == 0) && (map.mask == 0))
		return -ENXIO;

	bcmpmu_i2c_lock(acc);
	if (map.map == 0)
		err = i2c_smbus_read_byte_data(acc->i2c_client, map.addr);
	else if (map.map == 1)
		err = i2c_smbus_read_byte_data(acc->i2c_client1, map.addr);
	else
		err = -EIO;
	bcmpmu_i2c_unlock(acc);

	if (err < 0)
		return err;
	err = err & msk;
	err = err & map.mask;
	*val = err;

	return 0;
}

static int bcmpmu_i2c_write_device(struct bcmpmu *bcmpmu, int reg,
				   unsigned int value, unsigned int msk)
{
	struct bcmpmu_reg_map map;
	int err;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;

	if (reg >= PMU_REG_MAX)
		return -ENODEV;
	map = bcmpmu->regmap[reg];
	if ((map.addr == 0) && (map.mask == 0))
		return -ENXIO;

	bcmpmu_i2c_lock(acc);
	if (map.map == 0)
		err = i2c_smbus_read_byte_data(acc->i2c_client, map.addr);
	else if (map.map == 1)
		err = i2c_smbus_read_byte_data(acc->i2c_client1, map.addr);
	else
		err = -EIO;
	if (err < 0)
		goto err;

	err = err & ~msk;
	err = err & ~map.mask;
	value = value | err;

	if (map.map == 0)
		err =
		    i2c_smbus_write_byte_data(acc->i2c_client, map.addr, value);
	else if (map.map == 1)
		err =
		    i2c_smbus_write_byte_data(acc->i2c_client1, map.addr,
					      value);
	else
		err = -EIO;
      err:
	bcmpmu_i2c_unlock(acc);
	return err;
}

static int bcmpmu_i2c_read_device_direct(struct bcmpmu *bcmpmu, int map,
					 int addr, unsigned int *val,
					 unsigned int msk)
{
	int err;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	if ((addr == 0) && (msk == 0))
		return -ENODEV;

	bcmpmu_i2c_lock(acc);
	if (map == 0)
		err = i2c_smbus_read_byte_data(acc->i2c_client, addr);
	else if (map == 1)
		err = i2c_smbus_read_byte_data(acc->i2c_client1, addr);
	else
		err = -EIO;
	bcmpmu_i2c_unlock(acc);

	if (err < 0)
		return err;
	err = err & msk;
	*val = err;
	return 0;
}

static int bcmpmu_i2c_write_device_direct(struct bcmpmu *bcmpmu, int map,
					  int addr, unsigned int val,
					  unsigned int msk)
{
	int err;
	u8 value = (u8) val;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	if ((addr == 0) && (msk == 0))
		return -ENODEV;

	bcmpmu_i2c_lock(acc);
	if (map == 0)
		err = i2c_smbus_read_byte_data(acc->i2c_client, addr);
	else if (map == 1)
		err = i2c_smbus_read_byte_data(acc->i2c_client1, addr);
	else
		err = -EIO;
	if (err < 0)
		goto err;

	err = err & ~msk;
	value = value | err;

	if (map == 0)
		err = i2c_smbus_write_byte_data(acc->i2c_client, addr, value);
	else if (map == 1)
		err = i2c_smbus_write_byte_data(acc->i2c_client1, addr, value);
	else
		err = -EIO;
      err:
	bcmpmu_i2c_unlock(acc);
	return err;
}

static int bcmpmu_i2c_read_device_direct_bulk(struct bcmpmu *bcmpmu, int map,
					      int addr, unsigned int *val,
					      int len)
{
	int err;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	u8 *uval = (u8 *) val;
	int i;

	if (addr + len > acc->pagesize)
		return -ENODEV;

	bcmpmu_i2c_lock(acc);
	if (map == 0)
		err =
		    i2c_smbus_read_i2c_block_data(acc->i2c_client, addr, len,
						  uval);
	else if (map == 1)
		err =
		    i2c_smbus_read_i2c_block_data(acc->i2c_client1, addr, len,
						  uval);
	else
		err = -EIO;
	bcmpmu_i2c_unlock(acc);

	for (i = len; i > 0; i--)
		val[i - 1] = (unsigned int)uval[i - 1];

	if (err < 0)
		return err;
	return 0;
}

static int bcmpmu_i2c_write_device_direct_bulk(struct bcmpmu *bcmpmu, int map,
					       int addr, unsigned int *val,
					       int len)
{
	int err;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	u8 *uval = (u8 *) val;
	int i;

	if (addr + len > acc->pagesize)
		return -ENODEV;

	for (i = 0; i < len; i++)
		uval[i] = (u8) val[i];

	bcmpmu_i2c_lock(acc);
	if (map == 0)
		err =
		    i2c_smbus_write_i2c_block_data(acc->i2c_client, addr, len,
						   uval);
	else if (map == 1)
		err =
		    i2c_smbus_write_i2c_block_data(acc->i2c_client1, addr, len,
						   uval);
	else
		err = -EIO;
	bcmpmu_i2c_unlock(acc);

	if (err < 0)
		return err;
	return 0;
}
#endif /* #if !defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER) */

#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
/**
 *function must be called with i2c_mutex locked
 */
static int i2c_try_read_write(struct bcmpmu *bcmpmu, bool check_fifo,
			int trans_type, u8 reg_addr, u8 slave_id, u8 *value)
{
	struct bcmpmu_reg_map map = {};
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	int err = 0;
	int tries = PWRMGR_I2C_RDWR_MAX_TRIES;
	u8 clientaddr = 0;
	u8 temp;

	pr_debug("%s: trans %d addr %x, slave %x\n", __func__,
		 trans_type, reg_addr, slave_id);
	if (check_fifo) {

		if (trans_type == I2C_TRANS_READ)
			map = bcmpmu->regmap[PMU_REG_BUS_STATUS_READ_FIFO];
		else
			map = bcmpmu->regmap[PMU_REG_BUS_STATUS_WRITE_FIFO];

		if ((map.addr == 0) && (map.mask == 0)) {
			err = -ENXIO;
			goto err_out;
		}
		if (map.map == 0)
			clientaddr = acc->i2c_client->addr;
		else
			clientaddr = acc->i2c_client1->addr;
	}

	switch (trans_type) {
	case I2C_TRANS_READ:
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

			if (check_fifo &&
					last_i2c_trans == I2C_TRANS_WRITE) {
				err = pwr_mgr_pmu_reg_read(map.addr,
							   clientaddr, &temp);
				if (err == 0) {
					if (!(temp & map.mask)) {
						/**
						 * OK: Now we can try
						 *to read the register
						 */
						err =
						    pwr_mgr_pmu_reg_read
						    (reg_addr, slave_id, value);
						if (err == 0) {
							last_i2c_trans =
							    I2C_TRANS_READ;
							break;
						}
					}
				}
			} else {
				err = pwr_mgr_pmu_reg_read(reg_addr,
							   slave_id, value);
				if (err == 0) {
					last_i2c_trans = I2C_TRANS_READ;
					break;
				}
			}
			udelay(PWRMGR_I2C_RETRY_DELAY_US);
		}
		if (tries <= 0) {
			pr_info("ERROR: I2C SW SEQ Max Tries\n");
			err = -EAGAIN;
			break;
		}
		break;
	case I2C_TRANS_WRITE:
		while (tries--) {
			if (check_fifo &&
					last_i2c_trans == I2C_TRANS_WRITE) {
				err = pwr_mgr_pmu_reg_read(map.addr,
							   clientaddr, &temp);
				if (err == 0) {
					if (!(temp & map.mask)) {
						err =
						    pwr_mgr_pmu_reg_write
						    (reg_addr, slave_id,
						     *value);
						if (err == 0) {
							last_i2c_trans =
							    I2C_TRANS_WRITE;
							break;
						}
					}
				}
				udelay(PWRMGR_I2C_RETRY_DELAY_US);
			} else {
				err = pwr_mgr_pmu_reg_write(reg_addr, slave_id,
							    *value);
				if (err == 0) {
					last_i2c_trans = I2C_TRANS_WRITE;
					break;
				}
			}
		}
		if (tries <= 0) {
			err = -EAGAIN;
			pr_info("ERROR: I2C SW SEQ Write MAX Tries\n");
			break;
		}
		break;
	default:
		err = -EINVAL;
		break;
	}
      err_out:
	return err;
}
static int bcmpmu_i2c_pwrmgr_read(struct bcmpmu *bcmpmu, int reg,
				  unsigned int *val, unsigned int msk)
{
	struct bcmpmu_reg_map map;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	int err = 0;
	u8 temp;

	if (reg >= PMU_REG_MAX)
		return -ENODEV;
	map = bcmpmu->regmap[reg];
	if ((map.addr == 0) && (map.mask == 0))
		return -ENXIO;

	bcmpmu_i2c_lock(acc);

	pr_debug("%s\n", __func__);

	if (map.map == 0)
		err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_READ, map.addr,
					 acc->i2c_client->addr, &temp);
	else if (map.map == 1)
		err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_READ, map.addr,
					 acc->i2c_client1->addr, &temp);
	else
		err = -EIO;

	if (err < 0)
		goto out_unlock;

	temp &= msk;
	temp &= map.mask;
	*val = temp;
      out_unlock:
	bcmpmu_i2c_unlock(acc);
	return err;
}

static int bcmpmu_i2c_pwrmgr_write(struct bcmpmu *bcmpmu, int reg,
				   unsigned int value, unsigned int msk)
{
	struct bcmpmu_reg_map map;
	int err = 0;
	u8 temp;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;

	if (reg >= PMU_REG_MAX)
		return -ENODEV;

	map = bcmpmu->regmap[reg];
	if ((map.addr == 0) && (map.mask == 0))
		return -ENXIO;

	bcmpmu_i2c_lock(acc);

	pr_debug("%s\n", __func__);

	if (map.map == 0)
		err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_READ, map.addr,
					 acc->i2c_client->addr, &temp);
	else if (map.map == 1)
		err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_READ, map.addr,
					 acc->i2c_client1->addr, &temp);
	else
		err = -EIO;

	if (err < 0)
		goto out_unlock;

	temp &= ~msk;
	temp &= ~map.mask;
	temp |= (u8) value;

	if (map.map == 0)
		err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_WRITE,
								map.addr,
					 acc->i2c_client->addr, &temp);
	else if (map.map == 1)
		err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_WRITE,
								map.addr,
					 acc->i2c_client1->addr, &temp);
	else
		err = -ENODEV;
      out_unlock:
	bcmpmu_i2c_unlock(acc);
	return err;
}

static int bcmpmu_i2c_pwrmgr_read_direct(struct bcmpmu *bcmpmu,
					 int map, int addr,
					 unsigned int *val, unsigned int msk)
{
	int err = 0;
	u8 temp;
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	if ((addr == 0) && (msk == 0))
		return -ENODEV;

	bcmpmu_i2c_lock(acc);

	pr_debug("%s\n", __func__);

	if (map == 0)
		err = i2c_try_read_write(bcmpmu, false, I2C_TRANS_READ,
					 addr, acc->i2c_client->addr, &temp);
	else if (map == 1)
		err = i2c_try_read_write(bcmpmu, false, I2C_TRANS_READ,
					 addr, acc->i2c_client1->addr, &temp);
	else
		err = -EIO;

	if (err < 0)
		goto out_unlock;

	temp &= msk;
	*val = temp;
      out_unlock:
	bcmpmu_i2c_unlock(acc);
	return err;

}

static int bcmpmu_i2c_pwrmgr_write_direct(struct bcmpmu *bcmpmu, int map,
					  int addr, unsigned int val,
					  unsigned int msk)
{
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	int err = 0;
	u8 temp;
	u8 value = (u8) val;

	if ((addr == 0) && (msk == 0))
		return -ENODEV;

	bcmpmu_i2c_lock(acc);

	pr_debug("%s\n", __func__);

	if (map == 0)
		err = i2c_try_read_write(bcmpmu, false, I2C_TRANS_READ, addr,
					 acc->i2c_client->addr, &temp);
	else if (map == 1)
		err = i2c_try_read_write(bcmpmu, false, I2C_TRANS_READ, addr,
					 acc->i2c_client1->addr, &temp);
	else
		err = -EIO;

	if (err < 0)
		goto out_unlock;

	temp &= ~msk;
	temp |= (u8) value;

	if (map == 0)
		err = i2c_try_read_write(bcmpmu, false, I2C_TRANS_WRITE, addr,
					 acc->i2c_client->addr, &temp);
	else if (map == 1)
		err = i2c_try_read_write(bcmpmu, false, I2C_TRANS_WRITE, addr,
					 acc->i2c_client1->addr, &temp);
	else
		err = -EIO;
      out_unlock:
	bcmpmu_i2c_unlock(acc);
	return err;
}

static int bcmpmu_i2c_pwrmgr_read_direct_bulk(struct bcmpmu *bcmpmu,
					      int map, int addr,
					      unsigned int *val, int len)
{
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	int err = 0;
	int i;
	u8 temp;

	if (addr + len > acc->pagesize)
		return -ENODEV;

	if ((map != 0) && (map != 1))
		return -EIO;

	bcmpmu_i2c_lock(acc);
	pr_debug("%s\n", __func__);

	for (i = 0; i < len; i++) {
		if (map == 0)
			err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_READ,
						 addr + i,
						 acc->i2c_client->addr, &temp);
		else
			err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_READ,
						 addr + i,
						 acc->i2c_client1->addr, &temp);

		if (err < 0)
			break;
		val[i] = temp;
	}

	bcmpmu_i2c_unlock(acc);
	return err;
}

static int bcmpmu_i2c_pwrmgr_write_direct_bulk(struct bcmpmu *bcmpmu,
					       int map, int addr,
					       unsigned int *val, int len)
{
	struct bcmpmu_i2c *acc = (struct bcmpmu_i2c *)bcmpmu->accinfo;
	int err = 0;
	int i;
	u8 temp;

	if (addr + len > acc->pagesize)
		return -ENODEV;

	if ((map != 0) && (map != 1))
		return -EIO;

	bcmpmu_i2c_lock(acc);

	pr_debug("%s\n", __func__);

	for (i = 0; i < len; i++) {
		temp = val[i];

		if (map == 0)
			err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_WRITE,
						 addr + i,
						 acc->i2c_client->addr, &temp);
		else
			err = i2c_try_read_write(bcmpmu, true, I2C_TRANS_WRITE,
						 addr + i,
						 acc->i2c_client1->addr, &temp);
		if (err < 0)
			break;
	}

	bcmpmu_i2c_unlock(acc);
	return err;
}
#endif /* #if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER) */

int bcmpmu_i2c_set_dev_mode(struct bcmpmu *bcmpmu, int poll)
{
#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
	return pwr_mgr_set_i2c_mode(1);
#else
	return -EPERM;
#endif
}

static struct platform_device bcmpmu_core_device = {
	.name = "bcmpmu_core",
	.id = -1,
	.dev.platform_data = NULL,
};

static int bcmpmu_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct bcmpmu *bcmpmu;
	int ret = 0;
	struct bcmpmu_platform_data *pdata;
	struct i2c_client *clt;
	struct i2c_adapter *adp;
	struct bcmpmu_i2c *bcmpmu_i2c;

	pdata = (struct bcmpmu_platform_data *)i2c->dev.platform_data;

	printk(KERN_INFO "%s called\n", __func__);

	bcmpmu = kzalloc(sizeof(struct bcmpmu), GFP_KERNEL);
	if (bcmpmu == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		kfree(i2c);
		ret = -ENOMEM;
		goto err;
	}

	bcmpmu_i2c = kzalloc(sizeof(struct bcmpmu_i2c), GFP_KERNEL);
	if (bcmpmu_i2c == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	i2c_set_clientdata(i2c, bcmpmu);
	bcmpmu->dev = &i2c->dev;
	bcmpmu_i2c->i2c_client = i2c;

	adp = i2c_get_adapter(pdata->i2c_adapter_id);
	clt = i2c_new_dummy(adp, pdata->i2c_board_info_map1->addr);
	if (!clt)
		printk(KERN_ERR "%s: add new device for map1 failed\n",
		       __func__);

	clt->dev.platform_data = pdata;
	bcmpmu_i2c->i2c_client1 = clt;
	mutex_init(&bcmpmu_i2c->i2c_mutex);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcmpmu_i2c->i2c_lock, WAKE_LOCK_SUSPEND,
		"bcmpmu_i2c");
	bcmpmu_i2c->ref_count = 0;
#endif

#if defined(CONFIG_MFD_BCM_PWRMGR_SW_SEQUENCER)
	/**
	 * Initialize the power manager sequencer
	 */
	mach_init_sequencer();
	bcmpmu->read_dev = bcmpmu_i2c_pwrmgr_read;
	bcmpmu->write_dev = bcmpmu_i2c_pwrmgr_write;
	bcmpmu->read_dev_drct = bcmpmu_i2c_pwrmgr_read_direct;
	bcmpmu->write_dev_drct = bcmpmu_i2c_pwrmgr_write_direct;
	bcmpmu->read_dev_bulk = bcmpmu_i2c_pwrmgr_read_direct_bulk;
	bcmpmu->write_dev_bulk = bcmpmu_i2c_pwrmgr_write_direct_bulk;
	bcmpmu->set_dev_mode = bcmpmu_i2c_set_dev_mode;
	pr_info("%s:PWRMGR I2C Sequencer\n", __func__);
#else
	bcmpmu->read_dev = bcmpmu_i2c_read_device;
	bcmpmu->write_dev = bcmpmu_i2c_write_device;
	bcmpmu->read_dev_drct = bcmpmu_i2c_read_device_direct;
	bcmpmu->write_dev_drct = bcmpmu_i2c_write_device_direct;
	bcmpmu->read_dev_bulk = bcmpmu_i2c_read_device_direct_bulk;
	bcmpmu->write_dev_bulk = bcmpmu_i2c_write_device_direct_bulk;
	bcmpmu->set_dev_mode = bcmpmu_i2c_set_dev_mode;
#endif
	bcmpmu->pdata = pdata;
	bcmpmu_i2c->pagesize = pdata->i2c_pagesize;

	bcmpmu->accinfo = bcmpmu_i2c;

	bcmpmu_core_device.dev.platform_data = bcmpmu;
	platform_device_register(&bcmpmu_core_device);

	return ret;

      err:
	kfree(bcmpmu->accinfo);
	kfree(bcmpmu);
	return ret;
}

static int bcmpmu_i2c_remove(struct i2c_client *i2c)
{
	struct bcmpmu *bcmpmu = i2c_get_clientdata(i2c);

	platform_device_unregister(&bcmpmu_core_device);
	kfree(bcmpmu->accinfo);
	kfree(bcmpmu);

	return 0;
}

static const struct i2c_device_id bcmpmu_i2c_id[] = {
	{"bcmpmu", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bcmpmu_i2c_id);

static struct i2c_driver bcmpmu_i2c_driver = {
	.driver = {
		   .name = "bcmpmu",
		   .owner = THIS_MODULE,
		   },
	.probe = bcmpmu_i2c_probe,
	.remove = bcmpmu_i2c_remove,
	.id_table = bcmpmu_i2c_id,
};

static int __init bcmpmu_i2c_init(void)
{
	return i2c_add_driver(&bcmpmu_i2c_driver);
}

/* init early so consumer devices can complete system boot */
subsys_initcall(bcmpmu_i2c_init);

static void __exit bcmpmu_i2c_exit(void)
{
	i2c_del_driver(&bcmpmu_i2c_driver);
}

module_exit(bcmpmu_i2c_exit);

MODULE_DESCRIPTION("I2C support for BCM590XX PMIC");
MODULE_LICENSE("GPL");

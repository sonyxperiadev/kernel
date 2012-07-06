/*
 * linux/drivers/power/bq27520_battery.c
 *
 * TI BQ27520 Fuel Gauge interface
 *
 * Copyright (C) 2010-2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Authors: James Jacobsson <james.jacobsson@sonyericsson.com>
 *          Imre Sunyi <imre.sunyi@sonyericsson.com>
 *          Hiroyuki Namba <Hiroyuki.Namba@sonyericsson.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/err.h>

#include <linux/i2c/bq27520_battery.h>

#include <asm/mach-types.h>

#define REG_CMD_CONTROL 0x00
#define REG_CMD_TEMPERATURE 0x06
#define REG_CMD_VOLTAGE 0x08
#define REG_CMD_FLAGS 0x0A
#define REG_CMD_SOC 0x2C
#define REG_CMD_AVG_CURRENT 0x14
#define REG_CMD_HEALTH 0x28
#define REG_CMD_HEALTH_STATE 0x29
#define REG_CMD_INS_CURRENT 0x30
#define REG_CMD_DESIGNEDCAPACITY 0x3C
#define REG_CMD_APPSTATUS 0x6A

#define REG_EXT_CMD_DATA_FLASH_CLASS 0x3E
#define REG_EXT_CMD_DATA_FLASH_BLOCK 0x3F
#define REG_EXT_CMD_BLOCK_DATA 0x40
#define REG_EXT_CMD_BLOCK_DATA_CONTROL 0x61
#define REG_EXT_CMD_DATA_FLASH_CLASS_MF 0x39 /* Subclass ID 57 */
#define REG_EXT_CMD_DATA_FLASH_CLASS_IT 0x52 /* Subclass ID 82 */

#define SUB_CMD_NULL 0x0000
#define SUB_CMD_FW_VERSION 0x0002
#define SUB_CMD_HW_VERSION 0x0003
#define SUB_CMD_BAT_INSERT 0x000D
#define SUB_CMD_BAT_REMOVE 0x000E
#define SUB_CMD_ENTER_SEALED 0x0020
#define SUB_CMD_IT_ENABLE 0x0021
#define SUB_CMD_CHOOSE_A 0x0024
#define SUB_CMD_CHOOSE_B 0x0025
#define SUB_CMD_RESET 0x0041
#define SUB_CMD_ENTER_CLEAR_SEAL 0x0414
#define SUB_CMD_CLEAR_SEALED 0x3672
#define SUB_CMD_CLEAR_FULL_ACCESS_SEALED 0xFFFF

#define WAIT_ON_READ_SUB_CMD_US 500
/* According to datasheet this is the maximum time to program a
 * word in data flash memory.
*/
#define MAX_WORD_PROGRAMMING_TIME_MS 2
/* According to datasheet maximum i2c clock stretch is 144 ms */
#define MAX_I2C_CLOCK_STRETCH_MS 150

#define UNKNOWN_PROJ_NAME "xxxx"

#define FC_MASK 0x0200
#define BAT_DET_MASK 0x0008
#define SYSDOWN_MASK 0x2
#define LU_PROF_MASK 0x1
#define INIT_COMP_MASK 0x80
#define SEALED_MASK 0x6000
#define QEN_MASK 0x01

#define RETRY_MAX 5
#define FAKE_CAPACITY_BATT_ALIEN 50

#define READ_FC_TIMER 10

#define TEMP_WRITE_TIMEOUT_MS 2000
#define A_TEMP_COEF_DEFINE 2731

#define BITMASK_16 0xffff

#define USB_CHG  0x01
#define WALL_CHG 0x02
#define CRADLE_CHG 0x04

#define SOH_VALUE_READY 0x3

#define POLL_QEN_TIMEOUT_MS 2000
#define POLL_QEN_PERIOD_MS 100

/* CONTANTS / MACROS */
#define I2C_RETRY_MAX 3			/* retry 3 times */
#define FUEL_GAUGE_ROM_SLAVE_ADDR 0x0B	/* 7-bit slave addr, ROM mode */
/* REGISTER ADDRESS */
#define ENTER_ROM_REG 0x00
#define ENTER_ROM_DATA 0x0F00
#define LEAVE_ROM_REG1 0x00
#define LEAVE_ROM_DATA1 0x0F
#define LEAVE_ROM_REG2 0x64
#define LEAVE_ROM_DATA2 0x0F
#define LEAVE_ROM_REG3 0x65
#define LEAVE_ROM_DATA3 0x00
#define LEAVE_ROM_DELAY_MS 4000

#define IT_ENABLE_DELAY_MS 500

#define GOLDEN_PROJ_NAME_LEN 4

#define READ_VOLTAGE_FILTER 4500

/* Fuelgauge force reset sequence */
#define GPIO_FG_CE 13
#define MAX_GPIO_FG_CE_MS 500 /* 500ms */
#define MAX_AFTER_DEVICE_RESET_MS 300 /* 300ms */

#define INVALID_BATT_VOLTAGE_RETRY_MAX 5
#define INVALID_BATT_VOLTAGE_LOWER_LIMIT 2000
#define INVALID_BATT_VOLTAGE_UPPER_LIMIT 4300

#define SHUTDOWN_WAKELOCK_TIMEOUT (HZ * 5) /* 5sec */

#define IS_CHG_CONNECTED(old_status, new_status) \
			(!old_status && new_status)
#define IS_CHG_DISCONNECTED(old_status, new_status) \
			(old_status && !new_status)

/* #define DEBUG_FS */

/* Parameter update support.
 * Should not be necessary since golden file has same setting.
 */
/* #define SUPPORT_PARAMETER_UPDATE */

#ifdef DEBUG_FS
struct override_value {
	u8 active;
	int value;
};
#endif

struct bq27520_golden_info {
	u16 fw_compatible_version;
	u16 golden_file_version;
	char project_name[GOLDEN_PROJ_NAME_LEN];
};

struct bq27520_capacity_scaling {
	bool enable;
	int capacity_to_scale[2];
	int disable_capacity_level;
	int scaled_capacity;
};

struct bq27520_data {
	struct power_supply bat_ps;
	struct i2c_client *clientp;
	struct i2c_client *rom_clientp;
	int curr_mv;
	int curr_capacity;
	int curr_capacity_level;
	int curr_current;
	struct bq27520_platform_data *pdata;
	struct work_struct ext_pwr_change_work;
	struct work_struct soc_int_work;
	struct work_struct init_work;
	struct delayed_work fc_work;
	struct workqueue_struct *wq;
	int current_avg;
	int impedance;
	int flags;
	int technology;
	int bat_temp;
	int control_status;
	int app_status;
	int chg_connected;
	int designed_capacity;
	int health;
	struct mutex lock;
	struct mutex int_lock;
	struct mutex data_flash_lock;
	struct mutex control_reg_lock;
	struct wake_lock wake_lock;
	int got_technology;
	int lipo_bat_max_volt;
	int lipo_bat_min_volt;
	char *battery_dev_name;
	char *set_batt_charged_dev_name;
	int started_worker;
	bool detect_fc_soon;
	int polling_lower_capacity;
	int polling_upper_capacity;
	u8 suspended;
	u8 resume_int;
	u8 resume_temp;
	s8 sealed;
	bool run_init_after_rom;
	struct bq27520_block_table *udatap;
	struct bq27520_capacity_scaling cap_scale;

#ifdef DEBUG_FS
	struct override_value bat_volt_debug;
	struct override_value bat_curr_debug;
	struct override_value bat_cap_debug;
	struct override_value bat_cap_lvl_debug;
#endif
};

static atomic_t bq27520_init_ok = ATOMIC_INIT(0);

static int get_supplier_data(struct device *dev, void *data);
static int bq27520_write_control(struct bq27520_data *bd, int subcmd);
static int bq27520_read_bat_flags(struct power_supply *bat_ps);
static bool bq27520_read_soc(struct bq27520_data *bd);
static void bq27520_device_reset(struct bq27520_data *bd);

static int calculate_scaled_capacity(struct bq27520_data *bd)
{
	struct bq27520_capacity_scaling *cs = &bd->cap_scale;
	int capacity = bd->curr_capacity;

	if (!cs->enable)
		return capacity;

	/* Calculates the scaled capacity where remainder of the
	 * fraction is rounded to nearest integer.
	 */
	if (cs->capacity_to_scale[0] != cs->capacity_to_scale[1])
		capacity = min(100,
			       (bd->curr_capacity * cs->capacity_to_scale[0] +
				(cs->capacity_to_scale[1] >> 1)) /
			       cs->capacity_to_scale[1]);

	if (bd->chg_connected) {
		if (capacity < cs->disable_capacity_level) {
			cs->disable_capacity_level = capacity;
			dev_dbg(&bd->clientp->dev,
				"Cap to stop scale lowered %d%%\n",
				cs->disable_capacity_level);
		} else if (bd->curr_capacity_level !=
			   POWER_SUPPLY_CAPACITY_LEVEL_FULL) {
			if (bd->curr_capacity >= cs->disable_capacity_level) {
				dev_dbg(&bd->clientp->dev,
					"Disabling scaled capacity\n");
				cs->enable = false;
				capacity = bd->curr_capacity;
			} else {
				dev_dbg(&bd->clientp->dev,
					"Waiting in cap to level %d%%\n",
					bd->cap_scale.disable_capacity_level);
				capacity = cs->disable_capacity_level;
			}
		}
	}

	return capacity;
}

static int read_sysfs_interface(const char *pbuf, s32 *pvalue, u8 base)
{
	long long val;
	int rc;

	rc = strict_strtoll(pbuf, base, &val);
	if (!rc)
		*pvalue = (s32)val;

	return rc;
}

#ifdef DEBUG_FS
static ssize_t store_voltage(struct device *pdev, struct device_attribute *attr,
			     const char *pbuf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int rc = count;
	s32 mv;

	if (!read_sysfs_interface(pbuf, &mv, 10) &&
	    mv >= -1 && mv <= INT_MAX) {
		mutex_lock(&bd->lock);

		bd->bat_volt_debug.active = 0;

		if (mv >= 0) {
			bd->bat_volt_debug.active = 1;
			bd->bat_volt_debug.value = mv;
		}

		mutex_unlock(&bd->lock);

		power_supply_changed(&bd->bat_ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs set_voltage. "
		       "Expect [-1..%d]. -1 releases the debug value\n",
			INT_MAX);
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_current(struct device *pdev, struct device_attribute *attr,
			     const char *pbuf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int rc = count;
	s32 curr;

	if (!read_sysfs_interface(pbuf, &curr, 10) &&
	    curr >= -4001 && curr <= INT_MAX) {
		mutex_lock(&bd->lock);

		bd->bat_curr_debug.active = 0;

		if (curr >= -4000) {
			bd->bat_curr_debug.active = 1;
			bd->bat_curr_debug.value = curr;
		}

		mutex_unlock(&bd->lock);

		power_supply_changed(&bd->bat_ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs set_current. "
		       "Expect [-4001..%d]. -4001 releases the debug value\n",
			INT_MAX);
		rc = -EINVAL;
	}

	return rc;
}

static ssize_t store_capacity(struct device *pdev,
			      struct device_attribute *attr, const char *pbuf,
			      size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int rc = count;
	s32 cap;

	if (!read_sysfs_interface(pbuf, &cap, 10) &&
	    cap >= -1 && cap <= 100) {
		mutex_lock(&bd->lock);

		bd->bat_cap_debug.active = 0;

		if (cap >= 0) {
			bd->bat_cap_debug.active = 1;
			bd->bat_cap_debug.value = cap;
		}

		mutex_unlock(&bd->lock);

		power_supply_changed(&bd->bat_ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs set_capacity. "
			"Expect [-1..100]. -1 releases the debug value\n");
		rc = -EINVAL;
	}

	return rc;
}

static ssize_t store_capacity_level(struct device *pdev,
			      struct device_attribute *attr, const char *pbuf,
			      size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int rc = count;
	s32 lvl;

	if (!read_sysfs_interface(pbuf, &lvl, 10) &&
	    lvl >= -1 && lvl <= POWER_SUPPLY_CAPACITY_LEVEL_FULL) {
		mutex_lock(&bd->lock);

		bd->bat_cap_lvl_debug.active = 0;

		if (lvl >= 0) {
			bd->bat_cap_lvl_debug.active = 1;
			bd->bat_cap_lvl_debug.value = lvl;
		}

		mutex_unlock(&bd->lock);

		power_supply_changed(&bd->bat_ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs set_capacity_level. "
		       "Expect [-1..%u]. -1 releases the debug value\n",
			POWER_SUPPLY_CAPACITY_LEVEL_FULL);
		rc = -EINVAL;
	}

	return rc;
}
#endif /* DEBUG_FS */

static ssize_t show_capacity(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int capacity;

	/* Might end up here during ROM mode.
	 * Android shutsdown at 0% capacity.
	 * Prevent that by never reporting 0.
	 */
	if (bd->rom_clientp || bd->run_init_after_rom) {
		if (!bd->cap_scale.scaled_capacity)
			capacity = 1;
		else
			capacity = bd->cap_scale.scaled_capacity;

		return scnprintf(buf, PAGE_SIZE, "%d\n", capacity);
	} else if (!atomic_read(&bq27520_init_ok)) {
		return -EBUSY;
	}

	mutex_lock(&bd->lock);
	capacity = bd->cap_scale.scaled_capacity;

#ifdef DEBUG
	if (capacity != bd->curr_capacity)
		dev_dbg(&bd->clientp->dev,
			"Report scaled cap %d (origin %d)\n",
			capacity, bd->curr_capacity);
#endif

#ifdef DEBUG_FS
	if (bd->bat_cap_debug.active)
		capacity = bd->bat_cap_debug.value;
#endif
	mutex_unlock(&bd->lock);
	return scnprintf(buf, PAGE_SIZE, "%d\n", capacity);
}

/* Wrapper to i2c_smbus_read_i2c_block_data().
 * This one makes sure all 'length' bytes are read.
 */
static s32 safe_i2c_smbus_read_i2c_block_data(struct i2c_client *client,
					      u8 command, u8 length, u8 *values)
{
	s32 rc;
	u8 offs = 0;
	u8 retry = 0;
	u8 size;

	while (offs < length && retry <= I2C_RETRY_MAX) {
		if ((length - offs) > I2C_SMBUS_BLOCK_MAX)
			size = I2C_SMBUS_BLOCK_MAX;
		else
			size = length - offs;

		rc = i2c_smbus_read_i2c_block_data(client,
						   (u8)(command + offs),
						   size, values + offs);
		if (rc <= 0) {
			retry++;
			/* Wait:
			 * word-read-time * bytes-to-read / sizeof(word)
			 */
			msleep(MAX_WORD_PROGRAMMING_TIME_MS * retry * size / 2);
		} else {
			retry = 0;
			offs += rc;
		}
	}

	return (retry > I2C_RETRY_MAX) ? -EPERM : offs;
}

static int bq27520_get_control_status(struct bq27520_data *bd)
{
	int rc;

	mutex_lock(&bd->control_reg_lock);

	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					SUB_CMD_NULL);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed write control reg. rc=%d\n", rc);
		goto exit;
	}

	usleep(WAIT_ON_READ_SUB_CMD_US);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_CONTROL);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed read control reg. rc=%d\n", rc);
		goto exit;
	}
	bd->control_status = rc;
	dev_dbg(&bd->clientp->dev, "%s(): CONTROL_REG:0x%x\n", __func__, rc);

exit:
	mutex_unlock(&bd->control_reg_lock);
	return (rc < 0) ? rc : 0;
}

static bool bq27520_check_if_sealed(struct bq27520_data *bd)
{
	(void)bq27520_get_control_status(bd);
	bd->sealed = !!(bd->control_status & SEALED_MASK);

	return bd->sealed;
}

static int bq27520_unseal(struct bq27520_data *bd)
{
	int rc;
	int last_cmd = 0;

	mutex_lock(&bd->control_reg_lock);

	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					   SUB_CMD_ENTER_CLEAR_SEAL);
	if (rc < 0) {
		last_cmd = SUB_CMD_ENTER_CLEAR_SEAL;
		goto unseal_exit;
	}

	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					   SUB_CMD_CLEAR_SEALED);
	if (rc < 0) {
		last_cmd = SUB_CMD_CLEAR_SEALED;
		goto unseal_exit;
	}

	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					   SUB_CMD_CLEAR_FULL_ACCESS_SEALED);
	if (rc < 0) {
		last_cmd = SUB_CMD_CLEAR_FULL_ACCESS_SEALED;
		goto unseal_exit;
	}

	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					   SUB_CMD_CLEAR_FULL_ACCESS_SEALED);
	if (rc < 0) {
		last_cmd = SUB_CMD_CLEAR_FULL_ACCESS_SEALED;
		goto unseal_exit;
	}
	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	mutex_unlock(&bd->control_reg_lock);

	bd->sealed = 0;

	return rc;

unseal_exit:
	dev_err(&bd->clientp->dev, "Failed write unseal command :%#x\n",
			last_cmd);
	bd->sealed = -1;
	mutex_unlock(&bd->control_reg_lock);
	return rc;
}


static int bq27520_seal(struct bq27520_data *bd)
{
	int rc = 0;

#ifdef CONFIG_BATTERY_BQ27520_SEAL_FW
	mutex_lock(&bd->control_reg_lock);
	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					   SUB_CMD_ENTER_SEALED);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed enter sealed mode. rc=%d\n", rc);
		goto seal_exit;
	}
	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	mutex_unlock(&bd->control_reg_lock);

	if (bq27520_check_if_sealed(bd))
		dev_info(&bd->clientp->dev, "Seal firmware successful\n");
	else
		dev_info(&bd->clientp->dev, "Seal firmware failed\n");

	return rc;
seal_exit:
	bd->sealed = -1;
	mutex_unlock(&bd->control_reg_lock);
#endif
	return rc;
}

static int bq27520_get_fw_version(struct bq27520_data *bd,
				  u16 *fw_ver)
{
	s32 rc;

	mutex_lock(&bd->control_reg_lock);

	rc = i2c_smbus_write_word_data(bd->clientp,
					   REG_CMD_CONTROL,
					   SUB_CMD_FW_VERSION);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed write FW version command. rc=%d\n", rc);
		goto exit;
	}

	usleep(WAIT_ON_READ_SUB_CMD_US);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_CONTROL);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed read FW version. rc=%d\n", rc);
		goto exit;
	}
	*fw_ver = rc;

	dev_dbg(&bd->clientp->dev, "FW version=0x%x\n", rc);
exit:
	mutex_unlock(&bd->control_reg_lock);
	return (rc < 0) ? rc : 0;
}

static int bq27520_setup_to_read_df_class_block(struct bq27520_data *bd,
						u8 class, u8 block)
{
	s32 rc;

	rc = i2c_smbus_write_byte_data(bd->clientp,
				       REG_EXT_CMD_BLOCK_DATA_CONTROL,
				       0x00);
	if (!rc) {
		msleep(MAX_I2C_CLOCK_STRETCH_MS);
		rc = i2c_smbus_write_byte_data(bd->clientp,
					       REG_EXT_CMD_DATA_FLASH_CLASS,
					       class);
	}

	if (!rc) {
		msleep(MAX_I2C_CLOCK_STRETCH_MS);
		rc = i2c_smbus_write_byte_data(bd->clientp,
					       REG_EXT_CMD_DATA_FLASH_BLOCK,
					       block);
	}

	if (!rc)
		msleep(MAX_I2C_CLOCK_STRETCH_MS);

	return rc;
}

static int bq27520_get_golden_info(struct bq27520_data *bd,
				   struct bq27520_golden_info *gi)
{
	s32 rc;

	mutex_lock(&bd->data_flash_lock);

	/* Access Manufacturer Info block A.
	 * Two methods access depending if UNSEALED or SEALED
	 */
	if (bq27520_check_if_sealed(bd)) {
		dev_dbg(&bd->clientp->dev, "%s(): SEALED\n", __func__);
		rc = i2c_smbus_write_byte_data(bd->clientp,
					       REG_EXT_CMD_DATA_FLASH_BLOCK,
					       0x01);
		msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	} else {
		dev_dbg(&bd->clientp->dev, "%s(): UNSEALED\n", __func__);
		rc = bq27520_setup_to_read_df_class_block(bd,
				  REG_EXT_CMD_DATA_FLASH_CLASS_MF, 0x00);
	}

	if (!rc) {
		rc = safe_i2c_smbus_read_i2c_block_data(bd->clientp,
							REG_EXT_CMD_BLOCK_DATA,
							sizeof(*gi), (u8 *)gi);
		/* Watch out. 'rc' here holds the number of bytes read. */
		if (rc == sizeof(*gi)) {
#ifdef DEBUG
			unsigned int i;
			for (i = 0; i < sizeof(*gi); i++)
				dev_dbg(&bd->clientp->dev,
					"Block A[%u]: 0x%.2x\n",
					i, *((unsigned char *)gi + i));
#endif
			rc = 0;
			/* Version is stored in big endian format in register.
			 * Convert it to little endian here.
			 */
			gi->fw_compatible_version =
				ror16(gi->fw_compatible_version, 8);
			gi->golden_file_version =
				ror16(gi->golden_file_version, 8);
		} else if (rc >= 0) {
			rc = -ENOMSG;
		}
	}

	mutex_unlock(&bd->data_flash_lock);

	if (rc)
		dev_err(&bd->clientp->dev,
			"Failed get golden info. rc=%d\n", rc);

	return rc;
}

static int bq27520_read_it_enabled(struct bq27520_data *bd)
{
	s32 rc;

	if (bq27520_check_if_sealed(bd)) {
		rc = bq27520_unseal(bd);

		if (rc) {
			dev_err(&bd->clientp->dev,
			"Failed unseal when checking IT Enable. rc=%d\n", rc);
			return rc;
		}
	}

	mutex_lock(&bd->data_flash_lock);
	rc = bq27520_setup_to_read_df_class_block(bd,
				  REG_EXT_CMD_DATA_FLASH_CLASS_IT, 0x00);

	if (!rc)
		rc = i2c_smbus_read_byte_data(bd->clientp,
					      REG_EXT_CMD_BLOCK_DATA);

	mutex_unlock(&bd->data_flash_lock);


	return rc;
}

static int bq27520_make_sure_bat_is_removed(struct bq27520_data *bd)
{
	const unsigned int poll_cnt = POLL_QEN_TIMEOUT_MS / POLL_QEN_PERIOD_MS;
	int rc = bq27520_read_bat_flags(&bd->bat_ps);

	if (!rc && bd->flags & BAT_DET_MASK) {
		unsigned int i;
		dev_info(&bd->clientp->dev, "Writing BAT_REMOVE\n");
		bq27520_write_control(bd, SUB_CMD_BAT_REMOVE);

		for (i = 0; i < poll_cnt; i++) {
			msleep(MAX_WORD_PROGRAMMING_TIME_MS);
			rc = bq27520_read_bat_flags(&bd->bat_ps);
			if (!rc && bd->flags & BAT_DET_MASK)
				msleep(POLL_QEN_PERIOD_MS);
			else
				break;
		}
		if (i == poll_cnt && (bd->flags & BAT_DET_MASK))
			rc = -ETIME;
	}

	return rc;
}

static int bq27520_make_sure_it_enabled_is_set(struct bq27520_data *bd)
{
	int rc = bq27520_read_it_enabled(bd);
	if (!rc) {
		dev_info(&bd->clientp->dev, "IT Enable not set. Try to set.\n");

		/* Recommendation from TI:
		 * Battery must be removed before IT_ENABLED is set
		 */
		rc = bq27520_make_sure_bat_is_removed(bd);
		if (!rc) {
			rc = bq27520_write_control(bd, SUB_CMD_IT_ENABLE);
			if (!rc) {
				msleep(IT_ENABLE_DELAY_MS);
				rc = bq27520_read_it_enabled(bd);
			}
		}
	}

	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"IT Enable check failed. rc=%d\n", rc);
	} else if (!rc) {
		dev_err(&bd->clientp->dev, "IT Enable failed to set.\n");
		rc = -EFAULT;
	} else {
		dev_info(&bd->clientp->dev, "IT Enable confirmed to be set.\n");
		rc = 0;
	}

	return rc;
}

static int bq27520_last_step_production(struct bq27520_data *bd)
{
	int rc;
	/* Document says to do following in last production step:
	 * 1. Send RESET
	 * 2. Set IT_ENABLE
	 * 3. Set SEALED mode
	 *
	 * According to TI:
	 * Reset is made when new firmware is starting to execute.
	 * So no need to reset again.
	 */

	/* SEMC golden file has IT Enabled by default from some revision.
	 * For backward compatibility read out if it is set and if not, then
	 * set it here.
	 *
	 * Wait before reading out since when setting it takes some
	 * time for the gauge to write it internally in flash.
	 */
	msleep(IT_ENABLE_DELAY_MS);
	rc = bq27520_make_sure_it_enabled_is_set(bd);

	/* Do not write SEALED command as descibed in document.
	 * We can not change battery type in SEALED mode.
	 */
	return rc;
}

static int bq27520_recover_rom_mode(struct i2c_client *clientp,
				    struct i2c_client **rom_clientp)
{
	s32 rc;

	*rom_clientp = i2c_new_dummy(clientp->adapter,
				     FUEL_GAUGE_ROM_SLAVE_ADDR);
	if (!*rom_clientp) {
		dev_err(&clientp->dev, "Failed creating ROM i2c access\n");
		return -EIO;
	}

	/* Check if i2c responds */
	rc = i2c_smbus_read_byte_data(*rom_clientp, 0x66);
	if (rc < 0) {
		/* No, not in ROM mode */
		i2c_unregister_device(*rom_clientp);
		*rom_clientp = NULL;
	} else {
		/* Yes, we are in ROM mode */
		rc = 0;
	}

	return rc;
}

static int bq27520_enter_rom_mode(struct bq27520_data *bd)
{
	s32 rc;

	if (bd->rom_clientp)
		return -EALREADY;

	dev_info(&bd->clientp->dev, "Enter ROM mode\n");

	bd->rom_clientp = i2c_new_dummy(bd->clientp->adapter,
					FUEL_GAUGE_ROM_SLAVE_ADDR);
	if (!bd->rom_clientp) {
		dev_err(&bd->clientp->dev, "Failed creating ROM i2c access\n");
		return -EIO;
	}
	rc = i2c_smbus_write_word_data(bd->clientp,
				       ENTER_ROM_REG, ENTER_ROM_DATA);
	if (rc < 0) {
		dev_err(&bd->clientp->dev, "Fail enter ROM mode. rc=%d\n", rc);
		i2c_unregister_device(bd->rom_clientp);
		bd->rom_clientp = NULL;
	}

	return rc;
}

static int bq27520_exit_rom_mode(struct bq27520_data *bd)
{
	s32 rc;

	if (!bd->rom_clientp)
		return -EFAULT;

	dev_info(&bd->clientp->dev, "Leave ROM mode\n");

	rc = i2c_smbus_write_byte_data(bd->rom_clientp,
				       LEAVE_ROM_REG1, LEAVE_ROM_DATA1);
	if (rc < 0) {
		dev_err(&bd->clientp->dev, "Fail exit ROM mode. rc=%d\n", rc);
		goto unregister_rom;
	}
	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	rc = i2c_smbus_write_byte_data(bd->rom_clientp,
				       LEAVE_ROM_REG2, LEAVE_ROM_DATA2);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"%s: Send Checksum for LSB rc=%d\n",
			__func__, rc);
		goto unregister_rom;
	}
	msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	rc = i2c_smbus_write_byte_data(bd->rom_clientp,
				       LEAVE_ROM_REG3, LEAVE_ROM_DATA3);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"%s: Send Checksum for MSB rc=%d\n",
			__func__, rc);
	}

unregister_rom:
	i2c_unregister_device(bd->rom_clientp);
	bd->rom_clientp = NULL;

	msleep(LEAVE_ROM_DELAY_MS);
	return rc;
}

static ssize_t bq27520_fg_data_write(struct file *file, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct power_supply *psy = dev_get_drvdata(dev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	u8 retry = 0;
	size_t offs = 0;
	u8 length;
	s32 rc;

	dev_dbg(&bd->rom_clientp->dev, "%s(): pos 0x%x, size %u\n",
		__func__, (unsigned int)pos, size);

	if ((pos + size) > (0xFF + 1)) {
		dev_err(&bd->rom_clientp->dev,
			"Trying to write outside register map\n");
		return -EMSGSIZE;
	}

	while (offs < size && retry <= I2C_RETRY_MAX) {
		if ((size - offs) > I2C_SMBUS_BLOCK_MAX)
			length = I2C_SMBUS_BLOCK_MAX;
		else
			length = size - offs;

		rc = i2c_smbus_write_i2c_block_data(bd->rom_clientp,
						    (u8)(pos + offs),
						    length, buf + offs);
		if (rc < 0) {
			retry++;
			/* Wait:
			 * word-write-time * bytes-to-send / sizeof(word)
			 */
			msleep(MAX_WORD_PROGRAMMING_TIME_MS * retry *
			       length / 2);
		} else {
			retry = 0;
			offs += length;
		}
	}

	return (retry > I2C_RETRY_MAX) ? -EPERM : offs;
}

static ssize_t bq27520_fg_data_read(struct file *file, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t pos, size_t size)

{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct power_supply *psy = dev_get_drvdata(dev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);

	dev_dbg(&bd->rom_clientp->dev, "%s(): pos 0x%x, size %u\n",
		__func__, (unsigned int)pos, size);

	if ((pos + size) > (0xFF + 1)) {
		dev_err(&bd->rom_clientp->dev,
			"Trying to read outside register map\n");
		return -EMSGSIZE;
	}

	return safe_i2c_smbus_read_i2c_block_data(bd->rom_clientp, pos, size,
						  buf);
}

static struct bin_attribute bq27520_fg_data = {
	.attr = {
		.name = "fg_data",
		.mode = S_IRUSR|S_IWUSR,
	},
	.size = 256,
	.read = bq27520_fg_data_read,
	.write = bq27520_fg_data_write,
};

static ssize_t store_fg_cmd(struct device *pdev,
			    struct device_attribute *attr, const char *pbuf,
			    size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int rc;
	char cmdstr[10];

	rc = sscanf(pbuf, "%9s", cmdstr);
	if (rc != 1) {
		dev_dbg(&bd->clientp->dev, "%s: cmd read error rc=%d\n",
			__func__, rc);
			return -EBADMSG;
	}
	cmdstr[sizeof(cmdstr) - 1] = '\0';

	mutex_lock(&bd->lock);

	dev_dbg(&bd->clientp->dev, "%s(): (%s)\n", __func__, cmdstr);

	if (!strncmp(cmdstr, "start", sizeof(cmdstr))) {
		u8 tmp;
		if (bq27520_check_if_sealed(bd)) {
			dev_info(&bd->clientp->dev, "Can not enter ROM mode. "
				 "Must unseal device first\n");
			if (bq27520_unseal(bd)) {
				dev_err(&bd->clientp->dev,
					"Can not unseal device\n");
				goto end;
			}
		}

		if (bd->pdata->disable_algorithm)
			bd->pdata->disable_algorithm(true);

		rc = sysfs_create_bin_file(&pdev->kobj, &bq27520_fg_data);
		if (rc && rc != -EEXIST) {
			dev_err(&bd->clientp->dev,
				"Cannot create sysfs bin file. rc=%d\n", rc);
			goto end;
		}
		tmp = atomic_read(&bq27520_init_ok);
		bd->run_init_after_rom = true;
		atomic_set(&bq27520_init_ok, 0);
		rc = bq27520_enter_rom_mode(bd);
		if (rc && rc != -EALREADY) {
			atomic_set(&bq27520_init_ok, tmp);
			if (tmp)
				bd->run_init_after_rom = false;
			sysfs_remove_bin_file(&pdev->kobj, &bq27520_fg_data);
			if (bd->pdata->disable_algorithm)
				bd->pdata->disable_algorithm(false);
		}
	} else if (!strncmp(cmdstr, "end", sizeof(cmdstr))) {
		rc = bq27520_exit_rom_mode(bd);
		if (rc)
			goto end;
		if (bq27520_last_step_production(bd))
			dev_info(&bd->clientp->dev,
				 "Can not finalize last production step\n");
		sysfs_remove_bin_file(&pdev->kobj, &bq27520_fg_data);
		queue_work(bd->wq, &bd->init_work);
		if (bd->pdata->disable_algorithm)
			bd->pdata->disable_algorithm(false);
	} else {
		dev_dbg(&bd->clientp->dev, "%s: cmd not supported\n", __func__);
		rc = -EINVAL;
		goto end;
	}

	rc = strnlen(pbuf, sizeof(cmdstr));

end:
	mutex_unlock(&bd->lock);
	return rc;
}

static ssize_t show_fg_cmd(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	u16 fw_ver = 0;
	s32 rc;
	struct bq27520_golden_info gi;
	char name[GOLDEN_PROJ_NAME_LEN + 1];

	dev_dbg(&bd->clientp->dev, "%s()\n", __func__);

	if (bd->rom_clientp)
		/* In ROM mode. Return '0' */
		return scnprintf(buf, PAGE_SIZE,
				 "0x0000 0x0000 0x0000 xxxx %d\n",
				 atomic_read(&bq27520_init_ok));

	rc = bq27520_get_fw_version(bd, &fw_ver);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed getting FW version. rc=%d\n", rc);
		return rc;
	}

	rc = bq27520_get_golden_info(bd, &gi);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed getting expected FW version\n");
		return rc;
	}

	if (!gi.project_name[0])
		memcpy(name, UNKNOWN_PROJ_NAME, GOLDEN_PROJ_NAME_LEN);
	else
		memcpy(name, gi.project_name, GOLDEN_PROJ_NAME_LEN);
	name[GOLDEN_PROJ_NAME_LEN] = '\0';

	return scnprintf(buf, PAGE_SIZE, "0x%.4x 0x%.4x 0x%.4x %s %d\n", fw_ver,
			 gi.fw_compatible_version, gi.golden_file_version,
			 name, atomic_read(&bq27520_init_ok));
}

static ssize_t store_lock(struct device *pdev, struct device_attribute *attr,
			  const char *pbuf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);
	int rc;
	s32 lock = 0;

	rc = read_sysfs_interface(pbuf, &lock, 10);
	if (rc < 0) {
		dev_err(pdev, "Reading sysfs fg_lock failed.\n");
		return -EINVAL;
	}

	rc = count;
	if (lock == 0) {
		mutex_unlock(&bd->data_flash_lock);
		mutex_unlock(&bd->lock);
	} else if (lock == 1) {
		mutex_lock(&bd->lock);
		mutex_lock(&bd->data_flash_lock);
	} else {
		dev_err(pdev, "Wrong input to sysfs fg_lock. "
			"Expect [0..1].\n");
		rc = -EINVAL;
	}

	return rc;
}

static struct device_attribute sysfs_attrs[] = {
	__ATTR(capacity,     S_IRUGO, show_capacity, NULL),
	__ATTR(fg_cmd,       S_IRUSR|S_IWUSR|S_IROTH, show_fg_cmd,
		store_fg_cmd),
	__ATTR(fg_lock,      S_IWUSR, NULL, store_lock),
#ifdef DEBUG_FS
	__ATTR(set_voltage,  S_IWUSR, NULL, store_voltage),
	__ATTR(set_current,  S_IWUSR, NULL, store_current),
	__ATTR(set_capacity, S_IWUSR, NULL, store_capacity),
	__ATTR(set_capacity_level, S_IWUSR, NULL, store_capacity_level),
#endif /* DEBUG_FS */
};

static int sysfs_create_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(sysfs_attrs); i++)
		if (device_create_file(dev, &sysfs_attrs[i]))
			goto sysfs_create_attrs_failed;

	return 0;

sysfs_create_attrs_failed:
	dev_err(dev, "Failed creating sysfs attrs.\n");
	while (i--)
		device_remove_file(dev, &sysfs_attrs[i]);

	return -EIO;
}

static void sysfs_remove_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(sysfs_attrs); i++)
		(void)device_remove_file(dev, &sysfs_attrs[i]);
}

static short conv_short(int v)
{
	return (short)v;
}

static int bq27520_read_bat_voltage(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_VOLTAGE);
	if (rc < 0)
		return rc;
	if (rc > READ_VOLTAGE_FILTER)
		return 0;
	bd->curr_mv = rc;
	dev_dbg(&bd->clientp->dev, "%s() rc=%d\n", __func__, bd->curr_mv);
	return 0;
}

static int bq27520_read_bat_capacity(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_byte_data(bd->clientp, REG_CMD_SOC);
	if (rc < 0)
		return rc;
	if (atomic_read(&bq27520_init_ok) &&
	    !bd->chg_connected && bd->curr_capacity < rc)
		return 0;
	bd->curr_capacity = rc;
	dev_dbg(&bd->clientp->dev, "%s() rc=%d\n", __func__, bd->curr_capacity);
	return 0;
}

static int bq27520_read_bat_current(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_INS_CURRENT);
	if (rc < 0)
		return rc;
	bd->curr_current = (int)conv_short(rc);
	dev_dbg(&bd->clientp->dev, "%s() rc=%d\n", __func__, bd->curr_current);
	return 0;
}

static int bq27520_read_bat_current_avg(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_AVG_CURRENT);
	if (rc < 0)
		return rc;
	bd->current_avg = (int)conv_short(rc);
	dev_dbg(&bd->clientp->dev, "%s() rc=%d\n", __func__, bd->current_avg);
	return 0;
}

static int bq27520_read_bat_flags(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_FLAGS);
	if (rc < 0)
		return rc;
	bd->flags = rc;
	dev_dbg(&bd->clientp->dev, "%s() rc=0x%x\n", __func__, bd->flags);
	return 0;
}

static int bq27520_read_app_status(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_byte_data(bd->clientp, REG_CMD_APPSTATUS);
	if (rc < 0)
		return rc;
	bd->app_status = rc;
	dev_dbg(&bd->clientp->dev, "%s() rc=0x%x\n", __func__, bd->app_status);
	return 0;
}

static int bq27520_read_designed_capacity(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_DESIGNEDCAPACITY);
	if (rc < 0)
		return rc;

	bd->designed_capacity = rc;

	dev_dbg(&bd->clientp->dev, "%s() rc=%d\n", __func__,
						bd->designed_capacity);

	return 0;
}

static int bq27520_read_health(struct power_supply *bat_ps)
{
	s32 rc;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	rc = i2c_smbus_read_byte_data(bd->clientp, REG_CMD_HEALTH_STATE);
	if (rc < 0)
		return rc;

	if (rc != SOH_VALUE_READY) {
		bd->health = 100;
		return 0;
	}

	rc = i2c_smbus_read_byte_data(bd->clientp, REG_CMD_HEALTH);
	if (rc < 0)
		return rc;

	bd->health = rc;

	dev_dbg(&bd->clientp->dev, "%s() health=%d rc=%d\n",
		 __func__, bd->health, rc);

	return 0;
}

static int bq27520_write_control(struct bq27520_data *bd, int subcmd)
{
	s32 rc;

	mutex_lock(&bd->control_reg_lock);

	rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_CONTROL,
					   subcmd);
	if (rc < 0) {
		dev_err(&bd->clientp->dev,
			"Failed write sub command:0x%x. rc=%d\n", subcmd, rc);
		goto exit;
	}
	dev_dbg(&bd->clientp->dev, "%s() subcmd=0x%x rc=%d\n",
		__func__, subcmd, rc);

exit:
	mutex_unlock(&bd->control_reg_lock);
	return (rc < 0) ? rc : 0;
}

static int bq27520_wait_for_qen_set(struct bq27520_data *bd)
{
	const unsigned int poll_cnt = POLL_QEN_TIMEOUT_MS / POLL_QEN_PERIOD_MS;
	int rc = 0;
	unsigned int i;

	/* TI recommendation before writing temperature after firmware load:
	 * Poll for QEN set or wait at least 2000 ms.
	 *
	 * This will do both and stop when one of these two
	 * conditions are met.
	 */
	for (i = 0; i < poll_cnt; i++) {
		rc = bq27520_get_control_status(bd);

		if (!rc && bd->control_status & QEN_MASK)
			break;
		else
			msleep(POLL_QEN_PERIOD_MS);
	}

	if (i == poll_cnt && !(bd->control_status & QEN_MASK))
		rc = -ETIME;

	return rc;
}

static int bq27520_write_temperature(struct bq27520_data *bd, int temp)
{
	int k = temp + A_TEMP_COEF_DEFINE;
	s32 rc = i2c_smbus_write_word_data(bd->clientp, REG_CMD_TEMPERATURE, k);
	dev_dbg(&bd->clientp->dev, "%s() k=%d rc=%d\n", __func__, k, rc);
	return rc;
}

static int bq27520_make_sure_temperature_is_set(struct bq27520_data *bd,
						int temp)
{
	const unsigned int poll_cnt =
		TEMP_WRITE_TIMEOUT_MS / MAX_WORD_PROGRAMMING_TIME_MS;
	s32 rc = 0;
	int temp_check = 0;
	unsigned int i;

	for (i = 0; i < poll_cnt; i++) {
		rc = bq27520_write_temperature(bd, temp);
		if (rc < 0)
			break;
		msleep(MAX_WORD_PROGRAMMING_TIME_MS);
		rc = i2c_smbus_read_word_data(bd->clientp, REG_CMD_TEMPERATURE);
		if (rc < 0)
			break;

		temp_check = rc - A_TEMP_COEF_DEFINE;
		if (temp_check == temp)
			break;
	}

	if (rc >= 0 && i == poll_cnt && temp_check != temp)
		rc = -ETIME;

	if (rc < 0)
		dev_err(&bd->clientp->dev,
			"Failed writing temperature. rc = %d\n", rc);

	return rc;
}

static int bq27520_check_initialization_comp(struct bq27520_data *bd)
{
	int i;
	int rc;

	for (i = 0; i < RETRY_MAX; i++) {
		msleep(1000);
		rc = bq27520_get_control_status(bd);
		if (!rc && bd->control_status & INIT_COMP_MASK)
			return 0;
	}
	return -ETIME;
}

static int bq27520_battery_info_setting(struct bq27520_data *bd,
					int type, int temp)
{
	int rc;
	int subcmd = 0;

	if (type != POWER_SUPPLY_TECHNOLOGY_UNKNOWN) {
		rc = bq27520_make_sure_it_enabled_is_set(bd);
		if (rc)
			return rc;
	}

	bq27520_read_bat_flags(&bd->bat_ps);
	/* Only set BAT_INSERT if gauge sees no battery detect
	 * and the battery is identified externally.
	 *
	 * Unidentified batteries should not be set to detect in
	 * gauge. If set to detect, gauge will learn an unidentified battery.
	 *
	 * BAT_INSERT clears automatically when when gauge looses power
	 */
	if (type == POWER_SUPPLY_TECHNOLOGY_UNKNOWN) {
		dev_info(&bd->clientp->dev, "Writing BAT_REMOVE\n");
		bq27520_write_control(bd, SUB_CMD_BAT_REMOVE);
		return 0;
	}

	/* Recommendetion from TI:
	 * Write temperature before BAT_INSERT.
	 */
	bq27520_make_sure_temperature_is_set(bd, temp);
	if (!(bd->flags & BAT_DET_MASK)) {
		dev_info(&bd->clientp->dev, "Writing BAT_INSERT\n");
		bq27520_write_control(bd, SUB_CMD_BAT_INSERT);
	}

	rc = bq27520_check_initialization_comp(bd);
	if (rc)
		return rc;

	/* Sanity to be really sure that battery will be learned */
	rc = bq27520_wait_for_qen_set(bd);
	if (rc) {
		dev_err(&bd->clientp->dev, "QEN not set. "
			"Battery will not be learned. rc=%d\n", rc);
		return rc;
	}

	if (!bd->got_technology)
		return -EINVAL;

	rc = bq27520_read_app_status(&bd->bat_ps);
	if (rc)
		return rc;

	dev_info(&bd->clientp->dev, "%s() type=%d temp=%d status=%d\n",
		 __func__, type, temp, bd->app_status);

	if ((bd->app_status & LU_PROF_MASK) &&
		type == POWER_SUPPLY_TECHNOLOGY_LIPO)
		subcmd = SUB_CMD_CHOOSE_A;
	else if (!(bd->app_status & LU_PROF_MASK) &&
		type == POWER_SUPPLY_TECHNOLOGY_LiMn)
		subcmd = SUB_CMD_CHOOSE_B;

	if (subcmd) {
		bq27520_write_control(bd, subcmd);
		msleep(1000);

		/* TI:
		 * No need to send BAT_INSERT since it has already been
		 * sent in top of this function.
		 */
		rc = bq27520_check_initialization_comp(bd);
	}

	rc = bq27520_seal(bd);
	if (rc) {
		dev_err(&bd->clientp->dev,
			"Failed to seal firmware during initial phase\n");
		return rc;
	}

	return rc;
}

#ifdef SUPPORT_PARAMETER_UPDATE
static int bq27520_block_data_update(struct bq27520_data *bd)
{
	int i;
	int rc;

	for (i = 0; i < BQ27520_BTBL_MAX; i++) {
		rc = i2c_smbus_write_byte_data(bd->clientp,
			bd->udatap[i].adr,
			bd->udatap[i].data);
		if (rc < 0) {
			dev_err(&bd->clientp->dev, "%s() rc=%d adr=0x%x\n",
				__func__, rc, bd->udatap[i].adr);
			return rc;
		}
		msleep(MAX_WORD_PROGRAMMING_TIME_MS);
	}

	msleep(100);
	rc = bq27520_write_control(bd, SUB_CMD_RESET);
	if (rc < 0)
		dev_err(&bd->clientp->dev,
			"Failed write RESET sub command. rc=%d\n", rc);
	msleep(1000);
	return rc;
}
#endif

static void bq27520_init_worker(struct work_struct *work)
{
	struct bq27520_data *bd =
		container_of(work, struct bq27520_data, init_work);
	struct power_supply *ps;
	s32 volt;
	int i;

	for (i = 0; i < INVALID_BATT_VOLTAGE_RETRY_MAX; i++) {
		volt = i2c_smbus_read_word_data(bd->clientp, REG_CMD_VOLTAGE);
		dev_info(&bd->clientp->dev,
			"%d: Check battery voltage=%d\n", i + 1, volt);
		if (volt < 0) {
			dev_err(&bd->clientp->dev,
				"Failed read battery voltage. rc=%d\n", volt);
		} else if (volt <= INVALID_BATT_VOLTAGE_LOWER_LIMIT ||
			   volt >= INVALID_BATT_VOLTAGE_UPPER_LIMIT) {
			dev_dbg(&bd->clientp->dev,
			"Device RESET!! checked by not SYSDOWN but VBAT\n");
			bq27520_device_reset(bd);
			break;
		}
		msleep(1000);
	}

	if (bd->battery_dev_name) {
		for (i = 0; i < RETRY_MAX; i++) {
			ps = power_supply_get_by_name(bd->battery_dev_name);
			if (ps)
				get_supplier_data(ps->dev, &bd->bat_ps);

			if (bd->got_technology) {
#ifdef SUPPORT_PARAMETER_UPDATE
				if (bd->udatap)
					bq27520_block_data_update(bd);
#endif
				bq27520_battery_info_setting(bd,
							     bd->technology,
							     bd->bat_temp);
				break;
			}
			msleep(1000);
		}
	}
	msleep(1000);
	bq27520_read_soc(bd);
	bq27520_read_bat_current_avg(&bd->bat_ps);
	bq27520_read_designed_capacity(&bd->bat_ps);
	bq27520_read_bat_flags(&bd->bat_ps);
	if (bd->curr_capacity == 0 && !(bd->flags & SYSDOWN_MASK))
		bd->curr_capacity = 1;

	bd->cap_scale.scaled_capacity = calculate_scaled_capacity(bd);

	dev_info(&bd->clientp->dev, "%s(): capacity=%d flags=0x%x\n",
		 __func__, bd->curr_capacity, bd->flags);
	bd->run_init_after_rom = false;
	atomic_set(&bq27520_init_ok, 1);

	/* Notify externals that we have new data to share */
	power_supply_changed(&bd->bat_ps);
}

static int bq27520_bat_get_property(struct power_supply *bat_ps,
				    enum power_supply_property psp,
				    union power_supply_propval *val)
{
	int rc = 0;
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	if (!atomic_read(&bq27520_init_ok))
		return -EBUSY;

	mutex_lock(&bd->lock);

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = bq27520_read_bat_voltage(bat_ps);
		if (rc)
			break;
		val->intval = bd->curr_mv * 1000;
#ifdef DEBUG_FS
		if (bd->bat_volt_debug.active)
			val->intval = bd->bat_volt_debug.value * 1000;
#endif
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = bd->lipo_bat_max_volt * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = bd->lipo_bat_min_volt * 1000;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = bd->curr_capacity;
#ifdef DEBUG_FS
		if (bd->bat_cap_debug.active)
			val->intval = bd->bat_cap_debug.value;
#endif
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = bd->curr_capacity_level;
#ifdef DEBUG_FS
		if (bd->bat_cap_lvl_debug.active)
			val->intval = bd->bat_cap_lvl_debug.value;
#endif
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		rc = bq27520_read_bat_current(bat_ps);
		if (rc)
			break;
		val->intval = bd->curr_current * 1000;
#ifdef DEBUG_FS
		if (bd->bat_curr_debug.active)
			val->intval = bd->bat_curr_debug.value * 1000;
#endif
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		rc = bq27520_read_bat_current_avg(bat_ps);
		if (rc)
			break;
		val->intval = bd->current_avg * 1000;
#ifdef DEBUG_FS
		if (bd->bat_curr_debug.active)
			val->intval = bd->bat_curr_debug.value * 1000;
#endif
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL:
		rc = bq27520_read_health(bat_ps);
		if (rc)
			break;

		/* val->intval(umAh) =
		designed_capacity(mAh) * 1000 * health(%) / 100 */
		val->intval =
		bd->designed_capacity * bd->health * 10;

		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = bd->designed_capacity * 1000;

		break;

	default:
		rc = -EINVAL;
		break;
	}
	mutex_unlock(&bd->lock);
	return rc;
}

static void start_read_fc(struct bq27520_data *bd)
{
	dev_dbg(&bd->clientp->dev, "%s()\n", __func__);
	queue_delayed_work(bd->wq, &bd->fc_work, 0);
}

static void stop_read_fc(struct bq27520_data *bd)
{
	dev_dbg(&bd->clientp->dev, "%s()\n", __func__);
	if (delayed_work_pending(&bd->fc_work))
		cancel_delayed_work_sync(&bd->fc_work);
}

static void bq27520_read_fc_worker(struct work_struct *work)
{
	int rc;
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct bq27520_data *bd =
		container_of(dwork, struct bq27520_data, fc_work);

	mutex_lock(&bd->lock);
	rc = bq27520_read_bat_flags(&bd->bat_ps);
	mutex_unlock(&bd->lock);

	dev_dbg(&bd->clientp->dev, "%s() capacity=%d flags=0x%x\n", __func__,
		bd->curr_capacity, bd->flags);

	if (!rc) {
		u8 changed = 0;

		mutex_lock(&bd->lock);
		if (bd->flags & FC_MASK &&
		    bd->chg_connected &&
		    bd->curr_capacity_level !=
		    POWER_SUPPLY_CAPACITY_LEVEL_FULL) {
			if (!bd->detect_fc_soon) {
				dev_dbg(&bd->clientp->dev,
					"%s() FC=1 detected\n", __func__);
				bd->detect_fc_soon = true;
				goto exit_unlock;
			}
			dev_info(&bd->clientp->dev,
				 "Fully charged (SOC=%d%%)\n",
				 bd->curr_capacity);
			bd->curr_capacity_level =
				POWER_SUPPLY_CAPACITY_LEVEL_FULL;
			changed = 1;
			bd->cap_scale.enable = true;
			bd->cap_scale.capacity_to_scale[0] = 100;
			bd->cap_scale.capacity_to_scale[1] =
				bd->curr_capacity;
			bd->cap_scale.disable_capacity_level = 100;
		} else if (!(bd->flags & FC_MASK) &&
			   bd->curr_capacity_level !=
			   POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN) {
			if (!bd->detect_fc_soon) {
				dev_dbg(&bd->clientp->dev,
					"%s() FC=0 detected\n", __func__);
				bd->detect_fc_soon = true;
				goto exit_unlock;
			}
			dev_info(&bd->clientp->dev,
				 "Recharge needed (SOC=%d%%)\n",
				 bd->curr_capacity);
			bd->curr_capacity_level =
				POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
			changed = 1;
		}
		bd->detect_fc_soon = false;

		if (changed)
			bd->cap_scale.scaled_capacity =
				calculate_scaled_capacity(bd);
exit_unlock:
		mutex_unlock(&bd->lock);

		if (changed)
			power_supply_changed(&bd->bat_ps);
	}

	queue_delayed_work(bd->wq, &bd->fc_work, HZ * READ_FC_TIMER);
}

static bool bq27520_read_soc(struct bq27520_data *bd)
{
	bool valid_cap = false;
	if (bd->got_technology &&
	    bd->technology == POWER_SUPPLY_TECHNOLOGY_UNKNOWN) {
		if (!bq27520_read_bat_voltage(&bd->bat_ps)) {
			bd->curr_capacity =
			((clamp(bd->curr_mv,
			bd->lipo_bat_min_volt, bd->lipo_bat_max_volt) -
			bd->lipo_bat_min_volt) * 100) /
			(bd->lipo_bat_max_volt - bd->lipo_bat_min_volt);
			valid_cap = true;
		}
	} else if (!bq27520_read_bat_capacity(&bd->bat_ps))
		valid_cap = true;

	if (!bq27520_read_bat_flags(&bd->bat_ps) &&
	    (bd->flags & SYSDOWN_MASK)) {
		dev_info(&bd->clientp->dev, "Shutting down because of low "
			 "voltage (SOC = %u%%).\n", bd->curr_capacity);
		bd->curr_capacity = 0;
		valid_cap = true;
	} else if (valid_cap && bd->curr_capacity == 0) {
		bd->curr_capacity = 1;
		dev_info(&bd->clientp->dev, "SOC is 0%% and no SYSDOWN.\n");
	}

	return valid_cap;
}

static void bq27520_handle_soc_worker(struct work_struct *work)
{
	struct bq27520_data *bd =
		container_of(work, struct bq27520_data, soc_int_work);
	bool valid_cap;

	mutex_lock(&bd->lock);

	valid_cap = bq27520_read_soc(bd);

	(void)bq27520_get_control_status(bd);

	mutex_unlock(&bd->lock);

	if (valid_cap) {
		mutex_lock(&bd->lock);
		if (bd->chg_connected &&
		    bd->curr_capacity >= bd->polling_lower_capacity &&
			bd->curr_capacity <= bd->polling_upper_capacity) {
			if (!bd->started_worker) {
				bd->detect_fc_soon = false;
				start_read_fc(bd);
				bd->started_worker = 1;
			}
		} else {
			if (bd->started_worker) {
				stop_read_fc(bd);
				bd->started_worker = 0;
				bd->curr_capacity_level =
					POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
			}
		}

		/* As long as we are in fully charge mode scale the capacity
		 * to show 100 %.
		 */
		if (bd->curr_capacity_level ==
		    POWER_SUPPLY_CAPACITY_LEVEL_FULL) {
			bd->cap_scale.capacity_to_scale[0] = 100;
			bd->cap_scale.capacity_to_scale[1] =
				clamp(bd->curr_capacity,
				      bd->polling_lower_capacity,
				      bd->polling_upper_capacity);
			dev_dbg(&bd->clientp->dev,
				"Scale cap with 100/%d\n",
				bd->cap_scale.capacity_to_scale[1]);
		}

		if (bd->curr_capacity == 0)
			wake_lock_timeout(&bd->wake_lock,
				SHUTDOWN_WAKELOCK_TIMEOUT);

		bd->cap_scale.scaled_capacity =	calculate_scaled_capacity(bd);

		mutex_unlock(&bd->lock);

		power_supply_changed(&bd->bat_ps);
	}
	dev_info(&bd->clientp->dev,
		 "%s() capacity=%d (%d) flags=0x%x ctrl_status=0x%x valid=%d\n",
		 __func__, bd->curr_capacity, bd->cap_scale.scaled_capacity,
		 bd->flags, bd->control_status, valid_cap);
}

static irqreturn_t bq27520_soc_thread_irq(int irq, void *data)
{
	struct bq27520_data *bd = (struct bq27520_data *)data;

	if (atomic_read(&bq27520_init_ok)) {
		mutex_lock(&bd->int_lock);
		if (!bd->suspended)
			bq27520_handle_soc_worker(&bd->soc_int_work);
		else
			bd->resume_int = 1;
		mutex_unlock(&bd->int_lock);
	}

	return IRQ_HANDLED;
}

static void bq27520_update_online_status(struct bq27520_data *bd,
					enum power_supply_type type,
					int curr_val)
{
	int usb = bd->chg_connected & USB_CHG;
	int wall = bd->chg_connected & WALL_CHG;

	if (POWER_SUPPLY_TYPE_USB == type) {
		if (IS_CHG_CONNECTED(usb, curr_val)) {
			bd->chg_connected |= USB_CHG;
			dev_dbg(&bd->clientp->dev,
			"USB charger connected\n");
		}
		if (IS_CHG_DISCONNECTED(usb, curr_val)) {
			bd->chg_connected &= ~USB_CHG;
			dev_dbg(&bd->clientp->dev,
			"USB charger disconnected\n");
		}
	} else if (POWER_SUPPLY_TYPE_MAINS == type) {
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
		if (bd->pdata->get_ac_online_status) {
			int ac = bd->pdata->get_ac_online_status();
			int cradle = bd->chg_connected & CRADLE_CHG;
			int curr_wall = ac & WALL_CHG;
			int curr_cradle = ac & CRADLE_CHG;

			if (IS_CHG_CONNECTED(wall, curr_wall)) {
				bd->chg_connected |= WALL_CHG;
				dev_dbg(&bd->clientp->dev,
				"Wall charger connected\n");
			}

			if (IS_CHG_DISCONNECTED(wall, curr_wall)) {
				bd->chg_connected &= ~WALL_CHG;
				dev_dbg(&bd->clientp->dev,
				"Wall charger disconnected\n");
			}

			if (IS_CHG_CONNECTED(cradle, curr_cradle)) {
				bd->chg_connected |= CRADLE_CHG;
				dev_dbg(&bd->clientp->dev,
				"Cradle charger connected\n");
			}

			if (IS_CHG_DISCONNECTED(cradle, curr_cradle)) {
				bd->chg_connected &= ~CRADLE_CHG;
				dev_dbg(&bd->clientp->dev,
				"Cradle charger disconnected\n");
			}
		}
#else
		if (IS_CHG_CONNECTED(wall, curr_val)) {
			bd->chg_connected |= WALL_CHG;
			dev_dbg(&bd->clientp->dev,
			"Wall charger connected\n");
		}

		if (IS_CHG_DISCONNECTED(wall, curr_val)) {
			bd->chg_connected &= ~WALL_CHG;
			dev_dbg(&bd->clientp->dev,
			"Wall charger disconnected\n");
		}
#endif
	}
}

static int get_supplier_data(struct device *dev, void *data)
{
	struct power_supply *psy = (struct power_supply *)data;
	struct power_supply *pst = dev_get_drvdata(dev);
	unsigned int i;
	union power_supply_propval ret;
	struct bq27520_data *bd =
		container_of(psy, struct bq27520_data, bat_ps);

	for (i = 0; i < pst->num_supplicants; i++) {
		if (strncmp(pst->supplied_to[i], psy->name,
			sizeof(pst->supplied_to[i])))
			continue;

		if (!pst->get_property(pst, POWER_SUPPLY_PROP_TEMP, &ret)) {
			mutex_lock(&bd->lock);
			if (atomic_read(&bq27520_init_ok) &&
			    bd->bat_temp != ret.intval) {
				if (!bd->suspended)
					bq27520_write_temperature(bd,
								  ret.intval);
				else
					bd->resume_temp = 1;
			}
			bd->bat_temp = ret.intval;
			mutex_unlock(&bd->lock);
			dev_dbg(&bd->clientp->dev, "got temperature %d C\n",
				ret.intval);
		}

		if (!pst->get_property(pst, POWER_SUPPLY_PROP_TECHNOLOGY,
				       &ret)) {
			mutex_lock(&bd->lock);
			bd->technology = ret.intval;
			bd->got_technology = 1;
			mutex_unlock(&bd->lock);
			dev_dbg(&bd->clientp->dev, "got technology %d\n",
				ret.intval);
		}

		if (!pst->get_property(pst, POWER_SUPPLY_PROP_ONLINE, &ret)) {
			mutex_lock(&bd->lock);
			bq27520_update_online_status(bd, pst->type,
							ret.intval);
			mutex_unlock(&bd->lock);
		}
	}

	return 0;
}

static void bq27520_ext_pwr_change_worker(struct work_struct *work)
{
	struct bq27520_data *bd =
		container_of(work, struct bq27520_data, ext_pwr_change_work);
	int chg_connected = bd->chg_connected;
	int technology = bd->technology;

	class_for_each_device(power_supply_class, NULL, &bd->bat_ps,
			      get_supplier_data);

	if (chg_connected != bd->chg_connected) {
		mutex_lock(&bd->lock);
		if (!bd->chg_connected) {
			if (bd->started_worker) {
				stop_read_fc(bd);
				bd->started_worker = 0;
			}

			if (bd->cap_scale.enable) {
				if (bd->cap_scale.scaled_capacity != 100) {
					bd->cap_scale.capacity_to_scale[0] =
						bd->cap_scale.scaled_capacity;
					bd->cap_scale.capacity_to_scale[1] =
						bd->curr_capacity;
				} else {
					bd->cap_scale.capacity_to_scale[0] =
						100;
					bd->cap_scale.capacity_to_scale[1] =
					clamp(bd->curr_capacity,
					      bd->polling_lower_capacity,
					      bd->polling_upper_capacity);
				}

				dev_dbg(&bd->clientp->dev,
					"Cap to scale at disconn %d/%d\n",
					bd->cap_scale.capacity_to_scale[0],
					bd->cap_scale.capacity_to_scale[1]);
			}

			bd->curr_capacity_level =
				POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
		} else {
			if (bd->cap_scale.enable) {
				bd->cap_scale.disable_capacity_level =
					bd->cap_scale.scaled_capacity;
				dev_dbg(&bd->clientp->dev,
					"Cap to stop scale %d%%\n",
					bd->cap_scale.disable_capacity_level);
			}

			if (!bd->started_worker &&
			    bd->curr_capacity >= bd->polling_lower_capacity &&
			    bd->curr_capacity <= bd->polling_upper_capacity) {
				bd->detect_fc_soon = true;
				start_read_fc(bd);
				bd->started_worker = 1;
			}
		}
		mutex_unlock(&bd->lock);
	}

	if (technology != bd->technology)
		bq27520_battery_info_setting(bd, bd->technology, bd->bat_temp);
}

static void bq27520_bat_external_power_changed(struct power_supply *bat_ps)
{
	struct bq27520_data *bd =
		container_of(bat_ps, struct bq27520_data, bat_ps);

	queue_work(bd->wq, &bd->ext_pwr_change_work);
}

int bq27520_get_current_average(void)
{
	struct power_supply *psy = power_supply_get_by_name(BQ27520_NAME);
	struct bq27520_data *bd;

	if (!psy)
		return -EAGAIN;

	bd = container_of(psy, struct bq27520_data, bat_ps);
	mutex_lock(&bd->lock);
	bq27520_read_bat_current_avg(&bd->bat_ps);
	mutex_unlock(&bd->lock);
	return bd->current_avg;
}
EXPORT_SYMBOL_GPL(bq27520_get_current_average);

#ifdef CONFIG_SUSPEND
static int bq27520_pm_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq27520_data *bd = i2c_get_clientdata(client);

	mutex_lock(&bd->int_lock);
	bd->suspended = 1;
	mutex_unlock(&bd->int_lock);

	if (bd->got_technology &&
		bd->technology != POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		irq_set_irq_wake(client->irq, 1);

	flush_workqueue(bd->wq);

	return 0;
}

static int bq27520_pm_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq27520_data *bd = i2c_get_clientdata(client);

	bd->suspended = 0;
	if (bd->got_technology &&
		bd->technology != POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		irq_set_irq_wake(client->irq, 0);

	if (bd->resume_int) {
		bd->resume_int = 0;
		queue_work(bd->wq, &bd->soc_int_work);
	}

	if (bd->resume_temp) {
		bd->resume_temp = 0;
		bq27520_write_temperature(bd, bd->bat_temp);
	}

	return 0;
}
#else
#define bq27520_pm_suspend	NULL
#define bq27520_pm_resume	NULL
#endif

static int __exit bq27520_remove(struct i2c_client *client)
{
	struct bq27520_data *bd = i2c_get_clientdata(client);

	free_irq(client->irq, 0);

	if (work_pending(&bd->soc_int_work))
		cancel_work_sync(&bd->soc_int_work);

	if (work_pending(&bd->ext_pwr_change_work))
		cancel_work_sync(&bd->ext_pwr_change_work);

	if (work_pending(&bd->init_work))
		cancel_work_sync(&bd->init_work);

	if (delayed_work_pending(&bd->fc_work))
		cancel_delayed_work_sync(&bd->fc_work);

	destroy_workqueue(bd->wq);

	sysfs_remove_attrs(bd->bat_ps.dev);

	power_supply_unregister(&bd->bat_ps);

	i2c_set_clientdata(client, NULL);

	kfree(bd);
	return 0;
}

static enum power_supply_property bq27520_bat_main_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN
};

static const struct i2c_device_id bq27520_id[] = {
	{BQ27520_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bq27520_id);

static void bq27520_device_reset(struct bq27520_data *bd)
{
	dev_info(&bd->clientp->dev, "Fuelgauge Device Reset!\n");
	gpio_set_value(GPIO_FG_CE, 1);
	msleep(MAX_GPIO_FG_CE_MS);
	gpio_set_value(GPIO_FG_CE, 0);
	msleep(MAX_AFTER_DEVICE_RESET_MS);
}

static int bq27520_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int rc = 0;
	u16 fw_ver = 0;
	struct bq27520_platform_data *pdata;
	struct bq27520_data *bd;
	struct bq27520_golden_info gi;

	pdata = client->dev.platform_data;
	if (pdata && pdata->gpio_configure) {
		rc = pdata->gpio_configure(1);
		if (rc) {
			dev_err(&client->dev, "failed to gpio_configure\n");
			goto probe_exit;
		}
	}

	bd = kzalloc(sizeof(struct bq27520_data), GFP_KERNEL);
	if (!bd) {
		rc = -ENOMEM;
		goto probe_exit_hw_deinit;
	}

	bd->bat_ps.name = BQ27520_NAME;
	bd->bat_ps.type = POWER_SUPPLY_TYPE_BATTERY;
	bd->bat_ps.properties = bq27520_bat_main_props;
	bd->bat_ps.num_properties = ARRAY_SIZE(bq27520_bat_main_props);
	bd->bat_ps.get_property = bq27520_bat_get_property;
	bd->bat_ps.external_power_changed =
		bq27520_bat_external_power_changed;
	bd->bat_ps.use_for_apm = 1;
	bd->clientp = client;

	bd->polling_lower_capacity = 95;
	bd->polling_upper_capacity = 100;
	bd->run_init_after_rom = false;
	bd->sealed = -1;
	pdata = client->dev.platform_data;
	if (pdata) {
		bd->pdata = pdata;
		bd->battery_dev_name = pdata->battery_dev_name;
		bd->lipo_bat_max_volt = pdata->lipo_bat_max_volt;
		bd->lipo_bat_min_volt = pdata->lipo_bat_min_volt;
		bd->polling_lower_capacity = pdata->polling_lower_capacity;
		bd->polling_upper_capacity = pdata->polling_upper_capacity;
		bd->udatap = pdata->udatap;
		if (pdata->supplied_to) {
			bd->bat_ps.supplied_to = pdata->supplied_to;
			bd->bat_ps.num_supplicants = pdata->num_supplicants;
		}
	}

	mutex_init(&bd->lock);
	mutex_init(&bd->int_lock);
	mutex_init(&bd->data_flash_lock);
	mutex_init(&bd->control_reg_lock);

	bd->rom_clientp = NULL;

	rc = bq27520_get_fw_version(bd, &fw_ver);
	if (rc == -EIO || rc == -ENODEV || rc == -ENOTCONN) {
		rc = bq27520_recover_rom_mode(client, &bd->rom_clientp);
		if (rc < 0) {
			dev_err(&client->dev, "Failed recover ROM mode\n");
			bq27520_device_reset(bd);
		}
	} else if (rc < 0) {
		dev_err(&client->dev, "Failed getting FW version\n");
		bq27520_device_reset(bd);
	}

	if (bd->rom_clientp) {
		dev_info(&bd->clientp->dev, "In ROM mode\n");
	} else {
		rc = bq27520_get_golden_info(bd, &gi);
		if (rc < 0) {
			dev_err(&client->dev,
				"Failed getting expected FW version\n");
			goto probe_exit_mem_free;
		}

		dev_info(&bd->clientp->dev,
			 "FW v%x.%x (expect v%x.%x). Golden FW v%x.%x\n",
			 (fw_ver >> 8) & 0xFF, fw_ver & 0xFF,
			 (gi.fw_compatible_version >> 8) & 0xFF,
			 gi.fw_compatible_version & 0xFF,
			 (gi.golden_file_version >> 8) & 0xFF,
			 gi.golden_file_version & 0xFF);
	}

	bd->wq = create_singlethread_workqueue("batteryworker");
	if (!bd->wq) {
		dev_err(&client->dev, "Failed creating workqueue\n");
		rc = -EIO;
		goto probe_exit_mem_free;
	}

	INIT_WORK(&bd->init_work, bq27520_init_worker);
	INIT_WORK(&bd->ext_pwr_change_work, bq27520_ext_pwr_change_worker);
	INIT_WORK(&bd->soc_int_work, bq27520_handle_soc_worker);
	INIT_DELAYED_WORK(&bd->fc_work, bq27520_read_fc_worker);
	wake_lock_init(&bd->wake_lock, WAKE_LOCK_SUSPEND,
		       "bq27520_shutdown_lock");

	rc = power_supply_register(&client->dev, &bd->bat_ps);
	if (rc) {
		dev_err(&client->dev, "Failed to regist power supply\n");
		goto probe_exit_destroy_wq;
	}

	i2c_set_clientdata(client, bd);

	bd->got_technology = 0;
	bd->started_worker = 0;
	rc = request_threaded_irq(client->irq,
				NULL, bq27520_soc_thread_irq,
				IRQF_TRIGGER_FALLING | IRQF_DISABLED,
				BQ27520_NAME,
				bd);
	if (rc) {
		dev_err(&client->dev, "Failed requesting IRQ\n");
		goto probe_exit_unregister;
	}

	rc = sysfs_create_attrs(bd->bat_ps.dev);
	if (rc) {
		dev_err(&client->dev, "Complete sysfs support failed\n");
		goto probe_exit_unregister;
	}

	queue_work(bd->wq, &bd->init_work);
	return 0;

probe_exit_unregister:
	power_supply_unregister(&bd->bat_ps);
probe_exit_destroy_wq:
	destroy_workqueue(bd->wq);
probe_exit_mem_free:
	kfree(bd);
probe_exit_hw_deinit:
	if (pdata && pdata->gpio_configure)
		rc = pdata->gpio_configure(0);
probe_exit:
	return rc;
}

static const struct dev_pm_ops bq27520_pm = {
	.suspend = bq27520_pm_suspend,
	.resume = bq27520_pm_resume,
};

static struct i2c_driver bq27520_driver = {
	.driver = {
		.name = BQ27520_NAME,
		.owner = THIS_MODULE,
		.pm = &bq27520_pm,
	},
	.probe = bq27520_probe,
	.remove = __exit_p(bq27520_remove),
	.id_table = bq27520_id,
};

static int __init bq27520_init(void)
{
	int rc;

	rc = i2c_add_driver(&bq27520_driver);
	if (rc) {
		pr_err("%s: FAILED: i2c_add_driver rc=%d\n", __func__, rc);
		goto init_exit;
	}
	return 0;

init_exit:
	return rc;
}

static void __exit bq27520_exit(void)
{
	i2c_del_driver(&bq27520_driver);
}

module_init(bq27520_init);
module_exit(bq27520_exit);

MODULE_AUTHOR("James Jacobsson, Imre Sunyi, Hiroyuki Namba");
MODULE_LICENSE("GPL");

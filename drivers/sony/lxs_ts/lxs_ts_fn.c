/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts_fn.c
 *
 * LXS touch sub-functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts.h"

#define __LAYER_GPIO__

static int __gpio_init(struct lxs_ts *ts, int pin, unsigned long flags,
			const char *name, const char *title)
{
	int ret;

	if (!gpio_is_valid(pin)) {
		t_dev_warn(ts->dev, "%s: gpio_request_one ignored, invalid pin %d\n", title, pin);
		return 0;
	}

	ret = gpio_request_one(pin, flags, name);
	if (ret) {
		t_dev_err(ts->dev, "%s: gpio_request_one(%d, %d, %s) failed, %d\n",
			title, pin, (int)flags, name, ret);
		return ret;
	}

	t_dev_info(ts->dev, "%s: gpio_request_one(%d, %d, %s)\n",
		title, pin, (int)flags, name);

	return 0;
}

static void __gpio_free(struct lxs_ts *ts, int pin, const char *title)
{
	if (!gpio_is_valid(pin)) {
		t_dev_warn(ts->dev, "%s: gpio_free ignored, invalid pin %d\n", title, pin);
		return;
	}

	gpio_free(pin);

	t_dev_info(ts->dev, "%s: gpio_free(%d)\n", title, pin);
}

static void __gpio_set(struct lxs_ts *ts, int pin, bool val, const char *title)
{
	if (!gpio_is_valid(pin)) {
		t_dev_warn(ts->dev, "%s: gpio_set_vaue ignored, invalid pin %d\n", title, pin);
		return;
	}

	gpio_set_value(pin, val);

	t_dev_info(ts->dev, "%s: gpio_set_value(%d, %d)\n", title, pin, val);
}

static int lxs_ts_gpio_init_reset(struct lxs_ts *ts)
{
	if (ts->fquirks->gpio_init_reset)
		return ts->fquirks->gpio_init_reset(ts);

	if (ts->plat_data->use_skip_reset)
		return 0;

	return 0;
}

static void lxs_ts_gpio_free_reset(struct lxs_ts *ts)
{
	if (ts->fquirks->gpio_free_reset) {
		ts->fquirks->gpio_free_reset(ts);
		return;
	}

	if (ts->plat_data->use_skip_reset)
		return;
}

void lxs_ts_gpio_set_reset(struct lxs_ts *ts, bool val)
{
	if (ts->fquirks->gpio_set_reset) {
		ts->fquirks->gpio_set_reset(ts, val);
		return;
	}

	if (ts->plat_data->use_skip_reset)
		return;

	__gpio_set(ts, ts->plat_data->reset_pin, val, "gpio_reset");
}

static int lxs_ts_gpio_init_irq(struct lxs_ts *ts)
{
	if (ts->fquirks->gpio_init_irq)
		return ts->fquirks->gpio_init_irq(ts);

	return __gpio_init(ts, ts->plat_data->irq_pin, GPIOF_IN, LXS_TS_GPIO_IRQ, "gpio_irq");
}

static void lxs_ts_gpio_free_irq(struct lxs_ts *ts)
{
	if (ts->fquirks->gpio_free_irq) {
		ts->fquirks->gpio_free_irq(ts);
		return;
	}

	__gpio_free(ts, ts->plat_data->irq_pin, "gpio_irq");
}

int lxs_ts_init_gpios(struct lxs_ts *ts)
{
	int ret = 0;

	ret = lxs_ts_gpio_init_reset(ts);
	if (ret)
		return ret;

	ret = lxs_ts_gpio_init_irq(ts);
	if (ret) {
		lxs_ts_gpio_free_reset(ts);
		return ret;
	}

	return 0;
}

void lxs_ts_free_gpios(struct lxs_ts *ts)
{
	lxs_ts_gpio_free_irq(ts);

	lxs_ts_gpio_free_reset(ts);
}

int lxs_ts_irq_level_check(struct lxs_ts *ts)
{
	int pin = ts->plat_data->irq_pin;
	int i;

	if (!ts->plat_data->use_irq_verify)
		return 0;

	if (!gpio_is_valid(pin))
		return 0;

	//Check period : 8 msec(about 120Hz)
	for (i = 0; i < 8; i++) {
		if (gpio_get_value(pin))
			return 0;

		lxs_ts_delay(1);
	}

	t_dev_info(ts->dev, "gpio_irq: level low detected\n");

	return 1;
}

#define __LAYER_PINCTRL__

#if defined(__LXS_SUPPORT_PINCTRL)
#define PIN_STATE_ON	"on_state"
#define PIN_STATE_OFF	"off_state"

int lxs_ts_set_pinctrl(struct lxs_ts *ts, bool on)
{
	struct pinctrl *pinctrl = ts->pinctrl;
	struct pinctrl_state *state;
	const char *name = (on) ? PIN_STATE_ON : PIN_STATE_OFF;
	int ret;

	if (IS_ERR_OR_NULL(pinctrl))
		return 0;

	state = (on) ? ts->pinctrl_state_on : ts->pinctrl_state_off;
	if (IS_ERR_OR_NULL(state)) {
		return 0;
	}

	ret = pinctrl_select_state(pinctrl, state);
	if (ret) {
		t_dev_err(ts->dev, "pinctrl: set pin %s failed, %d\n", name, ret);
		return ret;
	}

	t_dev_info(ts->dev, "pinctrl: set pin %s\n", name);

	return 0;
}

static void lxs_ts_init_pinctrl(struct lxs_ts *ts)
{
	struct pinctrl *pinctrl;
	struct pinctrl_state *state_on;
	struct pinctrl_state *state_off;

	pinctrl = devm_pinctrl_get(ts->dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		t_dev_info(ts->dev, "pinctrl: not found\n");
		return;
	}

	state_on = pinctrl_lookup_state(pinctrl, PIN_STATE_ON);
	state_off = pinctrl_lookup_state(pinctrl, PIN_STATE_OFF);
	if (IS_ERR_OR_NULL(state_on) && IS_ERR_OR_NULL(state_off)) {
		devm_pinctrl_put(pinctrl);
		return;
	}

	ts->pinctrl = pinctrl;
	ts->pinctrl_state_on = state_on;
	ts->pinctrl_state_off = state_off;

	t_dev_info(ts->dev, "pinctrl: found\n");

	lxs_ts_set_pinctrl(ts, 1);
}

static void lxs_ts_free_pinctrl(struct lxs_ts *ts)
{
	if (IS_ERR_OR_NULL(ts->pinctrl))
		return;

	lxs_ts_set_pinctrl(ts, 0);

	devm_pinctrl_put(ts->pinctrl);

	ts->pinctrl = NULL;
	ts->pinctrl_state_on = NULL;
	ts->pinctrl_state_off = NULL;
}
#else	/* __LXS_SUPPORT_PINCTRL */
int lxs_ts_set_pinctrl(struct lxs_ts *ts, bool on)
{

}

static inline void lxs_ts_init_pinctrl(struct lxs_ts *ts)
{

}

static inline void lxs_ts_free_pinctrl(struct lxs_ts *ts)
{

}
#endif	/* __LXS_SUPPORT_PINCTRL */

#define __LAYER_POWER__

static void lxs_ts_power_reg(struct lxs_ts *ts, int on)
{
	return;
}

int lxs_ts_power_ctrl(struct lxs_ts *ts, int ctrl)
{
	struct lxs_ts_chip *chip = &ts->chip;

	switch (ctrl) {
	case TS_POWER_OFF:
		atomic_set(&chip->init, TC_IC_INIT_NEED);

		lxs_ts_power_reg(ts, 0);

		lxs_ts_delay(chip->drv_reset_low);
		break;

	case TS_POWER_ON:
		lxs_ts_power_reg(ts, 1);
		break;
	}

	return 0;
}

int lxs_ts_power_reset(struct lxs_ts *ts, int ctrl)
{
	struct lxs_ts_chip *chip = &ts->chip;

	switch (ctrl) {
	case TS_POWER_OFF:
		atomic_set(&chip->init, TC_IC_INIT_NEED);

		lxs_ts_gpio_set_reset(ts, 0);

		lxs_ts_delay(5);

		lxs_ts_power_ctrl(ts, ctrl);
		break;

	case TS_POWER_ON:
		lxs_ts_power_ctrl(ts, ctrl);

		lxs_ts_delay(5);

		lxs_ts_gpio_set_reset(ts, 1);
		break;

	case TS_POWER_HW_RESET:
		t_dev_info(ts->dev, "power_reset: reset\n");
		lxs_hal_reset(ts, TC_HW_RESET_ASYNC, 0);
		break;
	}

	return 0;
}

#define __LAYER_BUS__

static void __lxs_bus_err(struct lxs_ts *ts, u32 addr, u8 *buf, int size, int wr)
{
	int prt_len = 0;
	int prt_idx = 0;
	int prd_sz = size;

	if (!unlikely(t_bus_dbg_mask & DBG_TRC_ERR))
		return;

	while (size) {
		prt_len = min(size, 16);

		t_bus_err(ts->dev,
			"%s 0x%04X, 0x%04X buf[%3d~%3d] %*ph\n",
			(wr) ? "bus_write" : "bus_read",
			addr, prd_sz,
			prt_idx, prt_idx + prt_len - 1,
			prt_len, &buf[prt_idx]);

		size -= prt_len;
		prt_idx += prt_len;
	}
}


static void __lxs_bus_dbg(struct lxs_ts *ts, u32 addr, u8 *buf, int size, int wr)
{
	int prt_len = 0;
	int prt_idx = 0;
	int prd_sz = size;

	if (!unlikely(t_bus_dbg_mask & DBG_TRACE))
		return;

	while (size) {
		prt_len = min(size, 16);

		t_bus_dbg_trace(ts->dev,
			"%s 0x%04X, 0x%04X buf[%3d~%3d] %*ph\n",
			(wr) ? "bus_write" : "bus_read",
			addr, prd_sz,
			prt_idx, prt_idx + prt_len - 1,
			prt_len, &buf[prt_idx]);

		size -= prt_len;
		prt_idx += prt_len;
	}
}

int lxs_ts_bus_read(struct lxs_ts *ts, u32 addr, void *data, int size)
{
	struct lxs_ts_bus_msg *msg = &ts->rd_msg;
	u8 *tx_buf = msg->tx_buf;
	u8 *rx_buf = msg->rx_buf;
	u8 *dbuf;
	int bus_tx_hdr_size = ts->bus_tx_hdr_size;
	int bus_rx_hdr_size = ts->bus_rx_hdr_size;
//	int bus_tx_dummy_size = ts->bus_tx_dummy_size;
	int bus_rx_dummy_size = ts->bus_rx_dummy_size;
	int tx_size = bus_tx_hdr_size;
	int ret = -EIO;

	if (!ts->resume_done.done) {
		ret = wait_for_completion_interruptible_timeout(&ts->resume_done, msecs_to_jiffies(500));
		if (ret <= 0) {
			t_bus_err(ts->dev, "bus_read(0x%04X, 0x%04X): resume not handled, %d\n",
				addr, size, ret);
			return -EIO;
		}
	}

	if (ts->power_enabled == TS_POWER_OFF) {
		t_bus_err(ts->dev, "%s: POWER_STATUS : OFF\n", __func__);
		return -EIO;
	}

	mutex_lock(&ts->bus_lock);

	if (ts->bus_read == NULL) {
		t_bus_err(ts->dev, "no bus_read, %s(%d)\n",
			dev_name(ts->dev), ts->bus_type);
		goto out;
	}

	tx_buf[0] = (size > 4) ? 0x20 : 0x00;
	tx_buf[0] |= ((addr >> 8) & 0x0f);
	tx_buf[1] = (addr & 0xff);

	while (bus_rx_dummy_size--)
		tx_buf[tx_size++] = 0;

	msg->tx_size = tx_size;
	msg->rx_size = bus_rx_hdr_size + size;

	dbuf = &rx_buf[bus_rx_hdr_size];

	ret = ts->bus_read(ts);
	if (ret < 0) {
		t_bus_err(ts->dev, "bus_read error(0x%04X, 0x%04X, %*ph), %d\n",
			addr, size, tx_size, tx_buf, ret);
		__lxs_bus_err(ts, addr, dbuf, size, 0);
		goto out;
	}

	memcpy(data, dbuf, size);

	__lxs_bus_dbg(ts, addr, dbuf, size, 0);

	ret = size;

out:
	mutex_unlock(&ts->bus_lock);

	return ret;
}

int lxs_ts_bus_write(struct lxs_ts *ts, u32 addr, void *data, int size)
{
	struct lxs_ts_bus_msg *msg = &ts->wr_msg;
	u8 *tx_buf = msg->tx_buf;
	u8 *dbuf;
	int bus_tx_hdr_size = ts->bus_tx_hdr_size;
//	int bus_rx_hdr_size = ts->bus_rx_hdr_size;
	int is_spi = !!(ts->bus_type == BUS_SPI);
	int ret = -EIO;

	if (!ts->resume_done.done) {
		ret = wait_for_completion_interruptible_timeout(&ts->resume_done, msecs_to_jiffies(500));
		if (ret <= 0) {
			t_bus_err(ts->dev, "bus_write(0x%04X, 0x%04X): resume not handled, %d\n",
				addr, size, ret);
			return -EIO;
		}
	}

	if (ts->power_enabled == TS_POWER_OFF) {
		t_bus_err(ts->dev, "%s: POWER_STATUS : OFF\n", __func__);
		return -EIO;
	}

	mutex_lock(&ts->bus_lock);

	if (ts->bus_write == NULL) {
		t_bus_err(ts->dev, "no bus_write, %s(%d)\n",
			dev_name(ts->dev), ts->bus_type);
		goto out;
	}

	tx_buf[0] = (is_spi || (size > 4)) ? 0x60 : 0x40;
	tx_buf[0] |= ((addr >> 8) & 0x0f);
	tx_buf[1] = (addr & 0xff);

	msg->tx_size = bus_tx_hdr_size + size;

	dbuf = &tx_buf[bus_tx_hdr_size];

	memcpy(dbuf, data, size);

	ret = ts->bus_write(ts);
	if (ret < 0) {
		t_bus_err(ts->dev, "bus_write error(0x%04X, 0x%04X, %*ph), %d\n",
			addr, size, bus_tx_hdr_size, tx_buf, ret);
		__lxs_bus_err(ts, addr, dbuf, size, 1);
		goto out;
	}

	__lxs_bus_dbg(ts, addr, dbuf, size, 1);

	ret = size;

out:
	mutex_unlock(&ts->bus_lock);

	return ret;
}

static int lxs_ts_bus_alloc(struct lxs_ts *ts)
{
	char *tx_buf;
	int buf_size = ts->buf_len;

	if (!buf_size)
		buf_size = TS_MAX_BUF_SIZE;

	buf_size += TS_BUF_MARGIN;

	/* add margin for bus header */
	ts->buf_size = buf_size;

	tx_buf = kmalloc(buf_size<<1, GFP_KERNEL | GFP_DMA);
	if (tx_buf == NULL) {
		t_dev_err(ts->dev, "failed to allocate bus buffer\n");
		return -ENOMEM;
	}

	memset(&ts->rd_msg, 0, sizeof(ts->rd_msg));

	ts->rd_msg.tx_buf = tx_buf;
	ts->rd_msg.rx_buf = tx_buf + buf_size;

	memset(&ts->wr_msg, 0, sizeof(ts->wr_msg));

	ts->wr_msg.tx_buf = tx_buf;
	ts->wr_msg.rx_buf = NULL;

	return 0;
}

static void lxs_ts_bus_free(struct lxs_ts *ts)
{
	kfree(ts->rd_msg.tx_buf);

	memset(&ts->rd_msg, 0, sizeof(ts->rd_msg));
	memset(&ts->wr_msg, 0, sizeof(ts->wr_msg));
}

static struct lxs_ts *lxs_ts_init(struct device *dev, const struct lxs_ts_entry_data *entry_data)
{
	struct lxs_ts *ts = NULL;
	int ret = 0;

	ts = devm_kzalloc(dev, sizeof(*ts), GFP_KERNEL);
	if (ts == NULL)
		return NULL;

	ts->chip_type = entry_data->chip_type;

	strlcpy(ts->chip_id, entry_data->chip_id, sizeof(ts->chip_id));
	strlcpy(ts->chip_name, entry_data->chip_name, sizeof(ts->chip_name));

	ts->max_finger = (entry_data->max_finger) ? entry_data->max_finger : MAX_FINGER;

	ts->mode_allowed = entry_data->mode_allowed;

	ts->fw_size = entry_data->fw_size;

	ts->bus_type = entry_data->bus_type;
	ts->buf_len = entry_data->buf_len;

	switch (ts->bus_type) {
	case BUS_I2C:
		ts->chip_select = 0;
		ts->spi_mode = -1;
		ts->bits_per_word = -1;
		ts->max_freq = -1;

		if (entry_data->bus_custom) {
			ts->bus_tx_hdr_size = entry_data->bus_tx_hdr_size;
			ts->bus_rx_hdr_size = entry_data->bus_rx_hdr_size;
			ts->bus_tx_dummy_size = entry_data->bus_tx_dummy_size;
			ts->bus_rx_dummy_size = entry_data->bus_rx_dummy_size;
		} else {
			ts->bus_tx_hdr_size = I2C_BUS_TX_HDR_SZ;
			ts->bus_rx_hdr_size = I2C_BUS_RX_HDR_SZ;
			ts->bus_tx_dummy_size = I2C_BUS_TX_DUMMY_SZ;
			ts->bus_rx_dummy_size = I2C_BUS_RX_DUMMY_SZ;
		}
		break;
	case BUS_SPI:
		if (entry_data->bus_custom) {
			ts->chip_select = entry_data->chip_select;
			ts->spi_mode = entry_data->spi_mode;
			ts->bits_per_word = entry_data->bits_per_word;
			ts->max_freq = entry_data->max_freq;

			ts->bus_tx_hdr_size = entry_data->bus_tx_hdr_size;
			ts->bus_rx_hdr_size = entry_data->bus_rx_hdr_size;
			ts->bus_tx_dummy_size = entry_data->bus_tx_dummy_size;
			ts->bus_rx_dummy_size = entry_data->bus_rx_dummy_size;
		} else {
			ts->chip_select = 0;
			ts->spi_mode = SPI_MODE_0;
			ts->bits_per_word = 8;
			ts->max_freq = __CLOCK_MHZ(5);

			ts->bus_tx_hdr_size = SPI_BUS_TX_HDR_SZ;
			ts->bus_rx_hdr_size = SPI_BUS_RX_HDR_SZ;
			ts->bus_tx_dummy_size = SPI_BUS_TX_DUMMY_SZ;
			ts->bus_rx_dummy_size = SPI_BUS_RX_DUMMY_SZ;
		}
		break;
	}

	ts->reg_quirks = (struct lxs_ts_reg_quirk *)entry_data->reg_quirks;

	ts->fquirks = (struct lxs_ts_fquirks *)&entry_data->fquirks;

	ts->dev = dev;

	ret = lxs_ts_bus_alloc(ts);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to alloc bus buffer, %d\n", ret);
		devm_kfree(dev, ts);
		return NULL;
	}

	lxs_ts_init_pinctrl(ts);

	mutex_init(&ts->lock);
	mutex_init(&ts->bus_lock);
	mutex_init(&ts->reset_lock);

	device_init_wakeup(ts->dev, true);

	dev_set_drvdata(dev, ts);

	return ts;
}

static void lxs_ts_free(struct device *dev, struct lxs_ts *ts)
{
	dev_set_drvdata(dev, NULL);

	device_init_wakeup(ts->dev, false);

	mutex_destroy(&ts->lock);
	mutex_destroy(&ts->bus_lock);
	mutex_destroy(&ts->reset_lock);

	lxs_ts_free_pinctrl(ts);

	lxs_ts_bus_free(ts);

	devm_kfree(dev, ts);
}

#if IS_ENABLED(CONFIG_PM_SLEEP)
static int lxs_ts_bus_pm_suspend(struct device *dev)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	if (ts->shutdown_called) {
		t_dev_err(dev, "%s shutdown was called\n", __func__);
		return 0;
	}

	if (ts->state_core != TS_CORE_NORMAL)
		return 0;

	ts->suspend_is_on_going = 1;

	ts->state_pm = TS_DEV_PM_SUSPEND;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	reinit_completion(&ts->resume_done);
#else
	ts->resume_done.done = 0;
#endif

	t_dev_info(dev, "touch bus pm suspend done\n");

	ts->suspend_is_on_going = 0;

	return 0;
}

static int lxs_ts_bus_pm_resume(struct device *dev)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int ret = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return 0;

	ts->state_pm = TS_DEV_PM_RESUME;

	complete_all(&ts->resume_done);

	t_dev_info(dev, "touch bus pm resume done\n");

	return ret;
}

static const struct dev_pm_ops lxs_ts_bus_pm_ops = {
	.suspend	= lxs_ts_bus_pm_suspend,
	.resume 	= lxs_ts_bus_pm_resume,
};
#define DEV_BUS_PM_OPS	(&lxs_ts_bus_pm_ops)
#else	/* !CONFIG_PM_SLEEP */
#define DEV_BUS_PM_OPS	NULL
#endif	/* CONFIG_PM_SLEEP */

#if defined(__LXS_SUPPORT_I2C)

static int lxs_ts_i2c_read(struct lxs_ts *ts)
{
	struct lxs_ts_bus_msg *msg = &ts->rd_msg;
	struct i2c_client *client = to_i2c_client(ts->dev);
	struct i2c_msg *msgs = ts->msgs_rx;
	struct i2c_msg __msgs[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = msg->tx_size,
			.buf = msg->tx_buf,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = msg->rx_size,
			.buf = msg->rx_buf,
		},
	};
	int ret = 0;

	memcpy(msgs, __msgs, sizeof(__msgs));

	if ((msg->rx_size > ts->buf_size) ||
		(msg->tx_size > ts->buf_size)) {
		t_dev_err(&client->dev, "i2c rd: buffer overflow - rx %Xh, tx %Xh\n",
			msg->rx_size, msg->tx_size);
		return -EOVERFLOW;
	}

	ret = i2c_transfer(client->adapter, msgs, 2);

	return ret;
}

static int lxs_ts_i2c_write(struct lxs_ts *ts)
{
	struct lxs_ts_bus_msg *msg = &ts->wr_msg;
	struct i2c_client *client = to_i2c_client(ts->dev);
	struct i2c_msg *msgs = ts->msgs_tx;
	struct i2c_msg __msgs[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = msg->tx_size,
			.buf = msg->tx_buf,
		},
	};
	int ret = 0;

	memcpy(msgs, __msgs, sizeof(__msgs));

	if (msg->tx_size > ts->buf_size) {
		t_dev_err(&client->dev, "i2c wr: buffer overflow - tx %Xh\n",
			msg->tx_size);
		return -EOVERFLOW;
	}

	ret = i2c_transfer(client->adapter, msgs, 1);

	return ret;
}

static int lxs_ts_i2c_probe(struct i2c_client *i2c,
					const struct i2c_device_id *id)
{
	struct lxs_ts_if_driver *if_drv = NULL;
	struct lxs_ts *ts = NULL;
	struct device *dev = &i2c->dev;
	int ret = 0;

	t_dev_info(dev, "i2c: bus probe : %s/%s/%s\n",
		dev_name(dev->parent->parent),
		dev_name(dev->parent), dev_name(dev));

	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		t_dev_err(dev, "i2c: func not Supported\n");
		return -EIO;
	}

	if_drv = container_of(to_i2c_driver(dev->driver),
				struct lxs_ts_if_driver, bus.i2c_drv);

	ts = lxs_ts_init(dev, if_drv->entry_data);
	if (ts == NULL) {
		t_dev_err(dev, "i2c: failed to allocate ts data\n");
		return -ENOMEM;
	}

	ts->idx = if_drv->idx;

	ts->irq = i2c->irq;

	ts->bus_read = lxs_ts_i2c_read;
	ts->bus_write = lxs_ts_i2c_write;

	ret = lxs_ts_probe(ts);
	if (ret)
		goto out_plat;

	if_drv->idx++;

	return 0;

out_plat:
	lxs_ts_free(dev, ts);

	return ret;
}

static int lxs_ts_i2c_remove(struct i2c_client *i2c)
{
	struct lxs_ts *ts = dev_get_drvdata(&i2c->dev);

	if (ts == NULL)
		return 0;

	lxs_ts_remove(ts, 0);

	lxs_ts_free(&i2c->dev, ts);

	return 0;
}

static void lxs_ts_i2c_shutdown(struct i2c_client *i2c)
{
	struct lxs_ts *ts = dev_get_drvdata(&i2c->dev);

	if (ts == NULL)
		return;

	lxs_ts_remove(ts, 1);

	lxs_ts_free(&i2c->dev, ts);
}

static struct i2c_device_id lxs_ts_i2c_id[] = {
	{ "lxs,reserved", 0 },
	{ LXS_TS_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lxs_ts_i2c_id);

static int lxs_ts_i2c_register_driver(struct lxs_ts_if_driver *if_drv)
{
	const struct lxs_ts_entry_data *entry_data = if_drv->entry_data;
	struct i2c_driver *i2c_drv = &if_drv->bus.i2c_drv;
	struct i2c_device_id *id = lxs_ts_i2c_id;
	int ret = 0;

	i2c_drv->driver.name = LXS_TS_NAME;
	i2c_drv->driver.owner = THIS_MODULE;
	i2c_drv->driver.of_match_table = entry_data->of_match_table;
	i2c_drv->driver.pm = DEV_BUS_PM_OPS;

	i2c_drv->probe = lxs_ts_i2c_probe;
	i2c_drv->remove = lxs_ts_i2c_remove;
	i2c_drv->shutdown = lxs_ts_i2c_shutdown;
	i2c_drv->id_table = lxs_ts_i2c_id;

	/*
	 * for non-DTS case
	 * : change lxs_ts_i2c_id[0].name to compatible name
	 */
	memset((void *)id->name, 0, I2C_NAME_SIZE);
	snprintf((char *)id->name, I2C_NAME_SIZE, "%s", entry_data->of_match_table[0].compatible);
	id->driver_data = (typeof(id->driver_data))entry_data;

	ret = i2c_add_driver(i2c_drv);
	if (ret) {
		t_pr_err("i2c_register_driver[%s] failed, %d\n",
			i2c_drv->driver.name, ret);
		return ret;
	}

	return 0;
}

static void lxs_ts_i2c_unregister_driver(struct lxs_ts_if_driver *if_drv)
{
	i2c_del_driver(&if_drv->bus.i2c_drv);
}
#endif	/* __LXS_SUPPORT_I2C */

#if defined(__LXS_SUPPORT_SPI)

static int lxs_ts_spi_init(struct lxs_ts *ts)
{
	struct spi_device *spi = to_spi_device(ts->dev);
	u32 new_bits = ts->bits_per_word;
	u32 new_mode = ts->spi_mode;
	u32 new_freq = ts->max_freq;
	u32 old_bits = spi->bits_per_word;
	u32 old_mode = spi->mode;
	u32 old_freq = spi->max_speed_hz;
	int ret;

	if (spi_freq_out_of_range(new_freq)) {
		t_dev_err(ts->dev, "spi init: wrong freq: max_freq %d.%d Mhz(%d)\n",
			freq_to_mhz_unit(new_freq),
			freq_to_khz_top(new_freq),
			new_freq);
		return -EFAULT;
	}

	spi->bits_per_word = new_bits;
	spi->mode = new_mode;
	spi->max_speed_hz = new_freq;

	ret = spi_setup(spi);
	if (ret < 0) {
		spi->bits_per_word = old_bits;
		spi->mode = old_mode;
		spi->max_speed_hz = old_freq;

		t_dev_err(ts->dev, "spi init: setup failed, %d\n", ret);
		return ret;
	}

	t_dev_info(ts->dev, "spi init: %d.%d Mhz, mode %d, bpw %d, cs %d (%s)\n",
		freq_to_mhz_unit(spi->max_speed_hz),
		freq_to_khz_top(spi->max_speed_hz),
		spi->mode,
		spi->bits_per_word,
		spi->chip_select,
		dev_name(&spi->master->dev));

	return 0;
}

static int lxs_ts_spi_read(struct lxs_ts *ts)
{
	struct lxs_ts_bus_msg *msg = &ts->rd_msg;
	struct spi_device *spi = to_spi_device(ts->dev);
	struct spi_transfer *x = &ts->x_rx;
	struct spi_message *m = &ts->m_rx;
	int ret = 0;

	if ((msg->rx_size > ts->buf_size) ||
		(msg->tx_size > ts->buf_size)) {
		t_dev_err(&spi->dev, "spi rd: buffer overflow - rx %Xh, tx %Xh\n",
			msg->rx_size, msg->tx_size);
		return -EOVERFLOW;
	}

	memset(x, 0, sizeof(*x));

	spi_message_init(m);

	x->tx_buf = msg->tx_buf;
	x->rx_buf = msg->rx_buf;
	x->len = msg->rx_size;
	x->bits_per_word = spi->bits_per_word,
	x->speed_hz = spi->max_speed_hz,

	spi_message_add_tail(x, m);

	ret = spi_sync(spi, m);

	return ret;
}

static int lxs_ts_spi_write(struct lxs_ts *ts)
{
	struct lxs_ts_bus_msg *msg = &ts->wr_msg;
	struct spi_device *spi = to_spi_device(ts->dev);
	struct spi_transfer *x = &ts->x_tx;
	struct spi_message *m = &ts->m_tx;
	int ret = 0;

	if (msg->tx_size > ts->buf_size) {
		t_dev_err(&spi->dev, "spi wr: buffer overflow - tx %Xh\n",
			msg->tx_size);
		return -EOVERFLOW;
	}

	memset(x, 0, sizeof(*x));

	spi_message_init(m);

	x->tx_buf = msg->tx_buf;
	x->len = msg->tx_size;
	x->bits_per_word = spi->bits_per_word,
	x->speed_hz = spi->max_speed_hz,

	spi_message_add_tail(x, m);

	ret = spi_sync(spi, m);

	return ret;
}

static int lxs_ts_spi_probe(struct spi_device *spi)
{
	struct lxs_ts_if_driver *if_drv = NULL;
	struct lxs_ts *ts = NULL;
	struct device *dev = &spi->dev;
	int ret = 0;

	t_dev_info(dev, "spi: bus probe : %s/%s/%s\n",
		dev_name(dev->parent->parent),
		dev_name(dev->parent), dev_name(dev));

	if (spi->master->flags & SPI_MASTER_HALF_DUPLEX) {
		t_dev_err(dev, "spi: Full duplex not supported by master\n");
		return -EIO;
	}

	if_drv = container_of(to_spi_driver(dev->driver),
				struct lxs_ts_if_driver, bus.spi_drv);

	ts = lxs_ts_init(dev, if_drv->entry_data);
	if (ts == NULL) {
		t_dev_err(dev, "spi: failed to allocate ts data\n");
		return -ENOMEM;
	}

	ret = lxs_ts_spi_init(ts);
	if (ret) {
		goto out_plat;
	}

	ts->idx = if_drv->idx;

	ts->irq = spi->irq;

	ts->bus_read = lxs_ts_spi_read;
	ts->bus_write = lxs_ts_spi_write;

	ret = lxs_ts_probe(ts);
	if (ret)
		goto out_plat;

	if_drv->idx++;

	return 0;

out_plat:
	lxs_ts_free(dev, ts);

	return ret;
}

static int lxs_ts_spi_remove(struct spi_device *spi)
{
	struct lxs_ts *ts = dev_get_drvdata(&spi->dev);

	if (ts == NULL)
		return 0;

	lxs_ts_remove(ts, 0);

	lxs_ts_free(&spi->dev, ts);

	return 0;
}

static void lxs_ts_spi_shutdown(struct spi_device *spi)
{
	struct lxs_ts *ts = dev_get_drvdata(&spi->dev);

	if (ts == NULL)
		return;

	lxs_ts_remove(ts, 1);

	lxs_ts_free(&spi->dev, ts);
}

static struct spi_device_id lxs_ts_spi_id[] = {
	{ "lxs,reserved", 0 },
	{ LXS_TS_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, lxs_ts_spi_id);

static int lxs_ts_spi_register_driver(struct lxs_ts_if_driver *if_drv)
{
	const struct lxs_ts_entry_data *entry_data = if_drv->entry_data;
	struct spi_driver *spi_drv = &if_drv->bus.spi_drv;
	struct spi_device_id *id = lxs_ts_spi_id;
	int ret = 0;

	spi_drv->driver.name = LXS_TS_NAME;
	spi_drv->driver.owner = THIS_MODULE;
	spi_drv->driver.of_match_table = entry_data->of_match_table;
	spi_drv->driver.pm = DEV_BUS_PM_OPS;

	spi_drv->probe = lxs_ts_spi_probe;
	spi_drv->remove = lxs_ts_spi_remove;
	spi_drv->shutdown = lxs_ts_spi_shutdown;
	spi_drv->id_table = lxs_ts_spi_id;

	/*
	 * for non-DTS case
	 * : change lxs_ts_spi_id[0].name to compatible name
	 */
	memset((void *)id->name, 0, SPI_NAME_SIZE);
	snprintf((char *)id->name, SPI_NAME_SIZE, "%s", entry_data->of_match_table[0].compatible);
	id->driver_data = (typeof(id->driver_data))entry_data;

	ret = spi_register_driver(spi_drv);
	if (ret) {
		t_pr_err("spi_register_driver[%s] failed, %d\n",
			spi_drv->driver.name, ret);
		return ret;
	}

	return 0;
}

static void lxs_ts_spi_unregister_driver(struct lxs_ts_if_driver *if_drv)
{
	spi_unregister_driver(&if_drv->bus.spi_drv);
}
#endif	/* __LXS_SUPPORT_SPI */

static int __lxs_ts_if_drv_chk(struct lxs_ts_if_driver *if_drv)
{
	if (if_drv == NULL) {
		t_pr_err("NULL touch if_drv\n");
		return -ENODEV;
	}

	if (if_drv->entry_data == NULL) {
		t_pr_err("NULL touch if_drv entry_data\n");
		return -ENODEV;
	}
	if (!if_drv->entry_data->chip_type) {
		t_pr_err("NULL touch if_drv entry_data->chip_type\n");
		return -ENODEV;
	}

	if (if_drv->entry_data->of_match_table == NULL) {
		t_pr_err("NULL touch if_drv entry_data->of_match_table\n");
		return -ENODEV;
	}

	if (if_drv->entry_data->chip_id == NULL) {
		t_pr_err("NULL touch if_drv entry_data->chip_id\n");
		return -ENODEV;
	}

	if (if_drv->entry_data->chip_name == NULL) {
		t_pr_err("NULL touch if_drv entry_data->chip_name\n");
		return -ENODEV;
	}

	if (!if_drv->entry_data->mode_allowed) {
		t_pr_err("NULL touch if_drv entry_data->mode_allowed\n");
		return -ENODEV;
	}

	if (!if_drv->entry_data->fw_size) {
		t_pr_err("NULL touch if_drv entry_data->fw_size\n");
		return -ENODEV;
	}

	return if_drv->entry_data->bus_type;
}

int lxs_ts_register_driver(struct lxs_ts_if_driver *if_drv)
{
	int bus_type;
	int ret = -EINVAL;

	bus_type = __lxs_ts_if_drv_chk(if_drv);

	switch (bus_type) {
	case BUS_I2C:
	#if defined(__LXS_SUPPORT_I2C)
		ret = lxs_ts_i2c_register_driver(if_drv);
	#endif
		break;
	case BUS_SPI:
	#if defined(__LXS_SUPPORT_SPI)
		ret = lxs_ts_spi_register_driver(if_drv);
	#endif
		break;
	}

	if (ret)
		t_pr_err("%s(%d) if register failed, %d\n", LXS_TS_NAME, bus_type, ret);

	return ret;
}

void lxs_ts_unregister_driver(struct lxs_ts_if_driver *if_drv)
{
	int bus_type;

	bus_type = __lxs_ts_if_drv_chk(if_drv);

	switch (bus_type) {
	case BUS_I2C:
	#if defined(__LXS_SUPPORT_I2C)
		lxs_ts_i2c_unregister_driver(if_drv);
	#endif
		break;
	case BUS_SPI:
	#if defined(__LXS_SUPPORT_SPI)
		lxs_ts_spi_unregister_driver(if_drv);
	#endif
		break;
	}
}


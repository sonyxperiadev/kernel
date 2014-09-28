/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/pn547.h>
#include <linux/wakelock.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
#include <mach/msm_xo.h>
#include <linux/workqueue.h>
#endif
#ifdef CONFIG_NFC_PN547_PMC8974_CLK_REQ
#include <linux/clk.h>
#endif

#include <mach/board-nfc.h>

#define MAX_BUFFER_SIZE		512

#define NFC_DEBUG 0
#define MAX_TRY_I2C_READ	10
#define I2C_ADDR_READ_L		0x51
#define I2C_ADDR_READ_H		0x57

struct pn547_dev {
	wait_queue_head_t read_wq;
	struct mutex read_mutex;
	struct i2c_client *client;
	struct miscdevice pn547_device;
	void (*conf_gpio) (void);
	unsigned int ven_gpio;
	unsigned int firm_gpio;
	unsigned int irq_gpio;

	atomic_t irq_enabled;
	atomic_t read_flag;
	bool cancel_read;
	struct wake_lock nfc_wake_lock;
#ifdef CONFIG_NFC_PN547_PMC8974_CLK_REQ
	struct clk *nfc_clk;
#endif
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	unsigned int clk_req_gpio;
	unsigned int clk_req_irq;
	struct msm_xo_voter *nfc_clock;
	struct work_struct work_nfc_clock;
	struct workqueue_struct *wq_clock;
	bool clock_state;
#endif
	unsigned int pvdd_en_gpio;
};

static irqreturn_t pn547_dev_irq_handler(int irq, void *dev_id)
{
	struct pn547_dev *pn547_dev = dev_id;

	if (!gpio_get_value(pn547_dev->irq_gpio)) {
#if NFC_DEBUG
		pr_err("%s, irq_gpio = %d\n", __func__,
			gpio_get_value(pn547_dev->irq_gpio));
#endif
		return IRQ_HANDLED;
	}

	/* Wake up waiting readers */
	atomic_set(&pn547_dev->read_flag, 1);
	wake_up(&pn547_dev->read_wq);


#if NFC_DEBUG
	pr_info("pn547 : call\n");
#endif
	wake_lock_timeout(&pn547_dev->nfc_wake_lock, 2*HZ);
	return IRQ_HANDLED;
}

#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
static void nfc_work_func_clock(struct work_struct *work)
{
	struct pn547_dev *pn547_dev = container_of(work, struct pn547_dev,
					      work_nfc_clock);
	int ret = 0;

	if (gpio_get_value(pn547_dev->clk_req_gpio)) {
		if (pn547_dev->clock_state == false) {
			ret = msm_xo_mode_vote(pn547_dev->nfc_clock,
						MSM_XO_MODE_ON);
			if (ret < 0) {
				pr_err("%s:  Failed to vote for TCX0_A1 ON (%d)\n",
						__func__, ret);
			}
			pn547_dev->clock_state = true;
		}
	} else {
		if (pn547_dev->clock_state == true) {
			ret = msm_xo_mode_vote(pn547_dev->nfc_clock,
						MSM_XO_MODE_OFF);
			if (ret < 0) {
				pr_err("%s:  Failed to vote for TCX0_A1 OFF (%d)\n",
						__func__, ret);
			}
			pn547_dev->clock_state = false;
		}
	}
}

static irqreturn_t pn547_dev_clk_req_irq_handler(int irq, void *dev_id)
{
	struct pn547_dev *pn547_dev = dev_id;
	queue_work(pn547_dev->wq_clock, &pn547_dev->work_nfc_clock);
	return IRQ_HANDLED;
}
#endif

static ssize_t pn547_dev_read(struct file *filp, char __user *buf,
			      size_t count, loff_t *offset)
{
	struct pn547_dev *pn547_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE] = {0, };
	int ret = 0;
#ifdef CONFIG_NFC_PN544
	int readingWatchdog = 0;
#endif

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	pr_debug("%s : reading %zu bytes. irq=%s\n", __func__, count,
		 gpio_get_value(pn547_dev->irq_gpio) ? "1" : "0");

#if NFC_DEBUG
	pr_info("pn547 : + r\n");
#endif

	mutex_lock(&pn547_dev->read_mutex);

#ifdef CONFIG_NFC_PN544
wait_irq:
#endif

	if (!gpio_get_value(pn547_dev->irq_gpio)) {
		atomic_set(&pn547_dev->read_flag, 0);
		if (filp->f_flags & O_NONBLOCK) {
			pr_info("%s : O_NONBLOCK\n", __func__);
			ret = -EAGAIN;
			goto fail;
		}

#if NFC_DEBUG
		pr_info("pn547: wait_event_interruptible : in\n");
#endif
		if (!gpio_get_value(pn547_dev->irq_gpio))
			ret = wait_event_interruptible(pn547_dev->read_wq,
				atomic_read(&pn547_dev->read_flag));

#if NFC_DEBUG
		pr_info("pn547 :   h\n");
#endif

		if (pn547_dev->cancel_read) {
			pn547_dev->cancel_read = false;
			ret = -1;
			goto fail;
		}

		if (ret)
			goto fail;

	}

	/* Read data */
	ret = i2c_master_recv(pn547_dev->client, tmp, count);

#ifdef CONFIG_NFC_PN544
	/* If bad frame(from 0x51 to 0x57) is received from pn65n,
	* we need to read again after waiting that IRQ is down.
	* if data is not ready, pn65n will send from 0x51 to 0x57. */
	if ((I2C_ADDR_READ_L <= tmp[0] && tmp[0] <= I2C_ADDR_READ_H)
		&& readingWatchdog < MAX_TRY_I2C_READ) {
		pr_warn("%s: data is not ready yet.data = 0x%x, cnt=%d\n",
			__func__, tmp[0], readingWatchdog);
		usleep_range(2000, 2000); /* sleep 2ms to wait for IRQ */
		readingWatchdog++;
		goto wait_irq;
	}
#endif

#if NFC_DEBUG
	pr_info("pn547: i2c_master_recv\n");
#endif
	mutex_unlock(&pn547_dev->read_mutex);
	if (ret < 0) {
		pr_err("%s: i2c_master_recv returned %d\n", __func__,
				ret);
		return ret;
	}

	if (ret > count) {
		pr_err("%s: received too many bytes from i2c (%d)\n",
				__func__, ret);
		return -EIO;
	}

	if (copy_to_user(buf, tmp, ret)) {
		pr_err("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}
	return ret;

fail:
	mutex_unlock(&pn547_dev->read_mutex);
	return ret;
}

static ssize_t pn547_dev_write(struct file *filp, const char __user *buf,
			       size_t count, loff_t *offset)
{
	struct pn547_dev *pn547_dev;
	char tmp[MAX_BUFFER_SIZE] = {0, };
	int ret = 0, retry = 2;

	pn547_dev = filp->private_data;

#if NFC_DEBUG
	pr_info("pn547 : + w\n");
#endif

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (copy_from_user(tmp, buf, count)) {
		pr_err("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	pr_debug("%s : writing %zu bytes.\n", __func__, count);
	/* Write data */
	do {
		retry--;
	ret = i2c_master_send(pn547_dev->client, tmp, count);
		if (ret == count)
			break;
		usleep_range(6000, 10000); /* Retry, chip was in standby */
#if NFC_DEBUG
		pr_debug("%s : retry = %d\n", __func__, retry);
#endif
	} while (retry);

#if NFC_DEBUG
	pr_info("pn547 : - w\n");
#endif

	if (ret != count) {
		pr_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}

	return ret;
}

static int pn547_dev_open(struct inode *inode, struct file *filp)
{
	struct pn547_dev *pn547_dev = container_of(filp->private_data,
						   struct pn547_dev,
						   pn547_device);
	filp->private_data = pn547_dev;

	pr_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

	return 0;
}

static long pn547_dev_ioctl(struct file *filp,
			   unsigned int cmd, unsigned long arg)
{
	struct pn547_dev *pn547_dev = filp->private_data;

	switch (cmd) {
	case PN547_SET_PWR:
		if (arg == 2) {
			/* power on with firmware download (requires hw reset)
			 */
			gpio_set_value_cansleep(pn547_dev->ven_gpio, 1);
			gpio_set_value(pn547_dev->firm_gpio, 1);
			usleep_range(10000, 10050);
			gpio_set_value_cansleep(pn547_dev->ven_gpio, 0);
			usleep_range(10000, 10050);
			gpio_set_value_cansleep(pn547_dev->ven_gpio, 1);
			usleep_range(10000, 10050);
			if (atomic_read(&pn547_dev->irq_enabled) == 0) {
				atomic_set(&pn547_dev->irq_enabled, 1);
				enable_irq(pn547_dev->client->irq);
				enable_irq_wake(pn547_dev->client->irq);
			}
			pr_info("%s power on with firmware, irq=%d\n", __func__,
				atomic_read(&pn547_dev->irq_enabled));
		} else if (arg == 1) {
			/* power on */
			if (pn547_dev->conf_gpio)
				pn547_dev->conf_gpio();
			gpio_set_value(pn547_dev->firm_gpio, 0);
			gpio_set_value_cansleep(pn547_dev->ven_gpio, 1);
			usleep_range(10000, 10050);
			if (atomic_read(&pn547_dev->irq_enabled) == 0) {
				atomic_set(&pn547_dev->irq_enabled, 1);
				enable_irq(pn547_dev->client->irq);
				enable_irq_wake(pn547_dev->client->irq);
			}
			pr_info("%s power on, irq=%d\n", __func__,
				atomic_read(&pn547_dev->irq_enabled));
		} else if (arg == 0) {
			/* power off */
			if (atomic_read(&pn547_dev->irq_enabled) == 1) {
				atomic_set(&pn547_dev->irq_enabled, 0);
				disable_irq_wake(pn547_dev->client->irq);
				disable_irq_nosync(pn547_dev->client->irq);
			}
			pr_info("%s power off, irq=%d\n", __func__,
				atomic_read(&pn547_dev->irq_enabled));
			gpio_set_value(pn547_dev->firm_gpio, 0);
			gpio_set_value_cansleep(pn547_dev->ven_gpio, 0);
			usleep_range(10000, 10050);
		} else if (arg == 3) {
			pr_info("%s Read Cancel\n", __func__);
			pn547_dev->cancel_read = true;
			atomic_set(&pn547_dev->read_flag, 1);
			wake_up(&pn547_dev->read_wq);
		} else {
			pr_err("%s bad arg %lu\n", __func__, arg);
			return -EINVAL;
		}
		break;
	default:
		pr_err("%s bad ioctl %u\n", __func__, cmd);
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations pn547_dev_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = pn547_dev_read,
	.write = pn547_dev_write,
	.open = pn547_dev_open,
	.unlocked_ioctl = pn547_dev_ioctl,
};

#ifdef CONFIG_OF
static int pn547_parse_dt(struct device *dev,
	struct pn547_i2c_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	pdata->irq_gpio = of_get_named_gpio_flags(np, "nxp,irq_gpio",
		0, &pdata->irq_gpio_flags);

	pdata->ven_gpio = of_get_named_gpio_flags(np, "nxp,ven",
		0, &pdata->ven_gpio_flags);

	pdata->firm_gpio = of_get_named_gpio_flags(np, "nxp,dwld_en",
		0, &pdata->firm_gpio_flags);

	pdata->pvdd_en_gpio = of_get_named_gpio_flags(np, "nxp,pvdd_en",
		0, &pdata->pvdd_en_gpio_flags);

	if (pdata->firm_gpio < 0)
		of_property_read_u32(np, "nxp,firm-expander-gpio",
			&pdata->firm_gpio);

	pr_info("%s: irq : %d, ven : %d, firm : %d, pvdd_en : %d\n",
			__func__, pdata->irq_gpio, pdata->ven_gpio,
			pdata->firm_gpio, pdata->pvdd_en_gpio);

	return 0;
}
#else
static int pn547_parse_dt(struct device *dev,
	struct pn547_i2c_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static int pn547_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	int ret;
	int err;
	int addr;
	char tmp[4] = {0x20, 0x00, 0x01, 0x01};
	int addrcnt;
	struct pn547_i2c_platform_data *platform_data;
	struct pn547_dev *pn547_dev;

	if (client->dev.of_node) {
		platform_data = devm_kzalloc(&client->dev,
			sizeof(struct pn547_i2c_platform_data), GFP_KERNEL);
		if (!platform_data) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}
		err = pn547_parse_dt(&client->dev, platform_data);
		if (err)
			return err;
		err = board_nfc_parse_dt(&client->dev, platform_data);
		if (err < 0)
			return err;
	} else {
		platform_data = client->dev.platform_data;
	}

	if (platform_data == NULL) {
		pr_err("%s : nfc probe fail\n", __func__);
		return -ENODEV;
	}

	if (platform_data->dynamic_config) {
		ret = board_nfc_hw_lag_check(client, platform_data);
		if (ret < 0)
			return ret;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s : need I2C_FUNC_I2C\n", __func__);
		return -ENODEV;
	}

	ret = gpio_request(platform_data->irq_gpio, "nfc_int");
	if (ret)
		return -ENODEV;
	ret = gpio_request(platform_data->ven_gpio, "nfc_ven");
	if (ret)
		goto err_ven;
	ret = gpio_request(platform_data->firm_gpio, "nfc_firm");
	if (ret)
		goto err_firm;
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	ret = gpio_request(platform_data->clk_req_gpio, "nfc_clk_req");
	if (ret)
		goto err_clk_req;
#endif
	ret = gpio_request(platform_data->pvdd_en_gpio, "nfc_pvdd_en");
	if (ret)
		goto err_pvdd_en;
	pn547_dev = kzalloc(sizeof(*pn547_dev), GFP_KERNEL);
	if (pn547_dev == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	pn547_dev->nfc_clock = msm_xo_get(MSM_XO_TCXO_A1, "nfc");
	if (IS_ERR(pn547_dev->nfc_clock)) {
		ret = PTR_ERR(pn547_dev->nfc_clock);
		printk(KERN_ERR "%s: Couldn't get TCXO_A1 vote for NFC (%d)\n",
					__func__, ret);
		ret = -ENODEV;
		goto err_get_clock;
	}
	pn547_dev->clock_state = false;
#endif
#ifdef CONFIG_NFC_PN547_PMC8974_CLK_REQ
	pn547_dev->nfc_clk = clk_get(&client->dev, "nfc_clk");
	if (IS_ERR(pn547_dev->nfc_clk)) {
		ret = PTR_ERR(pn547_dev->nfc_clk);
		printk(KERN_ERR "%s: Couldn't get D1 (%d)\n",
					__func__, ret);
	} else {
		if (clk_prepare_enable(pn547_dev->nfc_clk))
			printk(KERN_ERR "%s: Couldn't prepare D1\n",
					__func__);
	}
#endif
	pr_info("%s : IRQ num %d\n", __func__, client->irq);

	pn547_dev->irq_gpio = platform_data->irq_gpio;
	pn547_dev->ven_gpio = platform_data->ven_gpio;
	pn547_dev->firm_gpio = platform_data->firm_gpio;
	pn547_dev->conf_gpio = platform_data->conf_gpio;
	pn547_dev->pvdd_en_gpio = platform_data->pvdd_en_gpio;
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	pn547_dev->clk_req_gpio = platform_data->clk_req_gpio;
	pn547_dev->clk_req_irq = platform_data->clk_req_irq;
#endif
	pn547_dev->client = client;

	/* init mutex and queues */
	init_waitqueue_head(&pn547_dev->read_wq);
	mutex_init(&pn547_dev->read_mutex);

	pn547_dev->pn547_device.minor = MISC_DYNAMIC_MINOR;
#ifdef CONFIG_NFC_PN547
	pn547_dev->pn547_device.name = "pn547";
#else
	pn547_dev->pn547_device.name = "pn544";
#endif
	pn547_dev->pn547_device.fops = &pn547_dev_fops;

	ret = misc_register(&pn547_dev->pn547_device);
	if (ret) {
		pr_err("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	pr_info("%s : requesting IRQ %d\n", __func__, client->irq);
	gpio_direction_input(pn547_dev->irq_gpio);
	gpio_direction_output(pn547_dev->ven_gpio, 0);
	gpio_direction_output(pn547_dev->firm_gpio, 0);
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	gpio_direction_input(pn547_dev->clk_req_gpio);
#endif

	i2c_set_clientdata(client, pn547_dev);
	wake_lock_init(&pn547_dev->nfc_wake_lock,
			WAKE_LOCK_SUSPEND, "nfc_wake_lock");
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	pn547_dev->wq_clock = create_singlethread_workqueue("nfc_wq");
	if (!pn547_dev->wq_clock) {
		ret = -ENOMEM;
		pr_err("%s: could not create workqueue\n", __func__);
		goto err_create_workqueue;
	}
	INIT_WORK(&pn547_dev->work_nfc_clock, nfc_work_func_clock);
#endif
	ret = request_irq(client->irq, pn547_dev_irq_handler,
			  IRQF_TRIGGER_RISING, "pn547", pn547_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}
	disable_irq_nosync(pn547_dev->client->irq);
	atomic_set(&pn547_dev->irq_enabled, 0);

#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	ret = request_irq(pn547_dev->clk_req_irq, pn547_dev_clk_req_irq_handler,
		IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING
			, "pn547_clk_req", pn547_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq(clk_req) failed\n");
		goto err_request_irq_failed;
	}

	enable_irq_wake(pn547_dev->clk_req_irq);
#endif

	gpio_set_value(pn547_dev->ven_gpio, 1);
	usleep_range(10000, 11000);

	for (addr = 0x28; addr < 0x2C; addr++) {
		client->addr = addr;
		addrcnt = 2;

		do {
			ret = i2c_master_send(client, tmp, 4);
			if (ret > 0) {
				pr_info("%s : i2c addr=0x%X\n",
					__func__, client->addr);
				break;
			}
			msleep(20);
		} while (addrcnt--);

		if (ret > 0)
			break;
	}
	gpio_set_value(pn547_dev->ven_gpio, 0);

	if (ret < 0) {
		pr_err("%s : fail to get i2c addr\n", __func__);
		goto err_request_irq_failed;
	}

	pr_info("%s : success\n", __func__);
	return 0;

err_request_irq_failed:
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
err_create_workqueue:
#endif
	misc_deregister(&pn547_dev->pn547_device);
	wake_lock_destroy(&pn547_dev->nfc_wake_lock);
err_misc_register:
	mutex_destroy(&pn547_dev->read_mutex);
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	msm_xo_put(pn547_dev->nfc_clock);
err_get_clock:
#endif
	kfree(pn547_dev);
err_exit:
err_pvdd_en:
	gpio_free(platform_data->pvdd_en_gpio);
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	gpio_free(platform_data->clk_req_gpio);
err_clk_req:
#endif
	gpio_free(platform_data->firm_gpio);
err_firm:
	gpio_free(platform_data->ven_gpio);
err_ven:
	gpio_free(platform_data->irq_gpio);
	pr_err("[pn547] pn547_probe fail!\n");
	return ret;
}

static int pn547_remove(struct i2c_client *client)
{
	struct pn547_dev *pn547_dev;

	pn547_dev = i2c_get_clientdata(client);
#ifdef CONFIG_NFC_PN547_PMC8974_CLK_REQ
	if (pn547_dev->nfc_clk)
		clk_unprepare(pn547_dev->nfc_clk);
#endif
	wake_lock_destroy(&pn547_dev->nfc_wake_lock);
	free_irq(client->irq, pn547_dev);
	misc_deregister(&pn547_dev->pn547_device);
	mutex_destroy(&pn547_dev->read_mutex);
	gpio_free(pn547_dev->irq_gpio);
	gpio_free(pn547_dev->ven_gpio);
	gpio_free(pn547_dev->firm_gpio);
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	gpio_free(pn547_dev->clk_req_gpio);
	msm_xo_put(pn547_dev->nfc_clock);
#endif
	gpio_free(pn547_dev->pvdd_en_gpio);
	kfree(pn547_dev);

	return 0;
}

static const struct i2c_device_id pn547_id[] = {
	{"pn547", 0},
	{}
};

#ifdef CONFIG_OF
static struct of_device_id nfc_match_table[] = {
	{ .compatible = "nxp,pn547", },
	{},
};
#else
#define nfc_match_table NULL
#endif

static struct i2c_driver pn547_driver = {
	.id_table = pn547_id,
	.probe = pn547_probe,
	.remove = pn547_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "pn547",
		.of_match_table = nfc_match_table,
	},
};

/*
 * module load/unload record keeping
 */

static int __init pn547_dev_init(void)
{
	pr_info("Loading pn547 driver\n");
	return i2c_add_driver(&pn547_driver);
}

module_init(pn547_dev_init);

static void __exit pn547_dev_exit(void)
{
	pr_info("Unloading pn547 driver\n");
	i2c_del_driver(&pn547_driver);
}

module_exit(pn547_dev_exit);

MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC PN547 driver");
MODULE_LICENSE("GPL");

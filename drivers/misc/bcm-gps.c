/******************************************************************************
* Copyright (C) 2013 Broadcom Corporation
*
* @file   /kernel/drivers/misc/bcm-gps.c
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation version 2.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
******************************************************************************/
/*
 * Broadcom GPS driver which supports Geo-fence.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <mach/io_map.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/delay.h>
#include <linux/pm_wakeup.h>
#include <linux/spinlock.h>
#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>


#ifndef BCM_GPS_GEO_FENCE_HOST_WAKE_ASSERT
#define BCM_GPS_GEO_FENCE_HOST_WAKE_ASSERT 1
#endif

#ifndef BCM_GPS_GEO_FENCE_HOST_WAKE_DEASSERT
#define BCM_GPS_GEO_FENCE_HOST_WAKE_DEASSERT \
		(!(BCM_GPS_GEO_FENCE_HOST_WAKE_ASSERT))
#endif

#define BRCM_GPS_KERN_MAGIC			'G'

/* IOCTL requests */
#define GPS_CMD_GET_PORT_NAME	_IO(BRCM_GPS_KERN_MAGIC, 0x1)
#define GPS_CMD_GET_BAUD_RATE	_IO(BRCM_GPS_KERN_MAGIC, 0x2)
#define	GPS_CMD_VREG_PWR_ON		_IO(BRCM_GPS_KERN_MAGIC, 0x3)
#define	GPS_CMD_VREG_PWR_OFF	_IO(BRCM_GPS_KERN_MAGIC, 0x4)
#define	GPS_CMD_GEOFENCE_ON		_IO(BRCM_GPS_KERN_MAGIC, 0x5)
#define	GPS_CMD_GEOFENCE_OFF		_IO(BRCM_GPS_KERN_MAGIC, 0x6)
#define GPS_PORT_NAME_SIZE	128
#define GPS_VREG_ON		1
#define GPS_VREG_OFF		0

/* #define pr_debug  pr_err */

struct bcm_gps_struct {
	 struct miscdevice misc;
	spinlock_t bcm_gps_lock;
	int host_irq;
};

struct bcm_gps_platform_data {
	int host_wake_gpio;
	long int baudrate;
	char port_name[GPS_PORT_NAME_SIZE];
	char gpio_n_stdby_path[GPS_PORT_NAME_SIZE];
	int	 gpio_regpu;
};

struct bcm_gps_entry_struct {
	struct bcm_gps_platform_data *pdata;
	struct bcm_gps_struct *pgps;
	struct platform_device *pdev;
	struct wakeup_source *host_wake_ws;
	int gps_geofence_started;
};

struct bcm_gps_entry_struct *gps_g;

static struct pin_config uartb3_config[2] = {
	{
		.name = PN_UBRTSN,
		.func = PF_GPIO47,
		.reg.b = {
			.drv_sth = 3,
			.input_dis = 0,
			.slew_rate_ctrl = 0,
			.pull_up = 0,
			.pull_dn = 0,
			.hys_en = 0,
		},
	},
	{
		.name = PN_UBRTSN,
		.func = PF_UB3RTSN,
		.reg.b = {
			.drv_sth = 3,
			.input_dis = 0,
			.slew_rate_ctrl = 0,
			.pull_up = 1,
			.pull_dn = 0,
			.hys_en = 0,
		},
	},
};

/* Structure to exchange data between driver and the user */
struct command_par {
		unsigned long param1;     /**< extra parameter for the target */
		int error;				/**< return value. This can be enum later */
		void *buf;				/**< buffer */
		int size;				/* buffer size */
		unsigned long retval;	/**< actual return value */
		unsigned long retval2;  /**< actual return value (2) */
		/* If we see the data is more, we can go with a pointer which need
		* to be allocated by the user and filled up the kernel.
		*/
};

static irqreturn_t bcm_gps_host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	unsigned long flags;
	struct bcm_gps_entry_struct *priv;
	priv = (struct bcm_gps_entry_struct *)dev;
	if (priv == NULL) {
		pr_err(
		"%s GPS: Error data pointer is null\n",
							__func__);
		return IRQ_HANDLED;
	}
	spin_lock_irqsave(&priv->pgps->bcm_gps_lock, flags);

	host_wake = gpio_get_value(
			priv->pdata->host_wake_gpio);
	if (BCM_GPS_GEO_FENCE_HOST_WAKE_ASSERT == host_wake) {
		__pm_stay_awake(priv->host_wake_ws);
		pinmux_set_pin_config(&uartb3_config[1]);
	} else {
		__pm_relax(priv->host_wake_ws);
	}
	spin_unlock_irqrestore(&priv->pgps->bcm_gps_lock, flags);
	pr_debug("%s GPS:Exiting.\n", __func__);
	return IRQ_HANDLED;
}

/*static int bcm_gps_init_hostwake(struct bcm_gps_entry_struct *priv)
{
	int rc = -1;

	pr_debug("%s GPS:Entering.\n", __func__);
	if ((priv == NULL) ||
		(priv->pdata->host_wake_gpio < 0)) {
		pr_err("%s GPS: invalid host-wake-gpio.\n",
			__func__);
		return -EFAULT;
	}

	rc = devm_gpio_request_one(&priv->pdev->dev,
				priv->pdata->host_wake_gpio,
				GPIOF_OUT_INIT_LOW,
				"host_wake_gpio");

	if (rc) {
		pr_err
	    ("%s: failed to configure host-wake-gpio err=%d\n",
		     __func__, rc);
		return rc;
	}
	gpio_direction_input(priv->pdata->host_wake_gpio);

	pr_debug("%s GPS:Exiting.\n", __func__);
	return rc;
}
*/

static void bcm_gps_clean_host_wake(
				struct bcm_gps_entry_struct *priv)
{
	pr_debug("%s GPS:Entering.\n", __func__);
	if ((priv == NULL) ||
		 (priv->pdata->host_wake_gpio < 0)) {
		pr_err("%s GPS: NULL ptr or invalid hw gpio.\n",
			__func__);
		return;
	}

	free_irq(priv->pgps->host_irq, gps_g);
	pr_debug("%s GPS:Exiting.\n", __func__);
}


void bcm_gps_start(void)
{
	int rc = -1;

	pr_debug("%s GPS: Entering.\n", __func__);
	if (gps_g != NULL && gps_g->gps_geofence_started) {
		pr_debug("%s GPS: geofence already started.\n", __func__);
		return;
	}

	if (gps_g != NULL) {
		pr_debug(
		"%s: GPS: data pointer is valid\n", __func__);

		rc = gpio_to_irq(
			gps_g->pdata->host_wake_gpio);
		if (rc < 0) {
			pr_err
		("%s: failed to configure GPS Host Mgmt err=%d\n",
				__func__, rc);
			return;
		}
		gps_g->pgps->host_irq = rc;
		pr_debug("%s: GPS: request host_wake_irq=%d\n",
			__func__, gps_g->pgps->host_irq);

		rc = request_irq(
			gps_g->pgps->host_irq,
			bcm_gps_host_wake_isr,
			(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			IRQF_NO_SUSPEND), "gps_host_wake", gps_g);
		if (rc) {
			pr_err("%s: request irq failed: err=%d\n",
						 __func__, rc);
		}

		rc = irq_set_irq_wake(gps_g->pgps->host_irq, 1);
		if (rc) {
			pr_err("%s: irq_set_irq_wake failed: err=%d\n",
						 __func__, rc);
		}
		gps_g->gps_geofence_started = 1;
	} else {
		pr_err("%s: GPS:data ptr null, Uninitialized\n",
						__func__);
		return;
	}
	pr_debug("%s GPS:Exiting.\n", __func__);
	return;
}


static long bcm_gps_ker_unlocked_ioctl(struct file *filp,
					unsigned int cmd,
					unsigned long arg)
{
	int ret = 0;
	int size = 0;
	void __user *argp = (void __user *)arg;
	struct command_par *user_ptr = (struct command_par *) argp;

	pr_debug("ioctl in gps : 0x%x\n", (unsigned int)user_ptr);

	switch (cmd) {
	case GPS_CMD_GET_PORT_NAME:
		if (copy_to_user((char *)user_ptr->buf,
				&gps_g->pdata->port_name,
				strlen(gps_g->pdata->port_name))) {
			pr_err("GPS:%s copy_to_user failed for Port name\n",
				 __func__);
			ret = -1;
		}

		size = strlen(gps_g->pdata->port_name);
		if (copy_to_user((int *)&user_ptr->size, &size, sizeof(int))) {
			pr_err("GPS:%s copy_to_user failed for Port name\n",
				 __func__);
			ret = -1;

		}

		pr_err("%s: port_name = %s", __func__, gps_g->pdata->port_name);
	break;
	case GPS_CMD_GET_BAUD_RATE:
		pr_err("%s: baudrate =%ld", __func__, gps_g->pdata->baudrate);
		if (copy_to_user((unsigned long *)&user_ptr->retval,
				&gps_g->pdata->baudrate,
				sizeof(unsigned long))) {
			pr_err("GPS:%s copy_to_user failed for baudrate\n",
				__func__);
			ret = -1;
		}
	break;

	case GPS_CMD_VREG_PWR_ON:
		gpio_set_value(gps_g->pdata->gpio_regpu, GPS_VREG_ON);
	break;
	case GPS_CMD_VREG_PWR_OFF:
		gpio_set_value(gps_g->pdata->gpio_regpu, GPS_VREG_OFF);
	break;
	case GPS_CMD_GEOFENCE_ON:
		bcm_gps_start();
	break;
	case GPS_CMD_GEOFENCE_OFF:
		/*bcm_gps_stop();*/
	break;

	default:
		pr_debug("%s: GPS: default ioctl\n", __func__);
		break;
	}

	return ret;
}

static int my_open(struct inode *i, struct file *f)
{
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	return 0;
}

static const struct of_device_id bcm_gps_of_match[] = {
		{ .compatible = "bcm,bcm-gps",},
		{ /* Sentinel */ },
	};

MODULE_DEVICE_TABLE(of, bcm_gps_of_match);
static const struct file_operations bcm_gps_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.unlocked_ioctl = bcm_gps_ker_unlocked_ioctl,
};

static int bcm_gps_probe(struct platform_device *pdev)
{
//	u32 val;
	int rc = -EINVAL;
	const struct of_device_id *match = NULL;
	struct bcm_gps_platform_data *pdata = NULL;
//	const char *prop;


	pr_err("%s: Enter gps_probe\n", __func__);
	match = of_match_device(bcm_gps_of_match, &pdev->dev);
	if (!match)
		pr_err("%s: **ERROR** No matcing device found\n", __func__);


	if (!gps_g) {
		gps_g = devm_kzalloc(&pdev->dev,
				sizeof(*gps_g),
				GFP_KERNEL);

		if (gps_g == NULL) {
			pr_err("%s: gps_g =NULL", __func__);
			return -ENOMEM;
		}
		gps_g->pdata = devm_kzalloc(&pdev->dev,
				sizeof(
				struct bcm_gps_platform_data
				),
				GFP_KERNEL);
		gps_g->pgps  = devm_kzalloc(&pdev->dev,
				sizeof(struct bcm_gps_struct),
				GFP_KERNEL);

		if (gps_g->pgps == NULL)
			return -ENOMEM;

		gps_g->host_wake_ws = devm_kzalloc(&pdev->dev,
				sizeof(struct wakeup_source),
				GFP_KERNEL);

		if (gps_g->host_wake_ws == NULL)
			return -ENOMEM;

		spin_lock_init(&gps_g->pgps->bcm_gps_lock);
		gps_g->gps_geofence_started = 0;

		gps_g->pdev = pdev;
	}

	if (!match && pdev->dev.platform_data) {
		pdata = pdev->dev.platform_data;
		strncpy(gps_g->pdata->port_name, pdata->port_name,
				strlen(pdata->port_name));
		pr_err("%s: port_name=%s", __func__, pdata->port_name);
		gps_g->pdata->baudrate = pdata->baudrate;
		gps_g->pdata->gpio_regpu = pdata->gpio_regpu;
		gps_g->pdata->host_wake_gpio = pdata->host_wake_gpio;
		pr_err("%s: platform_data YES", __func__);
	} else if (pdev->dev.of_node) {
		pr_err("%s: device node", __func__);

		gps_g->pdata->host_wake_gpio =
			of_get_named_gpio(pdev->dev.of_node,
			"bcm-gps-hostwake", 0);
		if (!gpio_is_valid(gps_g->pdata->host_wake_gpio)) {
			pr_err("%s: invalid host_wake_gpio ", __func__);
			return -ENODEV;
		}
		gps_g->pdata->gpio_regpu =
			of_get_named_gpio(pdev->dev.of_node,
			"bcm-gps-regpu-gpio", 0);
		if (!gpio_is_valid(gps_g->pdata->gpio_regpu)) {
			pr_err("%s: invalid regpu gpio", __func__);
			return -ENODEV;
		}
		gps_g->pdev = pdev;
	} else {
		pr_err("%s: **ERROR** NO platform data available\n",
							__func__);
		rc = -ENODEV;
		goto out;
	}

	bcm_gps_start();

	/*pr_info("%s: 2 - host_wake_gpio=%d\n",
		__func__,
		gps_g->pdata->host_wake_gpio);
	pr_err("%s:  Ignore hostwake: no geofence for now", __func__);

	bcm_gps_init_hostwake(gps_g);

	gps_g->host_wake_ws = wakeup_source_register("bcm-gps-hostwake-ws");

	if (gps_g->host_wake_ws == NULL) {
		pr_err(
			"%s: failed to register host-wake wakeup source\n",
			__func__);
		return -ENODEV;
	}
	 */

	gps_g->pgps->misc.minor = MISC_DYNAMIC_MINOR;
	gps_g->pgps->misc.name = "bcm-gps";
	gps_g->pgps->misc.fops = &bcm_gps_fops;

	pr_err("%s, now register gps driver", __func__);
	rc = misc_register(&gps_g->pgps->misc);
	if (rc) {
		pr_err("gps misc_register failed\n");
		goto error_exit;
	}

	pr_debug("%s GPS:Exiting.\n", __func__);
	return rc;

error_exit:
		kfree(gps_g->pgps);
		kfree(gps_g->pdata);
		kfree(gps_g);

out:
	pr_debug("%s GPS:Exiting after cleaning.\n",
							__func__);
	return rc;
}

static int bcm_gps_remove(struct platform_device *pdev)
{
	if (gps_g == NULL)
		return 0;

	if (gps_g->pdata) {
		bcm_gps_clean_host_wake(gps_g);
		wakeup_source_unregister(gps_g->host_wake_ws);
	}
	if (gps_g != NULL) {
		if ((pdev->dev.of_node != NULL) &&
				(gps_g->pdata != NULL)) {
			kfree(gps_g->pdata);
			kfree(gps_g->pgps);
			kfree(gps_g->host_wake_ws);
		}
		kfree(gps_g);
		gps_g = NULL;
	}
	pinmux_set_pin_config(&uartb3_config[0]);
	pr_debug("%s GPS:Exiting.\n", __func__);
	return 0;
}

static struct platform_driver bcm_gps_platform_driver = {
	.probe = bcm_gps_probe,
	.remove = bcm_gps_remove,
	.driver = {
		   .name = "bcm-gps",
		   .owner = THIS_MODULE,
		   .of_match_table = bcm_gps_of_match,
		   },
};



static int __init bcm_gps_init(void)
{
	int rc = -1;

	pr_debug("%s GPS: Entering.\n", __func__);
	rc = platform_driver_register(&bcm_gps_platform_driver);
	if (rc)
		pr_err(
		"GPS: driver register failed err=%d, Exiting %s.\n",
							rc, __func__);
	else
		pr_debug(
		"GPS: Init success. Exiting %s.\n", __func__);
	return rc;
}

static void __exit bcm_gps_exit(void)
{
	platform_driver_unregister(&bcm_gps_platform_driver);
	pr_debug("%s GPS:Exiting.\n", __func__);
}

module_init(bcm_gps_init);
module_exit(bcm_gps_exit);

MODULE_DESCRIPTION("bcm-gps");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");


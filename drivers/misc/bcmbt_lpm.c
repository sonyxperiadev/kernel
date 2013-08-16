/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* @file	drivers/bluetooth/brcm_bzhw.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/broadcom/bcmbt_lpm.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/tty.h>
#include <linux/delay.h>

#include <linux/of.h>
#include <linux/of_platform.h>

#ifdef CONFIG_BCM_BT_LPM

#define TIMER_PERIOD 2
struct bcmbt_lpm_entry_struct *priv_g;

struct bcmbt_ldisc_data {
	int	(*open)(struct tty_struct *);
	void	(*close)(struct tty_struct *);
};

static struct tty_ldisc_ops bcmbt_ldisc_ops;
static struct bcmbt_ldisc_data bcmbt_ldisc_saved;

static int bcmbt_lpm_init_clock(struct bcmbt_lpm_entry_struct *priv)
{
	int ret;
	if (priv == NULL) {
		pr_err
		("%s BLUETOOTH: core structure not initialized\n", __func__);
		return -1;
	}
	ret =
	    pi_mgr_qos_add_request(&priv->plpm->bt_wake_qos_node,
				   "bt_qos_node_bw",
				   PI_MGR_PI_ID_ARM_SUB_SYSTEM,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret < 0) {
		pr_err
	("%s BLUETOOTH:bt_qosNode_bw addition failed\n", __func__);
		return -1;
	}

	ret =
	    pi_mgr_qos_add_request(&priv->plpm->host_wake_qos_node,
				   "bt_qos_node_hw",
				   PI_MGR_PI_ID_ARM_SUB_SYSTEM,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret < 0) {
		pr_err
	("%s BLUETOOTH:bt_qosNode_hw addition failed\n", __func__);
		return -1;
	}

	pr_debug("%s BLUETOOTH:bt_qos_node_bw, bt_qos_node_hw" \
				 "SUCCESFULL\n", __func__);
	return 0;
}

static void hw_timer_expire(void *data)
{
	unsigned int host_wake;
	unsigned long flags;

	struct bcmbt_lpm_entry_struct *priv =
			(struct bcmbt_lpm_entry_struct *)data;

	spin_lock_irqsave(&priv->plpm->bcmbt_lpm_lock, flags);
	host_wake = gpio_get_value(priv->pdata->host_wake_gpio);
	if (host_wake == BCMBT_LPM_HOST_WAKE_DEASSERT) {
		pi_mgr_qos_request_update(&priv->plpm->host_wake_qos_node,
					PI_MGR_QOS_DEFAULT_VALUE);
		if (wake_lock_active(&priv->plpm->host_wake_lock))
			wake_unlock(&priv->plpm->host_wake_lock);
	}
	/* else do nothing if host_wake == BCMBT_LPM_HOST_WAKE_ASSERT */
	priv->plpm->hw_timer_st = IDLE;
	spin_unlock_irqrestore(&priv->plpm->bcmbt_lpm_lock, flags);
}

int bcmbt_lpm_assert_bt_wake(void)
{
	int rc;

	if (priv_g == NULL)
		pr_err("%s BLUETOOTH: structure corrupted or not init" \
				" - cannot assert bt_wake\n", __func__);

	if (unlikely((priv_g->pdata->bt_wake_gpio == -1)))
		return -EFAULT;

#ifdef CONFIG_HAS_WAKELOCK
	if (priv_g != NULL) {
		if (!wake_lock_active(&priv_g->plpm->bt_wake_lock))
			wake_lock(&priv_g->plpm->bt_wake_lock);
	}
#endif
	rc = pi_mgr_qos_request_update(&priv_g->plpm->bt_wake_qos_node, 0);
	gpio_set_value(priv_g->pdata->bt_wake_gpio, BCMBT_LPM_BT_WAKE_ASSERT);

	pr_debug("%s BLUETOOTH:ASSERT BT_WAKE\n", __func__);
	return 0;
}

int bcmbt_lpm_deassert_bt_wake(void)
{
	if (priv_g == NULL)
		pr_err("%s BLUETOOTH: structure corrupted" \
			" or not init - cannot assert bt_wake\n", __func__);

	if (unlikely((priv_g->pdata->bt_wake_gpio == -1)))
		return -EFAULT;

	gpio_set_value(priv_g->pdata->bt_wake_gpio, BCMBT_LPM_BT_WAKE_DEASSERT);

	pr_debug("BLUETOOTH:DEASSERT BT_WAKE\n");
	pi_mgr_qos_request_update(&priv_g->plpm->bt_wake_qos_node,
				PI_MGR_QOS_DEFAULT_VALUE);
#ifdef CONFIG_HAS_WAKELOCK
	if (priv_g != NULL) {
		if (wake_lock_active(&priv_g->plpm->bt_wake_lock))
				wake_unlock(&priv_g->plpm->bt_wake_lock);
	}
#endif
	pr_debug("%s BLUETOOTH:Exiting\n", __func__);
	return 0;
}

static int bcmbt_lpm_init_bt_wake(struct bcmbt_lpm_entry_struct *priv)
{
	int rc;

	if (priv->pdata->bt_wake_gpio < 0) {
		pr_err("%s: bt_wake_gpio=%d\n",
			__func__, priv->pdata->bt_wake_gpio);
		return 0;
	}

	rc = gpio_request(priv->pdata->bt_wake_gpio, "BT Power Mgmt");
	if (rc) {
		pr_err
	("%s: failed to configure BT Power Mgmt: bt_wake_gpio request err%d\n",
		     __func__, rc);
		return rc;
	}

	rc = gpio_direction_output(priv->pdata->bt_wake_gpio,
				BCMBT_LPM_BT_WAKE_DEASSERT);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&priv->plpm->bt_wake_lock,
			WAKE_LOCK_SUSPEND, "BTWAKE");
#endif
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return 0;

}

static void bcmbt_tty_cleanup(void)
{
	int err;

	err = tty_unregister_ldisc(N_BRCM_HCI);
	if (err)
		pr_err("can't unregister N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline removed\n");
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
}

static void bcmbt_lpm_clean_bt_wake(struct bcmbt_lpm_entry_struct *priv,
			 bool b_tty)
{
	if (priv->pdata->bt_wake_gpio < 0)
		return;

	pi_mgr_qos_request_update(&priv->plpm->bt_wake_qos_node,
				    PI_MGR_QOS_DEFAULT_VALUE);

	gpio_free((unsigned)priv->pdata->bt_wake_gpio);

	if (b_tty)
		bcmbt_tty_cleanup();

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&priv->plpm->bt_wake_lock);
#endif
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
}

static irqreturn_t bcmbt_lpm_host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	unsigned long flags;

	struct bcmbt_lpm_entry_struct *priv =
				(struct bcmbt_lpm_entry_struct *)dev;
	if (priv == NULL) {
		pr_err("%s BLUETOOTH: Error data pointer is null\n", __func__);
		return IRQ_HANDLED;
	}
	spin_lock_irqsave(&priv->plpm->bcmbt_lpm_lock, flags);
	host_wake = gpio_get_value(priv->pdata->host_wake_gpio);
	if (BCMBT_LPM_HOST_WAKE_ASSERT == host_wake) {
		if (priv->plpm->hw_timer_st == IDLE) {
			/* wake up peripheral clock */
			if (!wake_lock_active(&priv->plpm->host_wake_lock))
				wake_lock(&priv->plpm->host_wake_lock);

			pi_mgr_qos_request_update(
				      &priv->plpm->host_wake_qos_node, 0);
		}
		/* else do nothing if state == ACTIVE */
		spin_unlock_irqrestore(&priv->plpm->bcmbt_lpm_lock, flags);
	} else {
		if (priv->plpm->hw_timer_st == IDLE) {
			mod_timer(&priv->plpm->hw_timer,
				jiffies + msecs_to_jiffies(TIMER_PERIOD));
			priv->plpm->hw_timer_st = ACTIVE;
		}
		/* else do nothing if state == ACTIVE */
		spin_unlock_irqrestore(&priv->plpm->bcmbt_lpm_lock, flags);
	}
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return IRQ_HANDLED;
}

static int bcmbt_lpm_init_hostwake(struct bcmbt_lpm_entry_struct *priv)
{
	int rc;
	if (priv->pdata->host_wake_gpio < 0)
		return 0;
	rc = gpio_request(priv->pdata->host_wake_gpio, "BT Host Power Mgmt");
	if (rc) {
		pr_err
	    ("%s: failed to configure BT Host Mgmt: gpio_request err=%d\n",
		     __func__, rc);
		return rc;
	}
	gpio_direction_input(priv->pdata->host_wake_gpio);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&priv->plpm->host_wake_lock,
			WAKE_LOCK_SUSPEND, "BTWAKE");
#endif

	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return 0;
}

static void bcmbt_lpm_clean_host_wake(struct bcmbt_lpm_entry_struct *priv)
{
	if (priv->pdata->host_wake_gpio == -1)
		return;

	free_irq(priv->plpm->host_irq, priv_g);
	/* delete timer */
	del_timer(&priv->plpm->hw_timer);
	priv->plpm->hw_timer_st = IDLE;
	gpio_free((unsigned)priv->pdata->host_wake_gpio);
	pi_mgr_qos_request_update(&priv->plpm->host_wake_qos_node,
				    PI_MGR_QOS_DEFAULT_VALUE);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&priv->plpm->host_wake_lock);
#endif
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
}


void bcmbt_lpm_start(void)
{
	int rc;

	if (priv_g != NULL) {
		pr_debug("%s: BLUETOOTH: data pointer is valid\n", __func__);

		rc = gpio_to_irq(priv_g->pdata->host_wake_gpio);
		if (rc < 0) {
			pr_err
		   ("%s: failed to configure BT Host Mgmt err=%d\n",
		     __func__, rc);
		   goto exit_lock_host_wake;
		}
		priv_g->plpm->host_irq = rc;
		pr_debug("%s: BLUETOOTH: request_irq host_irq=%d\n",
			__func__, priv_g->plpm->host_irq);

		init_timer(&priv_g->plpm->hw_timer);
		priv_g->plpm->hw_timer.function =
			(void (*) (unsigned long))hw_timer_expire;
		priv_g->plpm->hw_timer_st = IDLE;
		priv_g->plpm->hw_timer.data = (unsigned long)priv_g;

		rc = request_irq(
			priv_g->plpm->host_irq, bcmbt_lpm_host_wake_isr,
			 (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			  IRQF_NO_SUSPEND), "bt_host_wake", priv_g);
		if (rc) {
			pr_err
		("%s: failed to configure BT Host Mgmt:request_irq err=%d\n",
			     __func__, rc);
		}
		pi_mgr_qos_request_update(&priv_g->plpm->bt_wake_qos_node,
				   0);
	} else {
		pr_err("%s: BLUETOOTH:data pointer null, NOT initialized\n",
			__func__);
		return;
	}
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return;

exit_lock_host_wake:
	gpio_free((unsigned)priv_g->pdata->host_wake_gpio);
	return;
}


static int bcmbt_get_bt_wake_state(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	unsigned long tmp;

	if (unlikely((priv_g->pdata->bt_wake_gpio == -1)))
		return -EFAULT;

	tmp = gpio_get_value(priv_g->pdata->bt_wake_gpio);

	pr_info("bcmbt_get_bt_wake_state(bt_wake:%d), gpio_get_value(tmp:%lu)\n",
		priv_g->pdata->bt_wake_gpio, tmp);

	if (copy_to_user(uarg, &tmp, sizeof(*uarg)))
		return -EFAULT;
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return 0;
}

static int bcmbt_tty_ioctl(struct tty_struct *tty, struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	int rc = -1;

	switch (cmd) {
	case TIO_ASSERT_BT_WAKE:
		rc = bcmbt_lpm_assert_bt_wake();
		break;

	case TIO_DEASSERT_BT_WAKE:
		rc = bcmbt_lpm_deassert_bt_wake();
		break;

	case TIO_GET_BT_WAKE_STATE:
		rc = bcmbt_get_bt_wake_state(arg);
		break;
	case TIO_GET_BT_UART_PORT:
		pr_err("%s: BLUETOOTH:Enter switch TIO_GET_BT_UART_PORT" \
					 " => Just break\n", __func__);
		break;
	case TIO_GET_BT_FIRMWARE:
		pr_err("%s: BLUETOOTH:Enter switch TIO_GET_BT_FIRMWARE" \
					 " => Just break\n", __func__);
		break;
	default:
		pr_debug("%s: BLUETOOTH: switch default\n", __func__);
		return n_tty_ioctl_helper(tty, file, cmd, arg);

	}
	pr_debug("%s: BLUETOOTH:Exit bcmbt_tty_ioctl, cmd=%d\n", __func__, cmd);
	return rc;
}

static int bcmbt_tty_open(struct tty_struct *tty)
{
	struct uart_state *state;

	state = tty->driver_data;
	priv_g->plpm->uport = state->uart_port;

	bcmbt_lpm_init_bt_wake(priv_g);
	bcmbt_lpm_init_hostwake(priv_g);
	bcmbt_lpm_start();
	pr_debug("bcmbt_tty_open()::open(): x%p", bcmbt_ldisc_saved.open);

	if (bcmbt_ldisc_saved.open)
		return bcmbt_ldisc_saved.open(tty);
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return 0;
}

static void bcmbt_tty_close(struct tty_struct *tty)
{
	struct uart_state *state;

	if (!priv_g || !priv_g->plpm) {
		pr_err("bcmbt_tty_close(): driver data already " \
		  "freed? priv_g: 0x%p, p_lpm: 0x%p", priv_g,
						 priv_g->plpm);
	   return;
	}

	priv_g->plpm->uport = 0;
	state = tty->driver_data;

	/* do not free tty ldisc as we are called from ldisc */
	bcmbt_lpm_clean_bt_wake(priv_g, false);
	bcmbt_lpm_clean_host_wake(priv_g);

	pr_debug("bcmbt_tty_close(line: %d)::close(): x%p",
			state->uart_port->line, bcmbt_ldisc_saved.close);

	if (bcmbt_ldisc_saved.close)
		bcmbt_ldisc_saved.close(tty);
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return;
}

static int bcmbt_tty_init(void)
{
	int err;
	pr_debug("%s: BLUETOOTH:\n", __func__);

	/* Inherit the N_TTY's ops */
	n_tty_inherit_ops(&bcmbt_ldisc_ops);

	bcmbt_ldisc_ops.owner = THIS_MODULE;
	bcmbt_ldisc_ops.name = "bcmbt_tty";
	bcmbt_ldisc_ops.ioctl = bcmbt_tty_ioctl;
	bcmbt_ldisc_saved.open = bcmbt_ldisc_ops.open;
	bcmbt_ldisc_saved.close = bcmbt_ldisc_ops.close;
	bcmbt_ldisc_ops.open = bcmbt_tty_open;
	bcmbt_ldisc_ops.close = bcmbt_tty_close;

	err = tty_register_ldisc(N_BRCM_HCI, &bcmbt_ldisc_ops);
	if (err)
		pr_err("can't register N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline registered\n");

	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return err;
}

static int bcmbt_lpm_probe(struct platform_device *pdev)
{
	int rc = 0;
	int val;
	struct bcmbt_platform_data *pdata = NULL;

	pr_debug("%s BLUETOOTH:Enter probe.\n", __func__);

	priv_g = kzalloc(sizeof(*priv_g), GFP_ATOMIC);
	if (!priv_g)
		return -ENOMEM;
	priv_g->pdata = kzalloc(sizeof(struct bcmbt_platform_data),
						    GFP_ATOMIC);
	if (!priv_g->pdata) {
		kfree(priv_g);
		return -ENOMEM;
	}
	priv_g->plpm  = kzalloc(sizeof(struct bcmbt_lpm_struct),
						    GFP_ATOMIC);
	if (!priv_g->plpm) {
		kfree(priv_g->pdata);
		kfree(priv_g);
		return -ENOMEM;
	}
	spin_lock_init(&priv_g->plpm->bcmbt_lpm_lock);

	/* Initialize clocks once */
	bcmbt_lpm_init_clock(priv_g);

	if (pdev->dev.platform_data) {
		pdata = pdev->dev.platform_data;
		priv_g->pdata->bt_wake_gpio   = pdata->bt_wake_gpio;
		priv_g->pdata->host_wake_gpio = pdata->host_wake_gpio;
		priv_g->pdata->bt_uart_port   = pdata->bt_uart_port;

	} else if (pdev->dev.of_node) {

		if (priv_g->pdata == NULL)
			goto clean_data;

		if (of_property_read_u32(pdev->dev.of_node,
				    "bt-wake-gpio", &val))
			goto clean_data;

		priv_g->pdata->bt_wake_gpio = val;

		if (of_property_read_u32(pdev->dev.of_node,
				    "host-wake-gpio", &val))
			goto clean_data;

		priv_g->pdata->host_wake_gpio = val;

		if (of_property_read_u32(pdev->dev.of_node,
				    "bt-uart-port", &val))
			goto clean_data;

		priv_g->pdata->bt_uart_port = val;
	} else {
		pr_err("%s: **ERROR** NO platform data available\n", __func__);
		goto clean_data;
	}

	pr_info("%s: bt_wake_gpio=%d, host_wake_gpio=%d, bt_uart_port=%d\n",
		__func__, priv_g->pdata->bt_wake_gpio,
		priv_g->pdata->host_wake_gpio, priv_g->pdata->bt_uart_port);

	/* register the tty line discipline driver */
	rc = bcmbt_tty_init();

	if (rc) {
		pr_err("%s: bcmbt_tty_init failed\n", __func__);
		kfree(priv_g->pdata);
		kfree(priv_g->plpm);
		kfree(priv_g);
		priv_g = 0;
		return -1;
	}

	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return 0;

clean_data:
	kfree(priv_g->pdata);
	kfree(priv_g->plpm);
	kfree(priv_g);
	pr_debug("%s BLUETOOTH:Exiting after cleaning.\n", __func__);
	return rc;
}

static int bcmbt_lpm_remove(struct platform_device *pdev)
{
	if (priv_g == NULL)
		return 0;

	if (priv_g->pdata) {
		bcmbt_lpm_clean_bt_wake(priv_g, true);
		bcmbt_lpm_clean_host_wake(priv_g);
		if (pi_mgr_qos_request_remove(&priv_g->plpm->bt_wake_qos_node))
			pr_info("%s failed to unregister " \
				"qos_bw client\n", __func__);
		if (pi_mgr_qos_request_remove(
				&priv_g->plpm->host_wake_qos_node))
			pr_info("%s failed to unregister" \
				"qos_hw client\n", __func__);
	}
	if (priv_g != NULL) {
		if ((pdev->dev.of_node != NULL) && (priv_g->pdata != NULL)) {
			kfree(priv_g->pdata);
			kfree(priv_g->plpm);
		}
		kfree(priv_g);
	}
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return 0;
}

static int bcmbt_lpm_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (priv_g == NULL)
		return 0;

	if (priv_g->plpm->uport != NULL) {
		pr_debug("%s BLUETOOTH:Calling togglerts with value 0" \
			" = enable successfully\n", __func__);
	}
	return 0;
}

static int bcmbt_lpm_resume(struct platform_device *pdev)
{
	if (priv_g == NULL)
		return 0;

	if (priv_g->plpm->uport != NULL) {
		pr_debug("%s BLUETOOTH:Calling togglerts with value 1" \
			" = enable successfully\n", __func__);
	}
	return 0;
}

static const struct of_device_id bcmbt_lpm_match[] = {
		{ .compatible = "bcm,btlpm",},
		{ /* Sentinel */ },
	};
MODULE_DEVICE_TABLE(of, bcmbt_lpm_match);

static struct platform_driver bcmbt_lpm_platform_driver = {
	.probe = bcmbt_lpm_probe,
	.remove = bcmbt_lpm_remove,
	.suspend = bcmbt_lpm_suspend,
	.resume = bcmbt_lpm_resume,
	.driver = {
		   .name = "bcmbt-lpm",
		   .owner = THIS_MODULE,
		   .of_match_table = bcmbt_lpm_match,
		   },
};

static int __init bcmbt_lpm_init(void)
{
	int rc = platform_driver_register(&bcmbt_lpm_platform_driver);
	if (rc)
		pr_err("%s bcmbt_lpm_platform_driver_register failed err=%d\n",
			__func__, rc);
	else
		pr_info("%s bcmbt_lpm_init success\n", __func__);

	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
	return rc;
}

static void __exit bcmbt_lpm_exit(void)
{
	platform_driver_unregister(&bcmbt_lpm_platform_driver);
	pr_debug("%s BLUETOOTH:Exiting.\n", __func__);
}

module_init(bcmbt_lpm_init);
module_exit(bcmbt_lpm_exit);

MODULE_DESCRIPTION("bcmbt-lpm");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_BRCM_HCI);
#endif

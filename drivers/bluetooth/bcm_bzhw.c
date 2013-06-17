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
#include <linux/broadcom/bcm_bzhw.h>
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

#ifdef CONFIG_BCM_BZHW
#define BZHW_CLOCK_ENABLE 1
#define BZHW_CLOCK_DISABLE 0

#ifndef BZHW_BT_WAKE_ASSERT
#define BZHW_BT_WAKE_ASSERT 1
#endif
#ifndef BZHW_BT_WAKE_DEASSERT
#define BZHW_BT_WAKE_DEASSERT (!(BZHW_BT_WAKE_ASSERT))
#endif

#ifndef BZHW_HOST_WAKE_ASSERT
#define BZHW_HOST_WAKE_ASSERT 1
#endif
#ifndef BZHW_HOST_WAKE_DEASSERT
#define BZHW_HOST_WAKE_DEASSERT (!(BZHW_HOST_WAKE_ASSERT))
#endif

#define TIMER_PERIOD 4

/* BZHW states */
enum bzhw_states_e {
	BZHW_ASLEEP,
	BZHW_ASLEEP_TO_AWAKE,
	BZHW_AWAKE,
	BZHW_AWAKE_TO_ASLEEP
};

struct bcmbzhw_struct *priv_g;
struct timer_list sleep_timer_bw;

static int bcm_bzhw_init_clock(struct bcmbzhw_struct *priv)
{
	int ret;
	if (priv == NULL) {
		pr_err
		("%s BLUETOOTH: core structure not initialized\n", __func__);
		return -1;
	}
	ret =
	    pi_mgr_qos_add_request(&priv->qos_node, "bt_qos_node",
				   PI_MGR_PI_ID_ARM_SUB_SYSTEM,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret < 0) {
		pr_err
	("%s BLUETOOTH:bt_qosNode addition failed\n", __func__);
		return -1;
	}
	pr_debug("%s BLUETOOTH:bt_qosNode SUCCESFULL\n", __func__);
	return 0;
}

void bcm_bzhw_timer_bt_wake(unsigned long data)
{
	int btwake, hostwake, ret;
	struct bcmbzhw_struct *priv = (struct bcmbzhw_struct *)data;
	pr_debug("%s BLUETOOTH:\n", __func__);
	btwake = gpio_get_value(priv->bzhw_data.gpio_bt_wake);
	hostwake = gpio_get_value(priv->bzhw_data.gpio_host_wake);

	if (hostwake == BZHW_HOST_WAKE_DEASSERT
		&& btwake == BZHW_BT_WAKE_DEASSERT) {
		pr_debug
		("%s BLUETOOTH:DEASSERT BT_WAKE off RTS shut UART clk\n",
		__func__);

		pi_mgr_qos_request_update(&priv->qos_node,
				      PI_MGR_QOS_DEFAULT_VALUE);
		priv->bzhw_state = BZHW_ASLEEP;
		del_timer(&sleep_timer_bw);

	if (wake_lock_active(&priv_g->bzhw_data.bt_wake_lock))
		wake_unlock(&priv_g->bzhw_data.bt_wake_lock);
	} else {
		pr_debug("%s BLUETOOTH: state changed restart timer\n",
		__func__);
		ret = timer_pending(&sleep_timer_bw);
		if (ret) {
			pr_debug("%s: Bluetooth: Is timer pending : Yes\n",
			__func__);
		} else {
			pr_debug("%s: Bluetooth: Is timer pending : No\n",
				__func__);
			ret = mod_timer(&sleep_timer_bw,
				  jiffies + 2*TIMER_PERIOD*HZ);
			if (ret)
				pr_debug("%s: Bluetooth: Active timer restarted\n",
				__func__);
			else
				pr_debug("%s: Bluetooth: Inactive timer restarted\n",
				__func__);
		}
	}
}

void bcm_bzhw_timer_host_wake(unsigned long data)
{
	int hostwake;
	int ret;
	struct bcmbzhw_struct *priv = (struct bcmbzhw_struct *)data;
	pr_debug("%s BLUETOOTH:\n", __func__);
	if (priv->bzhw_state == BZHW_AWAKE_TO_ASLEEP ||
		priv->bzhw_state == BZHW_ASLEEP_TO_AWAKE) {
		pr_debug("%s BLUETOOTH: state is Awake to Asleep\n", __func__);
		hostwake = gpio_get_value(priv->bzhw_data.gpio_host_wake);
		if (hostwake == BZHW_HOST_WAKE_DEASSERT) {
			pr_debug("%s BLUETOOTH: Asleep\n", __func__);
			pi_mgr_qos_request_update(&priv->qos_node,
				      PI_MGR_QOS_DEFAULT_VALUE);
			priv->bzhw_state = BZHW_ASLEEP;
			del_timer(&priv->sleep_timer_hw);
			if (wake_lock_active(&priv->bzhw_data.host_wake_lock))
				wake_unlock(&priv->bzhw_data.host_wake_lock);

		} else {
			pr_err("%s BLUETOOTH: state changed Asleep to Awake\n",
				__func__);
			priv->bzhw_state = BZHW_ASLEEP_TO_AWAKE;
			ret = timer_pending(&priv->sleep_timer_hw);
		if (ret) {
			pr_debug("%s: Bluetooth: Is timer pending : Yes\n",
				__func__);
		} else {
			pr_debug("%s: Bluetooth: Is timer pending : No\n",
				__func__);
			ret = mod_timer(&priv->sleep_timer_hw,
				  jiffies + TIMER_PERIOD*HZ);
			if (ret)
				pr_debug("%s: Bluetooth: Active timer deleted\n",
					__func__);
		       else
				pr_debug("%s: Bluetooth: Inactive timer deleted\n",
					__func__);
		   }
		}
	} else if (priv->bzhw_state == BZHW_ASLEEP) {
		pr_debug("%s BLUETOOTH: Already sleeping make sure qos node is released\n",
			__func__);
		pi_mgr_qos_request_update(&priv->qos_node,
			PI_MGR_QOS_DEFAULT_VALUE);
		del_timer(&priv->sleep_timer_hw);
			if (wake_lock_active(&priv->bzhw_data.host_wake_lock))
				wake_unlock(&priv->bzhw_data.host_wake_lock);
	} else {
	hostwake = gpio_get_value(priv->bzhw_data.gpio_host_wake);
	if (hostwake == BZHW_HOST_WAKE_DEASSERT) {
		pr_debug("%s BLUETOOTH: change state to Awake to Asleep\n",
			__func__);
		priv->bzhw_state = BZHW_AWAKE_TO_ASLEEP;
		ret = timer_pending(&priv->sleep_timer_hw);
		if (ret) {
			pr_debug("%s: Bluetooth: Is timer pending : Yes\n",
				__func__);
		} else {
		pr_debug("%s: Bluetooth: Is timer pending : No\n",
			__func__);
		ret = mod_timer(&priv->sleep_timer_hw,
			jiffies + 2*TIMER_PERIOD*HZ);
		if (ret)
			pr_debug("%s: Bluetooth: Active timer deleted\n",
				__func__);
		 else
			pr_debug("%s: Bluetooth: Inactive timer deleted\n",
					__func__);
		}
	} else if (hostwake == BZHW_HOST_WAKE_ASSERT) {
			pr_debug("%s BLUETOOTH: change state to Awake\n",
				__func__);
		if (!(wake_lock_active(&priv->bzhw_data.host_wake_lock)))
			wake_lock(&priv->bzhw_data.host_wake_lock);
			pi_mgr_qos_request_update(&priv->qos_node,
				      0);
			priv->bzhw_state = BZHW_AWAKE;
		} else {
			pr_debug("%s BLUETOOTH: Timer restarted\n", __func__);
			ret = timer_pending(&priv->sleep_timer_hw);
			if (ret) {
				pr_debug("%s: Bluetooth: Is timer pending : Yes\n",
					__func__);
			} else {
				pr_debug("%s: Bluetooth: Is timer pending:No\n",
					__func__);
				ret = mod_timer(&priv->sleep_timer_hw,
					  jiffies + 4*TIMER_PERIOD*HZ);
				if (ret)
					pr_debug("%s: Bluetooth: Active timer deleted\n",
						__func__);
				else
					pr_debug("%s: Bluetooth: Inactive timer deleted\n",
						__func__);
			}
		}
	}
}


void bcm_bzhw_request_clock_on(struct pi_mgr_qos_node *node)
{
	int ret;
	pr_debug("%s: bcm_bzhw_request_clock_on\n", __func__);
	if (node == NULL) {
		pr_err("%s: Node passed is null\n", __func__);
		return;
	}
	ret = pi_mgr_qos_request_update(node, 0);
	if (ret == 0)
		pr_debug
	("%s BLUETOOTH:UART OUT of retention\n", __func__);
	else
		pr_err
	("%s BLUETOOTH: UART OUT OF retention FAILED\n", __func__);

}
void bcm_bzhw_request_clock_off(struct pi_mgr_qos_node *node)
{
	int ret;
	if (node == NULL) {
		pr_err("%s: PI node passed is null\n", __func__);
		return;
	}
	pr_debug("%s: bcm_bzhw_request_clock_off\n", __func__);
	ret = pi_mgr_qos_request_update(node, PI_MGR_QOS_DEFAULT_VALUE);
	if (ret == 0)
		pr_debug
	("%s BLUETOOTH:UART in retention\n", __func__);
	else
		pr_err
	("%s BLUETOOTH:UART NOT in retention\n", __func__);

}

int bcm_bzhw_assert_bt_wake(int bt_wake_gpio, struct pi_mgr_qos_node *lqos_node,
				struct tty_struct *tty)
{
	struct uart_state *state;
	struct uart_port *port;
	int rc;
	if (lqos_node == NULL || tty == NULL) {
		pr_err("%s: Null pointers passed is null\n", __func__);
		return -1;
	}
	state = tty->driver_data;
	port = state->uart_port;
	gpio_set_value(bt_wake_gpio, BZHW_BT_WAKE_ASSERT);
	pr_debug("%s BLUETOOTH:ASSERT BT_WAKE\n", __func__);
	rc = pi_mgr_qos_request_update(lqos_node, 0);
	if (priv_g != NULL) {
		if (!wake_lock_active(&priv_g->bzhw_data.bt_wake_lock))
			wake_lock(&priv_g->bzhw_data.bt_wake_lock);
	}
	return 0;
}

int bcm_bzhw_deassert_bt_wake(int bt_wake_gpio, int host_wake_gpio)
{
	int ret;
	pr_debug("%s BLUETOOTH:DEASSERT BT_WAKE\n", __func__);
	gpio_set_value(bt_wake_gpio, BZHW_BT_WAKE_DEASSERT);
	if (gpio_get_value(host_wake_gpio) == BZHW_HOST_WAKE_DEASSERT) {
		ret = timer_pending(&sleep_timer_bw);
		if (ret) {
			pr_debug("%s: Bluetooth: Is timer pending Yes, so do nothing\n",
			__func__);
		if (priv_g != NULL) {
			if (wake_lock_active(&priv_g->bzhw_data.bt_wake_lock))
				wake_unlock(&priv_g->bzhw_data.bt_wake_lock);
			}
		} else {
			pr_debug("%s: Bluetooth: Is timer pending : No\n",
				__func__);
			ret = mod_timer(&sleep_timer_bw,
				  jiffies + 3*TIMER_PERIOD*HZ);
			if (ret)
				pr_debug("%s: Bluetooth: Active timer modified\n",
				__func__);
			else
				pr_debug("%s: Bluetooth: Inactive timer modified\n",
				__func__);
		}
	}
	return 0;
}

static int bcm_bzhw_init_bt_wake(struct bcmbzhw_struct *priv)
{
	int rc;

	if (priv->pdata->gpio_bt_wake < 0) {
		pr_debug("%s: gpio_bt_wake=%d\n",
			__func__, priv->pdata->gpio_bt_wake);
		return -EINVAL;
	}

	rc = gpio_request(priv->pdata->gpio_bt_wake, "BT Power Mgmt");
	if (rc) {
		pr_err
	("%s: failed to configure BT Power Mgmt: gpio_request err%d\n",
		     __func__, rc);
		return rc;
	}

	rc = gpio_direction_output(priv->pdata->gpio_bt_wake,
				BZHW_BT_WAKE_DEASSERT);
	wake_lock_init(&priv->bzhw_data.bt_wake_lock,
			WAKE_LOCK_SUSPEND, "BTWAKE");
	priv->bzhw_data.gpio_bt_wake = priv->pdata->gpio_bt_wake;
	return rc;

}

static void bcm_bzhw_clean_bt_wake(struct bcmbzhw_struct *priv)
{
	if (priv->pdata->gpio_bt_wake < 0)
		return;

	wake_lock_destroy(&priv->bzhw_data.bt_wake_lock);
	gpio_free((unsigned)priv->pdata->gpio_bt_wake);
	priv->bzhw_data.gpio_bt_wake = -1;

}

static irqreturn_t bcm_bzhw_host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;
	unsigned long flags;
	int ret = -1;
	struct bcmbzhw_struct *priv = (struct bcmbzhw_struct *)dev;
	if (priv == NULL) {
		pr_err("%s BLUETOOTH: Error data pointer is null\n", __func__);
		return IRQ_HANDLED;
	}
	spin_lock_irqsave(&priv->bzhw_lock, flags);
	host_wake = gpio_get_value(priv->bzhw_data.gpio_host_wake);
	if (BZHW_HOST_WAKE_ASSERT == host_wake) {
		pr_debug("%s BLUETOOTH: hostwake ISR Assert\n", __func__);
		/* wake up peripheral clock */
	if (!wake_lock_active(&priv->bzhw_data.host_wake_lock))
		wake_lock(&priv->bzhw_data.host_wake_lock);
		ret = pi_mgr_qos_request_update(&priv->qos_node, 0);
		priv->bzhw_state = BZHW_AWAKE;
		spin_unlock_irqrestore(&priv->bzhw_lock, flags);
	} else {
		pr_debug("%s BLUETOOTH: hostwake ISR DeAssert\n", __func__);
		if (priv->bzhw_state == BZHW_ASLEEP) {
			spin_unlock_irqrestore(&priv->bzhw_lock, flags);
		} else {
			priv->bzhw_state = BZHW_AWAKE_TO_ASLEEP;
			mod_timer(&priv->sleep_timer_hw,
				jiffies + TIMER_PERIOD*HZ);
		spin_unlock_irqrestore(&priv->bzhw_lock, flags);
		}
	}
	return IRQ_HANDLED;
}

static int bcm_bzhw_init_hostwake(struct bcmbzhw_struct *priv)
{
	int rc;
	if (priv->pdata->gpio_host_wake < 0)
		return -EINVAL;
	wake_lock_init(&priv->bzhw_data.host_wake_lock, WAKE_LOCK_SUSPEND,
		       "HOSTWAKE");

	rc = gpio_request(priv->pdata->gpio_host_wake, "BT Host Power Mgmt");
	if (rc) {
		pr_err
	    ("%s: failed to configure BT Host Mgmt: gpio_request err=%d\n",
		     __func__, rc);
		return rc;
	}
	rc = gpio_direction_input(priv->pdata->gpio_host_wake);
	rc = bcm_bzhw_init_clock(priv);
	priv->bzhw_data.gpio_host_wake = priv->pdata->gpio_host_wake;

	return rc;
}

static void bcm_bzhw_clean_host_wake(struct bcmbzhw_struct *priv)
{
	if (priv->pdata->gpio_host_wake == -1)
		return;
	if (priv->bzhw_data.host_irq >= 0) {
		free_irq(priv->bzhw_data.host_irq, bcm_bzhw_host_wake_isr);
		priv->bzhw_data.host_irq = -1;
	}

	wake_lock_destroy(&priv->bzhw_data.host_wake_lock);
	gpio_free((unsigned)priv->pdata->gpio_host_wake);
	priv->bzhw_data.gpio_host_wake = -1;
}

struct bcmbzhw_struct *bcm_bzhw_start(struct tty_struct* tty)
{
	struct uart_state *state;
	struct uart_port *port;
	int rc;
	if (priv_g != NULL) {
		pr_debug("%s: BLUETOOTH: data pointer is valid\n", __func__);
		priv_g->bcmtty = tty;
		state = tty->driver_data;
		port = state->uart_port;
		priv_g->uport = port;
		rc = gpio_to_irq(priv_g->bzhw_data.gpio_host_wake);
		if (rc < 0) {
			pr_err
		   ("%s: failed to configure BT Host Mgmt err=%d\n",
		     __func__, rc);
		   goto exit_lock_host_wake;
		}
		priv_g->bzhw_data.host_irq = rc;
		priv_g->bzhw_state = BZHW_AWAKE;
		init_timer(&priv_g->sleep_timer_hw);
		priv_g->sleep_timer_hw.expires =
				jiffies + 2*TIMER_PERIOD*HZ;
		priv_g->sleep_timer_hw.data = (unsigned long)priv_g;
		priv_g->sleep_timer_hw.function = bcm_bzhw_timer_host_wake;

		init_timer(&sleep_timer_bw);
		sleep_timer_bw.expires =
			jiffies + 3*TIMER_PERIOD*HZ;
		sleep_timer_bw.data = (unsigned long)priv_g;
		sleep_timer_bw.function = bcm_bzhw_timer_bt_wake;
		pr_debug("%s: BLUETOOTH: request_irq host_irq=%d\n",
			__func__, priv_g->bzhw_data.host_irq);
		rc = request_irq(
			priv_g->bzhw_data.host_irq, bcm_bzhw_host_wake_isr,
			 (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			  IRQF_NO_SUSPEND), "bt_host_wake", priv_g);
		 if (rc) {
			pr_err
		("%s: failed to configure BT Host Mgmt:request_irq err=%d\n",
			     __func__, rc);
		}
		if (priv_g->uport != NULL)
			serial8250_togglerts_afe(priv_g->uport, 1);

		return priv_g;
	} else {
		pr_err("%s: BLUETOOTH:data pointer null, NOT initialized\n",
			__func__);
		return NULL;
	}
exit_lock_host_wake:
	wake_lock_destroy(&priv_g->bzhw_data.host_wake_lock);
	gpio_free((unsigned)priv_g->pdata->gpio_host_wake);
	return NULL;

}


void bcm_bzhw_stop(struct bcmbzhw_struct *hw_val)
{
	if (hw_val == NULL)
		return;
	pr_debug("%s: BLUETOOTH: hw_val->bzhw_data.host_irq=%d\n",
		__func__, hw_val->bzhw_data.host_irq);
	hw_val->bzhw_state = BZHW_ASLEEP;
	del_timer(&hw_val->sleep_timer_hw);
	del_timer(&sleep_timer_bw);
	pi_mgr_qos_request_update(&hw_val->qos_node,
				    PI_MGR_QOS_DEFAULT_VALUE);

	if (wake_lock_active(&hw_val->bzhw_data.host_wake_lock))
		wake_unlock(&hw_val->bzhw_data.host_wake_lock);

	if (wake_lock_active(&hw_val->bzhw_data.bt_wake_lock))
		wake_unlock(&hw_val->bzhw_data.bt_wake_lock);
	if (hw_val->bzhw_data.host_irq >= 0)
		free_irq(hw_val->bzhw_data.host_irq, hw_val);

}

static int bcm_bzhw_probe(struct platform_device *pdev)
{

	int rc = 0;
	u32 val;

	priv_g = kzalloc(sizeof(*priv_g), GFP_ATOMIC);
	if (!priv_g)
		return -ENOMEM;

	spin_lock_init(&priv_g->bzhw_lock);

	if (pdev->dev.platform_data) {
		priv_g->pdata =
		(struct bcm_bzhw_platform_data *)pdev->dev.platform_data;
	} else if (pdev->dev.of_node) {
		priv_g->pdata = kzalloc(sizeof(struct bcm_bzhw_platform_data),
					GFP_ATOMIC);
		if (priv_g->pdata == NULL)
			goto error1;

		if (of_property_read_u32(pdev->dev.of_node,
					"bt-wake-gpio",
					&val))
			goto error2;
		priv_g->pdata->gpio_bt_wake = val;

		if (of_property_read_u32(pdev->dev.of_node,
					"host-wake-gpio",
					&val))
			goto error2;
		priv_g->pdata->gpio_host_wake = val;
	} else {
		pr_err("%s: **ERROR** NO platform data available\n", __func__);
		goto error1;
	}

	priv_g->bzhw_data.gpio_bt_wake = -1;
	priv_g->bzhw_data.gpio_host_wake = -1;
	priv_g->bzhw_data.host_irq = -1;
	pr_info("%s: gpio_bt_wake=%d, gpio_host_wake=%d\n", __func__,
		priv_g->pdata->gpio_bt_wake, priv_g->pdata->gpio_host_wake);

	priv_g->uport = NULL;
	priv_g->bzhw_state = BZHW_ASLEEP;
	rc = bcm_bzhw_init_bt_wake(priv_g);
	rc = bcm_bzhw_init_hostwake(priv_g);

	if (rc)
		goto error2;

	return 0;
error2:
	if (pdev->dev.of_node != NULL)
		kfree(priv_g->pdata);
error1:
	kfree(priv_g);
	return rc;
}

static int bcm_bzhw_remove(struct platform_device *pdev)
{
	if (priv_g == NULL)
		return 0;

	if (priv_g->pdata) {
		del_timer(&priv_g->sleep_timer_hw);
		bcm_bzhw_clean_bt_wake(priv_g);
		bcm_bzhw_clean_host_wake(priv_g);
		if (pi_mgr_qos_request_remove(&priv_g->qos_node))
			pr_info("%s failed to unregister qos client\n",
				__func__);
	}

	if (priv_g != NULL) {
		if ((pdev->dev.of_node != NULL) && (priv_g->pdata != NULL))
			kfree(priv_g->pdata);
		kfree(priv_g);
	}
	return 0;
}

static int bcm_bzhw_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (priv_g == NULL)
		return 0;
	if (priv_g->uport != NULL)
		serial8250_togglerts_afe(priv_g->uport, 0);
	return 0;
}

static int bcm_bzhw_resume(struct platform_device *pdev)
{
	if (priv_g == NULL)
		return 0;
	if (priv_g->uport != NULL)
		serial8250_togglerts_afe(priv_g->uport, 1);
	return 0;
}

static const struct of_device_id bcm_bzhw_match[] = {
	{ .compatible = "bcm,bzhw"},
	{ /* Sentinel */ }
};

static struct platform_driver bcm_bzhw_platform_driver = {
	.probe = bcm_bzhw_probe,
	.remove = bcm_bzhw_remove,
	.suspend = bcm_bzhw_suspend,
	.resume = bcm_bzhw_resume,
	.driver = {
		   .name = "bcm_bzhw",
		   .owner = THIS_MODULE,
		   .of_match_table = bcm_bzhw_match,
		   },
};

static int __init bcm_bzhw_init(void)
{
	int rc = platform_driver_register(&bcm_bzhw_platform_driver);
	if (rc)
		pr_err("%s bcm_bzhw_platform_driver_register failed err=%d\n",
			__func__, rc);
	else
		pr_info("%s bcm_bzhw_init success\n", __func__);

	return rc;
}

static void __exit bcm_bzhw_exit(void)
{
	platform_driver_unregister(&bcm_bzhw_platform_driver);
}

module_init(bcm_bzhw_init);
module_exit(bcm_bzhw_exit);

MODULE_DESCRIPTION("bcm_bzhw");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
#endif

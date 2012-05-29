/*****************************************************************************
* Copyright 2006 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>

#include <mach/usbh_cfg.h>

#include "bcm_usbh.h"

#define DEBUG
#ifdef DEBUG
#define dbg_printk(fmt, args...) printk(KERN_INFO "%s: " fmt, __func__, ## args)
#else
#define dbg_printk(fmt, args...)
#endif

struct usbh_ctrl_regs {
	u32 mode;
#define MODE_ULPI_TTL (1<<0)
#define MODE_ULPI_PHY (1<<1)
#define MODE_UTMI_TTL (1<<2)
#define MODE_UTMI_PHY (1<<3)
#define MODE_PORT_CFG(port, mode) ((mode) << (4 * port))

	u32 strap_q;
#define STRAP_PWR_STATE_VALID                (1 << 7)    /* ss_power_state_valid */
#define STRAP_SIM_MODE                       (1 << 6)    /* ss_simulation_mode */
#define STRAP_OHCI_CNTSEL_SIM                (1 << 5)    /* ohci_0_cntsel_i_n */
#define STRAP_PWR_STATE_NXT_VALID            (1 << 4)    /* ss_nxt_power_state_valid_i */
#define STRAP_PWR_STATE_NXT_SHIFT            2           /* ss_next_power_state_i */
#define STRAP_PWR_STATE_NXT_MASK             (3 << STRAP_PWR_STATE_NXT_SHIFT)
#define STRAP_PWR_STATE_SHIFT                0           /* ss_power_state_i */
#define STRAP_PWR_STATE_MASK                 (3 << STRAP_PWR_STATE_SHIFT)

	u32 framelen_adj_q;
	u32 framelen_adj_qx[USBH_NUM_PORTS];       
	u32 misc;
#define MISC_RESUME_R23_ENABLE               (1 << 4) /* ss_utmi_backward_enb_i */
#define MISC_RESUME_R23_UTMI_PLUS_DISABLE    (1 << 3) /* ss_resume_utmi_pls_dis_i */
#define MISC_ULPI_BYPASS_ENABLE              (1 << 2) /* ulpi_bypass_en_i */
#define MISC_PORT_PWRDWN_OVERCURRENT         (1 << 1) /* ss_autoppd_on_overcur_en_i */
#define MISC_OHCI_CLK_RESTART                (1 << 0) /* app_start_clk_i */

};

struct usbh_priv {
	atomic_t probe_done;
	volatile int init_cnt;
	struct mutex lock;
	struct device *dev;
	struct usbh_cfg hw_cfg;
	struct clk *peri_clk;
	struct clk *ahb_clk;
	struct clk *opt_clk;
	struct usbh_ctrl_regs __iomem *ctrl_regs;
};

static struct usbh_priv usbh_data;

int bcm_usbh_suspend(unsigned int host_index)
{
	return 0;
}
EXPORT_SYMBOL(bcm_usbh_suspend);

int bcm_usbh_resume(unsigned int host_index)
{
	return 0;
}
EXPORT_SYMBOL(bcm_usbh_resume);

static int usbh_clk_ctrl(struct usbh_priv *drv_data, int enable)
{
	int ret;
	struct usbh_cfg *hw_cfg = &drv_data->hw_cfg;

	if (enable) {
		/* peripheral clock */
		if (hw_cfg->peri_clk_name) {
			drv_data->peri_clk = clk_get(drv_data->dev, hw_cfg->peri_clk_name);
			if (IS_ERR_OR_NULL(drv_data->peri_clk))
				return -EINVAL;
			ret = clk_enable(drv_data->peri_clk);
			if (ret)
				goto err_put_peri_clk;
		}

		/* AHB clock */
		if (hw_cfg->ahb_clk_name) {
			drv_data->ahb_clk = clk_get(drv_data->dev, hw_cfg->ahb_clk_name);
			if (IS_ERR_OR_NULL(drv_data->ahb_clk)) {
				ret = EINVAL;
				goto err_disable_peri_clk;
			}
			ret = clk_enable(drv_data->ahb_clk);
			if (ret)
				goto err_put_ahb_clk;
		}

		/* optional clock (in the USB host case, that's the 12 MHz clock */
		if (hw_cfg->opt_clk_name) {
			drv_data->opt_clk = clk_get(drv_data->dev, hw_cfg->opt_clk_name);
			if (IS_ERR_OR_NULL(drv_data->opt_clk)) {
				ret = -EINVAL;
				goto err_disable_ahb_clk;
			}
			ret = clk_enable(drv_data->opt_clk);
			if (ret)
				goto err_put_opt_clk;
		}
	
		return 0;

err_put_opt_clk:
		if (drv_data->opt_clk) {
			clk_put(drv_data->ahb_clk);
			drv_data->ahb_clk = NULL;
		}
err_disable_ahb_clk:
		if (drv_data->ahb_clk)
			clk_disable(drv_data->ahb_clk);
err_put_ahb_clk:
		if (drv_data->ahb_clk) {
			clk_put(drv_data->ahb_clk);
			drv_data->ahb_clk = NULL;
		}
err_disable_peri_clk:
		if (drv_data->peri_clk)
			clk_disable(drv_data->peri_clk);
err_put_peri_clk:
		if (drv_data->peri_clk) {
			clk_put(drv_data->peri_clk);
			drv_data->peri_clk = NULL;
		}
			
		return ret;
	}
	else {
		if (drv_data->peri_clk) {
			clk_disable(drv_data->peri_clk);
			clk_put(drv_data->peri_clk);
			drv_data->peri_clk = NULL;
		}
		if (drv_data->ahb_clk) {
			clk_disable(drv_data->ahb_clk);
			clk_put(drv_data->ahb_clk);
			drv_data->ahb_clk = NULL;
		}
		if (drv_data->opt_clk) {
			clk_disable(drv_data->opt_clk);
			clk_put(drv_data->opt_clk);
			drv_data->opt_clk = NULL;
		}
	}
	return 0;
}

/*
 * Function to initialize USB host related low level hardware including PHY,
 * clocks, etc.
 *
 * TODO: expand support for more than one host in the future if needed
 */
int bcm_usbh_init(unsigned int host_index)
{
	int i, ret;
	u32 tmp;
	struct usbh_priv *drv_data = &usbh_data;
	struct usbh_cfg *hw_cfg = &drv_data->hw_cfg;

	if (atomic_read(&drv_data->probe_done) == 0)
		return -ENODEV;

	BUG_ON(drv_data->init_cnt < 0);

	mutex_lock(&drv_data->lock);

	if (++drv_data->init_cnt > 1) {
		mutex_unlock(&drv_data->lock);
		return 0;
	}

	/* enable clocks */
	ret = usbh_clk_ctrl(drv_data, 1);
	if (ret) {
		dev_err(drv_data->dev, "unable to enable one of the USBH clocks\n");
		goto err_exit;
	}

	for (i = 0; i < hw_cfg->num_ports; i++) {
		struct usbh_port_cfg *port = &hw_cfg->port[i];

		if (port->pwr_gpio >= 0) {
			ret = gpio_request(port->pwr_gpio, "usbh power");
			if (ret < 0)
				goto err_free_gpio;
			gpio_direction_output(port->pwr_gpio, 0);
			/* power enable */
			gpio_set_value(port->pwr_gpio, 1);
		}

		if (port->pwr_flt_gpio >= 0) {
			ret = gpio_request(port->pwr_flt_gpio, "usbh power fault");
			if (ret < 0)
				goto err_free_gpio;
			gpio_direction_input(port->pwr_flt_gpio);
		}

		if (port->reset_gpio >= 0) {
			ret = gpio_request(port->reset_gpio, "usbh ulpi reset");
			if (ret < 0)
				goto err_free_gpio;
			gpio_direction_output(port->reset_gpio, 1);

			/* now reset the ULPI interface */
			mdelay(10);
			gpio_set_value(port->reset_gpio, 0);
			mdelay(10);
			gpio_set_value(port->reset_gpio, 1);
			mdelay(10);
		}
	}

	/* configure each port to be in ULPI PHY mode */
	tmp = 0;
	for (i = 0; i < hw_cfg->num_ports; i++)
		tmp |= MODE_PORT_CFG(i, MODE_ULPI_PHY);
	writel(tmp, &drv_data->ctrl_regs->mode);

	writel(MISC_RESUME_R23_ENABLE | MISC_RESUME_R23_UTMI_PLUS_DISABLE,
			&drv_data->ctrl_regs->misc);
	writel(STRAP_PWR_STATE_VALID, &drv_data->ctrl_regs->strap_q);

	mutex_unlock(&drv_data->lock);

	dbg_printk("reg info:\n");
	dbg_printk(" Mode: 0x%08x", readl(&drv_data->ctrl_regs->mode));
	dbg_printk(" Strap: 0x%08x", readl(&drv_data->ctrl_regs->strap_q));
	dbg_printk(" Misc: 0x%08x", readl(&drv_data->ctrl_regs->misc));

	return 0;

err_free_gpio:
	for (i = 0; i < hw_cfg->num_ports; i++) {
		struct usbh_port_cfg *port = &hw_cfg->port[i];

		if (port->pwr_gpio >= 0) {
			/* disable port power */
			gpio_set_value(port->pwr_gpio, 0);
			gpio_free(port->pwr_gpio);
		}

		if (port->pwr_flt_gpio >= 0)
			gpio_free(port->pwr_flt_gpio);

		if (port->reset_gpio >= 0)
			gpio_free(port->reset_gpio);
	}

//err_disable_clk:
	usbh_clk_ctrl(drv_data, 0);

err_exit:
	drv_data->init_cnt--;
	mutex_unlock(&drv_data->lock);
	return ret;
}
EXPORT_SYMBOL(bcm_usbh_init);
	
/*
 * Function to terminate USB host related low level hardware including PHY,
 * clocks, etc.
 *
 * TODO: expand support for more than one host in the future if needed
 */
int bcm_usbh_term(unsigned int host_index)
{
	int i;
	u32 tmp;
	struct usbh_priv *drv_data = &usbh_data;
	struct usbh_cfg *hw_cfg = &drv_data->hw_cfg;

	if (atomic_read(&drv_data->probe_done) == 0)
		return -ENODEV;

	BUG_ON(drv_data->init_cnt < 0);

	mutex_lock(&drv_data->lock);
	
	if (--drv_data->init_cnt != 0) {
		mutex_unlock(&drv_data->lock);
		return 0;
	}

	/* free all GPIOs */
	for (i = 0; i < hw_cfg->num_ports; i++) {
		struct usbh_port_cfg *port = &hw_cfg->port[i];

		if (port->pwr_gpio >= 0) {
			/* disable port power */
			gpio_set_value(port->pwr_gpio, 0);
			gpio_free(port->pwr_gpio);
		}

		if (port->pwr_flt_gpio >= 0)
			gpio_free(port->pwr_flt_gpio);

		if (port->reset_gpio >= 0)
			gpio_free(port->reset_gpio);
	}

	tmp = readl(&drv_data->ctrl_regs->strap_q);
	tmp &= ~STRAP_PWR_STATE_VALID;
	writel(tmp, &drv_data->ctrl_regs->strap_q);

	/* disable clocks */
	usbh_clk_ctrl(drv_data, 0);
	mutex_unlock(&drv_data->lock);
	return 0;
}
EXPORT_SYMBOL(bcm_usbh_term);
	
static int __devinit usbh_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *iomem, *ioarea;

	memset(&usbh_data, 0, sizeof(usbh_data));

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "platform_data missing\n");
		ret = -EFAULT;
		goto err_exit;
	}
	memcpy(&usbh_data.hw_cfg, pdev->dev.platform_data,
			sizeof(usbh_data.hw_cfg));
	usbh_data.dev = &pdev->dev;
	
	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		dev_err(&pdev->dev, "no mem resource\n");
		ret = -ENODEV;
		goto err_exit;
	}

	/* mark the memory region as used */
	ioarea = request_mem_region(iomem->start, resource_size(iomem),
			pdev->name);
	if (!ioarea) {
		dev_err(&pdev->dev, "memory region already claimed\n");
		ret = -EBUSY;
		goto err_exit;
	}

	/* now map the I/O memory */
	usbh_data.ctrl_regs = (struct usbh_ctrl_regs __iomem *)
		ioremap(iomem->start, sizeof(usbh_data.ctrl_regs));
	if (!usbh_data.ctrl_regs) {
		dev_err(&pdev->dev, "failed to remap registers\n");
		ret = -ENOMEM;
		goto err_free_mem_region;
	}

	platform_set_drvdata(pdev, &usbh_data);
	mutex_init(&usbh_data.lock);
	usbh_data.init_cnt = 0;
	atomic_set(&usbh_data.probe_done, 1);

	return 0;

err_free_mem_region:
	release_mem_region(iomem->start, resource_size(iomem));

err_exit:
	memset(&usbh_data, 0, sizeof(usbh_data));
	return ret;
}

static int __devexit usbh_remove(struct platform_device *pdev)
{
	struct usbh_priv *drv_data = platform_get_drvdata(pdev);
	struct resource *iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	atomic_set(&drv_data->probe_done, 0);
	platform_set_drvdata(pdev, NULL);
	iounmap(drv_data->ctrl_regs);
	release_mem_region(iomem->start, resource_size(iomem));
	memset(&usbh_data, 0, sizeof(usbh_data));

	return 0;
}

static struct platform_driver usbh_driver = 
{
	.driver = {
		.name = "usbh",
		.owner = THIS_MODULE,
	},
	.probe   = usbh_probe,
	.remove  = usbh_remove,
};

static int __init usbh_init(void)
{
	return platform_driver_register(&usbh_driver);
}

static void __exit usbh_exit(void)
{
	platform_driver_unregister(&usbh_driver);
}

module_init(usbh_init);
module_exit(usbh_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom USB host low-level driver");
MODULE_LICENSE("GPL");

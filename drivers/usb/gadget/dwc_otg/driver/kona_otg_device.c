/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
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

/****************************************************************************/
/**
*  @file    bcmhana_otg_device.c
*
*  @brief   Instantiates an OTG device instance
*
*  @note    This is intended as a temporary solution until DWC OTG driver
*           can support multiple device instances.
*
*/
/****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/io.h>

#include <mach/lm.h>
#include <mach/irqs.h>
#include <linux/clk.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_hsotg.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
#ifdef DEBUG
	#define DBG_PRINT            printk
#else
	#define DBG_PRINT(fmt...)	do {} while (0)
#endif

#define	PHY_MODE_OTG		2
#define	PHY_MODE_DEVICE		1
#define	PHY_MODE_HOST		0
#define	BC11_OVR_KEY		0x2AAB

/* ---- Private Function Prototypes -------------------------------------- */
static void __exit dwc_otg_device_exit(void);
static int __init  dwc_otg_device_init(void);
static int __init  dwc_otg_device_register( unsigned irq, unsigned base_addr );

/* ---- Private Variables ------------------------------------------------ */

static unsigned int fshost = 0;
#if !defined(CONFIG_USB_DWC_OTG_HOST_MODE)
static unsigned int otghost = 0;
#else
static unsigned int otghost = 1;
#endif
static unsigned int otgdevice = 0;

static struct lm_device *lmdev = NULL;
static struct clk *otg_clk;

/*-------------------------------------------------------------------------*/

/* ==== Public Functions ================================================= */

/****************************************************************************
 * Module level definitions used to load / unload the DWC OTG Device
 ***************************************************************************/
module_param( fshost, uint, 0644 );
MODULE_PARM_DESC(fshost, "Load FSHOST device instead of HSOTG device");

module_param( otghost, uint, 0644 );
MODULE_PARM_DESC(otghost, "Force OTG host mode - Needed for FPGA v6.n images and later");

module_param( otgdevice, uint, 0644 );
MODULE_PARM_DESC(otgdevice, "Force OTG device mode - Needed for FPGA v6.n images and later");

MODULE_DESCRIPTION("DWC OTG Device");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.91a");

module_init(dwc_otg_device_init);
module_exit(dwc_otg_device_exit);


/* ==== Private Functions ================================================= */
static int __init otghost_setup(char *str)
{
	get_option(&str, &otghost);
	return 1;
}
__setup("otghost=", otghost_setup);

/****************************************************************************
 *
 ***************************************************************************/
static void __exit dwc_otg_device_exit(void)
{
	if ( lmdev != NULL ) {
		/* The lmdev memory will get freed as side effect of the
		* lm_device_unregister(), so don't do it here.
		*/
		lm_device_unregister(lmdev);
		lmdev = NULL;
	}
	if (otg_clk) {
		clk_disable(otg_clk);
		clk_put (otg_clk);
	}
}

static void kona_otg_phy_set_vbus_stat(void *hsotg_ctrl_base, bool on)
{
	unsigned long val;

	val = readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	if (on) {
		val |= (HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
			HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK);
	} else {
		val &= ~(HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
			 HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK);
	}

	writel(val, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
}

static void kona_otg_phy_set_non_driving(void *hsotg_ctrl_base, bool on)
{
	unsigned long val;

	/* set Phy to driving mode */
	val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);

	if (on)
		val |= HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK;
	else
		val &= ~HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK;

	writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
}

/**
 * Do OTG init. Currently only for testing dataline pulsing when Vbus is off
 */
static ssize_t do_konaotginit(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	void __iomem *hsotg_ctrl_base;

	/* map base address */
	hsotg_ctrl_base = ioremap (HSOTG_CTRL_BASE_ADDR, SZ_4K);
	if (!hsotg_ctrl_base) {
		return -ENOMEM;
	}

	kona_otg_phy_set_vbus_stat(hsotg_ctrl_base, false);
	kona_otg_phy_set_non_driving(hsotg_ctrl_base, true);
	kona_otg_phy_set_non_driving(hsotg_ctrl_base, false);

	/* unmap base address */
	iounmap(hsotg_ctrl_base);
	return count;
	
}

DEVICE_ATTR(konaotginit, S_IWUSR, NULL, do_konaotginit);

static ssize_t dump_konahsotgctrl(struct device *dev, 
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	void __iomem *hsotg_ctrl_base;

	/* map base address */
	hsotg_ctrl_base = ioremap (HSOTG_CTRL_BASE_ADDR, SZ_4K);
	if (!hsotg_ctrl_base) {
		return -ENOMEM;
	}
	printk("\nusbotgcontrol: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET));
	printk("\nphy_cfg: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_CFG_OFFSET));
	printk("\nphy_p1ctl: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET));
	printk("\nbc11_status: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_BC11_STATUS_OFFSET));
	printk("\nbc11_cfg: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_BC11_CFG_OFFSET));
	printk("\ntp_in: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_TP_IN_OFFSET));
	printk("\ntp_out: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_TP_OUT_OFFSET));
	printk("\nphy_ctrl: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_CTRL_OFFSET));
	printk("\nusbreg: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_USBREG_OFFSET));
	printk("\nusbproben: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_USBPROBEN_OFFSET));

	/* unmap base address */
	iounmap(hsotg_ctrl_base);

	return sprintf(buf, "\nkonahsotgctrl register dump");
}
DEVICE_ATTR(konahsotgctrldump, S_IRUSR, dump_konahsotgctrl, NULL);

/****************************************************************************
 *
 ***************************************************************************/
static int __init dwc_otg_device_init(void)
{
	int rc;

	if ( lmdev != NULL ) {
		rc = -EBUSY;
	}
//	else if ( fshost ) {
//		rc = dwc_otg_device_register (BCM_INT_ID_USB_FSHOST, FSHOST_BASE_ADDR);
//	}
	else {
		void __iomem *hsotg_ctrl_base;
		int val;
		unsigned long rate;

		printk("\n%s: Setting up USB OTG PHY and Clock\n", __func__);

#ifndef CONFIG_ARCH_SAMOA
		otg_clk = clk_get(NULL, "usb_otg_clk");
		if (!otg_clk) {
			printk("%s: error get clock\n", __func__);
			return -EIO;
		}

		rc = clk_enable(otg_clk);
		if (rc) {
			printk("%s: error enable clock\n", __func__);
			return -EIO;
		}
		rate = clk_get_rate(otg_clk);
		printk("usb_otg_clk rate %lu\n", rate);
#endif

		/* map base address */
		hsotg_ctrl_base = ioremap (HSOTG_CTRL_BASE_ADDR, SZ_4K);
		if (!hsotg_ctrl_base) {
			return -ENOMEM;
		}

		val = readl(hsotg_ctrl_base + HSOTG_CTRL_BC11_STATUS_OFFSET);
		if (val & HSOTG_CTRL_BC11_STATUS_BC_DONE_MASK){
			printk ("bc11 done\n");
		}
		else {
			printk ("bc11 not done\n");
		}
		/* force turn off VDP, enable sw_ovwr_set to take over the bc11 switches directly */
		val = (BC11_OVR_KEY<<HSOTG_CTRL_BC11_CFG_BC11_OVWR_KEY_SHIFT)
			| HSOTG_CTRL_BC11_CFG_SW_OVWR_EN_MASK;
		writel(val, hsotg_ctrl_base + HSOTG_CTRL_BC11_CFG_OFFSET);

		schedule_timeout_interruptible(HZ/1000*160);// Allow time for switches to disengage.

		/* clear bit 15 RDB error */
		val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
		val &= ~HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK;
		writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
		schedule_timeout_interruptible(HZ/10);

		/* set Phy to driving mode */
		val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
		val &= ~HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK;
		writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);

		schedule_timeout_interruptible(HZ/10);

		/* S/W reset Phy, actively low */
		val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
		val &= ~HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK;
		val &= ~HSOTG_CTRL_PHY_P1CTL_PHY_MODE_MASK;
		//val |= (otghost?PHY_MODE_HOST:PHY_MODE_DEVICE) << HSOTG_CTRL_PHY_P1CTL_PHY_MODE_SHIFT;	// use host or device mode
		val |= PHY_MODE_OTG << HSOTG_CTRL_PHY_P1CTL_PHY_MODE_SHIFT;			// use OTG mode
		writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);

		schedule_timeout_interruptible(HZ/10);

		/* bring Phy out of reset */
		val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
		val |= HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK;
		writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);

		schedule_timeout_interruptible(HZ/10);

		/* set the phy to functional state */
		val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_CFG_OFFSET);
		val &= ~HSOTG_CTRL_PHY_CFG_PHY_IDDQ_I_MASK;
		writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_CFG_OFFSET);

		schedule_timeout_interruptible(HZ/10);
		
		if (otghost) {
			printk(KERN_WARNING "%s: Set HSOTG_CTRL register for host mode\n", __func__);

			val = HSOTG_CTRL_USBOTGCONTROL_OTGSTAT2_MASK |
				HSOTG_CTRL_USBOTGCONTROL_OTGSTAT1_MASK |
				HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
				HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK |
				HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_MASK |
				HSOTG_CTRL_USBOTGCONTROL_USB_HCLK_EN_DIRECT_MASK |
				HSOTG_CTRL_USBOTGCONTROL_USB_ON_IS_HCLK_EN_MASK |
				HSOTG_CTRL_USBOTGCONTROL_USB_ON_MASK |
				HSOTG_CTRL_USBOTGCONTROL_PRST_N_SW_MASK |
				HSOTG_CTRL_USBOTGCONTROL_HRESET_N_SW_MASK |
				HSOTG_CTRL_USBOTGCONTROL_SOFT_PHY_RESETB_MASK |
				HSOTG_CTRL_USBOTGCONTROL_SOFT_DLDO_PDN_MASK |
				HSOTG_CTRL_USBOTGCONTROL_SOFT_ALDO_PDN_MASK;
			writel(val, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
		} else if (otgdevice) {
			printk(KERN_WARNING "%s: Set HSOTG_CTRL register for device mode\n", __func__);

			val= HSOTG_CTRL_USBOTGCONTROL_OTGSTAT2_MASK |
				HSOTG_CTRL_USBOTGCONTROL_OTGSTAT1_MASK |
				HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
				HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK |
				HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_MASK |
				HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK |
				HSOTG_CTRL_USBOTGCONTROL_USB_HCLK_EN_DIRECT_MASK |
				HSOTG_CTRL_USBOTGCONTROL_USB_ON_IS_HCLK_EN_MASK |
				HSOTG_CTRL_USBOTGCONTROL_USB_ON_MASK |
				HSOTG_CTRL_USBOTGCONTROL_PRST_N_SW_MASK |
				HSOTG_CTRL_USBOTGCONTROL_HRESET_N_SW_MASK |
				HSOTG_CTRL_USBOTGCONTROL_SOFT_PHY_RESETB_MASK |
				HSOTG_CTRL_USBOTGCONTROL_SOFT_DLDO_PDN_MASK |
				HSOTG_CTRL_USBOTGCONTROL_SOFT_ALDO_PDN_MASK;
			writel(val, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

		} else {
			val = readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
                        val |= (HSOTG_CTRL_USBOTGCONTROL_USB_HCLK_EN_DIRECT_MASK |
                                 HSOTG_CTRL_USBOTGCONTROL_USB_ON_IS_HCLK_EN_MASK);
                         writel(val, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
		}

		printk(KERN_WARNING "%s: HSOTG_CTRL_USBOTGCONTROL register=0x%08x\n",
			__func__, readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET));
		
		schedule_timeout_interruptible(HZ/10*3);

#if 0
		{
			int i;
			printk ("HSOTG_CTRL register\n");
			for (i=0; i<0x28; i+=4) {
				printk ("%x:	%08x\n", i, readl(hsotg_ctrl_base + i));
			}
		}
#endif
		rc = dwc_otg_device_register(BCM_INT_ID_USB_HSOTG, HSOTG_BASE_ADDR);

		if (NULL != lmdev)
			rc = device_create_file(&lmdev->dev, &dev_attr_konaotginit);

		if (!rc)
			rc = device_create_file(&lmdev->dev, &dev_attr_konahsotgctrldump);

		/* unmap base address */
		iounmap(hsotg_ctrl_base);

		printk("\n%s: Setup USB OTG PHY and Clock Completed\n", __func__);
	}

	return (rc);
}

/****************************************************************************
 *
 ***************************************************************************/
static int __init  dwc_otg_device_register( unsigned irq, unsigned base_addr )
{
	int rc = 0;

	lmdev = kmalloc(sizeof(struct lm_device), GFP_KERNEL);

	if ( lmdev == NULL ) {
		printk(KERN_ERR "dwc_otg_device_register(): kmalloc() failed\n");
		return (-ENOMEM);
	}

	memset( lmdev, 0 , sizeof(struct lm_device));

	lmdev->id  = -2;
	lmdev->irq = irq;
	lmdev->resource.flags = IORESOURCE_MEM;
	lmdev->resource.start = base_addr;
	lmdev->resource.end   = lmdev->resource.start + SZ_64K - 1;
	DBG_PRINT(KERN_ERR "dwc_otg_device_register(): irq=%d start=0x%08x end=0x%08x\n", lmdev->irq, lmdev->resource.start, lmdev->resource.end );

	if ( (rc = lm_device_register(lmdev)) < 0 ) {
		kfree(lmdev);
		lmdev = NULL;
	}

	return (rc);
}

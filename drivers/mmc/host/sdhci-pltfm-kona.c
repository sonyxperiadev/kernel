/*
 * sdhci-pltfm.c Support for SDHCI platform devices
 * Copyright (c) 2009 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Supports:
 * SDHCI platform devices specific for KONA
 *
 * Inspired by sdhci-pci.c, by Pierre Ossman
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/mmc/host.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>

#include <mach/sdio_platform.h>
#include "sdhci.h"

#ifdef CONFIG_APANIC_ON_MMC
#include <linux/mmc-poll/mmc_poll_stack.h>
#endif

#define SDHCI_SOFT_RESET            0x01000000

#define KONA_SDHOST_CORECTRL        0x8000
#define KONA_SDHOST_CD_PINCTRL      0x00000008
#define KONA_SDHOST_STOP_HCLK       0x00000004
#define KONA_SDHOST_RESET           0x00000002
#define KONA_SDHOST_EN              0x00000001

#define KONA_SDHOST_CORESTAT        0x8004
#define KONA_SDHOST_CD_SW           0x00000001

#define KONA_SDHOST_COREIMR         0x8008
#define KONA_SDHOST_IP              0x00000001

#define KONA_SDHOST_COREISR         0x800C
#define KONA_SDHOST_COREIMSR        0x8010
#define KONA_SDHOST_COREDBG1        0x8014
#define KONA_SDHOST_COREGPO_MASK    0x8018

#define DEV_NAME                "sdio"
#define MAX_DEV_NAME_SIZE       20

#define MAX_PROC_BUF_SIZE       256
#define MAX_PROC_NAME_SIZE      20
#define PROC_GLOBAL_PARENT_DIR  "sdhci-pltfm"
#define PROC_ENTRY_CARD_CTRL    "cardCtrl"

#define SD_DETECT_GPIO_DEBOUNCE_128MS	128

struct procfs {
	char name[MAX_PROC_NAME_SIZE];
	struct proc_dir_entry *parent;
};

struct sdio_dev {
	atomic_t initialized;
	struct device *dev;
	struct sdhci_host *host;
	unsigned long clk_hz;
	enum sdio_devtype devtype;
	int cd_gpio;
	struct sdio_wifi_gpio_cfg *wifi_gpio;
	struct procfs proc;
	struct clk *peri_clk;
	struct clk *sleep_clk;
	struct regulator * vddo_sd_regulator;
};

#ifdef CONFIG_MACH_BCM2850_FPGA
/* user can specify the clock when this driver is installed */
static unsigned int clock = 0;
module_param(clock, uint, 0444);
#endif

static struct proc_dir_entry *gProcParent;
static struct sdio_dev *gDevs[SDIO_DEV_TYPE_MAX];

#ifdef CONFIG_MACH_KONA_FPGA
int sdhci_pltfm_clk_enable(struct sdhci_host *host, int enable) { }
#else
int sdhci_pltfm_clk_enable(struct sdhci_host *host, int enable);
#endif
static int sdhci_pltfm_regulator_init(struct sdio_dev *dev, char * reg_name);

int sdio_dev_is_initialized(enum sdio_devtype devtype);



#define DRIVER_NAME "sdio"
/*
static void sdhci_dumpregs(struct sdhci_host *host)
{
	printk(KERN_DEBUG DRIVER_NAME ": ============== REGISTER DUMP ==============\n");

	printk(KERN_DEBUG DRIVER_NAME ": Sys addr: 0x%08x | Version:  0x%08x\n",
		sdhci_readl(host, SDHCI_DMA_ADDRESS),
		sdhci_readw(host, SDHCI_HOST_VERSION));
	printk(KERN_DEBUG DRIVER_NAME ": Blk size: 0x%08x | Blk cnt:  0x%08x\n",
		sdhci_readw(host, SDHCI_BLOCK_SIZE),
		sdhci_readw(host, SDHCI_BLOCK_COUNT));
	printk(KERN_DEBUG DRIVER_NAME ": Argument: 0x%08x | Trn mode: 0x%08x\n",
		sdhci_readl(host, SDHCI_ARGUMENT),
		sdhci_readw(host, SDHCI_TRANSFER_MODE));
	printk(KERN_DEBUG DRIVER_NAME ": Present:  0x%08x | Host ctl: 0x%08x\n",
		sdhci_readl(host, SDHCI_PRESENT_STATE),
		sdhci_readb(host, SDHCI_HOST_CONTROL));
	printk(KERN_DEBUG DRIVER_NAME ": Power:    0x%08x | Blk gap:  0x%08x\n",
		sdhci_readb(host, SDHCI_POWER_CONTROL),
		sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
	printk(KERN_DEBUG DRIVER_NAME ": Wake-up:  0x%08x | Clock:    0x%08x\n",
		sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
		sdhci_readw(host, SDHCI_CLOCK_CONTROL));
	printk(KERN_DEBUG DRIVER_NAME ": Timeout:  0x%08x | Int stat: 0x%08x\n",
		sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
		sdhci_readl(host, SDHCI_INT_STATUS));
	printk(KERN_DEBUG DRIVER_NAME ": Int enab: 0x%08x | Sig enab: 0x%08x\n",
		sdhci_readl(host, SDHCI_INT_ENABLE),
		sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
	printk(KERN_DEBUG DRIVER_NAME ": AC12 err: 0x%08x | Slot int: 0x%08x\n",
		sdhci_readw(host, SDHCI_ACMD12_ERR),
		sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
	printk(KERN_DEBUG DRIVER_NAME ": Caps:     0x%08x | Max curr: 0x%08x\n",
		sdhci_readl(host, SDHCI_CAPABILITIES),
		sdhci_readl(host, SDHCI_MAX_CURRENT));

	if (host->flags & SDHCI_USE_ADMA)
		printk(KERN_DEBUG DRIVER_NAME ": ADMA Err: 0x%08x | ADMA Ptr: 0x%08x\n",
		       readl(host->ioaddr + SDHCI_ADMA_ERROR),
		       readl(host->ioaddr + SDHCI_ADMA_ADDRESS));

	printk(KERN_DEBUG DRIVER_NAME ": ===========================================\n");
}
*/


/*
 * Get the base clock. Use central clock source for now. Not sure if different
 * clock speed to each dev is allowed
 */
static unsigned long sdhci_get_max_clk(struct sdhci_host *host)
{
   unsigned int i;

   for (i = 0; i < SDIO_DEV_TYPE_MAX; i++)
   {
      if (gDevs[i] != NULL && host == gDevs[i]->host)
      {
         return gDevs[i]->clk_hz;
      }
   }

   printk(KERN_ERR "unable to obtain sd max clock\n");
   return 0;
}

static unsigned int sdhci_get_timeout_clock(struct sdhci_host *host)
{
   return sdhci_get_max_clk(host);
}

static struct sdhci_ops sdhci_pltfm_ops = {
   .get_max_clk = sdhci_get_max_clk,
   .get_timeout_clock = sdhci_get_timeout_clock,
};

static int bcm_kona_sd_reset(struct sdio_dev *dev)
{
   struct sdhci_host *host = dev->host;
   unsigned int val;
   unsigned long timeout;

   /* Reset host controller by setting 'Software Reset for All' */
   sdhci_writeb(host, SDHCI_RESET_ALL, SDHCI_SOFTWARE_RESET);

   /* Wait for 100 ms max (100ms timeout is taken from sdhci.c) */
   timeout = jiffies + msecs_to_jiffies(100);

   while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL) {
      if (time_is_before_jiffies(timeout)) {
         dev_err(dev->dev, "Error: sd host is in reset!!!\n");
         return -EFAULT;
      }
   }

   /* reset the host using the top level reset */
   val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
   val |= KONA_SDHOST_RESET;
   sdhci_writel(host, val, KONA_SDHOST_CORECTRL);
   do {
      val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
   } while (0 == (val & KONA_SDHOST_RESET));

   /* bring the host out of reset */
   val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
   val &= ~KONA_SDHOST_RESET;
   /* Back-to-Back register write needs a delay of 1ms at bootup (min 10uS) */
   udelay(1000);
   sdhci_writel(host, val, KONA_SDHOST_CORECTRL);

   return 0;
}

static int bcm_kona_sd_init(struct sdio_dev *dev)
{
	struct sdhci_host *host = dev->host;
   unsigned int val;

   /* enable the interrupt from the IP core */
   val = sdhci_readl(host, KONA_SDHOST_COREIMR);
   val |= KONA_SDHOST_IP;
   sdhci_writel(host, val, KONA_SDHOST_COREIMR);

   /*
    * Enable DAT3 logic for card detection and enable the AHB clock to the
    * host
    */
   val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
   val |= /*KONA_SDHOST_CD_PINCTRL | */KONA_SDHOST_EN;
   /* Back-to-Back register write needs a delay of 1ms at bootup (min 10uS)*/
   udelay(1000);
   sdhci_writel(host, val, KONA_SDHOST_CORECTRL);

   return 0;
}

/*
 * Software emulation of the SD card insertion/removal. Set insert=1 for insert
 * and insert=0 for removal
 */
static int bcm_kona_sd_card_emulate(struct sdio_dev *dev, int insert)
{
   struct sdhci_host *host = dev->host;
   uint32_t val;
   unsigned long flags;

   /* this function can be called from various contexts including ISR */
   spin_lock_irqsave(&host->lock, flags);

#ifndef CONFIG_ARCH_ISLAND
   sdhci_pltfm_clk_enable(host, 1);
#endif
   /* Back-to-Back register write needs a delay of min 10uS. We keep 20uS */
   udelay(20);
   val = sdhci_readl(host, KONA_SDHOST_CORESTAT);

   if (insert) {
      val |= KONA_SDHOST_CD_SW;
      sdhci_writel(host, val, KONA_SDHOST_CORESTAT);
   } else {
      val &= ~KONA_SDHOST_CD_SW;
      sdhci_writel(host, val, KONA_SDHOST_CORESTAT);
   }

   spin_unlock_irqrestore(&host->lock, flags);

   //mmc_detect_change(host->mmc, msecs_to_jiffies(10));

   return 0;
}

static int
proc_card_ctrl_read(char *buffer, char **start, off_t off, int count,
		int *eof, void *data)
{
   unsigned int len = 0;
   struct sdio_dev *dev = (struct sdio_dev *)data;
   struct sdhci_host *host = dev->host;

   if (off > 0)
      return 0;

   len += sprintf(buffer + len, "SD/MMC card is %s\n",
         sdhci_readl(host, KONA_SDHOST_CORESTAT) & KONA_SDHOST_CD_SW ?
         "INSERTED" : "NOT INSERTED");

   return len;
}

static int
proc_card_ctrl_write(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
   int rc, insert;
   struct sdio_dev *dev = (struct sdio_dev *)data;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc) {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%d", &insert) != 1) {
      printk(KERN_ERR "echo <insert> > %s\n", PROC_ENTRY_CARD_CTRL);
      return count;
   }

   if (insert) {
      bcm_kona_sd_card_emulate(dev, 1);
      printk(KERN_INFO "Emulated card insert!\n");
   } else {
      bcm_kona_sd_card_emulate(dev, 0);
      printk(KERN_INFO "Emulated card remove!\n");
   }

   return count;
}

/*
 * Initialize the proc entries
 */
static int proc_init(struct platform_device *pdev)
{
   int rc;
   struct sdio_dev *dev = platform_get_drvdata(pdev);
   struct procfs *proc = &dev->proc;
   struct proc_dir_entry *proc_card_ctrl;

   snprintf(proc->name, sizeof(proc->name), "%s%d", DEV_NAME, pdev->id);

   proc->parent = proc_mkdir(proc->name, gProcParent);
   if (proc->parent == NULL)
      return -ENOMEM;

   proc_card_ctrl = create_proc_entry(PROC_ENTRY_CARD_CTRL, 0644, proc->parent);
   if (proc_card_ctrl == NULL) {
      rc = -ENOMEM;
      goto proc_exit;
   }
   proc_card_ctrl->read_proc = proc_card_ctrl_read;
   proc_card_ctrl->write_proc = proc_card_ctrl_write;
   proc_card_ctrl->data = dev;

   return 0;

proc_exit:
   remove_proc_entry(proc->name, gProcParent);
	return rc;
}

/*
 * Terminate and remove the proc entries
 */
static void proc_term(struct platform_device *pdev)
{
   struct sdio_dev *dev = platform_get_drvdata(pdev);
   struct procfs *proc = &dev->proc;

   remove_proc_entry(PROC_ENTRY_CARD_CTRL, proc->parent);
   remove_proc_entry(proc->name, gProcParent);
}

/*
 * SD card detection interrupt handler
 */
static irqreturn_t sdhci_pltfm_cd_interrupt(int irq, void *dev_id)
{
   struct sdio_dev *dev = (struct sdio_dev *)dev_id;

   /* card insert */
   if (gpio_get_value_cansleep(dev->cd_gpio) == 0)
      bcm_kona_sd_card_emulate(dev, 1);
   else /* card removal */
      bcm_kona_sd_card_emulate(dev, 0);

   return IRQ_HANDLED;
}

int sdhci_pltfm_clk_enable(struct sdhci_host *host, int enable)
{
#ifdef  CONFIG_ARCH_SAMOA
	return 0;
#else
	int ret = 0;
	struct sdio_dev *dev = sdhci_priv(host);
	BUG_ON (!dev);
	if (enable) {
		/* peripheral clock */
		ret = clk_enable(dev->peri_clk);
		if(ret)
			return ret;
	} else {
		clk_disable(dev->peri_clk);
	}
	return ret;
#endif
}

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
static void kona_sdio_status_notify_cb (int card_present, void *dev_id)
{
	 struct sdhci_host *host;
	 int ret;
	 int rc;
	 struct sdio_dev *dev;

	 printk(KERN_ERR " %s ENTRY\n",__FUNCTION__);
	 
	 rc = sdio_dev_is_initialized(SDIO_DEV_TYPE_WIFI);
	 if (rc <= 0)
	 	{
	 	printk(KERN_ERR " %s CARD IS NOT INITIALIZED\n",__FUNCTION__);
		return NULL;
	 	}
	 dev = gDevs[SDIO_DEV_TYPE_WIFI];

	 printk(KERN_ERR " %s DEV=%x\n",__FUNCTION__,dev);


	 host = dev_id;
	 if (host == NULL) {
		 pr_err("%s(): Invalid host structure pointer \r\n", __func__);
		 return;
	 }
	 printk(KERN_ERR " %s CALL EMULATION=%x\n",__FUNCTION__,dev);
		if(card_present)
	  bcm_kona_sd_card_emulate(dev, 1);
		else
	  bcm_kona_sd_card_emulate(dev, 0);			

	   printk(KERN_ERR " %s EMULATION DONE=%x\n",__FUNCTION__,dev);
	/* 
	 * TODO: The required implementtion to check the status of the card
	 * etc
	 */

	 /* Call the core function to rescan on the given host controller */

	 printk(KERN_ERR " %s MMC_DETECT_CHANGE\n",__FUNCTION__);	
	if(card_present)
	 mmc_detect_change(host->mmc, 100);

	 	 printk(KERN_ERR " %s MMC_DETECT_CHANGE DONE\n",__FUNCTION__);	
}
#endif

static int __devinit sdhci_pltfm_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct sdio_dev *dev;
	struct resource *iomem;
	struct sdio_platform_cfg *hw_cfg;
	char devname[MAX_DEV_NAME_SIZE];
	int ret;
	printk(KERN_ERR " %s ENTRY\n",__FUNCTION__);

	BUG_ON(pdev == NULL);

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "platform_data missing\n");
		ret = -EFAULT;
		goto err;
	}

	printk(KERN_ERR " %s GET PLATFORM DATA \n",__FUNCTION__);

	hw_cfg = (struct sdio_platform_cfg *)pdev->dev.platform_data;
	if (hw_cfg->devtype >= SDIO_DEV_TYPE_MAX) {
		dev_err(&pdev->dev, "unknown device type\n");
		ret = -EFAULT;
		goto err;
	}
	printk(KERN_ERR " %s GET PLATFORM RESOURCES\n",__FUNCTION__);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		ret = -ENOMEM;
		goto err;
	}

	if (pdev->dev.parent)
		host = sdhci_alloc_host(pdev->dev.parent, sizeof(struct sdio_dev));
	else
		host = sdhci_alloc_host(&pdev->dev, sizeof(struct sdio_dev));
	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err;
	}

	printk(KERN_ERR " %s ALLOC HOST\n",__FUNCTION__);

	host->hw_name = "bcm_kona_sd";
	host->ops = &sdhci_pltfm_ops;
	host->irq = platform_get_irq(pdev, 0);

	printk(KERN_ERR " %s GET IRQ\n",__FUNCTION__);


	if (hw_cfg->flags & KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE)
		host->mmc->caps |= MMC_CAP_NONREMOVABLE;

	if (!request_mem_region(iomem->start, resource_size(iomem),
		mmc_hostname(host->mmc))) {
		dev_err(&pdev->dev, "cannot request region\n");
		ret = -EBUSY;
		goto err_free_host;
	}

	host->ioaddr = ioremap(iomem->start, resource_size(iomem));
	if (!host->ioaddr) {
		dev_err(&pdev->dev, "failed to remap registers\n");
		ret = -ENOMEM;
		goto err_free_mem_region;
	}

	printk(KERN_ERR " %s MEM and IO REGION OKAY\n",__FUNCTION__);

	dev = sdhci_priv(host);
	dev->dev = &pdev->dev;
	dev->host = host;
	dev->devtype = hw_cfg->devtype;
	dev->cd_gpio = hw_cfg->cd_gpio;
	if (dev->devtype == SDIO_DEV_TYPE_WIFI)
		dev->wifi_gpio = &hw_cfg->wifi_gpio;

	printk(KERN_ERR " %s DEV TYPE %x\n",__FUNCTION__,dev->devtype);



	gDevs[dev->devtype] = dev;

	platform_set_drvdata(pdev, dev);

	snprintf(devname, sizeof(devname), "%s%d", DEV_NAME, pdev->id);

	/* enable clocks */
#ifdef CONFIG_MACH_BCM2850_FPGA
	if (clock) /* clock override */{
		dev->clk_hz = clock;
	}
	else {
		dev->clk_hz = gClock[dev->devtype];
	}
#else
	/* peripheral clock */
	dev->peri_clk = clk_get(&pdev->dev, hw_cfg->peri_clk_name);
	if (IS_ERR_OR_NULL (dev->peri_clk)) {
		ret = -EINVAL;
		goto err_unset_pltfm;
	}
	ret = clk_set_rate(dev->peri_clk, hw_cfg->peri_clk_rate);
	if (ret) {
		goto err_peri_clk_put;
	}

	/* sleep clock */
	dev->sleep_clk = clk_get(&pdev->dev, hw_cfg->sleep_clk_name);
	if (IS_ERR_OR_NULL (dev->sleep_clk)) {
		ret = -EINVAL;
		goto err_peri_clk_put;
	}

	ret = clk_enable(dev->sleep_clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable sleep clock for %s\n", devname);
		goto err_sleep_clk_put;
	}

	ret = sdhci_pltfm_clk_enable(host, 1);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize core clock for %s\n", devname);
		goto err_sleep_clk_disable;
	}
	dev->clk_hz = clk_get_rate(dev->peri_clk);
#endif

	if(hw_cfg->vddo_regulator_name)	{
		ret = sdhci_pltfm_regulator_init(dev, hw_cfg->vddo_regulator_name);
		if (ret < 0)
			goto err_term_clk;
	}

	ret = bcm_kona_sd_reset(dev);
	if (ret)
		goto err_term_clk;

	ret = bcm_kona_sd_init(dev);
	if (ret)
		goto err_reset;

	if (hw_cfg->is_8bit)
		host->mmc->caps |= MMC_CAP_8_BIT_DATA;

	host->quirks = SDHCI_QUIRK_NO_CARD_NO_RESET | SDHCI_QUIRK_BROKEN_TIMEOUT_VAL;

	/*Note that sdhci_add_host calls --> mmc_add_host, which in turn
	 *checks for the flag MMC_PM_IGNORE_PM_NOTIFY before registering a PM
	 *notifier for the specific instance of SDIO host controller. For
	 *WiFi case, we don't want to get notified, becuase then from there
	 *mmc_power_off is called which will reset the Host registers that
	 *needs to be re-programmed by starting SDIO handsake again. We want
	 *to prevent this in case of WiFi. So enable MMC_PM_IGNORE_PM_NOTIFY
	 *flag, so that notifier never gets registered.
	 */
	if (dev->devtype == SDIO_DEV_TYPE_WIFI) {
		/*The Wireless LAN drivers call the API sdio_get_host_pm_caps
		 *to know the PM capabilities of the driver, which would
		 *return pm_caps. While the internal code decides based on
		 *pm_flags, the pm_caps also should reflect the same.
		 */
		host->mmc->pm_caps = MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY;
		host->mmc->pm_flags = MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY;
	}

	/* Enable 1.8V DDR operation for e.MMC */
	if (dev->devtype == SDIO_DEV_TYPE_EMMC)
		host->mmc->caps |= MMC_CAP_1_8V_DDR;

	ret = sdhci_add_host(host);
	if (ret)
		goto err_reset;

	ret = proc_init(pdev);
	if (ret)
		goto err_rm_host;

	/* if device is eMMC, emulate card insert right here */
	if (dev->devtype == SDIO_DEV_TYPE_EMMC) {
		ret = bcm_kona_sd_card_emulate(dev, 1);
		if (ret) {
			dev_err(&pdev->dev, "unable to emulate card insertion\n");
			goto err_proc_term;
		}
		printk(KERN_INFO "%s card insert emulated!\n", devname);
	}  else if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {
		ret = gpio_request(dev->cd_gpio, "sdio cd");

		if (ret < 0) {
			dev_err(&pdev->dev, "Unable to request GPIO pin %d\n", dev->cd_gpio);
			goto err_proc_term;
		}
		gpio_direction_input(dev->cd_gpio);

		ret = request_threaded_irq(gpio_to_irq(dev->cd_gpio),
			NULL,
			sdhci_pltfm_cd_interrupt,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
			"sdio cd",
			dev);
		if (ret) {
			dev_err(&pdev->dev, "Unable to request card detection irq=%d for gpio=%d\n",
			gpio_to_irq(dev->cd_gpio), dev->cd_gpio);
			goto err_free_cd_gpio;
		}

		/* Set debounce for SD Card detect to maximum value (128ms)
		 *
		 * NOTE-1: If gpio_set_debounce() returns error we still continue
		 * with the default debounce value set. Another reason for
		 * doing this is that on rhea-ray boards the SD Detect GPIO is on
		 * GPIO Expander and gpio_set_debounce() will return error and if we
		 * return error from here, then probe() would fail and SD detection
		 * would always fail.
		 *
		 * NOTE-2: We also give a msleep() of the "debounce" time here so
		 * that we give enough time for the debounce to stabilize before
		 * we read the gpio value in gpio_get_value_cansleep().
		 */
		ret = gpio_set_debounce(dev->cd_gpio, (SD_DETECT_GPIO_DEBOUNCE_128MS*1000));
		if (ret < 0) {
			dev_err(&pdev->dev, "%s: gpio set debounce failed."
					"default debounce value assumed\n", __func__);
		}

		/* Sleep for 128ms to allow debounce to stabilize */
		msleep(SD_DETECT_GPIO_DEBOUNCE_128MS);

		/*
		 * Since the card detection GPIO interrupt is configured to be
		 * edge sensitive, check the initial GPIO value here, emulate
		 * only if the card is present
		 */
		if (gpio_get_value_cansleep(dev->cd_gpio) == 0)
			bcm_kona_sd_card_emulate(dev, 1);
	}

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	if ( (dev->devtype==SDIO_DEV_TYPE_WIFI) && 
	     (hw_cfg->register_status_notify != NULL)) {
		hw_cfg->register_status_notify(kona_sdio_status_notify_cb, host);
	}
	printk(KERN_ERR " %s CALL BACK IS REGISTERED\n",__FUNCTION__);

#endif

	atomic_set(&dev->initialized, 1);
	sdhci_pltfm_clk_enable(host, 0);
	
	printk(KERN_INFO "%s initialized properly\n", devname);

	return 0;

err_free_cd_gpio:
	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {
		gpio_free(dev->cd_gpio);
	}

err_proc_term:
	proc_term(pdev);

err_rm_host:
	sdhci_remove_host(host, 0);

err_reset:
	bcm_kona_sd_reset(dev);

err_term_clk:
	sdhci_pltfm_clk_enable(host, 0);

#ifndef CONFIG_MACH_BCM2850_FPGA
err_sleep_clk_disable:
	clk_disable(dev->sleep_clk);

err_sleep_clk_put:
	clk_put(dev->sleep_clk);

err_peri_clk_put:
	clk_put(dev->peri_clk);
#endif

err_unset_pltfm:
	platform_set_drvdata(pdev, NULL);
	iounmap(host->ioaddr);

err_free_mem_region:
	release_mem_region(iomem->start, resource_size(iomem));

err_free_host:
	sdhci_free_host(host);

err:
	printk(KERN_ERR "Probing of sdhci-pltfm %d failed: %d\n", pdev->id, ret);
	return ret;
}

static int __devexit sdhci_pltfm_remove(struct platform_device *pdev)
{
	struct sdio_dev *dev = platform_get_drvdata(pdev);
	struct sdhci_host *host = dev->host;
	struct resource *iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int dead;
	u32 scratch;

	atomic_set(&dev->initialized, 0);
	gDevs[dev->devtype] = NULL;

	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {
		free_irq(gpio_to_irq(dev->cd_gpio), dev);
		gpio_free(dev->cd_gpio);
	}

	proc_term(pdev);

	sdhci_pltfm_clk_enable(host, 1);
	dead = 0;
	scratch = readl(host->ioaddr + SDHCI_INT_STATUS);
	if (scratch == (u32)-1)
		dead = 1;
	sdhci_remove_host(host, dead);

	sdhci_pltfm_clk_enable(host, 0);

#ifndef CONFIG_MACH_BCM2850_FPGA
	clk_disable(dev->sleep_clk);
	clk_put(dev->sleep_clk);
	clk_put(dev->peri_clk);
#endif

	platform_set_drvdata(pdev, NULL);
	kfree(dev);
	iounmap(host->ioaddr);
	release_mem_region(iomem->start, resource_size(iomem));
	sdhci_free_host(host);

	return 0;
}

#ifdef CONFIG_PM
static int sdhci_pltfm_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret;
	struct sdio_dev *dev = platform_get_drvdata(pdev);
	struct sdhci_host *host = dev->host;
	
#if 0
	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0)
		free_irq(gpio_to_irq(dev->cd_gpio), dev);
#endif
	
	ret = sdhci_suspend_host(host, state);
	if (ret) {
		dev_err(&pdev->dev, "Unable to suspend sdhci host err=%d\n",
				ret);
		return ret;
	}

	return 0;
}

static int sdhci_pltfm_resume(struct platform_device *pdev)
{
	int ret;
	struct sdio_dev *dev = platform_get_drvdata(pdev);
	struct sdhci_host *host = dev->host;

	ret = sdhci_resume_host(host);
	if (ret) {
		dev_err(&pdev->dev, "Unable to resume sdhci host err=%d\n", ret);
		return ret;
	}

#if 0
	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {
		ret = request_irq(gpio_to_irq(dev->cd_gpio), sdhci_pltfm_cd_interrupt,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "sdio cd", dev);
		if (ret) {
			dev_err(&pdev->dev, "Unable to request card detection irq=%d for gpio=%d\n",
					gpio_to_irq(dev->cd_gpio), dev->cd_gpio);
			return ret;
		}
	}
#endif


#ifndef CONFIG_MMC_UNSAFE_RESUME
	/*
	 * card state might have been changed during system suspend.
	 * Need to sync up only if MMC_UNSAFE_RESUME is not enabled
	 */
	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {
		if (gpio_get_value_cansleep(dev->cd_gpio) == 0)
			bcm_kona_sd_card_emulate(dev, 1);
		else
			bcm_kona_sd_card_emulate(dev, 0);
	}
#endif
	return 0;
}
#else
#define sdhci_pltfm_suspend NULL
#define sdhci_pltfm_resume NULL
#endif  /* CONFIG_PM */

static struct platform_driver sdhci_pltfm_driver = {
	.driver = {
		.name	= "sdhci",
		.owner	= THIS_MODULE,
	},
	.probe = sdhci_pltfm_probe,
	.remove = __devexit_p(sdhci_pltfm_remove),
	.suspend = sdhci_pltfm_suspend,
	.resume = sdhci_pltfm_resume,
};

static int __init sdhci_drv_init(void)
{
   int rc;

   gProcParent = proc_mkdir(PROC_GLOBAL_PARENT_DIR, NULL);
   if (gProcParent == NULL) {
      printk(KERN_ERR "sdhci platform procfs install failed\n");
      return -ENOMEM;
   }

   rc = platform_driver_register(&sdhci_pltfm_driver);
   if (rc < 0) {
      printk(KERN_ERR "sdhci_drv_init failed\n");
      remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
      return rc;
   }

	return 0;
}

static void __exit sdhci_drv_exit(void)
{
	remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
	platform_driver_unregister(&sdhci_pltfm_driver);
}

fs_initcall(sdhci_drv_init);
module_exit(sdhci_drv_exit);

MODULE_DESCRIPTION("Secure Digital Host Controller Interface platform driver");
MODULE_AUTHOR("Mocean Laboratories <info@mocean-labs.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:sdhci");

int sdio_dev_is_initialized(enum sdio_devtype devtype)
{
   struct sdio_dev *dev;

   if (devtype >= SDIO_DEV_TYPE_MAX)
      return -EINVAL;

   if (gDevs[devtype] == NULL)
      return 0;

   dev = gDevs[devtype];

   return atomic_read(&dev->initialized);
}
EXPORT_SYMBOL(sdio_dev_is_initialized);

struct mmc_card *sdio_get_mmc_card(enum sdio_devtype devtype)
{
   int rc;
   struct sdio_dev *dev;

   rc = sdio_dev_is_initialized(devtype);
   if (rc <= 0)
      return NULL;

   dev = gDevs[devtype];
   return dev->host->mmc->card;
}
EXPORT_SYMBOL(sdio_get_mmc_card);

struct sdio_wifi_gpio_cfg *sdio_get_wifi_gpio(enum sdio_devtype devtype)
{
   int rc;
   struct sdio_dev *dev;

   rc = sdio_dev_is_initialized(devtype);
   if (rc <= 0)
      return NULL;

   dev = gDevs[devtype];

   return dev->wifi_gpio;
}
EXPORT_SYMBOL(sdio_get_wifi_gpio);

int sdio_card_emulate(enum sdio_devtype devtype, int insert)
{
   int rc;
   struct sdio_dev *dev;

   rc = sdio_dev_is_initialized(devtype);
   if (rc <= 0)
      return -EFAULT;

   dev = gDevs[devtype];

   return bcm_kona_sd_card_emulate(dev, insert);
}
EXPORT_SYMBOL(sdio_card_emulate);



int sdio_stop_clk(enum sdio_devtype devtype, int insert)

{
	int rc;
	struct sdio_dev *dev;

	
	rc = sdio_dev_is_initialized(devtype);
	if (rc <= 0)
	   return -EFAULT;
	
	dev = gDevs[devtype];
	struct sdhci_host *host = dev->host;

	
#ifndef CONFIG_ARCH_ISLAND
	   sdhci_pltfm_clk_enable(host, insert);
#endif


}

EXPORT_SYMBOL(sdio_stop_clk);



static int sdhci_pltfm_regulator_init(struct sdio_dev *dev, char * reg_name)
{
	int ret;

	dev->vddo_sd_regulator = regulator_get(NULL, reg_name);

	if(dev->vddo_sd_regulator)	{
		ret = regulator_enable(dev->vddo_sd_regulator);
		if (ret < 0)	{
			printk(KERN_ERR "%s: cant Enable regulator\n", reg_name);
			ret = -1;
		} else	{
			/* Configure 3.3V default */
			ret = regulator_set_voltage(dev->vddo_sd_regulator,
                                               3300000, 3300000);
			if (ret < 0)	{
				printk(KERN_ERR "%s: cant set 3.3V\n",reg_name);
				ret = -1;
			} else	{
				pr_info(KERN_INFO "%s: set to 3.3V\n", reg_name);
				ret = 0;
			}
		}
	} else {
		printk(KERN_ERR "%s:could not get regulator\n", reg_name);
		ret = -1;
	}
	return ret;
}

int sdhci_pltfm_set_3v3_signalling(struct sdhci_host *host)
{
	struct sdio_dev *dev = sdhci_priv(host);
	int ret = 0;

	if(dev->vddo_sd_regulator)	{
		ret = regulator_set_voltage(dev->vddo_sd_regulator, 3300000, 3300000);
		if(ret < 0)
			dev_err(dev->dev, "cant set vddo regulator to 3.3V!\n");
		else
			dev_dbg(dev->dev, "vddo regulator is set to 3.3V\n");
	}
	return ret;
}

int sdhci_pltfm_set_1v8_signalling(struct sdhci_host *host)
{
	struct sdio_dev *dev = sdhci_priv(host);
	int ret = 0;

	if(dev->vddo_sd_regulator)	{
		ret = regulator_set_voltage(dev->vddo_sd_regulator, 1800000, 1800000);
		if(ret < 0)
			dev_err(dev->dev, "Cant set vddo regulator to 1.8V!\n");
		else
			dev_dbg(dev->dev, "vddo regulator is set to 1.8V\n");
	}
	return ret;
}

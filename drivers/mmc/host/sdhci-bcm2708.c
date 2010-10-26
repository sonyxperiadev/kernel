/*
 * sdhci-bcm2708.c Support for SDHCI device on BCM2708
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

/* Supports:
 * SDHCI platform devices
 *
 * Inspired by sdhci-pci.c, by Pierre Ossman
 */

#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>

#include <linux/mmc/host.h>

#include <linux/io.h>

#include "sdhci.h"

#define DRIVER_NAME "bcm2708_sdhci"

/*****************************************************************************\
 *                                                                           *
 * SDHCI core callbacks                                                      *
 *                                                                           *
\*****************************************************************************/

u32 sdhci_bcm2708_readl(struct sdhci_host *host, int reg)
{
	u32 l = readl(host->ioaddr + reg);

#ifdef LOG_REGISTERS
	printk(KERN_ERR "%s: readl from 0x%02x, value 0x%08x\n",
               mmc_hostname(host->mmc), reg, l);
#endif

	return l;
}

u16 sdhci_bcm2708_readw(struct sdhci_host *host, int reg)
{
	u32 l = readl(host->ioaddr + (reg & ~3));
	u32 w = l >> (reg << 3 & 0x18) & 0xffff; 

#ifdef LOG_REGISTERS
	printk(KERN_ERR "%s: readw from 0x%02x, value 0x%04x\n",
               mmc_hostname(host->mmc), reg, w);
#endif

	return (u16)w;
}

u8 sdhci_bcm2708_readb(struct sdhci_host *host, int reg)
{
	u32 l = readl(host->ioaddr + (reg & ~3));
	u32 b = l >> (reg << 3 & 0x18) & 0xff;

#ifdef LOG_REGISTERS
	printk(KERN_ERR "%s: readb from 0x%02x, value 0x%02x\n",
               mmc_hostname(host->mmc), reg, b);
#endif

	return (u8)b;
}

void sdhci_bcm2708_writel(struct sdhci_host *host, u32 val, int reg)
{
#ifdef LOG_REGISTERS
	printk(KERN_ERR "%s: writel to 0x%02x, value 0x%08x\n",
               mmc_hostname(host->mmc), reg, val);
#endif

	writel(val, host->ioaddr + reg);
}

void sdhci_bcm2708_writew(struct sdhci_host *host, u16 val, int reg)
{
	static u32 shadow = 0;

	u32 p = reg == SDHCI_COMMAND ? shadow : readl(host->ioaddr + (reg & ~3));
	u32 s = reg << 3 & 0x18;
	u32 l = val << s;
	u32 m = 0xffff << s;

#ifdef LOG_REGISTERS
	printk(KERN_ERR "%s: writew to 0x%02x, value 0x%04x\n",
               mmc_hostname(host->mmc), reg, val);
#endif

	if (reg == SDHCI_TRANSFER_MODE) 
		shadow = (p & ~m) | l;
	else
		writel((p & ~m) | l, host->ioaddr + (reg & ~3));
}

void sdhci_bcm2708_writeb(struct sdhci_host *host, u8 val, int reg)
{
	u32 p = readl(host->ioaddr + (reg & ~3));
	u32 s = reg << 3 & 0x18;
	u32 l = val << s;
	u32 m = 0xff << s;

#ifdef LOG_REGISTERS
	printk(KERN_ERR "%s: writeb to 0x%02x, value 0x%02x\n",
               mmc_hostname(host->mmc), reg, val);
#endif

	writel((p & ~m) | l, host->ioaddr + (reg & ~3));
}

static unsigned int sdhci_bcm2708_get_max_clock(struct sdhci_host *host)
{
	return 100000000;	// this value is in Hz
}

static unsigned int sdhci_bcm2708_get_timeout_clock(struct sdhci_host *host)
{
	return 100000;		// this value is in kHz
}

static struct sdhci_ops sdhci_bcm2708_ops = {
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
	.read_l = sdhci_bcm2708_readl,
	.read_w = sdhci_bcm2708_readw,
	.read_b = sdhci_bcm2708_readb,
	.write_l = sdhci_bcm2708_writel,
	.write_w = sdhci_bcm2708_writew,
	.write_b = sdhci_bcm2708_writeb,
#else
#error The BCM2708 SDHCI driver needs CONFIG_MMC_SDHCI_IO_ACCESSORS to be set
#endif
        //.enable_dma = NULL,
        //.set_clock = NULL,
	.get_max_clock = sdhci_bcm2708_get_max_clock,
        //.get_min_clock = NULL,
	.get_timeout_clock = sdhci_bcm2708_get_timeout_clock
};

/*****************************************************************************\
 *                                                                           *
 * Device probing/removal                                                    *
 *                                                                           *
\*****************************************************************************/

static int __devinit sdhci_bcm2708_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct resource *iomem;
	int ret;

	BUG_ON(pdev == NULL);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		ret = -ENOMEM;
		goto err;
	}

	if (resource_size(iomem) != 0x100)
		dev_err(&pdev->dev, "Invalid iomem size. You may "
			"experience problems.\n");

	if (pdev->dev.parent)
		host = sdhci_alloc_host(pdev->dev.parent, 0);
	else
		host = sdhci_alloc_host(&pdev->dev, 0);

	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err;
	}

	host->hw_name = "platform";
	host->ops = &sdhci_bcm2708_ops;
	host->irq = platform_get_irq(pdev, 0);

	host->quirks = SDHCI_QUIRK_BROKEN_CARD_DETECTION |
	               SDHCI_QUIRK_CAP_VOLTAGE_BROKEN |
	               SDHCI_QUIRK_SPURIOUS_CRC_ACMD51;

	if (!request_mem_region(iomem->start, resource_size(iomem),
		mmc_hostname(host->mmc))) {
		dev_err(&pdev->dev, "cannot request region\n");
		ret = -EBUSY;
		goto err_request;
	}

	host->ioaddr = ioremap(iomem->start, resource_size(iomem));
	if (!host->ioaddr) {
		dev_err(&pdev->dev, "failed to remap registers\n");
		ret = -ENOMEM;
		goto err_remap;
	}

	ret = sdhci_add_host(host);
	if (ret)
		goto err_add_host;

	platform_set_drvdata(pdev, host);
        
	printk(KERN_INFO "%s: BCM2708 SDHC host at 0x%08llx\n",
               mmc_hostname(host->mmc), (unsigned long long)iomem->start);

	return 0;

err_add_host:
	iounmap(host->ioaddr);
err_remap:
	release_mem_region(iomem->start, resource_size(iomem));
err_request:
	sdhci_free_host(host);
err:
	dev_err(&pdev->dev, "probe failed, err %d\n", ret);
	return ret;
}

static int __devexit sdhci_bcm2708_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct resource *iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int dead;
	u32 scratch;

	dead = 0;
	scratch = readl(host->ioaddr + SDHCI_INT_STATUS);
	if (scratch == (u32)-1)
		dead = 1;

	sdhci_remove_host(host, dead);
	iounmap(host->ioaddr);
	release_mem_region(iomem->start, resource_size(iomem));
	sdhci_free_host(host);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sdhci_bcm2708_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= sdhci_bcm2708_probe,
	.remove		= __devexit_p(sdhci_bcm2708_remove),
};

/*****************************************************************************\
 *                                                                           *
 * Driver init/exit                                                          *
 *                                                                           *
\*****************************************************************************/

static int __init sdhci_drv_init(void)
{
	return platform_driver_register(&sdhci_bcm2708_driver);
}

static void __exit sdhci_drv_exit(void)
{
	platform_driver_unregister(&sdhci_bcm2708_driver);
}

module_init(sdhci_drv_init);
module_exit(sdhci_drv_exit);

MODULE_DESCRIPTION("Secure Digital Host Controller Interface platform driver");
MODULE_AUTHOR("Mocean Laboratories <info@mocean-labs.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:"DRIVER_NAME);


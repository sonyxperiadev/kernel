#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <mach/platform.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <mach/bcm2708_smi.h>

#include "bcm2708_nand.h"

#define SLOT			0
#define DATA_PORT		0x1C
#define COMMAND_PORT	0x1D
#define ADDRESS_PORT	0x1E

struct bcm2708_nand_info {

	/* mtd and nand framework related */
	struct mtd_info mtd;
	struct nand_chip chip;
	struct nand_hw_control controller;
	struct resource *nand_res;
	struct smi_periph_setup periph_setup;
};

static struct bcm2708_nand_info * bcm2708_nand_mtd_toinfo(struct mtd_info *mtd)
{
	return container_of(mtd, struct bcm2708_nand_info, mtd);
}

/* functions exported to mtd framework */
static void bcm2708_nand_hwcontrol(struct mtd_info *mtd, int cmd,
		unsigned int ctrl)
{
	struct bcm2708_nand_info *info = bcm2708_nand_mtd_toinfo(mtd);

	if (cmd == NAND_CMD_NONE)
		return;
	/* command */
	if (ctrl & NAND_CLE)
		smi_write(SLOT, COMMAND_PORT, sizeof(uint8_t), 1, (const void *)&cmd); 
	else
		smi_write(SLOT, ADDRESS_PORT, sizeof(uint8_t), 1, (const void *)&cmd);
}

#if 0 // Currently we force WP off in bootloader
// NAND WP is GPIO 16 active when high.
static void bcm2708_nand_set_wp(void)
{
	printf("setting nand wp\n");
	//writel(1<<16, GP_CLR0);
}

static void bcm2708_nand_clear_wp(void)
{
	printf("clearing nand wp\n");
	//writel(1<<16, GP_SET0);
}
#endif
#if 0
static int bcm2708_nand_wait_rb(struct mtd_info *mtd)
{
	int ret = 0;
	int timeout = 0;
	do {
		ret = readl(GP_LEV0);
		//printf("ready?: %08x %d", ret, ret & (1<<17));
		ret &= (1<<17);
		//printf("wb count ++ %d\n", timeout);
		timeout++;
		if (timeout > 1000000) {
			printf("Warning: possible lockup in nand wait\n");
		}
	} while (!ret);
	return ret;
}
#endif
#if 0
static int bcm2708_nand_devready(struct mtd_info *mtd)
{
	/* check DONE bit in SMI control register */
	smi_wait_transfer_complete();
	return 1;
}
#endif

static uint8_t bcm2708_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t read_data;

	/*read the byte and return */
	smi_read(SLOT, DATA_PORT, sizeof(uint8_t), 1, (const void *)&read_data);
	return read_data;
}

static void bcm2708_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) 
		smi_read(SLOT, DATA_PORT, sizeof(uint8_t), 1, (const void*)&buf[i]);
}

static void bcm2708_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;
	for (i = 0; i < len; i++);
	smi_write(SLOT, DATA_PORT, sizeof(uint8_t), 1, (const void *)&buf[i]);
}

static void bcm2708_nand_init_chip(struct bcm2708_nand_info *info)
{
	struct nand_chip *chip = &info->chip;

	/* fill in the nand specific data */
	chip->chip_delay	= 50;
	chip->options		= 0;
	chip->cmd_ctrl		= bcm2708_nand_hwcontrol; // NEEDED - nand_command
//	chip->dev_ready		= bcm2708_nand_devready; // NEEDED - nand_wait_ready
	chip->read_byte		= bcm2708_nand_read_byte; // NEEDED 
	chip->read_buf		= bcm2708_nand_read_buf;
	chip->write_buf		= bcm2708_nand_write_buf;

	chip->ecc.mode		= NAND_ECC_NONE;

	chip->controller	= &info->controller;
	chip->priv			= info;
	info->mtd.priv		= chip;
	info->mtd.owner		= THIS_MODULE;
}

#if 0
static void bcm2708_smi_init(struct bcm2708_nand_info *info)
{
	struct smi_periph_timings *rtime, *wtime;
	bcm2708_nand_print("=IN\n");

	info->periph_setup.mode					= SMI_MODE_80;
	info->periph_setup.pixel_format			= SMI_PIXEL_FORMAT_NAND;
	info->periph_setup.pixel_bits_swapped	= 0,
	info->periph_setup.tearing_effect		= 0,
	info->periph_setup.hvs_input			= 0,

	rtime = &info->periph_setup.read_timings;
	wtime = &info->periph_setup.write_timings;

	/* read timings */
	rtime->setup_time_in_ns		= 12;
	rtime->strobe_time_in_ns	= 30;
	rtime->hold_time_in_ns		= 5;
	rtime->pace_time_in_ns		= 30;
	rtime->transfer_width		= SMI_TRANSFER_WIDTH_8BIT;

	/* write timings */
	wtime->setup_time_in_ns		= 12;
	wtime->strobe_time_in_ns	= 30;
	wtime->hold_time_in_ns		= 5;
	wtime->pace_time_in_ns		= 30;
	wtime->transfer_width		= SMI_TRANSFER_WIDTH_8BIT;

	smi_setup_timing(SLOT, &info->periph_setup, 19200000); 

	bcm2708_nand_print("=OUT\n");
}
#endif

static int bcm2708_nand_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret = -ENOENT;
	struct bcm2708_nand_info *info;

	bcm2708_nand_print("probe=%p\n", pdev);	

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		ret = -ENOMEM;
		goto exit_error;
	}
	
	bcm2708_nand_init_chip(info);

//	bcm2708_smi_init(info);

	ret = nand_scan_ident(&info->mtd, 1);
	bcm2708_nand_print("NAND scan done ret=%d\n", ret);

	return 0;

exit_mem_error:
exit_rsrc_error:
	kfree(info);
exit_error:
	return ret;

}

static int __devexit bcm2708_nand_remove(struct platform_device *pdev)
{
	return 0;
}

#define bcm2708_nand_suspend    NULL
#define bcm2708_nand_resume     NULL

static struct platform_driver bcm2708_nand_driver = {
	.probe      = bcm2708_nand_probe,
	.remove     = __devexit_p(bcm2708_nand_remove),
	.suspend    = bcm2708_nand_suspend,
	.resume     = bcm2708_nand_resume,
	.driver     = {
		.name   = "bcm2708_nand",
		.owner  = THIS_MODULE,
	},

};
static int __init bcm2708_nand_init(void)
{
	printk(KERN_INFO"BCM2708 NAND Driver,(C) 2010 Braodcom Corp\n");
	return platform_driver_register(&bcm2708_nand_driver);

}

static void __exit bcm2708_nand_exit(void)
{
	platform_driver_unregister(&bcm2708_nand_driver);
}

module_init(bcm2708_nand_init);
module_exit(bcm2708_nand_exit);

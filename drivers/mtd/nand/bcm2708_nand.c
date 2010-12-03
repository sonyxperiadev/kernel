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
#include <mach/plat_nand.h>

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
};

static struct bcm2708_nand_info * bcm2708_nand_mtd_toinfo(struct mtd_info *mtd)
{
	return container_of(mtd, struct bcm2708_nand_info, mtd);
}

static struct bcm2708_platform_nand * to_nand_plat(struct platform_device *dev)
{
	return dev->dev.platform_data;
}

/* functions exported to mtd framework */
static void bcm2708_nand_hwcontrol(struct mtd_info *mtd, int cmd,
		unsigned int ctrl)
{
	if (cmd == NAND_CMD_NONE)
		return;
	/* command */
	if (ctrl & NAND_CLE)
		smi_write(SLOT, COMMAND_PORT, sizeof(uint8_t), 1, (const void *)&cmd); 
	else
		smi_write(SLOT, ADDRESS_PORT, sizeof(uint8_t), 1, (const void *)&cmd);
}

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
	for (i = 0; i < len; i++)
		smi_write(SLOT, DATA_PORT, sizeof(uint8_t), 1, (const void *)&buf[i]);
}

static void bcm2708_nand_init_chip(struct bcm2708_nand_info *info)
{
	struct nand_chip *chip = &info->chip;

	/* fill in the nand specific data */
	chip->chip_delay	= 50 ;
	chip->options		= 0;
	chip->cmd_ctrl		= bcm2708_nand_hwcontrol; 
	//	chip->dev_ready		= bcm2708_nand_devready; // NEEDED - nand_wait_ready
	chip->read_byte		= bcm2708_nand_read_byte; 
	chip->read_buf		= bcm2708_nand_read_buf;
	chip->write_buf		= bcm2708_nand_write_buf;

	chip->ecc.mode		= NAND_ECC_SOFT;

	chip->controller	= &info->controller;
	chip->priv			= info;
	info->mtd.priv		= chip;
	info->mtd.owner		= THIS_MODULE;
}

static int bcm2708_nand_probe(struct platform_device *pdev)
{
	int ret = -ENOENT;
	struct bcm2708_nand_info *info;
	struct bcm2708_platform_nand  *pinfo = to_nand_plat(pdev);


	bcm2708_nand_print("probe=%p\n", pdev);	

	if(pinfo == NULL) {
		dev_err(&pdev->dev, "platform data(partition info) not specified\n");
		ret = -ENXIO;
		goto exit_error;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		ret = -ENOMEM;
		goto exit_error;
	}

	spin_lock_init(&info->controller.lock);
	init_waitqueue_head(&info->controller.wq);

	bcm2708_nand_init_chip(info);
	bcm2708_nand_print("NAND init done\n");

	if(nand_scan(&info->mtd, 1)) {
		ret = -ENXIO;
		goto exit_mtd_error;
	}


	bcm2708_nand_print("Adding %d partitions\n", pinfo->nr_partitions);

	add_mtd_partitions(&info->mtd, pinfo->partitions, pinfo->nr_partitions);

	return 0;

exit_mtd_error:
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
